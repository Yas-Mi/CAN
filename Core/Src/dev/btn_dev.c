/*
 * btn_dev.c
 *
 *  Created on: 2022/10/29
 *      Author: User
 */
#include <string.h>
#include <stdlib.h>
#include "stm32f7xx.h"
#include "cmsis_os.h"
#include "console.h"

#include "btn_dev.h"

// ボタン状態
#define PUSHED			(0U)	// ボタンが離された
#define RELEASED		(1U)	// ボタンが押された
#define SHORT_PUSHED	(2U)	// 短押し
#define LONG_PUSHED		(3U)	// 長押し

// 短押し/長押し時間
#define BTN_SHORT_PUSHED_TIME (1500U)	// 短押し時間 (*)ボタンを押してから話すまでの時間が1000ms以下の場合、短押しと判定
#define BTN_LONG_PUSHED_TIME  (2500U)	// 長押し時間 (*)ボタンを押してからが2000ms以上経過した場合、長押しと判定

// ボタン状態を確認する周期
#define BTN_CHECK_PERIOD (100U)		// 100ms周期でボタン状態をチェック

// イベント
#define PERIODIC_EVENT		(0x01)

// イベント関数
typedef void (*EVENT_FUNC)(void);

// ボタンの状態管理ブロック
typedef struct {
	uint8_t  pre_status; 	// ボタンの前状態
	uint8_t  cur_status; 	// ボタンの状態
	uint32_t counter;		// ボタンが押された時間[ms] (= counter * BTN_CHECK_PERIOD)
	BTN_CALLBACK cb;		// コールバック関数
} BTN_INFO;

// 制御用ブロック
typedef struct {
	osThreadId		tsk_id;				// タスクID
	BTN_INFO		info[BTN_MAX];		// 各ボタンの情報
} BTN_CTL;
static BTN_CTL btn_ctl;
#define get_myself() (&btn_ctl)
static void callback_timer( void const *arg);
osTimerDef(btnTimer, callback_timer);

// ボタン固有情報
typedef struct {
	GPIO_TypeDef *gpio_grp;		// ボタンに割り当てられているGPIOグループ
	uint16_t     pin_no;		// GPIOのピン番号
} BTN_CFG;

// ボタン固有情報テーブル
static const BTN_CFG btn_info_tbl[BTN_MAX] = {
//   gpio_grp   pin_no
	{GPIOA,		GPIO_PIN_0},	// 上ボタンのGPIO情報
};

// コールバック
static void callback_timer(void const *arg)
{
	BTN_CTL *this = get_myself();

	// イベントセット
	osSignalSet(this->tsk_id, PERIODIC_EVENT);
}

// ボタンの状態を確認する関数
static void btn_dev_check(void)
{
	GPIO_PinState sts;
	GPIO_TypeDef  *gpio_grp;
	BTN_CTL		  *this = &btn_ctl;
	BTN_INFO	  *info;
	uint32_t	  pushed_time;
	uint16_t	  pin_no;
	uint8_t		  i;
	
	for (i = 0; i < BTN_MAX; i++) {
		// ボタンの情報を取得
		info = &(this->info[i]);
		gpio_grp = btn_info_tbl[i].gpio_grp;
		pin_no = btn_info_tbl[i].pin_no;
		// ボタンの状態を取得
		sts = HAL_GPIO_ReadPin(gpio_grp, pin_no);
		// ボタンが押された?
		if (sts == GPIO_PIN_SET) {
			// 長押し確定中は何もしない
			if (info->cur_status == LONG_PUSHED) {
				continue;
			}
			// 現状態を更新
			info->cur_status = PUSHED;
			// カウンタを進める
			info->counter++;
			// カウンタから時間[ms]に変換
			pushed_time = info->counter * BTN_CHECK_PERIOD;
			// 長押し判定
			if(pushed_time > BTN_LONG_PUSHED_TIME) {
				// 現状態を更新
				info->cur_status = LONG_PUSHED;
				// コールバック実行
				if (info->cb) {
					info->cb(BTN_LONG_PUSHED);
				}
				console_printf("long\n");
			} else {
				; // ボタン押し込み中
			}
		// ボタンが押されていない？
		} else {
			// 現状態を更新
			info->cur_status = RELEASED;
			// そもそもボタンが押されていた？ (長押し確定前)
			if (info->pre_status == PUSHED) {
				// カウンタから時間[ms]に変換
				pushed_time = info->counter * BTN_CHECK_PERIOD;
				// カウンタをクリア
				info->counter = 0;
				// 短押し判定
				if (pushed_time < BTN_SHORT_PUSHED_TIME) {
					// コールバック実行
					if (info->cb) {
						info->cb(BTN_SHORT_PUSHED);
					}
					console_printf("short\n");
				} else {
					;
				}
			// 長押し確定後にボタンがはなされた？
			} else if (info->pre_status == LONG_PUSHED) {
				// カウンタをクリア
				info->counter = 0;
			} else {
				// 特に何もしない
				; 
			}
		}
		// 前状態を更新
		info->pre_status = info->cur_status;
	}
	
	return;
}

// イベント処理
static const EVENT_FUNC event_func[] = {
	btn_dev_check,	// PERIODIC_EVENT
};

// 周期タスク
void BtnTask(void const * argument)
{
	osEvent evt;
	uint8_t evt_idx;
	EVENT_FUNC evt_func;
	
	/* Infinite loop */
	for(;;) {
		// イベント待機
		evt = osSignalWait(PERIODIC_EVENT, osWaitForever);
		// イベント受信
		if (evt.status == osEventSignal) {
			// イベント関数テーブルのインデックスに変換
			for (evt_idx = 0; evt_idx < 32; evt_idx++) {
				if (((evt.value.signals) & (1UL << evt_idx)) != 0) {
					break;
				}
			}
			if (evt_idx >= 32) {
				continue;
			}
			// イベント処理実行
			evt_func = event_func[evt_idx];
			evt_func();
		}
	}
}

// 内部関数
// ボタン初期化関数
void btn_dev_init(void)
{
	BTN_CTL *this = &btn_ctl;
	osTimerId id;
	uint8_t i;
	
	// 制御ブロックの初期化
	memset(this, 0, sizeof(BTN_CTL));
	
	// ボタンの状態をRELEASEDに設定
	for (i = 0; i < BTN_MAX; i++) {
		this->info[i].pre_status = RELEASED;
		this->info[i].cur_status = RELEASED;
	}
	
	// 自タスクのIDを制御ブロックに設定
	osThreadDef(Btn, BtnTask, osPriorityIdle, 0, 1024);
	this->tsk_id = osThreadCreate(osThread(Btn), NULL);
	
	// タイマ作成
	id = osTimerCreate(osTimer(btnTimer), osTimerPeriodic, NULL);
	
	// タイマ開始
	osTimerStart(id, BTN_CHECK_PERIOD);
	
	return;
}

// 上ボタンコールバック登録関数
osStatus BTN_dev_reg_callback(BTN_TYPE type, BTN_CALLBACK cb)
{
	BTN_CTL *this = get_myself();
	
	// コールバックがNULLの場合エラーを返す
	if ((type >= BTN_MAX)||(cb == NULL)) {
		return -1;
	}
	
	// コールバックを登録
	this->info[type].cb = cb;
	
	return 0;
}
