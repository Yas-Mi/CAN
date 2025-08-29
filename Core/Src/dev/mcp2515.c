/*
 * mcp2515.c
 *
 *  Created on: 2024/2/12
 *      Author: User
 */
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f7xx.h"
#include "cmsis_os.h"
#include "console.h"
#include "spi.h"

#include "can_common.h"

#include "mcp2515.h"

// status
#define ST_NOT_INTIALIZED	(0U) // not initialized
#define ST_INITIALIZED		(1U) // initialized
#define ST_OPENED			(2U) // opened
#define ST_START			(3U) // start

// macro
#define USE_SPI_CH			(SPI_CH_1)			// used spi ch
#define MCP2515_CLOCK_INPUT	(8*1000*1000)		// 16MHz
#define BRP_MAX				(2*2*2*2*2*2-1)		// BRPの最大値
#define TIMEOUT				(10)				// タイムアウト 10ms
#define TX_BUF_NUM			(3)					// 送信バッファ数
#define TARGET_SAMPL_POINT	(87)				// サンプリングポイント

// instruction set
#define MCP2515_INST_RESET			(0xC0)		// リセット
#define MCP2515_INST_READ_REG		(0x03)		// レジスタ読み込み
#define MCP2515_INST_READ_RXBUF		(0x90)		// RXバッファ読み込み
#define MCP2515_INST_WRITE_REG		(0x02)		// レジスタ書き込み
#define MCP2515_INST_WRITE_TXBUF	(0x40)		// TXバッファ書き込み
#define MCP2515_INST_TXREQ			(0x80)		// メッセージ送信要求
#define MCP2515_INST_READ_STATUS	(0xA0)		// 状態読み込み
#define MCP2515_INST_READ_RXSTATUS	(0xB0)		// RX状態読み込み
#define MCP2515_INST_CHANGE_BIT		(0x05)		// ビット変更

// register define
// CTRL
#define MCP2515_REG_CANSTAT			(0x0E)		// CAN状態レジスタ
#define MCP2515_REG_CANCTRL			(0x0F)		// CAN制御レジスタ

// Macro for register define
// TXB
#define MCP2515_REG_TXB_BASE_ADDR	(0x30)
#define MCP2515_REG_TXB_OFFSET		(0x10)
#define GET_TXB_BASE_ADDR(n)		(MCP2515_REG_TXB_BASE_ADDR+n*MCP2515_REG_TXB_OFFSET)
// RXB
#define MCP2515_REG_RXB_BASE_ADDR	(0x60)
#define MCP2515_REG_RXB_OFFSET		(0x10)
#define GET_RXB_BASE_ADDR(n)		(MCP2515_REG_RXB_BASE_ADDR+n*MCP2515_REG_RXB_OFFSET)
// RXF
#define MCP2515_REG_RXF_BASE_ADDR	(0x00)
#define MCP2515_REG_RXF_OFFSET1		(0x04)
#define MCP2515_REG_RXF_OFFSET2		(0x10)
#define GET_RXF_BASE_ADDR(n)		(MCP2515_REG_RXF_BASE_ADDR+(n/3)*MCP2515_REG_RXF_OFFSET2+(n%3)*MCP2515_REG_RXF_OFFSET1)	// (*)nはフィルタ数
// RXM
#define MCP2515_REG_RXM_BASE_ADDR	(0x20)
#define MCP2515_REG_RXM_OFFSET		(0x04)
#define GET_RXM_BASE_ADDR(n)		(MCP2515_REG_RXM_BASE_ADDR+n*MCP2515_REG_RXM_OFFSET)

// register define
// TXB
#define MCP2515_REG_TXRTSCTRL		(0x0D)							// TXnRTSピン制御と状態レジスタ
#define MCP2515_REG_TXBCTRL(n)		(GET_TXB_BASE_ADDR(n)+0x00)		// 送信バッファ制御レジスタ
#define MCP2515_REG_TXBSIDH(n)		(GET_TXB_BASE_ADDR(n)+0x01)		// 送信バッファの標準識別子 上位
#define MCP2515_REG_TXBSIDL(n)		(GET_TXB_BASE_ADDR(n)+0x02)		// 送信バッファの標準識別子 下位
#define MCP2515_REG_TXBEID8(n)		(GET_TXB_BASE_ADDR(n)+0x03)		// 送信バッファの拡張識別子 上位
#define MCP2515_REG_TXBEID0(n)		(GET_TXB_BASE_ADDR(n)+0x04)		// 送信バッファの拡張識別子 下位
#define MCP2515_REG_TXBDLC(n)		(GET_TXB_BASE_ADDR(n)+0x05)		// 送信バッファのデータ長コード
#define MCP2515_REG_TXBD(n, byte)	(GET_TXB_BASE_ADDR(n)+0x06+byte)// 送信バッファのデータ n個目のbyte
// RXB
#define MCP2515_REG_BFPCTRL			(0x0C)							// RXnBF ピン制御と状態
#define MCP2515_REG_RXBCTRL(n)		(GET_RXB_BASE_ADDR(n)+0x00)		// 受信バッファ制御レジスタ
#define MCP2515_REG_RXBSIDH(n)		(GET_RXB_BASE_ADDR(n)+0x01)		// 受信バッファの標準識別子 上位
#define MCP2515_REG_RXBSIDL(n)		(GET_RXB_BASE_ADDR(n)+0x02)		// 受信バッファの標準識別子 下位
#define MCP2515_REG_RXBEID8(n)		(GET_RXB_BASE_ADDR(n)+0x03)		// 受信バッファの拡張識別子 上位
#define MCP2515_REG_RXBEID0(n)		(GET_RXB_BASE_ADDR(n)+0x04)		// 受信バッファの拡張識別子 下位
#define MCP2515_REG_RXBDLC(n)		(GET_RXB_BASE_ADDR(n)+0x05)		// 受信バッファのデータ長コード
#define MCP2515_REG_RXBD0(n)		(GET_RXB_BASE_ADDR(n)+0x06)		// 受信バッファのデータ 0byte目
#define MCP2515_REG_RXBD1(n)		(GET_RXB_BASE_ADDR(n)+0x07)		// 受信バッファのデータ 1byte目
#define MCP2515_REG_RXBD2(n)		(GET_RXB_BASE_ADDR(n)+0x08)		// 受信バッファのデータ 2byte目
#define MCP2515_REG_RXBD3(n)		(GET_RXB_BASE_ADDR(n)+0x09)		// 受信バッファのデータ 3byte目
#define MCP2515_REG_RXBD4(n)		(GET_RXB_BASE_ADDR(n)+0x0A)		// 受信バッファのデータ 4byte目
#define MCP2515_REG_RXBD5(n)		(GET_RXB_BASE_ADDR(n)+0x0B)		// 受信バッファのデータ 5byte目
#define MCP2515_REG_RXBD6(n)		(GET_RXB_BASE_ADDR(n)+0x0C)		// 受信バッファのデータ 6byte目
#define MCP2515_REG_RXBD7(n)		(GET_RXB_BASE_ADDR(n)+0x0D)		// 受信バッファのデータ 7byte目
// RXF
#define MCP2515_REG_RXFS1DH(n)		(GET_RXF_BASE_ADDR(n)+0x00)		// フィルタの標準識別子 上位
#define MCP2515_REG_RXFS1DL(n)		(GET_RXF_BASE_ADDR(n)+0x01)		// フィルタの標準識別子 下位
#define MCP2515_REG_RXFEID8(n)		(GET_RXF_BASE_ADDR(n)+0x02)		// フィルタの拡張識別子 上位
#define MCP2515_REG_RXFEID0(n)		(GET_RXF_BASE_ADDR(n)+0x03)		// フィルタの標準識別子 下位
// RXM
#define MCP2515_REG_RXMS1DH(n)		(GET_RXM_BASE_ADDR(n)+0x00)		// フィルタの標準識別子 上位
#define MCP2515_REG_RXMS1DL(n)		(GET_RXM_BASE_ADDR(n)+0x01)		// フィルタの標準識別子 下位
#define MCP2515_REG_RXMEID8(n)		(GET_RXM_BASE_ADDR(n)+0x02)		// フィルタの拡張識別子 上位
#define MCP2515_REG_RXMEID0(n)		(GET_RXM_BASE_ADDR(n)+0x03)		// フィルタの標準識別子 下位
// configuratuin
#define MCP2515_REG_CNF1			(0x2A)							// コンフィグレーション1
#define MCP2515_REG_CNF2			(0x29)							// コンフィグレーション2
#define MCP2515_REG_CNF3			(0x28)							// コンフィグレーション3
// status
#define MCP2515_REG_TEC				(0x1C)							// 送信エラーカウンタ
#define MCP2515_REG_REC				(0x1D)							// 受信エラーカウンタ
#define MCP2515_REG_EFLG			(0x2D)							// エラーフラグ
// interrupt
#define MCP2515_REG_CANINTE			(0x2B)							// 割込み許可
#define MCP2515_REG_CANINTF			(0x2C)							// 割込みフラグ

