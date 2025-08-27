/*
 * standby_app.c
 *
 *  Created on: Aug 14, 2025
 *      Author: User
 */
#include <string.h>
#include <stdlib.h>
#include "stm32f7xx.h"
#include "cmsis_os.h"
#include "console.h"
#include "btn_dev.h"
#include "can_mng.h"

#include "standby_app.h"

// 状態
#define ST_UNINITIALIZED		(0)		// 未初期化
#define ST_INITIALIZED			(1)		// 初期化
#define ST_GO_STANDBY_PREPARE	(2)		// スタンバイ移行準備
#define ST_MAX					(3)

// イベント
#define GO_STANDBY_EVENT	(0x01)

// 関数ポインタ
typedef void (*EVENT_FUNC)(void);

// 制御用ブロック
typedef struct {
	uint32_t				state;									// 状態
	uint32_t				standby_prepare_bit;					// スタンバイ移行ビット
	osThreadId				StandbyHandle;							// タスクID
} STANDBY_APP_CTL;
static STANDBY_APP_CTL standby_app_ctl;
#define get_myself() (&standby_app_ctl)

// ボタンコールバック
void user_btn_callback(BTN_STATUS sts)
{
	STANDBY_APP_CTL *this = get_myself();
	
	// 短押しの場合
	if (sts == BTN_SHORT_PUSHED) {
		// イベントセット
		osSignalSet(this->StandbyHandle, GO_STANDBY_EVENT);
	}
}

// スタンバイ移行処理
static void go_standby(void)
{
	osStatus ercd;
	
	// スリープ
	if ((ercd = can_mng_sleep()) != osOK) {
		goto EXIT;
	}
	
	//Clear the WU FLAG
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	
	//Enable the WAKEUP PIN(PA0)
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
	//HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2);
	
	console_printf("go standby\n");
	
	// 文字列表示のため500ms待機
	osDelay(500);
	
	//Enter the standby mode
	HAL_PWR_EnterSTANDBYMode();
	
	// スタンバイ復帰後はリセットと同じ
	
EXIT:
	return;
}

// イベント処理
static const EVENT_FUNC event_func[] = {
	go_standby,	// GO_STANDBY_EVENT
};

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StandbyTask(void const * argument)
{
	osEvent evt;
	uint8_t evt_idx;
	EVENT_FUNC evt_func;
	/* USER CODE BEGIN 5 */
	/* Infinite loop */
	for(;;) {
		// イベント待機
		evt = osSignalWait(GO_STANDBY_EVENT, osWaitForever);
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
	/* USER CODE END 5 */
}

void standby_app_init(void)
{
	STANDBY_APP_CTL *this = get_myself();
	
	// コンテキスト初期化
	memset(this, 0, sizeof(STANDBY_APP_CTL));
	
	/* definition and creation of Standby */
	osThreadDef(Standby, StandbyTask, osPriorityLow, 0, 512);
	this->StandbyHandle = osThreadCreate(osThread(Standby), NULL);
	
	// コールバック登録
	BTN_dev_reg_callback(BTN_TYPE_USER_BTN, user_btn_callback);
	
	// 状態更新
	this->state = ST_INITIALIZED;
}
