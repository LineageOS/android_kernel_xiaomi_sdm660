// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/dma-buf.h>

#include "ion.h"
#include "ion_system_secure_heap.h"

#ifdef CONFIG_ION_LEGACY
#include "ion_legacy.h"
#endif

union ion_ioctl_arg {
	struct ion_allocation_data allocation;
	struct ion_heap_query query;
	struct ion_prefetch_data prefetch_data;
#ifdef CONFIG_ION_LEGACY
	struct ion_custom_data custom;
	struct ion_flush_data flush_data;
	struct ion_fd_data fd;
	struct ion_old_allocation_data old_allocation;
	struct ion_handle_data handle;
#endif
};

static int validate_ioctl_arg(unsigned int cmd, union ion_ioctl_arg *arg)
{
	switch (cmd) {
	case ION_IOC_HEAP_QUERY:
		if (arg->query.reserved0 ||
		    arg->query.reserved1 ||
		    arg->query.reserved2)
			return -EINVAL;
		break;
	default:
		break;
	}

	return 0;
}

/* fix up the cases where the ioctl direction bits are incorrect */
static unsigned int ion_ioctl_dir(unsigned int cmd)
{
	switch (cmd) {
#ifdef CONFIG_ION_LEGACY
	case ION_IOC_CLEAN_CACHES:
	case ION_IOC_INV_CACHES:
	case ION_IOC_CLEAN_INV_CACHES:
	case ION_IOC_CUSTOM:
	case ION_IOC_FREE:
		return _IOC_WRITE;
#endif
	default:
		return _IOC_DIR(cmd);
	}
}

#ifdef CONFIG_ION_LEGACY
long ion_legacy_cache_ioctl(unsigned int cmd, ion_user_handle_t handle,
			    int fd, unsigned int offset, unsigned int length)
{
	struct dma_buf *dmabuf;
	enum dma_data_direction direction;
	int buffer_fd = fd >= 0 ? fd : handle;
	int ret;

	switch (cmd) {
	case ION_IOC_CLEAN_CACHES:
		direction = DMA_TO_DEVICE;
		break;
	case ION_IOC_INV_CACHES:
		direction = DMA_FROM_DEVICE;
		break;
	case ION_IOC_CLEAN_INV_CACHES:
		direction = DMA_BIDIRECTIONAL;
		break;
	default:
		return -ENOTTY;
	}

	dmabuf = dma_buf_get(buffer_fd);
	if (IS_ERR(dmabuf))
		return PTR_ERR(dmabuf);

	if (offset > dmabuf->size || length > dmabuf->size - offset) {
		ret = -EINVAL;
		goto out;
	}

	ret = dma_buf_begin_cpu_access_partial(dmabuf, direction, offset,
					       length);
	if (!ret)
		ret = dma_buf_end_cpu_access_partial(dmabuf, direction, offset,
						     length);
out:
	dma_buf_put(dmabuf);
	return ret;
}

static long ion_legacy_custom_ioctl(const struct ion_custom_data *custom)
{
	struct ion_flush_data flush_data;

	switch (custom->cmd) {
	case ION_IOC_CLEAN_CACHES:
	case ION_IOC_INV_CACHES:
	case ION_IOC_CLEAN_INV_CACHES:
		break;
	default:
		return -ENOTTY;
	}

	if (copy_from_user(&flush_data, (void __user *)custom->arg,
			   sizeof(flush_data)))
		return -EFAULT;

	return ion_legacy_cache_ioctl(custom->cmd, flush_data.handle,
				      flush_data.fd, flush_data.offset,
				      flush_data.length);
}
#endif


long ion_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	unsigned int dir;
	union ion_ioctl_arg data;

	dir = ion_ioctl_dir(cmd);

	if (_IOC_SIZE(cmd) > sizeof(data))
		return -EINVAL;

	/*
	 * The copy_from_user is unconditional here for both read and write
	 * to do the validate. If there is no write for the ioctl, the
	 * buffer is cleared
	 */
	if (copy_from_user(&data, (void __user *)arg, _IOC_SIZE(cmd)))
		return -EFAULT;

	ret = validate_ioctl_arg(cmd, &data);
	if (ret) {
		pr_warn_once("%s: ioctl validate failed\n", __func__);
		return ret;
	}

	if (!(dir & _IOC_WRITE))
		memset(&data, 0, sizeof(data));

	switch (cmd) {
	case ION_IOC_ALLOC:
	{
		int fd;

		fd = ion_alloc_fd(data.allocation.len,
				  data.allocation.heap_id_mask,
				  data.allocation.flags);
		if (fd < 0)
			return fd;

		data.allocation.fd = fd;

		break;
	}
	case ION_IOC_HEAP_QUERY:
		ret = ion_query_heaps(&data.query);
		break;
	case ION_IOC_PREFETCH:
	{
		int ret;

		ret = ion_walk_heaps(data.prefetch_data.heap_id,
				     (enum ion_heap_type)
				     ION_HEAP_TYPE_SYSTEM_SECURE,
				     (void *)&data.prefetch_data,
				     ion_system_secure_heap_prefetch);
		if (ret)
			return ret;
		break;
	}
	case ION_IOC_DRAIN:
	{
		int ret;

		ret = ion_walk_heaps(data.prefetch_data.heap_id,
				     (enum ion_heap_type)
				     ION_HEAP_TYPE_SYSTEM_SECURE,
				     (void *)&data.prefetch_data,
				     ion_system_secure_heap_drain);

		if (ret)
			return ret;
		break;
	}
#ifdef CONFIG_ION_LEGACY
	case ION_IOC_CLEAN_CACHES:
	case ION_IOC_INV_CACHES:
	case ION_IOC_CLEAN_INV_CACHES:
		return ion_legacy_cache_ioctl(cmd, data.flush_data.handle,
					      data.flush_data.fd,
					      data.flush_data.offset,
					      data.flush_data.length);
	case ION_IOC_CUSTOM:
		return ion_legacy_custom_ioctl(&data.custom);
	case ION_OLD_IOC_ALLOC:
	{
		int fd;

		fd = ion_alloc_fd(data.old_allocation.len,
				  data.old_allocation.heap_id_mask,
				  data.old_allocation.flags);
		if (fd < 0)
			return fd;

		data.old_allocation.handle = fd;

		break;
	}
	case ION_IOC_FREE:
		/*
		 * libion passes 0 as the handle to check for this ioctl's
		 * existence and expects -ENOTTY on kernel 4.12+ as an indicator
		 * of having a new ION ABI. We want to use new ION as much as
		 * possible, so pretend that this ioctl doesn't exist when
		 * libion checks for it.
		 */
		if (!data.handle.handle)
			ret = -ENOTTY;

		break;
	case ION_IOC_SHARE:
	case ION_IOC_MAP:
		data.fd.fd = data.fd.handle;
		break;
	case ION_IOC_IMPORT:
		data.fd.handle = data.fd.fd;
		break;
#endif
	default:
		return -ENOTTY;
	}

	if (dir & _IOC_READ) {
		if (copy_to_user((void __user *)arg, &data, _IOC_SIZE(cmd)))
			return -EFAULT;
	}
	return ret;
}
