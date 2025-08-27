/*
 * can_common.h
 *
 *  Created on: 2024/3/13
 *      Author: ronald
 */

#ifndef CAN_COMMON_H_
#define CAN_COMMON_H_

// 外付けCAN、内蔵CANで共通する定義をここでする

// マクロ
#define CAN_COMMON_AUTO_MBX_ID			(0xFFFFFFFF)	// MBX_IDを自動で選択

// 状態
#define CAN_COMMON_STS_RXOVER	(0x00000001)			// 受信バッファオーバフラグ
#define CAN_COMMON_STS_TXBO		(0x00000002)			// バスオフ
#define CAN_COMMON_STS_TXEP		(0x00000004)			// 送信エラー パッシブエラー
#define CAN_COMMON_STS_RXEP		(0x00000008)			// 受信エラー パッシブ
#define CAN_COMMON_STS_TXWAR	(0x00000010)			// 送信エラー警告フラグ
#define CAN_COMMON_STS_RXWAR	(0x00000020)			// 受信エラー警告フラグ
#define CAN_COMMON_STS_EWARN	(0x00000040)			// エラー警告フラグ

// frame type
typedef enum {
	CAN_COMMON_FRAME_TYPE_STANDARD = 0,
	CAN_COMMON_FRAME_TYPE_EXTENDED,
	CAN_COMMON_FRAME_TYPE_MAX,
} CAN_COMMON_FRAME_TYPE;

// 受信メールボックス情報
#define FILTER_LAST		(0x00000000)	// フィルタは配列で定義するため、最後の要素に本定義を設定すること
typedef struct {
	uint32_t					mbox_id;		// メールボックスID
	CAN_COMMON_FRAME_TYPE		frame_type;		// frame type
	uint32_t					mask;			// mask
	const uint32_t				*p_filter;		// filter
} CAN_COMMON_RX_MAILBOX_INFO;

#endif /* CAN_COMMON_H_ */