// bit define
// CANCTRL
#define CANCTRL_REQOP(v)			(((v) & 0x7) << 5)				// 動作モード要求ビット
#define CANCTRL_ABAT				(1 << 4)						// すべての送信の停止ビット
#define CANCTRL_OSM					(1 << 3)						// ワンショットモードビット
#define CANCTRL_CLKEN				(1 << 2)						// CLKOUTピン有効化ビット
#define CANCTRL_CLKPRE(v)			(((v) & 0x3) << 0)				// CLKOUTピン分周設定ビット
// CANSTAT
#define CANSTAT_OPMOD(v)			(((v) & 0x7) << 5)				// 動作モードビット
#define CANSTAT_ICOD(v)				(((v) & 0x7) << 1)				// 割込みフラグコードビット
// TXB
#define TXB_ABTF					(1 << 6)						// メッセージ停止フラグ
#define TXB_MLOA					(1 << 5)						// メッセージはアービトレーションを失った
#define TXB_TXERR					(1 << 4)						// 送信エラー検出ビット
#define TXB_TXREQ					(1 << 3)						// メッセージ送信要求ビット
#define TXB_TXP						(3 << 0)						// 送信バッファ優先順位
// TXRTSCTRL
#define TXRTSCTRL_B2RTS				(1 << 5)						// TX2RTSピン状態ビット
#define TXRTSCTRL_B1RTS				(1 << 4)						// TX1RTSピン状態ビット
#define TXRTSCTRL_B0RTS				(1 << 3)						// TX0RTSピン状態ビット
#define TXRTSCTRL_B2RTSM			(1 << 2)						// TX2RTSピンモードビット
#define TXRTSCTRL_B1RTSM			(1 << 1)						// TX1RTSピンモードビット
#define TXRTSCTRL_B0RTSM			(1 << 0)						// TX0RTSピンモードビット
// TXB0S1D
#define TXB0S1D_EXDIE				(1 << 3)						// 拡張識別子イネーブルビット
#define TXB0S1D_EID					(1 << 3)						// 拡張識別子ビット
// TXBDLC
#define TXBDLC_RTR					(1 << 6)						// リモート送信要求ビット
#define TXBDLC_DLC(v)				(((v) & 0xF) << 0)				// データ長コード
// RXB0CTRL
#define RXB0CTRL_RXM(v)				(((v) & 0x3) << 5)				// 受信バッファ動作モードビット
#define RXB0CTRL_RXRTR				(1 << 3)						// リモート送信要求ビットを受信した
#define RXB0CTRL_BUKT				(1 << 2)						// 切り替え許可ビット
#define RXB0CTRL_BUKT1				(1 << 1)						// 上記と同じ
#define RXB0CTRL_FILHIT				(1 << 0)						// フィルタ一致ビット
// RXB1CTRL
#define RXB1CTRL_RXM(v)				(((v) & 0x3) << 5)				// 受信バッファ動作モードビット
#define RXB1CTRL_RXRTR				(1 << 3)						// リモート送信要求ビットを受信した
#define RXB1CTRL_FILHIT(v)			(((v) & 0x7) << 0)				// フィルタ一致ビット
// BFPCTRL
#define BFPCTRL_B1BFS				(1 << 5)						// RX1BFピン状態ビット
#define BFPCTRL_B0BFS				(1 << 4)						// RX0BFピン状態ビット
#define BFPCTRL_B1BFE				(1 << 3)						// RX1BFピン機能有効化ビット
#define BFPCTRL_B0BFE				(1 << 2)						// RX0BFピン機能有効化ビット
#define BFPCTRL_B1BFM				(1 << 1)						// RX1BFピン動作モードビット
#define BFPCTRL_B0BFM				(1 << 0)						// RX0BFピン動作モードビット
// RXBS1DL
#define RXBS1DL_SRR					(1 << 4)						// 標準フレームのリモート送信要求ビット
#define RXBS1DL_IDE					(1 << 3)						// 標準フレームのリモート送信要求ビット
#define RXBS1DL_EID(v)				(((v) & 0x3) << 0)				// 標準フレームのリモート送信要求ビット
// RXBDLC
#define RXBDLC_RTR					(1 << 6)						// リモート送信要求ビット
#define RXBDLC_RB1					(1 << 5)						// リモート送信要求ビット
#define RXBDLC_RB0					(1 << 4)						// リモート送信要求ビット
#define RXBDLC_DLC(v)				(((v) & 0xF) << 0)				// データ長コード
// RXFS1DL
#define RXFS1DL_EXDIE				(1 << 3)						// 拡張識別子有効化ビット
#define RXFS1DL_EID					(1 << 3)						// 拡張識別子ビット
// RXMS1DL
#define RXMS1DL_EID(v)				(((v) & 0x3) << 0)				// 拡張識別子マスクビット
// CNF1
#define CNF1_SJW(v)					(((v) & 0x3) << 6)				// 再同期ジャンプ幅長ビット
#define CNF1_BRP(v)					(((v) & 0x3F) << 0)				// ボーレート分周ビット
// CNF2
#define CNF2_BTLMODE				(1 << 7)						// PS2ビットタイム長ビット
#define CNF2_SAM					(1 << 6)						// サンプル店コンフィグレーションビット
#define CNF2_PHSEG1(v)				(((v) & 0x7) << 3)				// PS1長さビット
#define CNF2_PRSEG(v)				(((v) & 0x7) << 0)				// 伝搬セグメント長ビット
// CNF3
#define CNF3_SOF					(1 << 7)						// スタートオブフレーム信号ビット
#define CNF3_WAKFIL					(1 << 6)						// ウェイクアップフィルタビット
#define CNF3_PHSEG2(v)				(((v) & 0x3) << 0)				// PS2長さビット
// EFLG
#define EFLG_RX1OVR					(1 << 7)						// 受信バッファ1オーバーフローフラグビット
#define EFLG_RX0OVR					(1 << 6)						// 受信バッファ0オーバーフローフラグビット
#define EFLG_TXBO					(1 << 5)						// バスオフエラーフラグビット
#define EFLG_TXEP					(1 << 4)						// 送信エラー パッシブエラーフラグビット
#define EFLG_RXEP					(1 << 3)						// 受信エラー パッシブフラグビット
#define EFLG_TXWAR					(1 << 2)						// 送信エラー警告フラグビット
#define EFLG_RXWAR					(1 << 1)						// 受信エラー警告フラグビット
#define EFLG_EWARN					(1 << 0)						// 受信バッファ0オーバーフローフラグビット
// CANINTE
#define CANINTE_MERRE				(1 << 7)						// メッセージエラー割込み許可ビット
#define CANINTE_WAKIE				(1 << 6)						// ウェイクアップ割込み許可ビット
#define CANINTE_ERRIE				(1 << 5)						// エラー割込み許可ビット
#define CANINTE_TX2IE				(1 << 4)						// 送信バッファ2空割込み許可ビット
#define CANINTE_TX1IE				(1 << 3)						// 送信バッファ1空割込み許可ビット
#define CANINTE_TX0IE				(1 << 2)						// 送信バッファ0空割込み許可ビット
#define CANINTE_RX1IE				(1 << 1)						// 受信バッファ1フル割込み許可ビット
#define CANINTE_RX0IE				(1 << 0)						// 受信バッファ0フル割込み許可ビット
// CANINTF
#define CANINTF_MERRF				(1 << 7)						// メッセージエラー割込みフラグビット
#define CANINTF_WAKIF				(1 << 6)						// ウェイクアップ割込みフラグビット
#define CANINTF_ERRIF				(1 << 5)						// エラー割込みフラグビット
#define CANINTF_TX2IF				(1 << 4)						// 送信バッファ2空割込みフラグビット
#define CANINTF_TX1IF				(1 << 3)						// 送信バッファ1空割込みフラグビット
#define CANINTF_TX0IF				(1 << 2)						// 送信バッファ0空割込みフラグビット
#define CANINTF_RX1IF				(1 << 1)						// 受信バッファ1フル割込みフラグビット
#define CANINTF_RX0IF				(1 << 0)						// 受信バッファ0フル割込みフラグビット

