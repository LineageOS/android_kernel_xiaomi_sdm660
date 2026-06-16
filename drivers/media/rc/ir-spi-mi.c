 /*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 * Copyright (C) 2019 XiaoMi, Inc.
 * Author: Andi Shyti <andi.shyti@samsung.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SPI driven IR LED device driver
 */

#include <linux/fs.h>
#include <linux/idr.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/uaccess.h>
#include <media/lirc.h>

#define IR_SPI_DRIVER_NAME		"ir-spi"

#define IR_SPI_DEFAULT_FREQUENCY	960000
#define IR_SPI_BIT_PER_WORD		    32
#define IR_SPI_NAME_SIZE		    16

static DEFINE_IDA(ir_spi_ida);

struct ir_spi_data {
	u16 nusers;
	int power_gpio;

	u8 *buffer;

	struct miscdevice miscdev;
	int lirc_id;
	char miscdev_name[IR_SPI_NAME_SIZE];
	struct spi_device *spi;
	struct spi_transfer xfer;
	struct mutex mutex;
	struct regulator *regulator;
};

static ssize_t ir_spi_chardev_write(struct file *file,
					const char __user *buffer,
					size_t length, loff_t *offset)
{
	struct ir_spi_data *idata = file->private_data;
	bool please_free = false;
	int ret = 0;

	if (idata->xfer.len && (idata->xfer.len != length))
		return -EINVAL;

	mutex_lock(&idata->mutex);

	if (!idata->xfer.len) {
		idata->buffer = kmalloc(length, GFP_KERNEL);

		if (!idata->buffer) {
			ret = -ENOMEM;
			goto out_unlock;
		}

		idata->xfer.len = length;
		please_free = true;
	}

	if (copy_from_user(idata->buffer, buffer, length)) {
		ret = -EFAULT;
		goto out_free;
	}
#if 0
	ret = regulator_enable(idata->regulator);
	if (ret) {
		dev_err(&idata->spi->dev, "failed to power on the LED\n");
		goto out_free;
	}
#endif
	idata->xfer.tx_buf = idata->buffer;

	ret = spi_sync_transfer(idata->spi, &idata->xfer, 1);
	if (ret)
		dev_err(&idata->spi->dev, "unable to deliver the signal\n");
#if 0
	regulator_disable(idata->regulator);
#endif
out_free:
	if (please_free) {
		kfree(idata->buffer);
		idata->xfer.len = 0;
		idata->buffer = NULL;
	}

out_unlock:
	mutex_unlock(&idata->mutex);

	return ret ? ret : length;
}

static int ir_spi_chardev_open(struct inode *inode, struct file *file)
{
	struct miscdevice *miscdev = file->private_data;
	struct ir_spi_data *idata =
		container_of(miscdev, struct ir_spi_data, miscdev);
	int ret = 0;

	mutex_lock(&idata->mutex);

	if (unlikely(idata->nusers >= SHRT_MAX)) {
		dev_err(&idata->spi->dev, "device busy\n");
		ret = -EBUSY;
		goto out_unlock;
	}

	idata->nusers++;
	file->private_data = idata;

out_unlock:
	mutex_unlock(&idata->mutex);

	return ret;
}

static int ir_spi_chardev_close(struct inode *inode, struct file *file)
{
	struct ir_spi_data *idata = file->private_data;

	mutex_lock(&idata->mutex);
	idata->nusers--;

	/*
	 * check if someone else is using the driver,
	 * if not, then:
	 *
	 *  - reset length and frequency values to default
	 *  - shut down the LED
	 *  - free the buffer (NULL or ZERO_SIZE_PTR are noop)
	 */
	if (!idata->nusers) {
		idata->xfer.len = 0;
		idata->xfer.speed_hz = IR_SPI_DEFAULT_FREQUENCY;

		kfree(idata->buffer);
		idata->buffer = NULL;
	}

	mutex_unlock(&idata->mutex);

	return 0;
}

