#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "sdram.h"
#include "usmart.h"
#include "adc.h"
#include "dac.h"
/************************************************
 ALIENTEK ������STM32F429������ʵ��21
 DACʵ��-HAL�⺯����
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com  
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 �������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

TIM_HandleTypeDef htim6;

int main(void)
{
    u16 adcx;
	float temp;
 	u8 t=0;	 
	u16 dacval=0;
	u8 key;	
    HAL_Init();                     //��ʼ��HAL��   
    Stm32_Clock_Init(360,25,2,8);   //����ʱ��,180Mhz
    delay_init(180);                //��ʼ����ʱ����
    uart_init(115200);              //��ʼ��USART
    usmart_dev.init(90); 		    //��ʼ��USMART	
    LED_Init();                     //��ʼ��LED 
    KEY_Init();                     //��ʼ������
    SDRAM_Init();                   //��ʼ��SDRAM
    LCD_Init();                     //��ʼ��LCD
    MY_ADC_Init();                  //��ʼ��ADC1
    DAC1_Init();                    //��ʼ��DAC1
    WaveTable_Init();               //��ʼ������
    
    // TIM6 initialization for DAC trigger
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 0;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 179; // Default for 1kHz with 180MHz clock
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
    {
        while(1); // Error handler
    }
    HAL_TIM_Base_Start(&htim6);
    
	POINT_COLOR=RED; 
	LCD_ShowString(30,50,200,16,16,"Apollo STM32F4/F7");	
	LCD_ShowString(30,70,200,16,16,"DAC TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2016/1/24");	 
	LCD_ShowString(30,130,200,16,16,"WK_UP:+  KEY1:-");	 
	POINT_COLOR=BLUE;//��������Ϊ��ɫ      	 
	LCD_ShowString(30,150,200,16,16,"DAC VAL:");	      
	LCD_ShowString(30,170,200,16,16,"DAC VOL:0.000V");	      
	LCD_ShowString(30,190,200,16,16,"ADC VOL:0.000V"); 	
    
    // 信号发生器参数
    u16 freq = 100; // 默认1kHz
    const u16 MIN_FREQ = 100;   // 最小频率100Hz
    const u16 MAX_FREQ = 1000;  // 最大频率1kHz
    const u16 FREQ_STEP = 100;  // 步进100Hz
    u16 amp = 2000;  // 默认2V峰峰值
    u8 wave_type = 0; // 默认正弦波
    
    LCD_ShowString(30,210,200,16,16,"Wave Type: Sine");
    LCD_ShowString(30,230,200,16,16,"Freq: 100Hz");
    LCD_ShowString(30,250,200,16,16,"Amp: 2.0V");
    
    while(1)
    {
        t++;
        key=KEY_Scan(0);
        
        // 波形选择
        if(key==KEY0_PRES)
        {
            wave_type = (wave_type + 1) % 3;
            switch(wave_type)
            {
                case 0: LCD_ShowString(30,210,200,16,16,"Wave Type: Sine    "); break;
                case 1: LCD_ShowString(30,210,200,16,16,"Wave Type: Triangle"); break;
                case 2: LCD_ShowString(30,210,200,16,16,"Wave Type: Square  "); break;
            }
        }
        
        // 频率调节
        if(key==WKUP_PRES)
        {
            if(freq < MAX_FREQ) freq += FREQ_STEP; // 增加频率
            LCD_ShowxNum(70,230,freq,4,16,0);
            LCD_ShowString(114,230,200,16,16,"Hz   ");
            
            // 更新TIM6频率
            htim6.Init.Period = (180000 / freq) - 1; // 180MHz/(freq*WAVE_TABLE_SIZE)
            HAL_TIM_Base_Init(&htim6);
        }
        else if(key==KEY1_PRES)
        {
            if(freq > MIN_FREQ) freq -= FREQ_STEP; // 减小频率
            LCD_ShowxNum(70,230,freq,4,16,0);
            LCD_ShowString(114,230,200,16,16,"Hz   ");
            
            // 更新TIM6频率
            htim6.Init.Period = (180000 / freq) - 1; // 180MHz/(freq*WAVE_TABLE_SIZE)
            HAL_TIM_Base_Init(&htim6);
        }
        
        // 幅度调节
        if(key==KEY2_PRES)
        {
            if(amp < 3300) amp += 200; // 步进0.2V
            else amp = 0; // 达到最大值后回到最小值
            LCD_ShowxNum(70,250,amp/1000,1,16,0);
            LCD_ShowxNum(86,250,(amp%1000)/100,1,16,0);
            LCD_ShowString(102,250,200,16,16,"    ");
        }
        
        // 生成波形
        DAC1_Generate_Wave(freq, amp, wave_type);
        
        // 显示更新
        if(t==10)
        {
            LED0=!LED0;
            t=0;
        }
        
        delay_ms(1); // 更快的更新速率以获得更好的波形
    }
}