// Mode
#define NORMAL_MODE			(0)	// 通常動作モード
#define SLEEP_MODE			(1)	// スリープモード
#define LOOPBACK_MODE		(2)	// ループバックモード
#define LISTENONLY_MODE		(3)	// リスンオンリーモード
#define CONFIGRATION_MODE	(4)	// コンフィグレーションモード

// Mask
#define CANCTRL_REQOP_MASK	(0xE0)

// Rx STatus
#define RXB0_DATA_RECEIVED				(0x40)
#define RXB1_DATA_RECEIVED				(0x80)
#define RECEIVED_DATA_STANDARD			(0x00)
#define RECEIVED_DATA_STANDARD_REMOTE	(0x08)
#define RECEIVED_DATA_EXTENDED			(0x10)
#define RECEIVED_DATA_EXTENDED_REMOTE	(0x18)
#define RXF0_MATCH_FILTER				(0x00)
#define RXF1_MATCH_FILTER				(0x01)
#define RXF2_MATCH_FILTER				(0x02)
#define RXF3_MATCH_FILTER				(0x03)
#define RXF4_MATCH_FILTER				(0x04)
#define RXF5_MATCH_FILTER				(0x05)
#define RXF0_TRANS_MATCH_FILTER			(0x06)
#define RXF1_TRANS_MATCH_FILTER			(0x07)

// RXBnSIDL
#define RXBSIDL_SRR			(0x10)
#define RXBSIDL_IDE			(0x08)
#define GET_SID_10_3(v)		(((v) & 0xFF) >> 0)
#define GET_SID_2_0(v)		(((v) & 0xE0) >> 5)
#define GET_EID_17_16(v)	(((v) & 0x03) >> 0)
#define GET_EID_15_8(v)		(((v) & 0xFF) >> 0)
#define GET_EID_7_0(v)		(((v) & 0xFF) >> 0)

// RXBNDLC
#define GET_DLC(v)			(((v) & 0x0F) >> 0)

// RXF
#define RXB0_FILTER_NUM		(2)					// RXB0のフィルタ数
#define RXB1_FILTER_NUM		(4)					// RXB1のフィルタ数
#define RXF_FILTER_0		(0)					// フィルタ1
#define RXF_FILTER_1		(1)					// フィルタ2
#define RXF_FILTER_2		(2)					// フィルタ3
#define RXF_FILTER_3		(3)					// フィルタ4
#define RXF_FILTER_4		(4)					// フィルタ5
#define RXF_FILTER_5		(5)					// フィルタ6

// Set id
#define SET_SIDH(frame_type,id)		(frame_type == CAN_COMMON_FRAME_TYPE_STANDARD) ? (uint8_t)(id >> 3) : (uint8_t)((id >> 21) && 0xFF)
#define SET_SIDL(frame_type,id)		(frame_type == CAN_COMMON_FRAME_TYPE_STANDARD) ? (uint8_t)(id << 5) : (uint8_t)(((id >> 18) && 0x07) | ((id >> 16) && 0x03) | RXBSIDL_IDE)
#define SET_EID8(frame_type,id)		(frame_type == CAN_COMMON_FRAME_TYPE_STANDARD) ? 0 : (uint8_t)((id >> 8) && 0xFF)
#define SET_EID0(frame_type,id)		(frame_type == CAN_COMMON_FRAME_TYPE_STANDARD) ? 0 : (uint8_t)((id >> 0) && 0xFF)

typedef struct {
	uint32_t			status;						// 状態
} MCP2515_CTL;
static MCP2515_CTL mcp2515_ctl[MCP2515_DEV_MAX];
#define get_myself(dev) (&mcp2515_ctl[dev])

// spi open parameter
static const SPI_PAR spi_open_par = {
	50*1000,					// speed 500kHz
	SPI_CPOL_POSITIVE,			// Clock Polarity
	SPI_CPHA_FIRST_EDGE,    	// Clock Phase
	SPI_FRAME_FMT_MSB_FIRST,	// フレームフォーマット
	SPI_DATA_SIZE_8BIT,			// データサイズ
};

// MCP2515情報
typedef struct {
	SPI_CH ch;	// MCP2515を制御するためのSPIチャネル
} MCP2515_INFO;
static const MCP2515_INFO mcp2515_info[MCP2515_DEV_MAX] = {
	{SPI_CH_2},		// MCP2515 1 の情報
};

// ビット変更対象レジスタ
static const uint8_t modify_reg_tbl[] = {
	MCP2515_REG_BFPCTRL,
	MCP2515_REG_TXRTSCTRL,
	MCP2515_REG_CANCTRL,
	MCP2515_REG_CNF1,
	MCP2515_REG_CNF2,
	MCP2515_REG_CNF3,
	MCP2515_REG_CANINTE,
	MCP2515_REG_CANINTF,
	MCP2515_REG_TXBCTRL(0),
	MCP2515_REG_TXBCTRL(1),
	MCP2515_REG_TXBCTRL(2),
	MCP2515_REG_RXBCTRL(0),
	MCP2515_REG_RXBCTRL(1),
};

// 状態テーブル
typedef struct {
	uint8_t		reg_bit;
	uint32_t	ret_bit;
} MCP2515_STATUS_INFO;
static const MCP2515_STATUS_INFO status_info[] = {
//	reg_bit			ret_bit
	{EFLG_RX1OVR,	CAN_COMMON_STS_RXOVER},
	{EFLG_RX0OVR,	CAN_COMMON_STS_RXOVER},
	{EFLG_TXBO,		CAN_COMMON_STS_TXBO},
	{EFLG_TXEP,		CAN_COMMON_STS_TXEP},
	{EFLG_RXEP,		CAN_COMMON_STS_RXEP},
	{EFLG_TXWAR,	CAN_COMMON_STS_TXWAR},
	{EFLG_RXWAR,	CAN_COMMON_STS_TXWAR},
	{EFLG_EWARN,	CAN_COMMON_STS_EWARN},
};

// 受信メールボックス情報
static osStatus set_filter_0(MCP2515_DEV dev, uint32_t frame_type, uint32_t *p_filter);
static osStatus set_filter_1(MCP2515_DEV dev, uint32_t frame_type, uint32_t *p_filter);
typedef osStatus (*SET_FILTER)(MCP2515_DEV dev, uint32_t frame_type, uint32_t *p_filter);
static const SET_FILTER set_filter_tbl[MCP2515_DEV_RX_MBX_ID_MAX] = {
	set_filter_0,
	set_filter_1,
};

/* 割込みハンドラ */
void exti5_9_handler(void)
{
	;
}

