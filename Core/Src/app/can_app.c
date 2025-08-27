/*
 * can_app.c
 *
 *  Created on: Jul 31, 2025
 *      Author: User
 */
#include <string.h>
#include <stdlib.h>
#include "stm32f7xx.h"
#include "cmsis_os.h"
#include "console.h"
#include "can_mng.h"

#include "can_app.h"

// 受信フレーム1コールバック
static void recv_frame1_callback(uint8_t *p_data, uint8_t size)
{
	console_printf("0x300 %x %x %x %x %x %x %x %x\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);
}

// 受信フレーム2コールバック
static void recv_frame2_callback(uint8_t *p_data, uint8_t size)
{
	console_printf("0x400 %x %x %x %x %x %x %x %x\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);
}

// 受信フレーム3コールバック
static void recv_frame3_callback(uint8_t *p_data, uint8_t size)
{
	console_printf("0x500 %x %x %x %x %x %x %x %x\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);
}

// 受信フレーム4コールバック
static void recv_frame4_callback(uint8_t *p_data, uint8_t size)
{
	console_printf("0x600 %x %x %x %x %x %x %x %x\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);
}

// 受信フレーム5コールバック
static void recv_frame5_callback(uint8_t *p_data, uint8_t size)
{
	console_printf("0x700 %x %x %x %x %x %x %x %x\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);
}

// 初期化
void can_app_init(void)
{
	// コールバック登録
	can_mng_reg_recv_callback(CAN_MNG_RECV_FRAME_1, recv_frame1_callback);
	can_mng_reg_recv_callback(CAN_MNG_RECV_FRAME_2, recv_frame2_callback);
	can_mng_reg_recv_callback(CAN_MNG_RECV_FRAME_3, recv_frame3_callback);
	can_mng_reg_recv_callback(CAN_MNG_RECV_FRAME_4, recv_frame4_callback);
	can_mng_reg_recv_callback(CAN_MNG_RECV_FRAME_5, recv_frame5_callback);
	
	return;
}

// 取得関数
// (*) 後で作成

// セット関数
void set_sig1_7(uint8_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_1);

	// データセット
	p_send_data[7] = val;

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig1_6(uint8_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_1);

	// データセット
	p_send_data[6] = val;

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig1_5(uint8_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_1);

	// データセット
	p_send_data[5] = val;

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig1_4(uint8_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_1);

	// データセット
	p_send_data[4] = val;

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig1_3(uint8_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_1);

	// データセット
	p_send_data[3] = val;

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig1_2(uint8_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_1);

	// データセット
	p_send_data[2] = val;

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig1_1(uint8_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_1);

	// データセット
	p_send_data[1] = val;

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig1_0(uint8_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_1);

	// データセット
	p_send_data[0] = val;

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig2_3(uint16_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_2);

	// データセット
	p_send_data[7] = (uint8_t)((val >> 8) & 0xFF);
	p_send_data[6] = (uint8_t)((val >> 0) & 0xFF);

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig2_2(uint16_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_2);

	// データセット
	p_send_data[5] = (uint8_t)((val >> 8) & 0xFF);
	p_send_data[4] = (uint8_t)((val >> 0) & 0xFF);

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig2_1(uint16_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_2);

	// データセット
	p_send_data[3] = (uint8_t)((val >> 8) & 0xFF);
	p_send_data[2] = (uint8_t)((val >> 0) & 0xFF);

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig2_0(uint16_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_2);

	// データセット
	p_send_data[1] = (uint8_t)((val >> 8) & 0xFF);
	p_send_data[0] = (uint8_t)((val >> 0) & 0xFF);

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig3_1(uint32_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_3);

	// データセット
	p_send_data[7] = (uint8_t)((val >> 24) & 0xFF);
	p_send_data[6] = (uint8_t)((val >> 16) & 0xFF);
	p_send_data[5] = (uint8_t)((val >>  8) & 0xFF);
	p_send_data[4] = (uint8_t)((val >>  0) & 0xFF);

	// 割り込み禁止解除
	__enable_irq();
}
// セット関数
void set_sig3_0(uint32_t val)
{
	uint8_t *p_send_data;

	// 割り込み禁止
	__disable_irq();

	// 送信データ取得
	p_send_data = can_mng_get_send_data(CAN_MNG_SEND_FRAME_3);

	// データセット
	p_send_data[3] = (uint8_t)((val >> 24) & 0xFF);
	p_send_data[2] = (uint8_t)((val >> 16) & 0xFF);
	p_send_data[1] = (uint8_t)((val >>  8) & 0xFF);
	p_send_data[0] = (uint8_t)((val >>  0) & 0xFF);

	// 割り込み禁止解除
	__enable_irq();
}

// コマンド
// フレーム1設定
static void cmd_set_frame_1(int argc, char *argv[])
{
	uint8_t idx;
	uint8_t val;
	typedef void (*SET_FUNC)(uint8_t val);
	SET_FUNC set_func_tbl[8] = {
		set_sig1_0,
		set_sig1_1,
		set_sig1_2,
		set_sig1_3,
		set_sig1_4,
		set_sig1_5,
		set_sig1_6,
		set_sig1_7,
	};
	
	// 引数チェック
	if (argc < 3) {
		console_printf("set frame_1 <byte> <val>\n");
		return;
	}
	
	// 値設定
	idx = atoi(argv[2]);
	val = atoi(argv[3]);
	
	// 値セット
	set_func_tbl[idx](val);
}

// フレーム2設定
static void cmd_set_frame_2(int argc, char *argv[])
{
	uint8_t idx;
	uint16_t val;
	typedef void (*SET_FUNC)(uint16_t val);
	SET_FUNC set_func_tbl[4] = {
		set_sig2_3,
		set_sig2_2,
		set_sig2_1,
		set_sig2_0,
	};
	
	// 引数チェック
	if (argc < 3) {
		console_printf("set frame_2 <idx> <val>\n");
		return;
	}
	
	// 値設定
	idx = atoi(argv[2]);
	val = atoi(argv[3]);
	
	// 値セット
	set_func_tbl[idx](val);
}

// フレーム1設定
static void cmd_set_frame_3(int argc, char *argv[])
{
	uint8_t idx;
	uint32_t val;
	typedef void (*SET_FUNC)(uint32_t val);
	SET_FUNC set_func_tbl[2] = {
		set_sig3_1,
		set_sig3_0,
	};
	
	// 引数チェック
	if (argc < 3) {
		console_printf("set frame_3 <idx> <val>\n");
		return;
	}
	
	// 値設定
	idx = atoi(argv[2]);
	val = atoi(argv[3]);
	
	// 値セット
	set_func_tbl[idx](val);
}

// フレーム1設定
static void cmd_active(int argc, char *argv[])
{
	// アクティブ
	can_mng_active();
}

// コマンド設定関数
void can_app_set_cmd(void)
{
	COMMAND_INFO cmd;
	
	// コマンドの設定
	cmd.input = "set frame_1";
	cmd.func = cmd_set_frame_1;
	console_set_command(&cmd);
	// コマンドの設定
	cmd.input = "set frame_2";
	cmd.func = cmd_set_frame_2;
	console_set_command(&cmd);
	// コマンドの設定
	cmd.input = "set frame_3";
	cmd.func = cmd_set_frame_3;
	console_set_command(&cmd);
	// コマンドの設定
	cmd.input = "can active";
	cmd.func = cmd_active;
	console_set_command(&cmd);
}

