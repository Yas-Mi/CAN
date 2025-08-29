/*
 * can_drv.h
 *
 *  Created on: 2024/3/3
 *      Author: ronald
 */

#ifndef CANH_DRV_H_
#define CANH_DRV_H_

// CH情報
typedef enum {
	CAN_DRV_CH_1 = 0,
	CAN_DRV_CH_2,
	CAN_DRV_CH_MAX,
} CAN_DRV_CH;

// メールボックスID
#define CAN_DRV_CH_1_TX_MAILBOX_0		(0)		// 未使用
#define CAN_DRV_CH_1_TX_MAILBOXMAX		(1)		// 未使用
#define CAN_DRV_CH_1_RX_MAILBOX_0		(0)		// 未使用
#define CAN_DRV_CH_1_RX_MAILBOX_1		(1)		// 未使用
#define CAN_DRV_CH_1_RX_MAILBOX_MAX		(2)		// 未使用
#define CAN_DRV_CH_2_TX_MAILBOX_0		(0)		// 送信メールボックス0
#define CAN_DRV_CH_2_TX_MAILBOX_1		(1)		// 送信メールボックス1
#define CAN_DRV_CH_2_TX_MAILBOX_2		(2)		// 送信メールボックス2
#define CAN_DRV_CH_2_TX_MAILBOX_MAX		(3)
#define CAN_DRV_CH_2_RX_MAILBOX_0		(0)		// 受信メールボックス0
#define CAN_DRV_CH_2_RX_MAILBOX_1		(1)		// 受信メールボックス1
#define CAN_DRV_CH_2_RX_MAILBOX_MAX		(2)

// CANIF
// 内蔵CANと外付けCANのインタフェースは以下に合わせること
typedef osStatus (*CAN_DRV_OPEN)(uint32_t ch, uint32_t bit_rate);																					// オープン関数
typedef osStatus (*CAN_DRV_SET_MAILBOX)(uint32_t ch, CAN_COMMON_RX_MAILBOX_INFO *mailbox_info);														// メールボックス設定
typedef osStatus (*CAN_DRV_START)(uint32_t ch);																										// 開始関数
typedef osStatus (*CAN_DRV_STOP)(uint32_t ch);																										// 停止関数
typedef osStatus (*CAN_DRV_SLEEP)(uint32_t ch);																										// スリープ関数
typedef osStatus (*CAN_DRV_SEND)(uint32_t ch, uint32_t mbx_id, CAN_COMMON_FRAME_TYPE frame_type, uint32_t can_id, uint8_t *data, uint8_t size);		// 送信関数
typedef osStatus (*CAN_DRV_RX_CHECK)(uint32_t dev, uint8_t *p_ret);																					// 受信チェック
typedef osStatus (*CAN_DRV_RECV)(uint32_t ch, uint32_t mbx_id, uint32_t *p_can_id, uint8_t *p_data, uint8_t *p_size);								// 受信関数
typedef osStatus (*CAN_DRV_GET_STATUS)(uint32_t ch, uint32_t *p_sts);																			// 状態取得関数

// プロトタイプ
extern void can_drv_init(void);
extern osStatus can_drv_open(CAN_DRV_CH ch);
extern osStatus can_drv_set_mailbox(CAN_DRV_CH ch, CAN_COMMON_RX_MAILBOX_INFO *p_mbx_info);
extern osStatus can_drv_start(CAN_DRV_CH ch);
extern osStatus can_drv_stop(CAN_DRV_CH ch);
extern osStatus can_drv_sleep(CAN_DRV_CH ch);
extern osStatus can_drv_send(CAN_DRV_CH ch, uint32_t mbx_id, CAN_COMMON_FRAME_TYPE frame_type, uint32_t can_id, uint8_t *p_data, uint8_t size);
extern osStatus can_drv_rx_check(CAN_DRV_CH ch, uint32_t *p_ret);
extern osStatus can_drv_recv(CAN_DRV_CH ch, uint32_t mbx_id, uint32_t *p_can_id, uint8_t *p_data, uint8_t *p_size);
extern osStatus can_drv_get_status(CAN_DRV_CH ch, uint32_t *p_sts);

extern void can_drv_set_cmd(void);

#endif /* CANH_DRV_H_ */
