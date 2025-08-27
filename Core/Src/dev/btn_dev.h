/*
 * BTN_dev.h
 *
 *  Created on: 2022/11/13
 *      Author: User
 */

#ifndef DEV_BTN_DEV_H_
#define DEV_BTN_DEV_H_

// ボタンの状態
typedef enum {
	BTN_SHORT_PUSHED = 0,
	BTN_LONG_PUSHED,
	BTN_STATUS_MAX,
} BTN_STATUS;

// ボタンの種類
typedef enum {
	BTN_TYPE_USER_BTN = 0,	//!< 上ボタン
	BTN_MAX,
} BTN_TYPE;

// コールバック関数定義
typedef void (*BTN_CALLBACK)(BTN_STATUS sts);

// 公開関数
extern void btn_dev_init(void);
extern osStatus BTN_dev_reg_callback(BTN_TYPE type, BTN_CALLBACK cb);

#endif /* DEV_BTV_DEV_H_ */
