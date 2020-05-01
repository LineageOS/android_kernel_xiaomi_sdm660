/****************************************************************************************
 *
 * @File Name   : lct_tp_gesture.c
 * @Author      : wanghan
 * @E-mail      : <wanghan@longcheer.com>
 * @Create Time : 2018-09-30 17:34:43
 * @Description : Enable/Disable touchpad.
 *
 ****************************************************************************************/

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <linux/module.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

/*
 * DEFINE CONFIGURATION
 ****************************************************************************************
 */
#define TP_GESTURE_LOG_ENABLE
#define TP_GESTURE_TAG           "LCT_TP_GESTURE"

#ifdef TP_GESTURE_LOG_ENABLE
#define TP_LOGW(log, ...) printk(KERN_WARNING "[%s] %s (line %d): " log, TP_GESTURE_TAG, __func__, __LINE__, ##__VA_ARGS__)
#define TP_LOGE(log, ...) printk(KERN_ERR "[%s] %s ERROR (line %d): " log, TP_GESTURE_TAG, __func__, __LINE__, ##__VA_ARGS__)
#else
#define TP_LOGW(log, ...) {}
#define TP_LOGE(log, ...) {}
#endif

/*
 * DATA STRUCTURES
 ****************************************************************************************
 */
typedef int (*tp_gesture_cb_t)(bool enable_tp);

typedef struct lct_tp{
	bool enable_tp_gesture_flag;
	tp_gesture_cb_t pfun;
}lct_tp_t;

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static lct_tp_t *lct_tp_p = NULL;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

static int lct_creat_proc_tp_entry(void);
static ssize_t lct_proc_tp_gesture_read(struct file *file, char __user *buf, size_t size, loff_t *ppos);
static ssize_t lct_proc_tp_gesture_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos);
static const struct file_operations lct_proc_tp_gesture_fops = {
	.read		= lct_proc_tp_gesture_read,
	.write		= lct_proc_tp_gesture_write,
};


int init_lct_tp_gesture(tp_gesture_cb_t callback)
{
	if (NULL == callback) {
		TP_LOGE("callback is NULL!\n");
		return -EINVAL;
	}

	TP_LOGW("Initialization tp_gesture node!\n");
	lct_tp_p = kzalloc(sizeof(lct_tp_t), GFP_KERNEL);
	if (IS_ERR_OR_NULL(lct_tp_p)){
		TP_LOGE("kzalloc() request memory failed!\n");
		return -ENOMEM;
	}
	lct_tp_p->pfun = callback;
	lct_tp_p->enable_tp_gesture_flag = false;

	lct_creat_proc_tp_entry();

	return 0;
}
EXPORT_SYMBOL(init_lct_tp_gesture);

bool get_lct_tp_gesture_status(void)
{
	return lct_tp_p->enable_tp_gesture_flag;
}
EXPORT_SYMBOL(get_lct_tp_gesture_status);

static int lct_creat_proc_tp_entry(void)
{
	struct proc_dir_entry *proc_entry_tp;

	proc_entry_tp = proc_create_data("tp_gesture", 0444, NULL, &lct_proc_tp_gesture_fops, NULL);
	if (IS_ERR_OR_NULL(proc_entry_tp)) {
		TP_LOGE("add /proc/tp_gesture error!\n");
		return -1;
	}
	TP_LOGW("/proc/tp_gesture is okay!\n");

	return 0;
}

static ssize_t lct_proc_tp_gesture_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	ssize_t cnt=0;
	char *page = NULL;

	page = kzalloc(128, GFP_KERNEL);
	if (IS_ERR_OR_NULL(page))
		return -ENOMEM;

	cnt = sprintf(page, "%s", (lct_tp_p->enable_tp_gesture_flag ? "1\n" : "0\n"));

	cnt = simple_read_from_buffer(buf, size, ppos, page, cnt);
	TP_LOGW("Touchpad status : %s", page);

	kfree(page);
	return cnt;
}

static ssize_t lct_proc_tp_gesture_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
	int ret;
	ssize_t cnt = 0;
	char *page = NULL;
	unsigned int input = 0;

	page = kzalloc(128, GFP_KERNEL);
	if (IS_ERR_OR_NULL(page))
		return -ENOMEM;
	cnt = simple_write_to_buffer(page, 128, ppos, buf, size);
	if (cnt <= 0)
		return -EINVAL;
	if (sscanf(page, "%u", &input) != 1)
		return -EINVAL;

	if (input > 0) {
		if (lct_tp_p->enable_tp_gesture_flag)
			goto exit;
		TP_LOGW("Enbale Touchpad Gesture ...\n");
		ret = lct_tp_p->pfun(true);
		if (ret) {
			TP_LOGW("Enable Touchpad Gesture Failed! ret=%d\n", ret);
			goto exit;
		}
		lct_tp_p->enable_tp_gesture_flag = true;
	} else {
		if (!lct_tp_p->enable_tp_gesture_flag)
			goto exit;
		TP_LOGW("Disable Touchpad Gesture ...\n");
		ret = lct_tp_p->pfun(false);
		if (ret) {
			TP_LOGW("Disable Touchpad Gesture Failed! ret=%d\n", ret);
			goto exit;
		}
		lct_tp_p->enable_tp_gesture_flag = false;
	}
	TP_LOGW("Set Touchpad Gesture successfully!\n");

exit:
	kfree(page);
	return cnt;
}

MODULE_DESCRIPTION("Touchpad Work Contoller Driver");
MODULE_LICENSE("GPL");

