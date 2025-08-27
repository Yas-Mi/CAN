/*
 * iodefine.h
 *
 *  Created on: 2024/10/09
 *      Author: user
 */

#ifndef PERI_IODEFINE_H_
#define PERI_IODEFINE_H_

#include "stm32f7xx.h"	// これがiodefineなるもの

// 便利マクロ
// ビットセット
#define set_bit(reg,bno)				((reg)|=(1<<bno))
#define set_bit_t(type,reg,bno)			((reg)|=(type)(1<<bno))

// ビットクリア
#define clr_bit(reg,bno)				((reg)&=(~(1<<bno)))
#define clr_bit_t(type,reg,bno)			((reg)&=(type)(~(1<<bno)))

// フィールド値取得
#define get_field(reg,msk)				(((reg)&(msk))>>get_sft(msk))
#define get_field_t(type,reg,msk)		((type)(((reg)&(msk))>>get_sft(msk)))

// フィールド値設定
#define set_field(reg,msk,val)			((reg)=(((reg)&(~(msk)))|((val)<<get_sft(msk))))
#define set_field_t(type,reg,msk,val)	((reg)=(type)((type)((reg)&(~(msk)))|(type)((val)<<get_sft(msk))))

// シフト数取得 (*)LSB側の1を見つけることでシフト値を計算
#define get_sft(bits)	(((bits)&(1UL<<0))?0:\
						 ((bits)&(1UL<<1))?1:\
						 ((bits)&(1UL<<2))?2:\
						 ((bits)&(1UL<<3))?3:\
						 ((bits)&(1UL<<4))?4:\
						 ((bits)&(1UL<<5))?5:\
						 ((bits)&(1UL<<6))?6:\
						 ((bits)&(1UL<<7))?7:\
						 ((bits)&(1UL<<8))?8:\
						 ((bits)&(1UL<<9))?9:\
						 ((bits)&(1UL<<10))?10:\
						 ((bits)&(1UL<<11))?11:\
						 ((bits)&(1UL<<12))?12:\
						 ((bits)&(1UL<<13))?13:\
						 ((bits)&(1UL<<14))?14:\
						 ((bits)&(1UL<<15))?15:\
						 ((bits)&(1UL<<16))?16:\
						 ((bits)&(1UL<<17))?17:\
						 ((bits)&(1UL<<18))?18:\
						 ((bits)&(1UL<<19))?19:\
						 ((bits)&(1UL<<20))?20:\
						 ((bits)&(1UL<<21))?21:\
						 ((bits)&(1UL<<22))?22:\
						 ((bits)&(1UL<<23))?23:\
						 ((bits)&(1UL<<24))?24:\
						 ((bits)&(1UL<<25))?25:\
						 ((bits)&(1UL<<26))?26:\
						 ((bits)&(1UL<<27))?27:\
						 ((bits)&(1UL<<28))?28:\
						 ((bits)&(1UL<<29))?29:\
						 ((bits)&(1UL<<30))?30:\
						 ((bits)&(1UL<<31))?31:32)

#endif /* PERI_IODEFINE_H_ */