//====MCPアクセス処理====
static osStatus can_spi_send_rcv(MCP2515_DEV dev, uint8_t *snd_data, uint8_t *rcv_data, uint32_t size)
{
	osStatus ercd;
	SPI_CH ch;
	
	// SPIのチャネル取得
	ch = mcp2515_info[dev].ch;
	
	// アサート
	if ((ercd = spi_nss_on(ch, 1)) != osOK) {
		goto EXIT;
	}
	
	// データ送受信
	if ((ercd = spi_send_recv(ch, snd_data, rcv_data, size)) != osOK) {
		;
	}
	
EXIT:
	// ネゲート
	if ((ercd = spi_nss_off(ch, 1)) != osOK) {
		;
	}
	
	return ercd;
}

// リセット
static osStatus reset(MCP2515_DEV dev)
{
	uint8_t snd_data;
	osStatus ret;
	
	// データ設定
	snd_data = MCP2515_INST_RESET;
	
	// SPI送信
	ret = can_spi_send_rcv(dev, &snd_data, NULL, sizeof(snd_data));
	
	return ret;
}

// レジスタリード
static osStatus read_reg(MCP2515_DEV dev, uint8_t reg, uint8_t *data)
{
	uint8_t snd_data[3];
	uint8_t rcv_data[3];
	osStatus ret;
	
	// データ設定
	snd_data[0] = MCP2515_INST_READ_REG;
	snd_data[1] = reg;
	snd_data[2] = 0x00;	// (*)ダミー
	
	// SPI送信
	ret = can_spi_send_rcv(dev, snd_data, rcv_data, sizeof(snd_data));
	
	// 受信データ設定
	*data = rcv_data[2];
	
	return ret;
}

// レジスタライト
static osStatus write_reg(MCP2515_DEV dev, uint8_t reg, uint8_t data)
{
	uint8_t snd_data[3];
	osStatus ret;
	
	// データ設定
	snd_data[0] = MCP2515_INST_WRITE_REG;
	snd_data[1] = reg;
	snd_data[2] = data;
	
	// SPI送信
	ret = can_spi_send_rcv(dev, snd_data, NULL, sizeof(snd_data));
	
	return ret;
}

// ビット変更
static osStatus modify_bit(MCP2515_DEV dev, uint8_t reg, uint8_t mask, uint8_t data)
{
	uint8_t i;
	uint8_t tbl_size;
	uint8_t snd_data[4];
	osStatus ret;
	
	// ビット変更できるレジスタかどうかチェック
	tbl_size = sizeof(modify_reg_tbl)/sizeof(modify_reg_tbl[0]);
	for (i = 0; i < tbl_size; i++) {
		if (modify_reg_tbl[i] == reg) {
			break;
		}
	}
	if (i >= tbl_size) {
		return osErrorParameter;
	}
	
	// データ設定
	snd_data[0] = MCP2515_INST_CHANGE_BIT;
	snd_data[1] = reg;
	snd_data[2] = mask;
	snd_data[3] = data;
	
	// SPI送信
	ret = can_spi_send_rcv(dev, snd_data, NULL, sizeof(snd_data));
	
	return ret;
}

// 状態読み込み
static osStatus read_status(MCP2515_DEV dev, uint8_t *data)
{
	uint8_t snd_data[2];
	uint8_t rcv_data[2];
	osStatus ret;
	
	// データ設定
	snd_data[0] = MCP2515_INST_READ_STATUS;
	snd_data[1] = 0x00;	// (*)ダミー
	
	// SPI送信
	ret = can_spi_send_rcv(dev, snd_data, rcv_data, sizeof(snd_data));
	
	// 受信データ設定
	*data = rcv_data[1];
	
	return ret;
}

// RX状態読み込み
static osStatus read_rxstatus(MCP2515_DEV dev, uint8_t *data)
{
	uint8_t snd_data[2];
	uint8_t rcv_data[2];
	osStatus ret;
	
	// データ設定
	snd_data[0] = MCP2515_INST_READ_RXSTATUS;
	snd_data[1] = 0x00;	// (*)ダミー
	
	// SPI送信
	ret = can_spi_send_rcv(dev, snd_data, rcv_data, sizeof(snd_data));
	
	// 受信データ設定
	*data = rcv_data[1];
	
	return ret;
}

// 指定したレジスタが特定の値になるまで待機
static osStatus wait_sts(MCP2515_DEV dev, uint8_t reg, uint8_t mask, uint8_t sts)
{
	osStatus ercd;
	uint8_t data;
	uint8_t timeout = 10;
	
	while (timeout--) {
		// 値読み出し
		if ((ercd = read_reg(dev, reg, &data)) != osOK) {
			continue;
		}
		// チェック
		if ((data & mask) == (sts & mask)) {
			break;
		}
		ercd = osEventTimeout;
		osDelay(1);
	}
	
	return ercd;
}

// TXB設定 (*)TXBnCTRL以外
static osStatus set_txb(MCP2515_DEV dev, uint32_t mbox_id, CAN_COMMON_FRAME_TYPE frame_type, uint32_t can_id, uint8_t *data, uint8_t size)
{
	uint8_t send_data[15];
	uint8_t base_addr;
	osStatus ret = -1;
	uint8_t i;
	
	// ベースアドレス取得
	base_addr = MCP2515_REG_TXBSIDH(mbox_id);
	
	// ===データ設定===
	// 命令
	send_data[0] = MCP2515_INST_WRITE_REG;
	// アドレス
	send_data[1] = base_addr;
	// ID
	if (frame_type == CAN_COMMON_FRAME_TYPE_STANDARD) {
		send_data[2] = SET_SIDH(frame_type, can_id);	// SIDH
		send_data[3] = SET_SIDL(frame_type, can_id);	// SIDL
		send_data[4] = SET_EID8(frame_type, can_id);	// EID8
		send_data[5] = SET_EID0(frame_type, can_id);	// EID0
		
	} else if (frame_type == CAN_COMMON_FRAME_TYPE_EXTENDED) {
		// 拡張は後で
		
	} else {
		goto SET_TXB_EXIT;
		
	}
	// DLC
	send_data[6] = TXBDLC_DLC(size);
	// データ設定
	for (i = 0; i < size; i++) {
		send_data[7+i] = *data++;
	}
	
	// SPI送信
	ret = can_spi_send_rcv(dev, send_data, NULL, sizeof(send_data));
	
SET_TXB_EXIT:
	return ret;
}

// マスク設定
static osStatus set_mask(MCP2515_DEV dev, uint32_t mbox_id, uint32_t frame_type, uint32_t mask)
{
	uint8_t send_data[6];
	osStatus ret = -1;
	
	// ===マスク設定===
	// 命令
	send_data[0] = MCP2515_INST_WRITE_REG;
	// アドレス
	send_data[1] = MCP2515_REG_RXMS1DH(mbox_id);
	// ID
	send_data[2] = SET_SIDH(frame_type, mask);	// SIDH
	send_data[3] = SET_SIDL(frame_type, mask);	// SIDL
	send_data[4] = SET_EID8(frame_type, mask);	// EID8
	send_data[5] = SET_EID0(frame_type, mask);	// EID0
	
	// SPI送信
	if ((ret = can_spi_send_rcv(dev, send_data, NULL, sizeof(send_data))) != osOK) {
		goto EXIT;
	}
	
	//console_printf("set_mask success\n");
	
EXIT:
	return ret;
}

