/*
 * usart_drv.h
 *
 *  Created on: 2024/10/14
 *      Author: user
 */

#ifndef DRV_USART_DRV_H_
#define DRV_USART_DRV_H_

typedef enum {
	USART_DRV_DEV_CONSOLE = 0,
	USART_DRV_DEV_MAX,
} USART_DRV_DEV;

extern osStatus usart_drv_init(void);
extern osStatus usart_drv_open(USART_DRV_DEV dev);
extern int32_t usart_drv_send(USART_DRV_DEV dev, uint8_t *p_data, uint32_t size, int32_t tmout);
extern int32_t usart_drv_recv(USART_DRV_DEV dev, uint8_t *p_data, uint32_t size, int32_t tmout);


#endif /* DRV_USART_DRV_H_ */
