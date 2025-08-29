/*
 * can_drv_.c
 *
 *  Created on: 2025/7/22
 *      Author: ronald
 */
#include <string.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "console.h"
#include "can_common.h"
#include "mcp2515.h"
#include "can.h"

#include "can_drv.h"

// 状態
#define ST_UNINITIALIZED	(0)		// 未初期化
#define ST_INITIALIZED		(1)		// 初期化
#define ST_OPENED			(2)		// オープン済み
#define ST_START			(3)		// 開始状態
#define ST_MAX				(4)

// マクロ
#define SEND_RETRY_CNT		(10)	// 送信のリトライ回数

// CHタイプ
typedef enum {
	CAN_DRV_CH_TYPE_INTERNAL = 0,	// 内部CAN
	CAN_DRV_CH_TYPE_EXTERNAL_1,		// 外部CAN(MCP2515)
//	CAN_DRV_CH_TYPE_EXTERNAL_2,		// 外部CAN(その他)
	CAN_DRV_CH_TYPE_MAX,
} CAN_DRV_CH_TYPE;

// CANIFリスト
typedef struct {
	CAN_DRV_OPEN			open;
	CAN_DRV_SET_MAILBOX		set_mailbox;
	CAN_DRV_START			start;
	CAN_DRV_STOP			stop;
	CAN_DRV_SLEEP			sleep;
	CAN_DRV_SEND			send;
	CAN_DRV_RX_CHECK		rx_check;
	CAN_DRV_RECV			recv;
	CAN_DRV_GET_STATUS		get_status;
} CAN_DRV_FUNC_LIST;

// 内部CANIFテーブル
const CAN_DRV_FUNC_LIST in_func_info = {
	can_open,
	can_set_mailbox,
	can_start,
	can_stop,
	can_sleep,
	can_send,
	can_rx_check,
	can_recv,
	can_get_status
};

// 外部CANIFテーブル
const CAN_DRV_FUNC_LIST ex_func_info = {
	mcp2515_dev_open,
	mcp2515_dev_set_mailbox,
	mcp2515_dev_start,
	mcp2515_dev_stop,
	mcp2515_dev_sleep,
	mcp2515_dev_send,
	mcp2515_dev_rx_check,
	mcp2515_dev_get_rx_data,
	mcp2515_dev_get_status,
};

// CANIFテーブル
const CAN_DRV_FUNC_LIST *can_drv_func_list_tbl[CAN_DRV_CH_TYPE_MAX] = {
	&in_func_info,
	&ex_func_info,
};

// chタイプ
typedef struct {
	CAN_DRV_CH_TYPE		type;		// chタイプ
	uint8_t				ch;			// 物理チャネル
	uint32_t			bit_rate;	// ビットレート
} CAN_DRV_CH_INFO;

// チャネルタイプテーブル
static const CAN_DRV_CH_INFO can_drv_ch_info[CAN_DRV_CH_MAX] = {
//	type							periferal_ch	bit_rate
	{CAN_DRV_CH_TYPE_INTERNAL,		CAN_CH_1,		250*1000},	// CAN_DRV_CH_1
	{CAN_DRV_CH_TYPE_EXTERNAL_1,	MCP2515_DEV_1,  250*1000},	// CAN_DRV_CH_2
//	{CAN_DRV_CH_TYPE_INTERNAL,		CAN_CH_2,  		250*1000},	// CAN_DRV_CH_3
};

// 制御用ブロック
typedef struct {
	CAN_DRV_CH		ch;			// チャネル	
	uint32_t		state;		// 状態
} CAN_DRV_CTL;
static CAN_DRV_CTL can_drv_ctl[CAN_DRV_CH_MAX];
#define get_myself(ch) (&can_drv_ctl[ch])

// 初期化
void can_drv_init(void)
{
	CAN_DRV_CTL *this;
	CAN_DRV_CH ch;
	
	// 状態を更新
	for (ch = 0; ch < CAN_DRV_CH_MAX; ch++) {
		// コンテキスト取得
		this = get_myself(ch);
		// コンテキスト初期化
		memset(this, 0, sizeof(CAN_DRV_CTL));
		// チャネルセット
		this->ch = ch;
		// 状態の更新
		this->state = ST_INITIALIZED;
	}
}

