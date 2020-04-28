#ifndef __LCT_TP_GESTURE_H__
#define __LCT_TP_GESTURE_H__

typedef int (*tp_gesture_cb_t)(bool enable_tp);

extern int init_lct_tp_gesture(tp_gesture_cb_t callback);
extern bool get_lct_tp_gesture_status(void);

#endif

