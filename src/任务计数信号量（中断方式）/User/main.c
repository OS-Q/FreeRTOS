/*
*********************************************************************************************************
*
*	ģ������ : ������ģ�顣
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ��ʵ����ҪѧϰFreeRTOS������֪ͨʵ�ּ����ź������жϷ�ʽ��
*              ʵ��Ŀ�ģ�
*                1. ѧϰFreeRTOS������֪ͨʵ�ּ����ź������жϷ�ʽ��
*              ʵ�����ݣ�
*                2. ���°���K1����ͨ�����ڴ�ӡ����ִ�����
*                   ������      ����״̬ ���ȼ�   ʣ��ջ �������
*                   vTaskUserIF     R       1       334     1
*                   IDLE            R       0       120     5
*                   vTaskMsgPro     B       3       486     3
*                   vTaskLED        B       2       484     2
*                   vTaskStart      B       4       490     4
*
*                   ������       ���м���         ʹ����
*                   vTaskUserIF     2021            3%
*                   IDLE            54297           95%
*                   vTaskMsgPro     1               <1%
*                   vTaskLED        0               <1%
*                   vTaskStart      283             <1%
*                   �����������ʹ��SecureCRT��V4���������д�������鿴��ӡ��Ϣ��
*                    vTaskTaskUserIF ���񣺰�����Ϣ����
*                    vTaskLED ����       ��LED��˸
*                    vTaskMsgPro ����    ��ʹ�ú���ulTaskNotifyTake���ն�ʱ���жϷ�������Ϣ
*                    vTaskStart ����     ������ɨ��
*                 3. ��������ת̬�Ķ������£������洮�ڴ�ӡ��ĸB, R, D, S��Ӧ��
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 4. ��ʵ��ͨ������xTaskNotifyGive��ulTaskNotifyTake��ʵ�ּ����ź������ܡ�
*                 5. ��ʵ������ź�������ʵ�������ͬ����
*
*              ע�����
*                 1. ��ʵ���Ƽ�ʹ�ô������SecureCRT��Ҫ�����ڴ�ӡЧ�������롣�������
*                    V4��������������С�
*                 2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��    ����         ����            ˵��
*       V1.0    2015-08-19   Eric2013    1. ST�̼��⵽V3.6.1�汾
*                                        2. BSP������V1.2
*                                        3. FreeRTOS�汾V8.2.2
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "includes.h"

/*
**********************************************************************************************************
											��������
**********************************************************************************************************
*/
static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void AppTaskCreate (void);
static void TIM_CallBack1(void);

/*
**********************************************************************************************************
											��������
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;

/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: ��׼c������ڡ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int main(void)
{
	/* Ӳ����ʼ����ʼ�� */
	bsp_Init(); 
	
	/* ��ʼ��һ����ʱ���жϣ����ȸ��ڵδ�ʱ���жϣ������ſ��Ի��׼ȷ��ϵͳ��Ϣ */
	vSetupSysInfoTest();
	
	/* �������� */
	AppTaskCreate();
	
    /* �������ȣ���ʼִ������ */
    vTaskStartScheduler();

	/* ���ϵͳ���������ǲ������е�����ģ����е����Ｋ�п����ǿ�������heap�ռ䲻����ɴ���ʧ�� */
	while(1);
}