// オープン
osStatus can_drv_open(CAN_DRV_CH ch)
{
	CAN_DRV_CTL *this;
	const CAN_DRV_CH_INFO *p_info;
	CAN_DRV_OPEN open;
	osStatus ercd;
	
	// パラメータチェック
	if (ch >= CAN_DRV_CH_MAX) {
		return osErrorParameter;
	}
	
	// コンテキスト取得
	this = get_myself(ch);
	
	// 初期化してないならエラー返して終了
	if (this->state != ST_INITIALIZED) {
		return osErrorResource;
	}
	
	// チャネルタイプ取得
	p_info = &(can_drv_ch_info[ch]);
	
	// オープン関数取得
	open = can_drv_func_list_tbl[p_info->type]->open;
	
	// オープン実行
	ercd = open(p_info->ch, p_info->bit_rate);
	if (ercd != osOK) {
		console_printf("can_drv_open failed\n");
		goto EXIT;
	}
	
	// 状態の更新
	this->state = ST_OPENED;
	
EXIT:
	return ercd;
}

// メールボックス設定
osStatus can_drv_set_mailbox(CAN_DRV_CH ch, CAN_COMMON_RX_MAILBOX_INFO *p_mbx_info)
{
	CAN_DRV_CTL *this;
	const CAN_DRV_CH_INFO *p_info;
	CAN_DRV_SET_MAILBOX set_mailbox;
	osStatus ercd;
	
	// パラメータチェック
	if (ch >= CAN_DRV_CH_MAX) {
		return osErrorParameter;
	}
	
	// コンテキスト取得
	this = get_myself(ch);
	
	// 初期化してないならエラー返して終了
	if (this->state != ST_OPENED) {
		return osErrorResource;
	}
	
	// チャネルタイプ取得
	p_info = &(can_drv_ch_info[ch]);
	
	// 開始関数取得
	set_mailbox = can_drv_func_list_tbl[p_info->type]->set_mailbox;
	
	// 開始実行
	ercd = set_mailbox(p_info->ch, p_mbx_info);
	if (ercd != osOK) {
		console_printf("can_drv_set_mailbox failed\n");
		goto EXIT;
	}
	
EXIT:
	return ercd;
}

// 開始関数
osStatus can_drv_start(CAN_DRV_CH ch)
{
	CAN_DRV_CTL *this;
	const CAN_DRV_CH_INFO *p_info;
	CAN_DRV_START start;
	osStatus ercd;
	
	// パラメータチェック
	if (ch >= CAN_DRV_CH_MAX) {
		return osErrorParameter;
	}
	
	// コンテキスト取得
	this = get_myself(ch);
	
	// 初期化してないならエラー返して終了
	if (this->state != ST_OPENED) {
		return osErrorResource;
	}
	
	// チャネルタイプ取得
	p_info = &(can_drv_ch_info[ch]);
	
	// 開始関数取得
	start = can_drv_func_list_tbl[p_info->type]->start;
	
	// 開始実行
	ercd = start(p_info->ch);
	if (ercd != osOK) {
		console_printf("can_drv_start failed\n");
		goto EXIT;
	}
	
	// 状態の更新
	this->state = ST_START;
	
EXIT:
	return ercd;
}

// 停止関数
osStatus can_drv_stop(CAN_DRV_CH ch)
{
	CAN_DRV_CTL *this;
	const CAN_DRV_CH_INFO *p_info;
	CAN_DRV_STOP stop;
	osStatus ercd;
	
	// パラメータチェック
	if (ch >= CAN_DRV_CH_MAX) {
		return osErrorParameter;
	}
	
	// コンテキスト取得
	this = get_myself(ch);
	
	// 開始状態でないならエラー返して終了
	if (this->state != ST_START) {
		return osErrorResource;
	}
	
	// チャネルタイプ取得
	p_info = &(can_drv_ch_info[ch]);
	
	// 開始関数取得
	stop = can_drv_func_list_tbl[p_info->type]->stop;
	
	// 開始実行
	ercd = stop(p_info->ch);
	if (ercd != osOK) {
		console_printf("can_drv_stop failed\n");
		goto EXIT;
	}
	
	// 状態の更新
	this->state = ST_OPENED;
	
EXIT:
	return ercd;
}

