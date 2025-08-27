/*
 * can_mng.h
 *
 *  Created on: 2024/3/3
 *      Author: ronald
 */

#ifndef CANH_MNG_H_
#define CANH_MNG_H_

// CANフレームの定義
// 送信
#define CAN_MNG_SEND_FRAME_1	(0)
#define CAN_MNG_SEND_FRAME_2	(1)
#define CAN_MNG_SEND_FRAME_3	(2)
#define CAN_MNG_SEND_FRAME_4	(3)
#define CAN_MNG_SEND_FRAME_5	(4)
#define CAN_MNG_SEND_FRAME_6	(5)
#define CAN_MNG_SEND_FRAME_7	(6)
#define CAN_MNG_SEND_FRAME_8	(7)
#define CAN_MNG_SEND_FRAME_9	(8)
#define CAN_MNG_SEND_FRAME_10	(9)
#define CAN_MNG_SEND_FRAME_MAX	(10)
// 受信
#define CAN_MNG_RECV_FRAME_1	(0)
#define CAN_MNG_RECV_FRAME_2	(1)
#define CAN_MNG_RECV_FRAME_3	(2)
#define CAN_MNG_RECV_FRAME_4	(3)
#define CAN_MNG_RECV_FRAME_5	(4)
#define CAN_MNG_RECV_FRAME_MAX	(5)

// 受信コールバック
typedef void(*CAN_MNG_RECV_CALLBACK)(uint8_t *p_data, uint8_t size);

// プロトタイプ
extern void can_mng_init(void);
extern osStatus can_mng_active(void);
extern osStatus can_mng_inactive(void);
extern osStatus can_mng_sleep(void);
extern uint8_t *can_mng_get_recv_data(uint32_t frame_type);
extern uint8_t *can_mng_get_send_data(uint32_t frame_type);
extern osStatus can_mng_reg_recv_callback(uint32_t frame_type, CAN_MNG_RECV_CALLBACK callback);

extern void can_mng_set_cmd(void);

#endif /* CANH_MNG_H_ */
