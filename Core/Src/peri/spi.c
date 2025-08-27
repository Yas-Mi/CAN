/*
 * spi.c
 *
 *  Created on: 2024/10/09
 *      Author: user
 */
#include <string.h>
#include "stm32f7xx.h"
#include "stm32f7xx_hal_rcc.h"
#include "cmsis_os.h"
#include "iodefine.h"
#include "spi.h"

// 状態定義
#define ST_INIT		(0)		// 初期状態
#define ST_CLOSE	(1)		// クローズ状態
#define ST_OPEN		(2)		// オープン状態
#define ST_MAX		(3)		// 最大値

// イベント
#define SEND_RESULT_OK	(0x00000001)
#define SEND_RESULT_NG	(0x00000002)

// マクロ
#define SLEEP_TIME	(100)	// スリープ時間[ms]

// 制御ブロック
typedef struct {
	uint32_t		status;			// 状態
	osThreadId		id;				// 送受信を行っているタスクID
	uint8_t			*p_snd_data;	// 送信データポインタ
	uint8_t			*p_rcv_data;	// 受信データポインタ
	uint32_t		snd_sz;			// 送信サイズ
	uint32_t		rcv_sz;			// 受信サイズ
} SPI_CB;
static SPI_CB spi_cb[SPI_CH_MAX];
#define get_myself(ch) (&spi_cb[ch])

// チャネル情報
typedef struct {
	GPIO_TypeDef*	p_gpio_grp;			// グループ
	uint16_t 		pin;				// GPIOピン
} NSS_GPIO_INFO;
typedef struct {
	SPI_TypeDef		*p_reg;			// ベースアドレス
	IRQn_Type		irqn;			// 割り込み番号
	uint32_t		priority;		// 割り込み優先度
	NSS_GPIO_INFO	nss_gpio_info;	// NSSのGPIO情報
} CH_INFO;
static const CH_INFO ch_info_tbl[SPI_CH_MAX] = {
//	{SPI1,	SPI1_IRQn,	5,	{GPIOB, GPIO_PIN_13}},
	{SPI2,	SPI2_IRQn,	5,	{GPIOA, GPIO_PIN_11}},
//	{SPI3,	SPI3_IRQn,	5,	{GPIOB, GPIO_PIN_15}},
//	{SPI4,	SPI4_IRQn,	5,	{GPIOB, GPIO_PIN_15}},
};
#define get_reg(ch)		(ch_info_tbl[ch].p_reg)
#define get_irqn(ch)	(ch_info_tbl[ch].irqn)
#define get_pri(ch)		(ch_info_tbl[ch].priority)
#define assert_nss(ch)	HAL_GPIO_WritePin(ch_info_tbl[ch].nss_gpio_info.p_gpio_grp,ch_info_tbl[ch].nss_gpio_info.pin,GPIO_PIN_RESET)
#define negate_nss(ch)	HAL_GPIO_WritePin(ch_info_tbl[ch].nss_gpio_info.p_gpio_grp,ch_info_tbl[ch].nss_gpio_info.pin,GPIO_PIN_SET)

// 共通割り込み処理
void spi_common_handler(SPI_CH ch)
{
	SPI_CB *this = get_myself(ch);
	SPI_TypeDef *p_reg;
	uint16_t status;
	uint16_t cr2;
	uint8_t dummy_data;
	
	// ベースレジスタ取得
	p_reg = get_reg(ch);
	
	// ステータス取得
	status = p_reg->SR;
	
	// エラー処理
	if (status & ( SPI_SR_MODF | SPI_SR_OVR )) {
		// 送受信失敗
		osSignalSet(this->id, SEND_RESULT_NG);
		return;
	}
	
	// CR2レジスタ値取得
	cr2 = p_reg->CR2;
	
	// 受信処理
	if (((cr2 & SPI_CR2_RXNEIE) != 0)&&((status & SPI_SR_RXNE) != 0)) {
		// データ受信
		dummy_data = (uint8_t)(p_reg->DR);
		// データ送信時にダミーデータを受信してしまうため読み捨てる
		if (this->rcv_sz != 0) {
			// 受信バッファが用意されているならセット
			if (this->p_rcv_data != NULL) {
				*(this->p_rcv_data++) = dummy_data;
			}
			this->rcv_sz--;
			
		// もう全部受信した
		} else {
			// 受信割り込み禁止
			clr_bit_t(uint16_t, p_reg->CR2, SPI_CR2_RXNEIE_Pos);
			
		}
		
	}
	
	// 送信処理
	if (((cr2 & SPI_CR2_TXEIE) != 0)&&((status & SPI_SR_TXE) != 0)) {
		// まだ送信データがある？
		if (this->snd_sz != 0) {
			// データ送信
			//*((uint8_t*)(p_reg->DR)) = *(this->p_snd_data++);
			*((uint8_t*)&(p_reg->DR)) = *(this->p_snd_data++);
			this->snd_sz--;
			
		// もう送信データはない？
		} else {
			// 送信割込み禁止
			clr_bit_t(uint16_t, p_reg->CR2, SPI_CR2_TXEIE_Pos);
			
		}
	}
	// 送受信全部終わった
	if ((this->snd_sz == 0) && (this->rcv_sz == 0)) {
		// 一応送受信割込み禁止
		clr_bit_t(uint16_t, p_reg->CR2, SPI_CR2_TXEIE_Pos);
		clr_bit_t(uint16_t, p_reg->CR2, SPI_CR2_RXNEIE_Pos);
		// エラー割込みも
		clr_bit_t(uint16_t, p_reg->CR2, SPI_CR2_ERRIE_Pos);
		// 送受信完了
		osSignalSet(this->id, SEND_RESULT_OK);
		
	}
}

