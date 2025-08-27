/*
 * can_mng.c
 *
 *  Created on: 2025/7/22
 *      Author: ronald
 */
#include <string.h>
#include <stdlib.h>
#include "stm32f7xx.h"
#include "cmsis_os.h"
#include "console.h"
#include "can_common.h"
#include "can_drv.h"

#include "can_mng.h"

// 状態
#define ST_UNINITIALIZED	(0)		// 未初期化
#define ST_INITIALIZED		(1)		// 初期化
#define ST_ACTIVE			(2)		// アクティブ
#define ST_MAX				(3)

// チャネル状態
#define CH_ST_TXERR_BMP		(1UL << 0)
#define CH_ST_RXERR_BMP		(1UL << 1)
#define CH_ST_BUSOFF_BMP	(1UL << 2)

// マクロ
#define CTRL_PERIOD			(100)	// 制御する周期
#define RX_PERIOD_CHECK	

// CANID情報
// 送信
#define SEND_CANID_0x100	(0x100)
#define SEND_CANID_0x101	(0x101)
#define SEND_CANID_0x102	(0x102)
#define SEND_CANID_0x103	(0x103)
#define SEND_CANID_0x104	(0x104)
#define SEND_CANID_0x105	(0x105)
#define SEND_CANID_0x106	(0x106)
#define SEND_CANID_0x107	(0x107)
#define SEND_CANID_0x108	(0x108)
#define SEND_CANID_0x109	(0x109)
// 受信
#define RECV_CANID_0x300	(0x300)
#define RECV_CANID_0x400	(0x400)
#define RECV_CANID_0x500	(0x500)
#define RECV_CANID_0x600	(0x600)
#define RECV_CANID_0x700	(0x700)

// イベント
#define PERIODIC_EVENT		(0x01)
#define RECEIVED_EVENT		(0x02)
#define ACTIVE_EVENT		(0x04)
#define INACTIVE_EVENT		(0x08)
#define SLEEP_EVENT			(0x10)
#define ALL_EVENT			(PERIODIC_EVENT|RECEIVED_EVENT|ACTIVE_EVENT|INACTIVE_EVENT|SLEEP_EVENT)

// スリープ完了イベント
#define SLEEP_SET_OK		(0x01)
#define SLEEP_SET_NG		(0x02)

// イベント関数
typedef void (*EVENT_FUNC)(void);

// 送信用バッファ
uint8_t send_frame_1_buff[8];
uint8_t send_frame_2_buff[8];
uint8_t send_frame_3_buff[8];
uint8_t send_frame_4_buff[8];
uint8_t send_frame_5_buff[8];
uint8_t send_frame_6_buff[8];
uint8_t send_frame_7_buff[8];
uint8_t send_frame_8_buff[8];
uint8_t send_frame_9_buff[8];
uint8_t send_frame_10_buff[8];
// 受信バッファ
uint8_t recv_frame_1_buff[8];
uint8_t recv_frame_2_buff[8];
uint8_t recv_frame_3_buff[8];
uint8_t recv_frame_4_buff[8];
uint8_t recv_frame_5_buff[8];

