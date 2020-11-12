/*
*********************************************************************************************************
*
*	ģ������ : ������ģ�顣
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ��ʵ����ҪѧϰFreeRTOS���¼���־�飨�жϷ�ʽ��
*              ʵ��Ŀ�ģ�
*                1. ѧϰFreeRTOS���¼���־�飨�жϷ�ʽ��
*              ʵ�����ݣ�
*                2. ���°���K1����ͨ�����ڴ�ӡ����ִ�����
*                   ������      ����״̬ ���ȼ�   ʣ��ջ �������
*                   vTaskUserIF     R       1       334     1
*                   IDLE            R       0       120     5
*                   vTaskLED        B       2       484     2
*                   vTaskMsgPro     B       3       476     3
*                   vTaskStart      B       4       490     4
*                   Tmr Svc         B       5       234     6
*
*                   ������       ���м���         ʹ����
*                   vTaskUserIF     2445            1%
*                   IDLE            211291          98%
*                   vTaskLED        0               <1%
*                   vTaskMsgPro     0               <1%
*                   vTaskStart      1065            <1%
*                   Tmr Svc         1               <1%
*                   �����������ʹ��SecureCRT��V4���������д�������鿴��ӡ��Ϣ��
*                    vTaskTaskUserIF ���񣺰�����Ϣ����
*                    vTaskLED ����       ��LED��˸
*                    vTaskMsgPro ����    ��ʹ�ú���xEventGroupWaitBits���ն�ʱ���жϵ��¼���־
*                    vTaskStart ����     ������ɨ��
*                 3. ��������ת̬�Ķ������£������洮�ڴ�ӡ��ĸB, R, D, S��Ӧ��
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 4. K2�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro�����¼���־������bit0��
*                 5. K3�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro�����¼���־������bit1��
*                 6. ����vTaskMsgProֻ�н��յ�bit0��bit1���������˲�ִ�д��ڴ�ӡ��Ϣ��
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
*	Copyright (C), 2015-2020, ������www.OS-Q.comm
*
*********************************************************************************************************
*/
#include "includes.h"

/*
**********************************************************************************************************
											�궨��
**********************************************************************************************************
*/
#define BIT_0	(1 << 0)
#define BIT_1	(1 << 1)
#define BIT_ALL (BIT_0 | BIT_1)

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
static void AppObjCreate (void);
static void TIM_CallBack1(void);
static void TIM_CallBack2(void);

/*
**********************************************************************************************************
											��������
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static EventGroupHandle_t xCreatedEventGroup = NULL;

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

	/* ��������ͨ�Ż��� */
	AppObjCreate();

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

				/* K2�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro�����¼���־ */
				case KEY_DOWN_K2:
					printf("K2�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro�����¼���־\r\n");
					bsp_StartHardTimer(1 ,50000, (void *)TIM_CallBack1);
					break;

				/* K3�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro�����¼���־ */
				case KEY_DOWN_K3:
					printf("K3�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro�����¼���־\r\n");
					bsp_StartHardTimer(2 ,50000, (void *)TIM_CallBack2);
					break;


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
*	����˵��: ʹ�ú���xEventGroupWaitBits���ն�ʱ���жϷ��͵��¼���־
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 3
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* ����ӳ�100ms */

    while(1)
    {
		/* ��K2������������bit0��K3������������bit1 */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* �¼���־���� */
							         BIT_ALL,            /* �ȴ�bit0��bit1������ */
							         pdTRUE,             /* �˳�ǰbit0��bit1����� */
							         pdTRUE,             /* ����ΪpdTRUE��ʾ�ȴ�bit1��bit0��������*/
							         xTicksToWait); 	 /* �ȴ��ӳ�ʱ�� */

		if((uxBits & BIT_ALL) == BIT_ALL)
		{
			/* ���յ�bit1��bit0�������õ���Ϣ */
			printf("���յ�bit0��bit1�������õ���Ϣ\r\n");
		}
		else
		{
			/* ��ʱ������ע������յ�һ���������µ���Ϣʱ������uxBits����ӦbitҲ�Ǳ����õ� */
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
*	�� �� ��: TIM_CallBack1��TIM_CallBack2
*	����˵��: ��ʱ���жϵĻص��������˺�����bsp_StartHardTimer�����á�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void TIM_CallBack1(void)
{
	BaseType_t xResult;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	/* ������vTaskMsgPro�����¼���־ */
	xResult = xEventGroupSetBitsFromISR(xCreatedEventGroup, /* �¼���־���� */
									    BIT_0 ,             /* ����bit0 */
									    &xHigherPriorityTaskWoken );

	/* ��Ϣ���ɹ����� */
	if( xResult != pdFAIL )
	{
		/* ���xHigherPriorityTaskWoken = pdTRUE����ô�˳��жϺ��е���ǰ������ȼ�����ִ�� */
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

static void TIM_CallBack2(void)
{
	BaseType_t xResult;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	/* ������vTaskMsgPro�����¼���־ */
	xResult = xEventGroupSetBitsFromISR(xCreatedEventGroup, /* �¼���־���� */
									    BIT_1,              /* ����bit1 */
									    &xHigherPriorityTaskWoken );

	/* ��Ϣ���ɹ����� */
	if( xResult != pdFAIL )
	{
		/* ���xHigherPriorityTaskWoken = pdTRUE����ô�˳��жϺ��е���ǰ������ȼ�����ִ�� */
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
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

/*
*********************************************************************************************************
*	�� �� ��: AppObjCreate
*	����˵��: ��������ͨ�Ż���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
	/* �����¼���־�� */
	xCreatedEventGroup = xEventGroupCreate();

	if(xCreatedEventGroup == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }
}

/***************************** ������www.OS-Q.comm (END OF FILE) *********************************/
