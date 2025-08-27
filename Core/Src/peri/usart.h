/*
 * usart.h
 *
 *  Created on: 2024/10/09
 *      Author: user
 */

#ifndef PERI_USART_H_
#define PERI_USART_H_

// チャネル
typedef enum {
	USART_CH_1 = 0,
	USART_CH_2,
//	USART_CH_3,
//	USART_CH_4 = 0,
//	USART_CH_5,
//	USART_CH_6,
//	USART_CH_7,
//	USART_CH_8,
	USART_CH_MAX,
} USART_CH;

// データ長
typedef enum {
	USART_LEN_8 = 0,
	USART_LEN_7,
	USART_LEN_MAX,
} USART_LEN;

// ストップビット
typedef enum {
	USART_STOPBIT_1 = 0,
	USART_STOPBIT_2,
	USART_STOPBIT_MAX,
} USART_STOPBIT;

// パリティ
typedef enum {
	USART_PARITY_DISABLE = 0,
	USART_PARITY_EVEN,
	USART_PARITY_ODD,
	USART_PARITY_MAX,
} USART_PARITY;

// オープンパラメータ
typedef struct {
	USART_LEN		len;		// データ長
	USART_STOPBIT	stopbit;	// ストップビット
	USART_PARITY	parity;		// パリティ
	uint32_t		baudrate;	// ボーレート
} USART_OPEN_PAR;

typedef void (*USART_CALLBACK)(USART_CH ch, void* p_ctx);

extern osStatus usart_init(void);
extern osStatus usart_open(USART_CH ch, USART_OPEN_PAR *open_par, USART_CALLBACK send_cb, USART_CALLBACK recv_cb, USART_CALLBACK err_cb, void* p_ctx);
extern int32_t usart_send(USART_CH ch, uint8_t *p_data, uint32_t size);
extern int32_t usart_recv(USART_CH ch, uint8_t *p_data, uint32_t size);
extern osStatus usart_close(USART_CH ch);

#endif /* PERI_USART_H_ */