/*
*********************************************************************************************************
*	�� �� ��: vTaskTaskUserIF
*	����˵��: ������Ϣ����		
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 1  (��ֵԽС���ȼ�Խ�ͣ������uCOS�෴)
*********************************************************************************************************
*/
static void vTaskTaskUserIF(void *pvParameters)
{
	uint8_t ucKeyCode;
	uint8_t pcWriteBuffer[500];

    while(1)
    {
		ucKeyCode = bsp_GetKey();
		
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				/* K1������ ��ӡ����ִ����� */
				case KEY_DOWN_K1:
					printf("=================================================\r\n");
					printf("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
					vTaskList((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
				
					printf("\r\n������       ���м���         ʹ����\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
					break;
				
				/* K2�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro����Ϣ */
				case KEY_DOWN_K2:
					printf("K2�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro����Ϣ\r\n");
					bsp_StartHardTimer(1 ,50000, (void *)TIM_CallBack1);
				
				/* �����ļ�ֵ������ */
				default:                     
					break;
			}
		}
		
		vTaskDelay(10);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: vTaskLED
*	����˵��: LED��˸
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 2  
*********************************************************************************************************
*/
static void vTaskLED(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 200;

	/* ��ȡ��ǰ��ϵͳʱ�� */
    xLastWakeTime = xTaskGetTickCount();
	
    while(1)
    {
       	bsp_LedToggle(2);
		bsp_LedToggle(3);
		
		/* vTaskDelayUntil�Ǿ����ӳ٣�vTaskDelay������ӳ١�*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/*
*********************************************************************************************************
*	�� �� ��: vTaskMsgPro
*	����˵��: ʹ�ú���ulTaskNotifyTake���ն�ʱ���жϷ��͵���Ϣ��
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 3  
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	const TickType_t xBlockTime = pdMS_TO_TICKS(500); /* �������ȴ�ʱ��Ϊ500ms */
	uint32_t ulNotifiedValue;
	
    while(1)
    {
		ulNotifiedValue = ulTaskNotifyTake(pdFALSE,  /* 1. �˲�������ΪpdFALSE�����յ���notification value��һ 
											            2. �˲�������ΪpdTRUE�����յ���notification value���� */
						                   xBlockTime); /* ���޵ȴ� */
		if( ulNotifiedValue > 0 )
        {
			/* ���յ���Ϣ */
			printf("����vTaskMsgPro���յ���Ϣ��ulNotifiedValue = %d\r\n", ulNotifiedValue);
        }
        else
        {
			/* ��ʱ */
			bsp_LedToggle(1);
			bsp_LedToggle(4);
        }
    }
}

/*
*********************************************************************************************************
*	�� �� ��: vTaskStart
*	����˵��: ��������Ҳ����������ȼ�����
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 4  
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
    while(1)
    {
		/* ����ɨ�� */
		bsp_KeyScan();
        vTaskDelay(10);
    }
}

/*
*********************************************************************************************************
*	�� �� ��: TIM_CallBack1
*	����˵��: ��ʱ���жϵĻص��������˺�����bsp_StartHardTimer�����á�		  			  
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void TIM_CallBack1(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	/* 
		��������֪ͨ, �����������Σ���������xHandleTaskMsgPro�״ν��յ���
	    notification value��������
	*/
	vTaskNotifyGiveFromISR(xHandleTaskMsgPro, &xHigherPriorityTaskWoken);
	vTaskNotifyGiveFromISR(xHandleTaskMsgPro, &xHigherPriorityTaskWoken);
	vTaskNotifyGiveFromISR(xHandleTaskMsgPro, &xHigherPriorityTaskWoken);

	/* ���xHigherPriorityTaskWoken = pdTRUE����ô�˳��жϺ��е���ǰ������ȼ�����ִ�� */
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
    xTaskCreate(    vTaskTaskUserIF,   /* ������  */
                    "vTaskUserIF",     /* ������    */
                    512,               /* stack��С����λword��Ҳ����4�ֽ� */
                    NULL,              /* �������  */
                    1,                 /* �������ȼ�*/
                    NULL );            /* ������  */
	
	
	xTaskCreate(    vTaskLED,    /* ������  */
                    "vTaskLED",  /* ������    */
                    512,         /* stack��С����λword��Ҳ����4�ֽ� */
                    NULL,        /* �������  */
                    2,           /* �������ȼ�*/
                    &xHandleTaskLED );   /* ������  */
	
	xTaskCreate(    vTaskMsgPro,     /* ������  */
                    "vTaskMsgPro",   /* ������    */
                    512,             /* stack��С����λword��Ҳ����4�ֽ� */
                    NULL,            /* �������  */
                    3,               /* �������ȼ�*/
                    &xHandleTaskMsgPro );  /* ������  */
	
	
	xTaskCreate(    vTaskStart,     /* ������  */
                    "vTaskStart",   /* ������    */
                    512,            /* stack��С����λword��Ҳ����4�ֽ� */
                    NULL,           /* �������  */
                    4,              /* �������ȼ�*/
                    NULL );         /* ������  */
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