// フィルタ設定
static osStatus set_filter_0(MCP2515_DEV dev, uint32_t frame_type, uint32_t *p_filter)
{
	uint8_t send_data[10];
	osStatus ret = -1;
	uint8_t ofst;
	uint8_t size = 2;
	
	// ===フィルタ設定===
	// 命令
	send_data[0] = MCP2515_INST_WRITE_REG;
	// アドレス
	send_data[1] = MCP2515_REG_RXFS1DH(RXF_FILTER_0);
	// フィルタ設定
	for (ofst = 0; ofst < RXB0_FILTER_NUM; ofst++, p_filter++) {
		// 最後のフィルタ
		if (p_filter == NULL) {
			break;
		}
		// フィルタ設定
		send_data[2 + (ofst * 4) + 0] = SET_SIDH(frame_type, *p_filter);	// SIDH
		send_data[2 + (ofst * 4) + 1] = SET_SIDL(frame_type, *p_filter);	// SIDL
		send_data[2 + (ofst * 4) + 2] = SET_EID8(frame_type, *p_filter);	// EID8
		send_data[2 + (ofst * 4) + 3] = SET_EID0(frame_type, *p_filter);	// EID0
		size += 4;
	}
	
	// SPI送信
	if ((ret = can_spi_send_rcv(dev, send_data, NULL, size)) != osOK) {
		goto EXIT;
	}
	
	//console_printf("set_filter_0 success\n");
	
EXIT:
	return ret;
}

// フィルタ設定
static osStatus set_filter_1(MCP2515_DEV dev, uint32_t frame_type, uint32_t *p_filter)
{
	uint8_t send_data[14];
	osStatus ret = -1;
	uint8_t ofst;
	uint8_t size = 2;

	// ===フィルタ設定===
	// 命令
	send_data[0] = MCP2515_INST_WRITE_REG;
	// アドレス
	send_data[1] = MCP2515_REG_RXFS1DH(RXF_FILTER_2);
	// フィルタ2設定
	send_data[2] = SET_SIDH(frame_type, *p_filter);
	send_data[3] = SET_SIDL(frame_type, *p_filter);
	send_data[4] = SET_EID8(frame_type, *p_filter);
	send_data[5] = SET_EID0(frame_type, *p_filter);
	// フィルタ2の設定をいったん送信 (*)フィルタ2とフィルタ3はアドレスが離れているため、連続で送信できない
	if ((ret = can_spi_send_rcv(dev, send_data, NULL, 6)) != osOK) {
		goto EXIT;
	}

	// フィルタ2以降の設定
	// アドレス
	send_data[1] = MCP2515_REG_RXFS1DH(RXF_FILTER_3);
	// フィルタを先に進めとく
	p_filter++;
	// フィルタ設定
	for (ofst = 0; ofst < (RXB1_FILTER_NUM - 1); ofst++, p_filter++) {
		// 最後のフィルタ
		if (p_filter == NULL) {
			break;
		}
		// フィルタ設定
		send_data[2 + (ofst * 4) + 0] = SET_SIDH(frame_type, *p_filter);	// SIDH
		send_data[2 + (ofst * 4) + 1] = SET_SIDL(frame_type, *p_filter);	// SIDL
		send_data[2 + (ofst * 4) + 2] = SET_EID8(frame_type, *p_filter);	// EID8
		send_data[2 + (ofst * 4) + 3] = SET_EID0(frame_type, *p_filter);	// EID0
		size += 4;

	}

	// SPI送信
	if ((ret = can_spi_send_rcv(dev, send_data, NULL, size)) != osOK) {
		goto EXIT;
	}

	//console_printf("set_filter_1 success\n");

EXIT:
	return ret;
}

// 受信CAN設定
static osStatus set_rx_info(MCP2515_DEV dev, uint32_t mbox_id, uint32_t frame_type, uint32_t mask, uint32_t *p_filter)
{
	osStatus ret;
	
	// マスク設定
	if((ret = set_mask(dev, mbox_id, frame_type, mask)) !=  osOK) {
		goto EXIT;
	}
	
	// フィルタ設定
	if((ret = set_filter_tbl[mbox_id](dev, frame_type, p_filter)) !=  osOK) {
		goto EXIT;
	}
	
	//console_printf("set_rx_info success\n");
	
EXIT:
	return ret;
}

// フリーな送信バッファを取得
static osStatus get_free_tx_buf(MCP2515_DEV dev, uint32_t *mbx_id)
{
	uint32_t i;
	uint8_t ctrl;
	osStatus ercd;
	
	for (i = 0; i < MCP2515_DEV_TX_MBX_ID_MAX; i++) {
		// CTRLレジスタ読み込み
		ercd = read_reg(dev, MCP2515_REG_TXBCTRL(i), &ctrl);
		// TXREQがクリアされていれば空いている
		if ((ercd == osOK) && ((ctrl & TXB_TXREQ) == 0)) {
			*mbx_id = i;
			return osOK;
		}
	}
	
	return osErrorResource;  // 全て使用中
}

// 送信アボート
static osStatus tx_abort(MCP2515_DEV dev)
{
	osStatus ercd;
	uint32_t i;
	uint32_t retry_cnt = 10;
	uint32_t stop_bmp;
	uint8_t ctrl = 0;
	
	// アボート要求
	ercd = modify_bit(dev, MCP2515_REG_CANCTRL, CANCTRL_ABAT, CANCTRL_ABAT);
	if (ercd != osOK) {
		goto EXIT;
	}
	
	// アボート待ち
	while (retry_cnt--) {
		stop_bmp = 0;
		for (i = 0; i < MCP2515_DEV_TX_MBX_ID_MAX; i++) {
			// CTRLレジスタ読み込み
			ercd = read_reg(dev, MCP2515_REG_TXBCTRL(i), &ctrl);
			// spiエラー
			if (ercd != osOK) {
				break;
				
			// 送信要求がクリアされていない
			} else if ((ctrl & TXB_TXREQ) != 0) {
				break;
				
			// クリアされた
			} else {
				stop_bmp |= (1UL << i);
				
			}
		}
		// 全部アボートできた
		if (stop_bmp == ((1UL << MCP2515_DEV_TX_MBX_ID_MAX) - 1)) {
			ercd = osOK;
			break;
		}
		ercd = osEventTimeout;
	}
	
EXIT:
	return ercd;
}

// 受信データがあるかどうかのチェック
static uint8_t check_rx_data(MCP2515_DEV dev, uint32_t mbx_id)
{
	osStatus ercd;
	uint8_t status;
	uint8_t ret = 0;
	
	// 受信状態取得
	ercd = read_rxstatus(dev, &status);
	if (ercd != osOK) {
		ret = 0;
		goto EXIT;
	}
	
	// RXB0、RXB1の受信状態を取得(*)6bit右シフトは状態をLSBに合わせるため
	status = ((status & (RXB0_DATA_RECEIVED | RXB1_DATA_RECEIVED)) >> 6);
	
	// 受信した！
	if (status & ((uint8_t)1 << mbx_id)) {
		ret = 1;
	}
	
EXIT:
	return ret;
}

static osStatus clr_rx_flag(MCP2515_DEV dev, uint32_t mbx_id)
{
	osStatus ercd;
	uint8_t data;
	uint8_t clr_sts = 0;
	
	// フラグ状態取得
	ercd = read_reg(dev, MCP2515_REG_CANINTF, &data);
	if (ercd != osOK) {
		goto EXIT;
	}
	
	// 受信フラグのクリア
	clr_sts = data & ~((uint8_t)1 << mbx_id);

	// データセット
	ercd = write_reg(dev, MCP2515_REG_CANINTF, clr_sts);
	if (ercd != osOK) {
		goto EXIT;
	}
	
EXIT:
	return ercd;
}