static long ir_spi_chardev_ioctl(struct file *file, unsigned int cmd,
						unsigned long arg)
{
	__u32 p;
	s32 ret;
	struct ir_spi_data *idata = file->private_data;

	switch (cmd) {
	case LIRC_GET_FEATURES:
		return put_user((__u32)LIRC_CAN_SEND_RAW, (__u32 __user *) arg);

	case LIRC_GET_LENGTH:
		return put_user(idata->xfer.len, (__u32 __user *) arg);

	case LIRC_SET_REC_FILTER: {
		void *new;

		ret = get_user(p, (__u32 __user *) arg);
		if (ret)
			return ret;

		/*
		 * the user is trying to set the same
		 * length of the current value
		 */
		if (idata->xfer.len == p)
			return 0;

		/*
		 * multiple users should use the driver with the
		 * length, otherwise return EPERM same data
		 */
		if (idata->nusers > 1)
			return -EPERM;

		/*
		 * if the buffer is already allocated, reallocate it with the
		 * desired value. If the desired value is 0, then the buffer is
		 * freed from krealloc()
		 */
		if (idata->xfer.len)
			new = krealloc(idata->buffer, p, GFP_KERNEL);
		else
			new = kmalloc(p, GFP_KERNEL);

		if (!new)
			return -ENOMEM;

		mutex_lock(&idata->mutex);
		idata->buffer = new;
		idata->xfer.len = p;
		mutex_unlock(&idata->mutex);

		return 0;
	}

	case LIRC_GET_SEND_CARRIER:
		return put_user(idata->xfer.speed_hz, (__u32 __user *) arg);

	case LIRC_SET_SEND_CARRIER:
		ret = get_user(p, (__u32 __user *) arg);
		if (ret)
			return ret;

		/*
		 * The frequency cannot be obviously set to '0',
		 * while, as in the case of the data length,
		 * multiple users should use the driver with the same
		 * frequency value, otherwise return EPERM
		 */
		if (!p || ((idata->nusers > 1) && p != idata->xfer.speed_hz))
			return -EPERM;

		mutex_lock(&idata->mutex);
		idata->xfer.speed_hz = p;
		mutex_unlock(&idata->mutex);
		return 0;
	}

	return -EINVAL;
}

static const struct file_operations ir_spi_fops = {
	.owner   = THIS_MODULE,
	.write   = ir_spi_chardev_write,
	.open    = ir_spi_chardev_open,
	.release = ir_spi_chardev_close,
	.llseek  = noop_llseek,
	.unlocked_ioctl = ir_spi_chardev_ioctl,
	.compat_ioctl   = ir_spi_chardev_ioctl,
};

static int ir_spi_probe(struct spi_device *spi)
{
	struct ir_spi_data *idata;
	int ret;

	idata = devm_kzalloc(&spi->dev, sizeof(*idata), GFP_KERNEL);
	if (!idata)
		return -ENOMEM;
#if 0
	idata->regulator = devm_regulator_get(&spi->dev, "irda_regulator");
	if (IS_ERR(idata->regulator))
		return PTR_ERR(idata->regulator);
#endif

	mutex_init(&idata->mutex);

	idata->spi = spi;

	idata->xfer.bits_per_word = IR_SPI_BIT_PER_WORD;
	idata->xfer.speed_hz = IR_SPI_DEFAULT_FREQUENCY;

	ret = ida_simple_get(&ir_spi_ida, 0, 0, GFP_KERNEL);
	if (ret < 0)
		return ret;

	idata->lirc_id = ret;
	snprintf(idata->miscdev_name, sizeof(idata->miscdev_name),
		 "lirc%d", idata->lirc_id);

	idata->miscdev.minor = MISC_DYNAMIC_MINOR;
	idata->miscdev.name = idata->miscdev_name;
	idata->miscdev.fops = &ir_spi_fops;
	idata->miscdev.parent = &spi->dev;

	spi_set_drvdata(spi, idata);

	ret = misc_register(&idata->miscdev);
	if (ret) {
		ida_simple_remove(&ir_spi_ida, idata->lirc_id);
		dev_err(&spi->dev, "unable to register /dev/%s\n",
			idata->miscdev_name);
		return ret;
	}

	dev_info(&spi->dev, "registered /dev/%s\n", idata->miscdev_name);

	return 0;
}

static int ir_spi_remove(struct spi_device *spi)
{
	struct ir_spi_data *idata = spi_get_drvdata(spi);

	misc_deregister(&idata->miscdev);
	ida_simple_remove(&ir_spi_ida, idata->lirc_id);

	return 0;
}

static const struct of_device_id ir_spi_of_match[] = {
	{ .compatible = "ir-spi" },
	{},
};

static struct spi_driver ir_spi_driver = {
	.probe = ir_spi_probe,
	.remove = ir_spi_remove,
	.driver = {
		.name = IR_SPI_DRIVER_NAME,
		.of_match_table = ir_spi_of_match,
	},
};

module_spi_driver(ir_spi_driver);

MODULE_AUTHOR("Andi Shyti <andi.shyti@samsung.com>");
MODULE_DESCRIPTION("SPI IR LED");
MODULE_LICENSE("GPL v2");
