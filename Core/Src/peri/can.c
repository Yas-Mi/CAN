/*
 * can.c
 *
 *  Created on: Jul 30, 2025
 *      Author: User
 */
#include <string.h>
#include "stm32f7xx.h"
#include "stm32f7xx_hal_rcc.h"
#include "cmsis_os.h"
#include "iodefine.h"

#include "can.h"

// オープン関数
osStatus can_open(CAN_CH ch, uint32_t bit_rate)
{
	return osOK;
}

// メールボックス設定
osStatus can_set_mailbox(CAN_CH ch, CAN_COMMON_RX_MAILBOX_INFO *mailbox_info)
{
	return osOK;
	
}

// 開始関数
osStatus can_start(CAN_CH ch)
{
	return osOK;
}

// 停止関数
osStatus can_stop(CAN_CH ch)
{
	return osOK;
}

// スリープ関数
osStatus can_sleep(CAN_CH ch)
{
	return osOK;
}

// 送信関数
osStatus can_send(CAN_CH ch, uint32_t mbx_id, CAN_COMMON_FRAME_TYPE frame_type, uint32_t can_id, uint8_t *data, uint8_t size)
{
	return osOK;
}

// 受信データチェック
osStatus can_rx_check(CAN_CH ch, uint8_t *p_ret)
{
	return osOK;
}

// 受信関数
osStatus can_recv(CAN_CH ch, uint32_t mbx_id, uint32_t *p_can_id, uint8_t *p_data, uint8_t *p_size)
{
	return osOK;
}

// 状態取得関数
osStatus can_get_status(CAN_CH ch, uint32_t *p_sts)
{
	return osOK;
}
