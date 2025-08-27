/*
 * console.h
 *
 *  Created on: 2024/10/09
 *      Author: user
 */

#ifndef APL_CONSOLE_H_
#define APL_CONSOLE_H_

// コマンドの関数定義
typedef void (*COMMAND)(int argc, char *argv[]);

// コマンド関数情報
typedef struct {
	char* input;
	COMMAND func;
} COMMAND_INFO;

extern osStatus console_init(void);
extern void console_printf(const char *fmt, ...);

extern osStatus console_set_command(COMMAND_INFO *cmd_info);

#endif /* APL_CONSOLE_H_ */