// 受信データを取得
static osStatus get_rx_data(MCP2515_DEV dev, uint32_t mbox_id, uint8_t *p_is_eid, uint32_t *p_can_id, uint8_t *p_data, uint8_t *p_size)
{
	uint8_t recv_data[16];
	uint8_t send_data[16];
	osStatus ret;
	uint8_t i;
	
	// ===データ設定===
	// 命令
	send_data[0] = MCP2515_INST_READ_REG;
	// アドレス
	send_data[1] = MCP2515_REG_RXBCTRL(mbox_id);
	
	// SPI送信
	ret = can_spi_send_rcv(dev, send_data, recv_data, sizeof(send_data));
	if (ret != osOK) {
		goto EXIT;
	}
	
	// recv_data[0]  : 無視
	// recv_data[1]  : 無視
	// recv_data[2]  : RXBnCTRL
	// recv_data[3]  : RXBnSIDH
	// recv_data[4]  : RXBnSIDL
	// recv_data[5]  : RXBnEID8
	// recv_data[6]  : RXBnEID0
	// recv_data[7]  : RXBnDLC
	// recv_data[8]  : RXBnD0
	// recv_data[9]  : RXBnD1
	// recv_data[10] : RXBnD2
	// recv_data[11] : RXBnD3
	// recv_data[12] : RXBnD4
	// recv_data[13] : RXBnD5
	// recv_data[14] : RXBnD6
	// recv_data[15] : RXBnD7
	
	// 拡張ID
	if ((recv_data[4] & RXBSIDL_IDE) != 0) {
		*p_can_id = (GET_SID_10_3(recv_data[3]) << 20) | (GET_SID_2_0(recv_data[4]) << 17) |
				   (GET_EID_17_16(recv_data[4]) << 16) | (GET_EID_15_8(recv_data[5]) << 8) |
				   (GET_EID_7_0(recv_data[6]) << 0);
		*p_is_eid = 1;
		
	// 標準ID
	} else {
		*p_can_id = (GET_SID_10_3(recv_data[3]) << 3) | (GET_SID_2_0(recv_data[4]));
		
	}

	// dlc取得
	*p_size = GET_DLC(recv_data[7]);
	
	// データ取得
	for (i = 0; i < *p_size; i++) {
		*(p_data++) = recv_data[8+i];
	}
	
EXIT:
	return ret;
	
}

// ウェイクアップ要因チェック
static osStatus check_wkup(MCP2515_DEV dev)
{
	osStatus ercd;
	uint8_t data;
	
	// フラグ状態取得
	ercd = read_reg(dev, MCP2515_REG_CANINTF, &data);
	if (ercd != osOK) {
		goto EXIT;
	}
	
	// チェック
	if ((data & CANINTF_WAKIF) != 0) {
		console_printf("wakeup!\n");
		// クリア
		ercd = modify_bit(dev, MCP2515_REG_CANINTF, CANINTF_WAKIF, ~CANINTF_WAKIF);
		if (ercd != osOK) {
			goto EXIT;
		}
	}
	
EXIT:
	return ercd;
}

// MCP2515ビットレート設定
static osStatus mcpl2515_set_bitrate(MCP2515_DEV dev, uint32_t bit_rate)
{
	uint8_t brp;				// BRP
	float cps;					// 指定ビットレートの1clock当たりの時間
	float tq;					// Time Quonta
	uint32_t actual_bit_rate;	// 実際に設定されるビットレート
	uint8_t all_tq_num;
	uint8_t sync, prop_seg, phase_seg1, phase_seg2;
	uint8_t cur_sampl;
	uint8_t i;
	osStatus ercd = osOK;
	
	// TQを算出
	brp = 0;
	tq = ((float)(2 * (brp + 1)) / (float)MCP2515_CLOCK_INPUT);
	
	// ビットタイムを算出（してビットレートにするためには全部で何TQかを算出）
	cps = ((float)1 / (float)bit_rate);
	all_tq_num = cps / tq;
	//console_printf("all_tq_num:%d\n", all_tq_num);
	
	// その他設定
	sync = 1;
	prop_seg = 6; // とりあえず2TQ固定
	for (i = 0; i < all_tq_num; i++) {
		phase_seg1 = i;
		// 現在のサンプリングポイントを算出
		cur_sampl = (uint8_t)(((float)(sync + prop_seg + phase_seg1) / (float)all_tq_num) * 100);
		// 指定のサンプリングポイントになったら終了
		if (cur_sampl >= TARGET_SAMPL_POINT) {
			break;
		}
	}
	
	// 良いphase_seg1がなかった
	if (i == all_tq_num) {
		ercd = osErrorParameter;
		goto EXIT;
	}
	
	// phase_seg2算出
	phase_seg2 = all_tq_num - (sync + prop_seg + phase_seg1);
	//console_printf("sync:%d\n", sync);
	//console_printf("prop_seg:%d\n", prop_seg);
	//console_printf("phase_seg1:%d\n", phase_seg1);
	//console_printf("phase_seg2:%d\n", phase_seg2);
	
	// 誤差を計算して表示
	actual_bit_rate = (uint32_t)(1 / (all_tq_num * tq));
	//console_printf("set_bit_rate:%d\n", bit_rate);
	//console_printf("actual_bit_rate:%d\n", actual_bit_rate);
	
	// レジスタ書き込み
	write_reg(dev, MCP2515_REG_CNF1, CNF1_SJW(0)|CNF1_BRP(brp));
	write_reg(dev, MCP2515_REG_CNF2, CNF2_BTLMODE|CNF2_SAM|CNF2_PHSEG1(phase_seg1-1)|CNF2_PRSEG(prop_seg-1));
	write_reg(dev, MCP2515_REG_CNF3, CNF3_SOF|CNF3_PHSEG2(phase_seg2-1));
	
EXIT:
	return ercd;
}


// 外部公開関数
// 初期化関数
void mcp2515_dev_init(void)
{
	MCP2515_CTL *this;
	MCP2515_DEV dev;
	
	for (dev = 0; dev < MCP2515_DEV_MAX; dev++) {
		// 制御ブロックの取得
		this = get_myself(dev);
		// 制御ブロックの初期化
		memset(this, 0, sizeof(MCP2515_CTL));
		// 状態の更新
		this->status = ST_INITIALIZED;
		
	}
	
	return;
}

// オープン関数
osStatus mcp2515_dev_open(MCP2515_DEV dev, uint32_t bit_rate)
{
	MCP2515_CTL *this;
	uint32_t ret;
	SPI_CH ch;
	
	// devチェック
	if (dev >= MCP2515_DEV_MAX) {
		return osErrorParameter;
	}
	
	// 制御ブロック取得
	this = get_myself(dev);
	
	// 未初期化の場合はエラーを返して終了
	if (this->status != ST_INITIALIZED) {
		return -1;
	}
	
	// SPIのチャネル取得
	ch = mcp2515_info[dev].ch;
	
	// SPIオープン
	ret = spi_open(ch, &spi_open_par);
	if (ret != osOK) {
		console_printf("spi_open error\n");
		goto OPEN_ERROR;
	}
	
	// 割り込み有効
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	
	// ウェイクアップチェック
	ret = check_wkup(dev);
	if (ret != osOK) {
		console_printf("check_wkup error\n");
		goto OPEN_ERROR;
	}
	
	// デバイス初期化
	ret = reset(dev);
	if (ret != osOK) {
		console_printf("reset error\n");
		goto OPEN_ERROR;
	}
	
	// ビットレート設定
	ret = mcpl2515_set_bitrate(dev, bit_rate);
	if (ret != osOK) {
		console_printf("mcpl2515_set_bitrate error\n");
		goto OPEN_ERROR;
	}
	
	// 状態を更新
 	this->status = ST_OPENED;
	
OPEN_ERROR:
	return ret;
}

// オープン関数
osStatus mcp2515_dev_set_mailbox(MCP2515_DEV dev, CAN_COMMON_RX_MAILBOX_INFO *p_mbox_info)
{
	MCP2515_CTL *this;
	osStatus ercd;
	
	// devチェック
	if (dev >= MCP2515_DEV_MAX) {
		return osErrorParameter;
	}
	
	// NULLチェック
	if (p_mbox_info == NULL) {
		return osErrorParameter;
	}
	if (p_mbox_info->p_filter == NULL) {
		return osErrorParameter;
	}
	
	// 制御ブロック取得
	this = get_myself(dev);
	
	// オープンでない場合はエラーを返して終了
	if (this->status != ST_OPENED) {
		return osErrorParameter;
	}
	
	// マスク設定
	if((ercd = set_rx_info(dev, p_mbox_info->mbox_id, p_mbox_info->frame_type, p_mbox_info->mask, p_mbox_info->p_filter)) !=  osOK) {
		goto EXIT;
	}
	
EXIT:
	return osOK;
}