// スリープ関数
osStatus can_drv_sleep(CAN_DRV_CH ch)
{
	CAN_DRV_CTL *this;
	const CAN_DRV_CH_INFO *p_info;
	CAN_DRV_SLEEP sleep;
	osStatus ercd;
	
	// パラメータチェック
	if (ch >= CAN_DRV_CH_MAX) {
		return osErrorParameter;
	}
	
	// コンテキスト取得
	this = get_myself(ch);
	
	// 開始状態でないならエラー返して終了
	if (this->state != ST_START) {
		return osErrorResource;
	}
	
	// チャネルタイプ取得
	p_info = &(can_drv_ch_info[ch]);
	
	// スリープ関数取得
	sleep = can_drv_func_list_tbl[p_info->type]->sleep;
	
	// 開始実行
	ercd = sleep(p_info->ch);
	if (ercd != osOK) {
		console_printf("can_drv_sleep failed\n");
		goto EXIT;
	}
	
EXIT:
	return ercd;
}

// 送信
osStatus can_drv_send(CAN_DRV_CH ch, uint32_t mbx_id, CAN_COMMON_FRAME_TYPE frame_type, uint32_t can_id, uint8_t *p_data, uint8_t size)
{
	CAN_DRV_CTL *this ;
	const CAN_DRV_CH_INFO *p_info;
	CAN_DRV_SEND send;
	osStatus ercd;
	uint8_t retry_cnt = SEND_RETRY_CNT;
	
	// パラメータチェック
	if ((ch >= CAN_DRV_CH_MAX) || 
		(frame_type >= CAN_COMMON_FRAME_TYPE_MAX) ||
		(p_data == NULL) ||
		(size == 0)) {
		return osErrorParameter;
	}
	
	// コンテキスト取得
	this = get_myself(ch);
	
	// オープンしてないならエラー返して終了
	if (this->state != ST_START) {
		return osErrorResource;
	}
	
	// チャネルタイプ取得
	p_info = &(can_drv_ch_info[ch]);
	
	// 開始関数取得
	send = can_drv_func_list_tbl[p_info->type]->send;
	
	// 送信
	while (retry_cnt--) {
		// 送信
		ercd = send(p_info->ch, mbx_id, frame_type, can_id, p_data, size);
		// 送信出来たら終了
		if (ercd == osOK) {
			break;
		}
		console_printf("can_drv_send failed(%d)\n", retry_cnt);
	}
	
	return ercd;
}

// 受信データチェック
osStatus can_drv_rx_check(CAN_DRV_CH ch, uint32_t *p_ret)
{
	CAN_DRV_CTL *this ;
	const CAN_DRV_CH_INFO *p_info;
	CAN_DRV_RX_CHECK rx_check;
	osStatus ercd;
	
	// パラメータチェック
	if ((ch >= CAN_DRV_CH_MAX) || 
		(p_ret == NULL)) {
		return osErrorParameter;
	}
	
	// コンテキスト取得
	this = get_myself(ch);
	
	// オープンしてないならエラー返して終了
	if (this->state != ST_START) {
		return osErrorResource;
	}
	
	// チャネルタイプ取得
	p_info = &(can_drv_ch_info[ch]);
	
	// 開始関数取得
	rx_check = can_drv_func_list_tbl[p_info->type]->rx_check;
	
	// 受信
	ercd = rx_check(p_info->ch, p_ret);
	if (ercd != osOK) {
		console_printf("can_drv_rx_check failed\n");
	}
	
	return ercd;
}