// 送信フレーム情報
typedef struct {
	CAN_DRV_CH					ch;			// チャネル
	uint32_t					mbx_id;		// メールボックスID
	CAN_COMMON_FRAME_TYPE		frame_type;	// フレームタイプ
	uint32_t					can_id;		// CAN ID
	uint32_t					period;		// 周期
	uint8_t						size;		// サイズ
	uint8_t						*p_data;	// 送信バッファポインタ
} SEND_FRAME_INFO;
static const SEND_FRAME_INFO send_frame_info_tbl[CAN_MNG_SEND_FRAME_MAX] = {
//	ch				mbx_id					frame_type						can_id				
	{CAN_DRV_CH_2,	CAN_COMMON_AUTO_MBX_ID,	CAN_COMMON_FRAME_TYPE_STANDARD,	SEND_CANID_0x100,	100/CTRL_PERIOD,	8,	send_frame_1_buff},		// CAN_MNG_INFO_SEND_FRAME_1
	{CAN_DRV_CH_2,	CAN_COMMON_AUTO_MBX_ID,	CAN_COMMON_FRAME_TYPE_STANDARD,	SEND_CANID_0x101,	200/CTRL_PERIOD,	8,	send_frame_2_buff},		// CAN_MNG_INFO_SEND_FRAME_2
	{CAN_DRV_CH_2,	CAN_COMMON_AUTO_MBX_ID,	CAN_COMMON_FRAME_TYPE_STANDARD,	SEND_CANID_0x102,	300/CTRL_PERIOD,	8,	send_frame_3_buff},		// CAN_MNG_INFO_SEND_FRAME_3
	{CAN_DRV_CH_2,	CAN_COMMON_AUTO_MBX_ID,	CAN_COMMON_FRAME_TYPE_STANDARD,	SEND_CANID_0x103,	400/CTRL_PERIOD,	8,	send_frame_4_buff},		// CAN_MNG_INFO_SEND_FRAME_4
	{CAN_DRV_CH_2,	CAN_COMMON_AUTO_MBX_ID,	CAN_COMMON_FRAME_TYPE_STANDARD,	SEND_CANID_0x104,	500/CTRL_PERIOD,	8,	send_frame_5_buff},		// CAN_MNG_INFO_SEND_FRAME_5
	{CAN_DRV_CH_2,	CAN_COMMON_AUTO_MBX_ID,	CAN_COMMON_FRAME_TYPE_STANDARD,	SEND_CANID_0x105,	600/CTRL_PERIOD,	8,	send_frame_6_buff},		// CAN_MNG_INFO_SEND_FRAME_6
	{CAN_DRV_CH_2,	CAN_COMMON_AUTO_MBX_ID,	CAN_COMMON_FRAME_TYPE_STANDARD,	SEND_CANID_0x106,	700/CTRL_PERIOD,	8,	send_frame_7_buff},		// CAN_MNG_INFO_SEND_FRAME_7
	{CAN_DRV_CH_2,	CAN_COMMON_AUTO_MBX_ID,	CAN_COMMON_FRAME_TYPE_STANDARD,	SEND_CANID_0x107,	800/CTRL_PERIOD,	8,	send_frame_8_buff},		// CAN_MNG_INFO_SEND_FRAME_8
	{CAN_DRV_CH_2,	CAN_COMMON_AUTO_MBX_ID,	CAN_COMMON_FRAME_TYPE_STANDARD,	SEND_CANID_0x108,	900/CTRL_PERIOD,	8,	send_frame_9_buff},		// CAN_MNG_INFO_SEND_FRAME_9
	{CAN_DRV_CH_2,	CAN_COMMON_AUTO_MBX_ID,	CAN_COMMON_FRAME_TYPE_STANDARD,	SEND_CANID_0x109,	1000/CTRL_PERIOD,	8,	send_frame_10_buff},	// CAN_MNG_INFO_SEND_FRAME_10
};

// 受信フレーム情報
typedef struct {
	CAN_DRV_CH					ch;			// チャネル
	uint32_t					mbx_id;		// メールボックスID
	CAN_COMMON_FRAME_TYPE		frame_type;	// フレームタイプ
	uint32_t					can_id;		// CAN ID
	uint8_t						size;		// サイズ
	uint8_t						*p_data;	// 受信バッファポインタ
} RECV_FRAME_INFO;
static const RECV_FRAME_INFO recv_frame_info_tbl[CAN_MNG_RECV_FRAME_MAX] = {
//	ch				mbx_id						frame_type						can_id				
	{CAN_DRV_CH_2,	CAN_DRV_CH_2_RX_MAILBOX_0, CAN_COMMON_FRAME_TYPE_STANDARD,	RECV_CANID_0x300, 8, recv_frame_1_buff},		// CAN_MNG_RECV_FRAME_1
	{CAN_DRV_CH_2,	CAN_DRV_CH_2_RX_MAILBOX_0, CAN_COMMON_FRAME_TYPE_STANDARD,	RECV_CANID_0x400, 8, recv_frame_2_buff},		// CAN_MNG_RECV_FRAME_2
	{CAN_DRV_CH_2,	CAN_DRV_CH_2_RX_MAILBOX_1, CAN_COMMON_FRAME_TYPE_STANDARD,	RECV_CANID_0x500, 8, recv_frame_3_buff},		// CAN_MNG_RECV_FRAME_3
	{CAN_DRV_CH_2,	CAN_DRV_CH_2_RX_MAILBOX_1, CAN_COMMON_FRAME_TYPE_STANDARD,	RECV_CANID_0x600, 8, recv_frame_4_buff},		// CAN_MNG_RECV_FRAME_4
	{CAN_DRV_CH_2,	CAN_DRV_CH_2_RX_MAILBOX_1, CAN_COMMON_FRAME_TYPE_STANDARD,	RECV_CANID_0x700, 8, recv_frame_5_buff},		// CAN_MNG_RECV_FRAME_5
};