// スタート関数
osStatus mcp2515_dev_start(MCP2515_DEV dev)
{
	MCP2515_CTL *this;
	osStatus ercd;
	
	// devチェック
	if (dev >= MCP2515_DEV_MAX) {
		return osErrorParameter;
	}
	
	// 制御ブロック取得
	this = get_myself(dev);
	
	// 未オープンの場合はエラーを返して終了
	if (this->status != ST_OPENED) {
		return osErrorParameter;
	}
	
	// ノーマルモードに遷移させる
	ercd = modify_bit(dev, MCP2515_REG_CANCTRL, CANCTRL_REQOP_MASK, CANCTRL_REQOP(NORMAL_MODE));
	if (ercd != osOK) {
		goto START_EXIT;
	}
	
	// 状態を更新
 	this->status = ST_START;
	
START_EXIT:
	return ercd;
}

// ストップ関数
osStatus mcp2515_dev_stop(MCP2515_DEV dev)
{
	MCP2515_CTL *this;
	osStatus ercd;
	
	// devチェック
	if (dev >= MCP2515_DEV_MAX) {
		return osErrorParameter;
	}
	
	// 制御ブロック取得
	this = get_myself(dev);
	
	// 開始中でない場合はエラーを返して終了
	if (this->status != ST_START) {
		return osErrorParameter;
	}
	
	// 送信アボート
	ercd = tx_abort(dev);
	if (ercd != osOK) {
		goto STOP_EXIT;
	}
	
	console_printf("abort success\n");
	
	// コンフィグレーション状態に遷移させる
	ercd = modify_bit(dev, MCP2515_REG_CANCTRL, CANCTRL_REQOP_MASK, CANCTRL_REQOP(CONFIGRATION_MODE));
	if (ercd != osOK) {
		goto STOP_EXIT;
	}
	
	// コンフィグレーション状態待ち
	ercd = wait_sts(dev, MCP2515_REG_CANCTRL, CANCTRL_REQOP_MASK, CANCTRL_REQOP(CONFIGRATION_MODE));
	if (ercd != osOK) {
		goto STOP_EXIT;
	}
	
	console_printf("go to config mode\n");
	
	// 状態を更新
 	this->status = ST_OPENED;
	
STOP_EXIT:
	return ercd;
}

// スリープ関数
osStatus mcp2515_dev_sleep(MCP2515_DEV dev)
{
	MCP2515_CTL *this;
	osStatus ercd;
	
	// devチェック
	if (dev >= MCP2515_DEV_MAX) {
		return osErrorParameter;
	}
	
	// 制御ブロック取得
	this = get_myself(dev);
	
	// 開始中でない場合はエラーを返して終了
	if (this->status != ST_START) {
		return osErrorParameter;
	}
	
	// 送信アボート
	ercd = tx_abort(dev);
	if (ercd != osOK) {
		goto SLEEP_EXIT;
	}
	
	console_printf("abort success\n");
	
	// スリープモードに遷移させる
	ercd = modify_bit(dev, MCP2515_REG_CANCTRL, CANCTRL_REQOP_MASK, CANCTRL_REQOP(SLEEP_MODE));
	if (ercd != osOK) {
		goto SLEEP_EXIT;
	}
	
	// スリープ状態待ち
	ercd = wait_sts(dev, MCP2515_REG_CANCTRL, CANCTRL_REQOP_MASK, CANCTRL_REQOP(SLEEP_MODE));
	if (ercd != osOK) {
		goto SLEEP_EXIT;
	}
	
	console_printf("go to can sleep mode\n");
	
	// ウェイクアップ割込み有効
	ercd = modify_bit(dev, MCP2515_REG_CANINTE, CANINTE_WAKIE, CANINTE_WAKIE);
	if (ercd != osOK) {
		goto SLEEP_EXIT;
	}
	
	console_printf("set wakeup int\n");
	
SLEEP_EXIT:
	return ercd;
}

// 送信関数 (*) ブロッキング
osStatus mcp2515_dev_send(MCP2515_DEV dev, uint32_t mbx_id, CAN_COMMON_FRAME_TYPE frame_type, uint32_t can_id, uint8_t *data, uint8_t size)
{
	MCP2515_CTL *this;
	int32_t timeout = TIMEOUT;
	osStatus ercd = -1;
	uint8_t ctrl;
	
	// パラメータチェック
	if ((dev >= MCP2515_DEV_MAX) ||
		(mbx_id >= MCP2515_DEV_TX_MBX_ID_MAX) ||
		(frame_type >= CAN_COMMON_FRAME_TYPE_MAX) ||
		(data == NULL) ||
		(size == 0)) {
		return osErrorParameter;
	}
	
	// 制御ブロック取得
	this = get_myself(dev);
	
	// 開始状態でなければ終了
	if (this->status != ST_START) {
		return osErrorParameter;
	}
	
	// 自動でメールボックス選択
	if (mbx_id == CAN_COMMON_AUTO_MBX_ID) {
		if ((ercd = get_free_tx_buf(dev, &mbx_id)) != osOK) {
			console_printf("get_free_tx_buf error\n");
			goto SEND_EXIT;
		}
		
	// メールボックス指定
	} else {
		// 指定したメールボックスが空いているかチェック
		ercd = read_reg(dev, MCP2515_REG_TXBCTRL(mbx_id), &ctrl);
		if ((ercd != osOK) || ((ctrl & TXB_TXREQ) != 0)) {
			ercd = osErrorResource;
			goto SEND_EXIT;
		}
	}
	
	// データ設定
	ercd = set_txb(dev, mbx_id, frame_type, can_id, data, size);
	if (ercd != osOK) {
		console_printf("set_txb error\n");
		goto SEND_EXIT;
	}
	
	// メッセージ送信要求ビットをセット
	ercd = modify_bit(dev, MCP2515_REG_TXBCTRL(mbx_id), TXB_TXREQ, TXB_TXREQ);
	if (ercd != osOK) {
		console_printf("modify_bit error\n");
		goto SEND_END_PROC;
	}
	
	// 送信完了待ち 
	while (timeout--) {
		// CTRLレジスタ読み込み
		ercd = read_reg(dev, MCP2515_REG_TXBCTRL(mbx_id), &ctrl);
		// SPIのエラー発生
		if (ercd != osOK) {
			break;
			
		// CANエラー発生
		} else if ((ctrl & (TXB_MLOA | TXB_TXERR)) != 0) {
			ercd = osErrorParameter;
			break;
			
		// ちゃんと送信できた
		} else if (((ctrl & TXB_TXREQ) == 0)) {
			break;
			
		// その他
		} else {
			;
			
		}
		ercd = osEventTimeout;
	}
	
SEND_END_PROC:
	// 送信要求をクリアしておく
	if (ercd != osOK) {
		modify_bit(dev, MCP2515_REG_TXBCTRL(mbx_id), TXB_TXREQ, ~TXB_TXREQ);
		// ★ 送信要求のクリアミスったらどうしましょう
	}
	
SEND_EXIT:
	return ercd;
}

// 受信データチェック
osStatus mcp2515_dev_rx_check(MCP2515_DEV dev, uint8_t *p_ret)
{
	MCP2515_CTL *this;
	uint8_t i;
	uint8_t ret = 0;
	
	// パラメータチェック
	if ((dev >= MCP2515_DEV_MAX) ||
		(p_ret == NULL)) {
		return osErrorParameter;
	}
	
	// 制御ブロック取得
	this = get_myself(dev);
	
	// 開始状態でなければ終了
	if (this->status != ST_START) {
		return osErrorParameter;
	}
	
	// 受信チェック
	for (i = 0; i < MCP2515_DEV_RX_MBX_ID_MAX; i++) {
		ret = check_rx_data(dev, i);
		*p_ret |= (ret << i);
	}
	
	return osOK;
}