// 割り込みハンドラ
//void SPI1_IRQHandler(void)
//{
//	spi_common_handler(SPI_CH_1);
//}
void SPI2_IRQHandler(void)
{
	spi_common_handler(SPI_CH_2);
}
//void SPI3_IRQHandler(void)
//{
//	spi_common_handler(SPI_CH_3);
//}
//void SPI4_IRQHandler(void)
//{
//	spi_common_handler(SPI_CH_4);
//}

// 指定したボーレートからレジスタ設定値を計算する関数
static uint32_t calc_br(SPI_CH ch, uint32_t baudrate)
{
	uint32_t spi_clk;
	uint8_t power_of_2;
	uint8_t i;
	uint8_t br;
	
	// SPIクロック取得(*)APB1は45Mhz
	spi_clk = 54*1000*1000;
	
	// BR値を計算
	for(i = 0; i < 8; i++) {
		power_of_2 = (1 << i);
		if ((spi_clk/power_of_2) < baudrate) {
			break;
		}
	}
	
	// br値を計算
	if (i > 0) {
		br = i - 1;
	}
	
	return br;
}

// コンフィグ
static osStatus spi_config(SPI_CH ch, SPI_PAR *p_par)
{
	SPI_TypeDef *p_reg;
	uint16_t br = 0;
	
	// パラメータチェック
	if ((p_par->cpol >= SPI_CPOL_MAX)		||	// 長さチェック
		(p_par->cpha >= SPI_CPHA_MAX)		||	// ストップビットチェック
		(p_par->fmt  >= SPI_FRAME_FMT_MAX)	||	// パリティチェック
		(p_par->size >= SPI_DATA_SIZE_MAX)) {		// パリティチェック
		return osErrorParameter;
	}
	
	// ベースレジスタ取得
	p_reg = get_reg(ch);
	
	// ビットレートを設定
	br = calc_br(ch, p_par->bps);
	set_field_t(uint16_t, p_reg->CR1, SPI_CR1_BR, br);
	// 極性を設定
	if (p_par->cpol != 0) {
		set_bit_t(uint16_t, p_reg->CR1, SPI_CR1_CPOL_Pos);
	}
	if (p_par->cpha != 0) {
		set_bit_t(uint16_t, p_reg->CR1, SPI_CR1_CPHA_Pos);
	}
	// フォーマットを設定
	if (p_par->fmt != 0) {
		set_bit_t(uint16_t, p_reg->CR1, SPI_CR1_LSBFIRST_Pos);
	}
	
	// NSSピンの設定(*)NSSはGPIOで制御
	set_bit_t(uint16_t, p_reg->CR1, SPI_CR1_SSM_Pos);
	set_bit_t(uint16_t, p_reg->CR1, SPI_CR1_SSI_Pos);
	// マスターに設定
	set_bit_t(uint16_t, p_reg->CR1, SPI_CR1_MSTR_Pos);
	
	// 割り込み有効
    HAL_NVIC_SetPriority(get_irqn(ch), get_pri(ch), 0);
    HAL_NVIC_EnableIRQ(get_irqn(ch));
	
	return osOK;
}

// SPE有効化
static void spi_enable(SPI_TypeDef *p_reg)
{
	// SPI有効
	set_bit_t(uint16_t, p_reg->CR1, SPI_CR1_SPE_Pos);
}

// SPE無効化
static void spi_disable(SPI_TypeDef *p_reg)
{
	uint16_t status;
	uint8_t t;
	uint8_t timeout = 100; // [ms]
	
	// ステータス取得
	status = p_reg->SR;
	
	// BUSYじゃなくなるまで待つ
	// (*) まあタイムアウトなんて起きないでしょう
	while ((status & SPI_SR_BSY) != 0) {
		// タイムアウト発生
		if (t++ > timeout) {
			break;
		}
		// 1ms待機
		osDelay(1);
	};
	
	// SPI有効
	clr_bit_t(uint16_t, p_reg->CR1, SPI_CR1_SPE_Pos);
}

