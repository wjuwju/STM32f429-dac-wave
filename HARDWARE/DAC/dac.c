#include "dac.h"
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

DAC_HandleTypeDef DAC1_Handler;//DAC���

//��ʼ��DAC
void DAC1_Init(void)
{
    DAC_ChannelConfTypeDef DACCH1_Config;
    
    DAC1_Handler.Instance=DAC;
    HAL_DAC_Init(&DAC1_Handler);                 //��ʼ��DAC
    
    DACCH1_Config.DAC_Trigger=DAC_TRIGGER_NONE;             //��ʹ�ô�������
    DACCH1_Config.DAC_OutputBuffer=DAC_OUTPUTBUFFER_DISABLE;//DAC1�������ر�
    HAL_DAC_ConfigChannel(&DAC1_Handler,&DACCH1_Config,DAC_CHANNEL_1);//DACͨ��1����
    
    HAL_DAC_Start(&DAC1_Handler,DAC_CHANNEL_1);  //����DACͨ��1
}

//DAC�ײ�������ʱ�����ã����� ����
//�˺����ᱻHAL_DAC_Init()����
//hdac:DAC���
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{      
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_DAC_CLK_ENABLE();             //ʹ��DACʱ��
    __HAL_RCC_GPIOA_CLK_ENABLE();			//����GPIOAʱ��
	
    GPIO_Initure.Pin=GPIO_PIN_4;            //PA4
    GPIO_Initure.Mode=GPIO_MODE_ANALOG;     //ģ��
    GPIO_Initure.Pull=GPIO_NOPULL;          //����������
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
}

//����ͨ��1�����ѹ
//vol:0~3300,����0~3.3V
void DAC1_Set_Vol(u16 vol)
{
	double temp=vol;
	temp/=1000;
	temp=temp*4096/3.3;
    HAL_DAC_SetValue(&DAC1_Handler,DAC_CHANNEL_1,DAC_ALIGN_12B_R,temp);//12λ�Ҷ������ݸ�ʽ����DACֵ
}

// 波形表大小
#define WAVE_TABLE_SIZE 256
#include "math.h"

// 正弦波表
u16 sine_table[WAVE_TABLE_SIZE];
// 三角波表
static u16 triangle_table[WAVE_TABLE_SIZE];

// 定时器句柄
extern TIM_HandleTypeDef TIM3_Handler;

// 初始化波形表
void WaveTable_Init(void)
{
    for(int i=0; i<WAVE_TABLE_SIZE; i++)
    {
        // 正弦波表
        sine_table[i] = (u16)(2048 * (1 + sin(2*3.14159*i/WAVE_TABLE_SIZE)));
        
        // 三角波表
        if(i < WAVE_TABLE_SIZE/2)
            triangle_table[i] = (u16)(4096 * i / (WAVE_TABLE_SIZE/2));
        else
            triangle_table[i] = (u16)(4096 * (WAVE_TABLE_SIZE - i) / (WAVE_TABLE_SIZE/2));
    }
}

// 使用定时器触发DAC转换
// freq: 频率(Hz)
// amp: 幅度(0~3300)
// type: 波形类型 0:正弦波 1:三角波 2:方波
void DAC1_Generate_Wave(u16 freq, u16 amp, u8 type)
{
    static u16 index = 0;
    u16 value;
    
    // 根据类型选择波形
    switch(type)
    {
        case 0: // 正弦波
            value = (u16)((amp * sine_table[index]) / 4096);
            break;
            
        case 1: // 三角波
            value = (u16)((amp * triangle_table[index]) / 4096);
            break;
            
        case 2: // 方波
            value = (index < WAVE_TABLE_SIZE/2) ? amp : 0;
            break;
            
        default:
            value = 0;
            break;
    }
    
    DAC1_Set_Vol(value);
    
    // 更新索引
    index++;
    if(index >= WAVE_TABLE_SIZE) index = 0;
    
    // 配置定时器频率
    // 计算实际需要的定时器中断频率 = 波形表大小 * 期望波形频率
    uint32_t timer_freq = WAVE_TABLE_SIZE * freq;
    
    // 计算预分频值和周期值
    uint32_t prescaler = (SystemCoreClock / timer_freq) / 0x10000;
    uint32_t period = (SystemCoreClock / (prescaler + 1)) / timer_freq - 1;
    
    // 确保period值在有效范围内
    if(period > 0xFFFF) period = 0xFFFF;
    if(period < 1) period = 1;
    
    // 停止定时器再重新配置
    HAL_TIM_Base_Stop_IT(&TIM3_Handler);
    __HAL_TIM_SET_PRESCALER(&TIM3_Handler, prescaler);
    __HAL_TIM_SET_AUTORELOAD(&TIM3_Handler, period);
    HAL_TIM_Base_Start_IT(&TIM3_Handler);
}