// 受信チェック関数
osStatus mcp2515_dev_get_rx_data(MCP2515_DEV dev, uint32_t mbx_id, uint32_t *p_can_id, uint8_t *p_data, uint8_t *p_size)
{
	MCP2515_CTL *this;
	osStatus ercd;
	uint8_t is_eid;
	
	// パラメータチェック
	if (mbx_id >= MCP2515_DEV_RX_MBX_ID_MAX) {
		return osErrorParameter;
	}
	
	if ((p_can_id == NULL) ||
		(p_data == NULL) ||
		(p_size == NULL)) {
		return osErrorParameter;
	}
	
	// 制御ブロック取得
	this = get_myself(dev);
	
	// 開始状態でなければ終了
	if (this->status != ST_START) {
		return osErrorParameter;
	}
	
	// 受信チェック
	if (check_rx_data(dev, mbx_id) == 0) {
		return osErrorResource;
	}
	
	// 受信データ取得
	if ((ercd = get_rx_data(dev, mbx_id, &is_eid, p_can_id, p_data, p_size)) != osOK) {
		goto EXIT;
	}
	
	// 受信フラグクリア
	if ((ercd = clr_rx_flag(dev, mbx_id)) != osOK) {
		goto EXIT;
	}
	
EXIT:
	return ercd;
}

// 状態取得関数
osStatus mcp2515_dev_get_status(MCP2515_DEV dev, uint32_t *p_sts)
{
	MCP2515_CTL *this;
	const MCP2515_STATUS_INFO *p_info;
	osStatus ercd;
	uint8_t can_sts = 0;
	uint8_t i;
	uint32_t sts_ret = 0;
	
	// パラメータチェック
	if (dev >= MCP2515_DEV_MAX) {
		return osErrorParameter;
	}
	if (p_sts == NULL) {
		return osErrorParameter;
	}
	
	// 制御ブロック取得
	this = get_myself(dev);
	
	// 開始状態でなければ終了
	if (this->status != ST_START) {
		return osErrorParameter;
	}
	
	// CTRLレジスタ読み込み
	ercd = read_reg(dev, MCP2515_REG_EFLG, &can_sts);
	if (ercd != osOK) {
		goto EXIT;
	}
	
	// 返す状態を設定
	for (i = 0; i < sizeof(status_info)/sizeof(status_info[0]); i++) {
		// 状態情報取得
		p_info = &status_info[i];
		if ((can_sts & p_info->reg_bit) != 0) {
			sts_ret |= p_info->ret_bit;
		}
	}
	*p_sts = sts_ret;
	
EXIT:
	return ercd;
}

// コマンド
static void mcp2515_cmd_open(int argc, char *argv[])
{
	uint32_t ret;
	
	ret = mcp2515_dev_open(MCP2515_DEV_1, 250*1000);
	if (ret != osOK) {
		console_printf("mcp2515_dev_open error\n");
	}
}

static void mcp2515_cmd_read_reg(int argc, char *argv[])
{
	uint32_t ret;
	uint8_t reg;
	uint8_t data = 0;
	
	// 引数チェック
	if (argc < 2) {
		console_printf("mcp2515 read_reg <addr>\n");
		return;
	}
	
	// 値設定
	reg = atoi(argv[2]);
	
	// 読み込み
	ret = read_reg(MCP2515_DEV_1, reg, &data);
	if (ret != osOK) {
		console_printf("read_reg error\n");
		goto CMD_READ_REG_EXIT;
	}
	
	console_printf("%x\n", data);
	
CMD_READ_REG_EXIT:
	return;
}

static void mcp2515_cmd_read_status(int argc, char *argv[])
{
	uint32_t ret;
	uint8_t data = 0;
	
	// 読み込み
	ret = read_status(MCP2515_DEV_1, &data);
	if (ret != osOK) {
		console_printf("read_status error\n");
		goto CMD_READ_REG_EXIT;
	}
	
	console_printf("%x\n", data);
	
CMD_READ_REG_EXIT:
	return;
}

static void mcp2515_cmd_write_reg(int argc, char *argv[])
{
	uint32_t ret;
	uint8_t reg;
	uint8_t data = 0;
	
	// 引数チェック
	if (argc < 3) {
		console_printf("mcp2515 write_reg <addr> <val>\n");
		return;
	}
	
	// 値設定
	reg = atoi(argv[2]);
	data = atoi(argv[3]);
	
	// 書き込み
	ret = write_reg(MCP2515_DEV_1, reg, data);
	if (ret != osOK) {
		console_printf("write_reg error\n");
		goto CMD_READ_REG_EXIT;
	}
	
	console_printf("%x\n", data);
	
CMD_READ_REG_EXIT:
	return;
}

static void mcp2515_cmd_change_bit(int argc, char *argv[])
{
	uint32_t ret;
	uint8_t reg;
	uint8_t data = 0;
	uint8_t msk = 0;
	
	// 引数チェック
	if (argc < 4) {
		console_printf("mcp2515 change_bit <addr> <msk> <data>\n");
		return;
	}
	
	// 値設定
	reg = atoi(argv[2]);
	msk = atoi(argv[3]);
	data = atoi(argv[4]);
	
	// 書き込み
	ret = modify_bit(MCP2515_DEV_1, reg, msk, data);
	if (ret != osOK) {
		console_printf("write_reg error\n");
		goto CMD_READ_REG_EXIT;
	}
	
	console_printf("%x\n", data);
	
CMD_READ_REG_EXIT:
	return;
}

static void mcp2515_cmd_start(int argc, char *argv[])
{
	uint32_t ret;
	
	// 開始
	ret = mcp2515_dev_start(MCP2515_DEV_1);
	if (ret != osOK) {
		console_printf("start error\n");
		goto EXIT;
	}
	
EXIT:
	return;
}

static void mcp2515_cmd_send(int argc, char *argv[])
{
	uint32_t ret;
	uint8_t mbx_id;
	uint32_t can_id;
	uint8_t i;
	uint8_t data[8];
	
	// 引数チェック
	if (argc < 3) {
		console_printf("mcp2515 send <mbx_id> <can_id>\n");
		return;
	}
	
	// 値設定
	mbx_id = atoi(argv[2]);
	can_id = atoi(argv[3]);
	for (i = 0; i < 8; i++) {
		data[i] = i;
	}
	
	// 送信
	ret = mcp2515_dev_send(MCP2515_DEV_1, mbx_id, CAN_COMMON_FRAME_TYPE_STANDARD, can_id, data, 8);
	if (ret != osOK) {
		console_printf("send error\n");
		goto EXIT;
	}
	
EXIT:
	return;
}

// コマンド設定関数
void mcp2515_set_cmd(void)
{
	COMMAND_INFO cmd;
	
	// コマンドの設定
	cmd.input = "mcp2515 open";
	cmd.func = mcp2515_cmd_open;
	console_set_command(&cmd);
	cmd.input = "mcp2515 read_reg";
	cmd.func = mcp2515_cmd_read_reg;
	console_set_command(&cmd);
	cmd.input = "mcp2515 read_status";
	cmd.func = mcp2515_cmd_read_status;
	console_set_command(&cmd);
	cmd.input = "mcp2515 write_reg";
	cmd.func = mcp2515_cmd_write_reg;
	console_set_command(&cmd);
	cmd.input = "mcp2515 change_bit";
	cmd.func = mcp2515_cmd_change_bit;
	console_set_command(&cmd);
	cmd.input = "mcp2515 start";
	cmd.func = mcp2515_cmd_start;
	console_set_command(&cmd);
	cmd.input = "mcp2515 send";
	cmd.func = mcp2515_cmd_send;
	console_set_command(&cmd);
}