// メールボックス情報 受信 CH1
static const uint32_t filter0_ch1[] =
{
	FILTER_LAST,
};
static const uint32_t filter1_ch1[] =
{
	FILTER_LAST,
};

static const CAN_COMMON_RX_MAILBOX_INFO rx_mailbox_info_ch1[CAN_DRV_CH_1_RX_MAILBOX_MAX] =
{	//mbx_id						frame_type							mask			filter
	{CAN_DRV_CH_1_RX_MAILBOX_0,		CAN_COMMON_FRAME_TYPE_STANDARD,		0x00000000,		filter0_ch1},
	{CAN_DRV_CH_1_RX_MAILBOX_1,		CAN_COMMON_FRAME_TYPE_STANDARD,		0x00000000,		filter1_ch1},
};

// メールボックス情報 受信 CH2
static const uint32_t filter0_ch2[] =
{
	RECV_CANID_0x300,
	RECV_CANID_0x400,
	FILTER_LAST,
};
static const uint32_t filter1_ch2[] =
{
	RECV_CANID_0x500,
	RECV_CANID_0x600,
	RECV_CANID_0x700,
	FILTER_LAST,
};
static const CAN_COMMON_RX_MAILBOX_INFO rx_mailbox_info_ch2[CAN_DRV_CH_2_RX_MAILBOX_MAX] =
{	//mbx_id						frame_type							mask			filter
	{CAN_DRV_CH_2_RX_MAILBOX_0,		CAN_COMMON_FRAME_TYPE_STANDARD,		0x000007FF,		filter0_ch2},
	{CAN_DRV_CH_2_RX_MAILBOX_1,		CAN_COMMON_FRAME_TYPE_STANDARD,		0x000007FF,		filter1_ch2},
};

// スリープ情報
typedef struct {
	uint8_t is_sleep;
} SLEEP_INFO;
static const SLEEP_INFO sleep_ch_tbl[CAN_DRV_CH_MAX] = {
	{0},	// CAN_DRV_CH_1
	{1},	// CAN_DRV_CH_2
};

// メールボックステーブル
typedef struct {
	uint8_t								mbox_num;
	const CAN_COMMON_RX_MAILBOX_INFO	*p_mbox_info;
} CH_MBOX_INFO;
static const CH_MBOX_INFO ch_mbox_info[CAN_DRV_CH_MAX] = {
	{CAN_DRV_CH_1_RX_MAILBOX_MAX,	rx_mailbox_info_ch1},
	{CAN_DRV_CH_2_RX_MAILBOX_MAX,	rx_mailbox_info_ch2},
};

// 制御用ブロック
typedef struct {
	uint32_t				state;									// 状態
	uint32_t				ch_state[CAN_DRV_CH_MAX];				// チャネルの状態
	osThreadId				CanMngHandle;							// タスクID
	uint32_t				t[CAN_MNG_SEND_FRAME_MAX];				// タイマ
	CAN_MNG_RECV_CALLBACK	rcv_callback[CAN_MNG_RECV_FRAME_MAX];	// 受信コールバック
	osThreadId				sleep_req_tsk_id;						// スリープ要求タスクID
} CAN_MNG_CTL;
static CAN_MNG_CTL can_mng_ctl;
#define get_myself() (&can_mng_ctl)
static void callback_timer( void const *arg);
osTimerDef(Timer, callback_timer);

// コールバック
static void callback_timer( void const *arg)
{
	CAN_MNG_CTL *this = get_myself();
	
	// イベントセット
	osSignalSet(this->CanMngHandle, PERIODIC_EVENT);
}