// 受信
osStatus can_drv_recv(CAN_DRV_CH ch, uint32_t mbx_id, uint32_t *p_can_id, uint8_t *p_data, uint8_t *p_size)
{
	CAN_DRV_CTL *this ;
	const CAN_DRV_CH_INFO *p_info;
	CAN_DRV_RECV recv;
	osStatus ercd;
	
	// パラメータチェック
	if ((ch >= CAN_DRV_CH_MAX) || 
		(p_can_id == NULL) ||
		(p_data == NULL) ||
		(p_size == NULL)) {
		return osErrorParameter;
	}
	
	// コンテキスト取得
	this = get_myself(ch);
	
	// オープンしてないならエラー返して終了
	if (this->state != ST_START) {
		return osErrorResource;
	}
	
	// チャネルタイプ取得
	p_info = &(can_drv_ch_info[ch]);
	
	// 開始関数取得
	recv = can_drv_func_list_tbl[p_info->type]->recv;
	
	// 受信
	ercd = recv(p_info->ch, mbx_id, p_can_id, p_data, p_size);
	if ((ercd != osOK) && (ercd != osErrorResource)) {
		// osErrorResourceはデータがないということなので、エラーではない
		console_printf("can_drv_recv failed\n");
	}
	
	return ercd;
}

// 状態
osStatus can_drv_get_status(CAN_DRV_CH ch, uint32_t *p_sts)
{
	CAN_DRV_CTL *this ;
	const CAN_DRV_CH_INFO *p_info;
	CAN_DRV_GET_STATUS get_status;
	osStatus ercd;
	
	// パラメータチェック
	if ((ch >= CAN_DRV_CH_MAX) || 
		(p_sts == NULL)) {
		return osErrorParameter;
	}
	
	// コンテキスト取得
	this = get_myself(ch);
	
	// オープンしてないならエラー返して終了
	if (this->state != ST_START) {
		return osErrorResource;
	}
	
	// チャネルタイプ取得
	p_info = &(can_drv_ch_info[ch]);
	
	// 開始関数取得
	get_status = can_drv_func_list_tbl[p_info->type]->get_status;
	
	// 状態取得
	ercd = get_status(p_info->ch, p_sts);
	if (ercd != osOK) {
		console_printf("can_drv_get_status failed\n");
	}
	
	return ercd;
}

// コマンド
static void can_drv_cmd_open(int argc, char *argv[])
{
	uint8_t ch;
	uint32_t ret;
	
	// 引数チェック
	if (argc < 2) {
		console_printf("can_drv open <ch>\n");
		return;
	}
	
	// 値設定
	ch = atoi(argv[2]);
	
	// オープン
	ret = can_drv_open(ch);
	if (ret != osOK) {
		console_printf("open error\n");
		goto EXIT;
	}
	
	console_printf("open success\n");
	
EXIT:
	return;
	
}

static void can_drv_cmd_start(int argc, char *argv[])
{
	uint8_t ch;
	uint32_t ret;
	
	// 引数チェック
	if (argc < 2) {
		console_printf("can_drv open <ch>\n");
		return;
	}
	
	// 値設定
	ch = atoi(argv[2]);
	
	// オープン
	ret = can_drv_start(ch);
	if (ret != osOK) {
		console_printf("start error\n");
		goto EXIT;
	}
	
	console_printf("start success\n");
	
EXIT:
	return;
	
}

static void can_drv_cmd_send(int argc, char *argv[])
{
	uint32_t ret;
	uint8_t ch;
	uint8_t mbx_id;
	uint32_t can_id;
	uint8_t i;
	uint8_t data[8];
	
	// 引数チェック
	if (argc < 4) {
		console_printf("can_drv send <ch> <mbx_id> <can_id>\n");
		return;
	}
	
	// 値設定
	ch = atoi(argv[2]);
	mbx_id = atoi(argv[3]);
	can_id = atoi(argv[4]);
	for (i = 0; i < 8; i++) {
		data[i] = i;
	}
	
	// 送信
	ret = can_drv_send(ch, mbx_id, CAN_COMMON_FRAME_TYPE_STANDARD, can_id, data, 8);
	if (ret != osOK) {
		console_printf("send error\n");
		goto EXIT;
	}
	
	console_printf("send success\n");
	
EXIT:
	return;
	
}

// コマンド設定関数
void can_drv_set_cmd(void)
{
	COMMAND_INFO cmd;
	
	// コマンドの設定
	cmd.input = "can_drv open";
	cmd.func = can_drv_cmd_open;
	console_set_command(&cmd);
	cmd.input = "can_drv start";
	cmd.func = can_drv_cmd_start;
	console_set_command(&cmd);
	cmd.input = "can_drv send";
	cmd.func = can_drv_cmd_send;
	console_set_command(&cmd);
}
