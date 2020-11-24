
#include "bsp.h"


/* ��ʱ��Ƶ�ʣ�50usһ���ж� */
#define  timerINTERRUPT_FREQUENCY	20000

/* �ж����ȼ� */
#define  timerHIGHEST_PRIORITY		1

/* ��ϵͳ���� */
volatile uint32_t ulHighFrequencyTimerTicks = 0UL;

/*******************************************************************************
**������Ϣ ��
**�������� ��
**������� ����
**������� ����
********************************************************************************/
void vSetupSysInfoTest(void)
{
	bsp_SetTIMforInt(TIM6, timerINTERRUPT_FREQUENCY, timerHIGHEST_PRIORITY, 0);
}
/*******************************************************************************
**������Ϣ ��
**�������� ��
**������� ����
**������� ����
********************************************************************************/
void TIM6_IRQHandler( void )
{
	if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		ulHighFrequencyTimerTicks++;
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	}
}

/*----------------------- (C) COPYRIGHT 2020 www.OS-Q.comm --------------------*/