// メールボックス 受信処理
static osStatus mbox_rx_proc(CAN_DRV_CH ch, uint32_t ret)
{
	CAN_MNG_CTL *this = &can_mng_ctl;
	const CH_MBOX_INFO *p_ch_info;
	const RECV_FRAME_INFO *p_frame_info;
	uint8_t mbox_idx;
	uint32_t can_id;
	osStatus ercd;
	uint8_t data[8];
	uint8_t size;
	uint8_t i;
	
	p_ch_info = &ch_mbox_info[ch];
	// 受信したメールボックスからデータを取得
	for (mbox_idx = 0; mbox_idx < p_ch_info->mbox_num; mbox_idx++) {
		// 受信あり
		if ((ret & (1UL << mbox_idx)) != 0) {
			if ((ercd = can_drv_recv(ch, mbox_idx, &can_id, data, &size)) != osOK) {
				continue;
			}
			// コールバック処理
			for (i = 0; i < CAN_MNG_RECV_FRAME_MAX; i++) {
				// フレーム情報取得
				p_frame_info = &recv_frame_info_tbl[i];
				// コールバック呼び出し
				if ((can_id == p_frame_info->can_id) && (this->rcv_callback[i] != NULL)) {
					// コピー
					memcpy(p_frame_info->p_data, data, p_frame_info->size);
					// 呼び出し
					this->rcv_callback[i](p_frame_info->p_data, p_frame_info->size);
					break;
				}
			}
		}
	}
	
	return ercd;
}

// 受信処理
static void recv_func(void)
{
	osStatus ercd;
	uint32_t ret = 0;
	CAN_DRV_CH ch;
	
	// 受信チェック
	for (ch = 0; ch < CAN_DRV_CH_MAX; ch++) {
		// 受信チェック
		ercd = can_drv_rx_check(ch, &ret);
		if ((ercd != osOK) || (ret == 0)) {
			continue;
		}
		// メールボックス受信処理
		ercd = mbox_rx_proc(ch, ret);
		if (ercd != osOK) {
			continue;
		}
	}
}

// 送信処理
static void send_func(void)
{
	CAN_MNG_CTL *this = &can_mng_ctl;
	const SEND_FRAME_INFO *p_s_info;
	uint8_t send_frame[8];
	osStatus ercd;
	uint8_t i;
	
	// 全送信フレームチェック
	for (i = 0; i < CAN_MNG_SEND_FRAME_MAX; i++) {
		// 送信情報取得
		p_s_info = &send_frame_info_tbl[i];
		// バスオフなら送信しない
		if ((this->ch_state[p_s_info->ch] & CH_ST_BUSOFF_BMP) != 0) {
			continue;
		}
		// 送信周期になった
		if (this->t[i]++ == p_s_info->period) {
			// いったん送信データを取得
			__disable_irq();
			memcpy(send_frame, p_s_info->p_data, sizeof(send_frame));
			__enable_irq();
			// 送信
			ercd = can_drv_send(p_s_info->ch, p_s_info->mbx_id, p_s_info->frame_type, p_s_info->can_id, send_frame, sizeof(send_frame));
			if (ercd != osOK) {
				console_printf("send failed\n");
			}
			// クリア
			this->t[i] = 0;
		}
	}
}

// 状態更新処理
static void sts_update(void)
{
	CAN_MNG_CTL *this = &can_mng_ctl;
	CAN_DRV_CH ch;
	osStatus ercd;
	uint32_t sts = 0;
	
	// 全チャネルチェック
	for (ch = 0; ch < CAN_DRV_CH_MAX; ch++) {
		// 状態取得
		ercd = can_drv_get_status(ch, &sts);
		if (ercd != osOK) {
			continue;
		}
		// とりあえずバスオフチェックのみチェック
		// バスオフ発生
		if (((sts & CAN_COMMON_STS_RXOVER) != 0) && ((this->ch_state[ch] & CH_ST_RXERR_BMP) == 0)) {
			console_printf("busoff occur\n");
			this->ch_state[ch] |= CH_ST_BUSOFF_BMP;
			
		// バスオフ復帰
		} else if (((sts & CAN_COMMON_STS_RXOVER) == 0) && ((this->ch_state[ch] & CH_ST_RXERR_BMP) != 0)) {
			console_printf("busoff recovery\n");
			this->ch_state[ch] &= ~CH_ST_BUSOFF_BMP;
			
		// その他
		} else {
			;
		}
	}
}

