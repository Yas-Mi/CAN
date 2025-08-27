/*
 * can.h
 *
 *  Created on: Jul 30, 2025
 *      Author: User
 */

#ifndef SRC_PERI_CAN_H_
#define SRC_PERI_CAN_H_

#include "can_common.h"

typedef enum {
	CAN_CH_1 = 0,
	CAN_CH_2,
	CAN_CH_MAX,
} CAN_CH;

extern osStatus can_open(CAN_CH ch, uint32_t bit_rate);																						// オープン関数
extern osStatus can_set_mailbox(CAN_CH ch, CAN_COMMON_RX_MAILBOX_INFO *mailbox_info);														// メールボックス設定
extern osStatus can_start(CAN_CH ch);																										// 開始関数
extern osStatus can_stop(CAN_CH ch);																										// 停止関数
extern osStatus can_sleep(CAN_CH ch);																										// スリープ関数
extern osStatus can_send(CAN_CH ch, uint32_t mbx_id, CAN_COMMON_FRAME_TYPE frame_type, uint32_t can_id, uint8_t *data, uint8_t size);		// 送信関数
extern osStatus can_rx_check(CAN_CH ch, uint8_t *p_ret);																					// 受信チェック
extern osStatus can_recv(CAN_CH ch, uint32_t mbx_id, uint32_t *p_can_id, uint8_t *p_data, uint8_t *p_size);									// 受信関数
extern osStatus can_get_status(CAN_CH ch, uint32_t *p_sts);																			// 状態取得関数

#endif /* SRC_PERI_CAN_H_ */
