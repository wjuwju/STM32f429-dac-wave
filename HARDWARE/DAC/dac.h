#ifndef __DAC_H
#define __DAC_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32F429������
//DAC��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/6
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

extern DAC_HandleTypeDef DAC1_Handler;//DAC���

void DAC1_Init(void);
void DAC1_Set_Vol(u16 vol);
void DAC1_Generate_Wave(u16 freq, u16 amp, u8 type);
#endif