// 周期処理
static void periodic_func(void)
{
	CAN_MNG_CTL *this = get_myself();
	
	// アクティブでないならエラー返して終了
	if (this->state != ST_ACTIVE) {
		return;
	}
	
	// 状態更新処理
	sts_update();
	
#ifdef RX_PERIOD_CHECK
	// 周期受信処理
	recv_func();
#endif
	
	// 送信処理
	send_func();
}

// アクティブ
// [メモ]
// CH毎にopenするということはあまりないのでactiveとする
// activeでデバイス、ドライバをすべてopenする
// また、メールボックスの設定もする
static void active_func(void)
{
	CAN_MNG_CTL *this = get_myself();
	const CAN_COMMON_RX_MAILBOX_INFO *p_info;
	const CH_MBOX_INFO *p_ch_info;
	osStatus ercd;
	CAN_DRV_CH ch;
	uint32_t i;
	
	// 初期化してなかったらエラー返して終了
	if (this->state != ST_INITIALIZED) {
		return;
	}
	
	// 全ch
	for (ch = 0; ch < CAN_DRV_CH_MAX; ch++) {
		// オープン
		if ((ercd = can_drv_open(ch)) != osOK) {
			goto EXIT;
		}
		// 受信メールボックス設定
		p_ch_info = &ch_mbox_info[ch];
		for (i = 0; i < p_ch_info->mbox_num; i++) {
			// 受信メールボックス情報取得
			p_info = &(p_ch_info->p_mbox_info[i]);
			if ((ercd = can_drv_set_mailbox(ch, p_info)) != osOK) {
				goto EXIT;
			}
		}
		// 開始
		if ((ercd = can_drv_start(ch)) != osOK) {
			goto EXIT;
		}
	}
	
	// 状態更新
	this->state = ST_ACTIVE;
	
EXIT:
	return;
}

// インアクティブ
static void inactive_func(void)
{
	CAN_MNG_CTL *this = get_myself();
	osStatus ercd;
	CAN_DRV_CH ch;
	
	// アクティブでないならエラー返して終了
	if (this->state != ST_ACTIVE) {
		return;
	}
	
	// 全ch
	for (ch = 0; ch < CAN_DRV_CH_MAX; ch++) {
		// 開始
		if ((ercd = can_drv_stop(ch)) != osOK) {
			goto EXIT;
		}
	}
	
	// 状態更新
	this->state = ST_INITIALIZED;
	
EXIT:
	return;
}

// スリープ
static void sleep_func(void)
{
	CAN_MNG_CTL *this = get_myself();
	osStatus ercd;
	CAN_DRV_CH ch;
	const SLEEP_INFO *p_info;
	
	// アクティブでないならエラー返して終了
	if (this->state != ST_ACTIVE) {
		osSignalSet(this->sleep_req_tsk_id, SLEEP_SET_NG);
	}
	
	// 全ch
	for (ch = 0; ch < CAN_DRV_CH_MAX; ch++) {
		// スリープ情報取得
		p_info = &sleep_ch_tbl[ch];
		// スリープ対象でなければ次へ
		if (p_info->is_sleep == 0) {
			continue;
		}
		// スリープ
		if ((ercd = can_drv_sleep(ch)) != osOK) {
			goto EXIT;
		}
	}
	
	// 状態更新
	this->state = ST_INITIALIZED;
	
	osSignalSet(this->sleep_req_tsk_id, SLEEP_SET_OK);
	return;
	
EXIT:
	osSignalSet(this->sleep_req_tsk_id, SLEEP_SET_NG);
	return;
}

// イベント処理
static const EVENT_FUNC event_func[] = {
	periodic_func,	// PERIODIC_EVENT
	recv_func,		// RECEIVED_EVENT
	active_func,	// ACTIVE_EVENT
	inactive_func,	// INACTIVE_EVENT
	sleep_func,		// SLEEP_EVENT
};

