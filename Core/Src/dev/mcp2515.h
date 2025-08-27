/*
 * mcp2515.h
 *
 *  Created on: 2024/2/28
 *      Author: User
 */

#ifndef DEV_MCP2515_H_
#define DEV_MCP2515_H_

#include "can_common.h"

// 定義
// 送信メールボックスID
#define MCP2515_DEV_TX_MBX_ID_0			(0)
#define MCP2515_DEV_TX_MBX_ID_1			(1)
#define MCP2515_DEV_TX_MBX_ID_2			(2)
#define MCP2515_DEV_TX_MBX_ID_MAX		(3)

// 受信メールボックスID
#define MCP2515_DEV_RX_MBX_ID_0			(0)
#define MCP2515_DEV_RX_MBX_ID_1			(1)
#define MCP2515_DEV_RX_MBX_ID_MAX		(2)

// デバイス
typedef enum {
	MCP2515_DEV_1 = 0,
	MCP2515_DEV_MAX,
} MCP2515_DEV;

// マスクとフィルタの考え
/*
RXM0 → RXF0,RXF1
RXM1 → RXF2,RXF3,RXF4,RXF5

例えばRXF0=0x101, RXF1=0x201, RXM0=0b001111110000(0:マスクしない、1:マスクする)の場合、
0x10Xと0x20Xを受信する

*/

extern void mcp2515_dev_init(void);
extern osStatus mcp2515_dev_open(MCP2515_DEV dev, uint32_t bit_rate);
extern osStatus mcp2515_dev_set_mailbox(MCP2515_DEV dev, CAN_COMMON_RX_MAILBOX_INFO *p_mbox_info);
extern osStatus mcp2515_dev_start(MCP2515_DEV dev);
extern osStatus mcp2515_dev_stop(MCP2515_DEV dev);
extern osStatus mcp2515_dev_sleep(MCP2515_DEV dev);
extern osStatus mcp2515_dev_send(MCP2515_DEV dev, uint32_t mbx_id, CAN_COMMON_FRAME_TYPE frame_type, uint32_t can_id, uint8_t *data, uint8_t size);
extern osStatus mcp2515_dev_rx_check(MCP2515_DEV dev, uint8_t *p_ret);
extern osStatus mcp2515_dev_get_rx_data(MCP2515_DEV dev, uint32_t mbx_id, uint32_t *p_can_id, uint8_t *p_data, uint8_t *p_size);
extern osStatus mcp2515_dev_get_status(MCP2515_DEV dev, uint32_t *p_sts);

extern void mcp2515_set_cmd(void);
#endif /* DEV_MCP2515_H_ */