// 初期化
osStatus spi_init(void)
{
	SPI_CB *this;
	SPI_CH ch;
	
	for (ch = 0; ch < SPI_CH_MAX; ch++) {
		// 制御ブロックの取得
		this = get_myself(ch);
		// 制御ブロックのクリア
		memset(this, 0, sizeof(SPI_CB));
		// ★割り込みの登録★
		
		// クローズ状態に更新
		this->status = ST_CLOSE;
	}
	
	return osOK;
}

// オープン関数
osStatus spi_open(SPI_CH ch, SPI_PAR *p_par)
{
	SPI_CB *this;
	osStatus ercd;
	
	// パラメータチェック
	if (ch >= SPI_CH_MAX) {
		return osErrorParameter;
	}
	if (p_par == NULL) {
		return osErrorParameter;
	}
	
	// 制御ブロック取得
	this = get_myself(ch);
	
	// クローズ状態でなければ終了
	if (this->status != ST_CLOSE) {
		return osErrorParameter;
	}
	
	// レジスタ設定
	if ((ercd = spi_config(ch, p_par)) != osOK) {
		goto EXIT;
	}
	
	// とりあえずNSSはあげとく
	spi_nss_off(ch, 1);
	
	// 状態をオープンにする
	this->status = ST_OPEN;
	
EXIT:
	return ercd;
}

// クローズ関数
osStatus spi_close(SPI_CH ch)
{
	// ★後で実装★
	return osOK;
}

// 送受信関数
// リエントラントは考えてないんで、複数の人が同じCHを使わないでね
osStatus spi_send_recv(SPI_CH ch, uint8_t *snd_data, uint8_t *rcv_data, uint32_t snd_sz)
{
	SPI_CB *this;
	SPI_TypeDef *p_reg;
	osEvent event;
	osStatus ercd;
	
	// パラメータチェック
	if (ch >= SPI_CH_MAX) {
		return -1;
	}
	if (snd_data == NULL) {
		return -1;
	}
	
	// 制御ブロック取得
	this = get_myself(ch);
	
	// オープン状態でなければ終了
	if (this->status != ST_OPEN) {
		return -1;
	}
	
	// ベースレジスタ取得
	p_reg = get_reg(ch);
	
	// データ情報設定
	this->p_snd_data = snd_data;
	this->snd_sz = snd_sz;
	this->p_rcv_data = rcv_data;
	this->rcv_sz = snd_sz;
	
	// タスクID取得
	this->id = osThreadGetId();
	
	// 割り込み有効
	set_bit_t(uint16_t, p_reg->CR2, SPI_CR2_TXEIE_Pos);
	set_bit_t(uint16_t, p_reg->CR2, SPI_CR2_RXNEIE_Pos);
	set_bit_t(uint16_t, p_reg->CR2, SPI_CR2_ERRIE_Pos);
	
	// SPI有効
	spi_enable(p_reg);
	
	// 待機
	if ((ercd = osDelay(1)) != osOK) {
		goto EXIT;
	}
	
	// 送信完了待ち
	event = osSignalWait((SEND_RESULT_OK | SEND_RESULT_NG), SLEEP_TIME);
	// 送信できなかった
	if (event.status != osEventSignal) {
		ercd = osErrorParameter;
		
	// 送信途中に何かしらのエラーが起きた
	} else if (event.value.signals == SEND_RESULT_NG) {
		ercd = osErrorParameter;
		
	// 送信できた
	} else if (event.value.signals == SEND_RESULT_OK) {
		ercd = osOK;
		
	// その他
	} else {
		;
		
	}
	
	// 待機
	if ((ercd = osDelay(1)) != osOK) {
		goto EXIT;
	}
	
EXIT:
	// SPI無効
	spi_disable(p_reg);
	
	return ercd;
}

// NSSオン
osStatus spi_nss_on(SPI_CH ch, uint32_t dly_time)
{
	osStatus status;
	
	// パラメータチェック
	if (ch >= SPI_CH_MAX) {
		return -1;
	}
	
	// 指定した時間待機
	if ((status = osDelay(dly_time)) != osOK) {
		goto EXIT;
	}
	status = osOK;
	
	// アサート
	assert_nss(ch);
	
EXIT:
	return status;
}

// NSSオフ
osStatus spi_nss_off(SPI_CH ch, uint32_t dly_time)
{
	osStatus status;
	
	// パラメータチェック
	if (ch >= SPI_CH_MAX) {
		return -1;
	}
	
	// 指定した時間待機
	if ((status = osDelay(dly_time)) != osOK) {
		goto EXIT;
	}
	status = osOK;
	
	// ネゲート
	negate_nss(ch);
	
EXIT:
	return status;
}