/* USER CODE BEGIN Header_StartCanTask */
/**
* @brief Function implementing the CanTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCanTask */
static void CanMngTask(void const * argument)
{
	osEvent evt;
	uint8_t evt_idx;
	EVENT_FUNC evt_func;
	
	/* USER CODE BEGIN StartCanTask */
	/* Infinite loop */
	for(;;) {
		// イベント待機
		evt = osSignalWait(ALL_EVENT, osWaitForever);
		// イベント受信
		if (evt.status == osEventSignal) {
			// クリア
			//osSignalClear(this->CanMngHandle, evt.value.signals);
			// イベント関数テーブルのインデックスに変換
			for (evt_idx = 0; evt_idx < 32; evt_idx++) {
				if (((evt.value.signals) & (1UL << evt_idx)) != 0) {
					// イベント処理実行
					evt_func = event_func[evt_idx];
					evt_func();
				}
			}
		}
	}
	/* USER CODE END StartCanTask */
}

// 初期化
void can_mng_init(void)
{
	CAN_MNG_CTL *this = get_myself();
	osTimerId id;
	
	// コンテキスト初期化
	memset(this, 0, sizeof(CAN_MNG_CTL));

	// タスク生成
	osThreadDef(CanMng, CanMngTask, osPriorityLow, 0, 1024);
	this->CanMngHandle = osThreadCreate(osThread(CanMng), NULL);

	// タイマ作成
	id = osTimerCreate(osTimer(Timer), osTimerPeriodic, NULL);

	// タイマ開始
	osTimerStart(id, CTRL_PERIOD);

	// 状態更新
	this->state = ST_INITIALIZED;
	
}

// アクティブ
// [メモ]
// CH毎にopenするということはあまりないのでactiveとする
// activeでデバイス、ドライバをすべてopenする
// また、メールボックスの設定もする
osStatus can_mng_active(void)
{
	CAN_MNG_CTL *this = get_myself();
	
	// イベントセット
	return osSignalSet(this->CanMngHandle, ACTIVE_EVENT);
}

// インアクティブ
osStatus can_mng_inactive(void)
{
	CAN_MNG_CTL *this = get_myself();
	
	// イベントセット
	return osSignalSet(this->CanMngHandle, INACTIVE_EVENT);
}

// スリープ
// 一つしかスリープ要求タスクIDを覚えられないため、複数個所から呼ばないこと
osStatus can_mng_sleep(void)
{
	CAN_MNG_CTL *this = get_myself();
	osStatus ercd = osErrorParameter;
	osEvent evt;
	
		// タスクID取得
	this->sleep_req_tsk_id = osThreadGetId();
	
	// イベントセット
	osSignalSet(this->CanMngHandle, SLEEP_EVENT);
	
	// スリープ設定待ち
	evt = osSignalWait(SLEEP_SET_OK|SLEEP_SET_NG, osWaitForever);
	if (evt.status == osEventSignal) {
		// 設定完了
		if ((evt.value.signals & SLEEP_SET_OK) != 0) {
			ercd = osOK;
		}
	}
	
	return ercd;
}

// 受信フレームのデータを取得
// 割込み禁止して呼ぶこと
uint8_t *can_mng_get_recv_data(uint32_t frame_type)
{
	const RECV_FRAME_INFO *p_info;

	// パラメータチェック
	if (frame_type >= CAN_MNG_RECV_FRAME_MAX) {
		return NULL;
	}

	// 受信フレーム取得
	p_info = &recv_frame_info_tbl[frame_type];

	return p_info->p_data;
}

// 送信フレームのデータを取得
// 割込み禁止して呼ぶこと
uint8_t *can_mng_get_send_data(uint32_t frame_type)
{
	const SEND_FRAME_INFO *p_info;

	// パラメータチェック
	if (frame_type >= CAN_MNG_RECV_FRAME_MAX) {
		return NULL;
	}

	// 受信フレーム取得
	p_info = &send_frame_info_tbl[frame_type];

	return p_info->p_data;
}

// 受信コールバック関数登録
osStatus can_mng_reg_recv_callback(uint32_t frame_type, CAN_MNG_RECV_CALLBACK callback)
{
	CAN_MNG_CTL *this = &can_mng_ctl;

	// パラメータチェック
	if ((frame_type >= CAN_MNG_RECV_FRAME_MAX) ||
		(callback == NULL)) {
		return osErrorParameter;
	}

	// コールバック登録
	this->rcv_callback[frame_type] = callback;

	return osOK;
}