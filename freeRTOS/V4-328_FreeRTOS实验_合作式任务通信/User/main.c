/*
*********************************************************************************************************
*
*	ģ������ : ������ģ�顣
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ��ʵ����ҪѧϰFreeRTOS�ĺ���ʽ����ͨѶ
*              ʵ��Ŀ�ģ�
*                1. ѧϰFreeRTOS�ĺ���ʽ����ͨѶ
*              ʵ�����ݣ�
*                2. ���°���K1����ͨ�����ڴ�ӡ����ִ�����
*                   ������      ����״̬ ���ȼ�   ʣ��ջ �������
*                   vTaskUserIF     R       1       334     1
*                   IDLE            R       0       86      5
*                   vTaskLED        B       2       484     2
*                   vTaskMsgPro     B       3       480     3
*                   vTaskStart      B       4       490     4
*
*                   ������       ���м���         ʹ����
*                   vTaskUserIF     1240            <1%
*                   IDLE            238159          98%
*                   vTaskLED        1               <1%
*                   vTaskMsgPro     40              <1%
*                   vTaskStart      1207            <1%
*                   �����������ʹ��SecureCRT��V4���������д�������鿴��ӡ��Ϣ��
*                    vTaskTaskUserIF ���񣺰�����Ϣ����
*                    vTaskLED ����       ��LED��˸
*                    vTaskMsgPro ����    ��ʹ�ú���xSemaphoreTake��������vTaskTaskUserIF���͵�ͬ���ź�
*                    vTaskStart ����     ������ɨ��
*                 3. ��������ת̬�Ķ������£������洮�ڴ�ӡ��ĸB, R, D, S��Ӧ��
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 4. K2������ ֱ�ӷ���ͬ���źŸ�����vTaskMsgPro ��
*                 5. ����ʽ������ȣ�����ʽ����֮�䲻���໥��ռ�����ǿ��Ա����������ȼ���������ռ��
*                 6. ���һ��Ӧ����ͬʱ���к���ʽ�������������������Щ��Ҫ������ʽ���Ⱥ������ڿ�
*                    ������Ĺ��Ӻ������档
*                 7. ��ʵ����ʾ����ʽ����vCoRoutineLEDSyn������ʽ����vCoRoutineLED������Ϣ�������ݡ�
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
static void AppCoTaskCreate(void);
static void AppTaskCreate (void);
static void AppObjCreate (void);
static void vCoRoutineLED(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex);
static void vCoRoutineLEDSyn(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex);

/*
**********************************************************************************************************
											��������
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static SemaphoreHandle_t  xSemaphore = NULL;
static xQueueHandle xCoRoutineQueue = NULL;

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
	
	/* ��������ʽ���� */
	AppCoTaskCreate();
	
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
				
				/* K2������ ֱ�ӷ���ͬ���źŸ�����vTaskMsgPro */
				case KEY_DOWN_K2:
					printf("K2�����£�ֱ�ӷ���ͬ���źŸ�����vTaskMsgPro\r\n");
					xSemaphoreGive(xSemaphore);
				
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
		bsp_LedToggle(1);
		bsp_LedToggle(2);

		/* vTaskDelayUntil�Ǿ����ӳ٣�vTaskDelay������ӳ١�*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/*
*********************************************************************************************************
*	�� �� ��: vTaskMsgPro
*	����˵��: ʹ�ú���xSemaphoreTake��������vTaskTaskUserIF���͵�ͬ���ź�
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 3  
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	BaseType_t xResult;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(300); /* �������ȴ�ʱ��Ϊ300ms */
	
    while(1)
    {
		xResult = xSemaphoreTake(xSemaphore, (TickType_t)xMaxBlockTime);
		
		if(xResult == pdTRUE)
		{
			/* ���յ�ͬ���ź� */
			printf("���յ�ͬ���ź�\r\n");
		}
		else
		{
			/* ��ʱ */
			bsp_LedToggle(3);
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
*	�� �� ��: vCoRoutineLEDSyn
*	����˵��: ����ʽ����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void vCoRoutineLEDSyn(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex)
{
	 /* ����ʽ�����еı���������static���͵� */
	 static uint8_t ucNumberToPost = 0;
	 static portBASE_TYPE xResult;

    /* ÿ������ʽ����ʼ��ʱ�������� */
    crSTART( xHandle );

    for( ;; )
    {
        crQUEUE_SEND( xHandle,
                      xCoRoutineQueue,
                      &ucNumberToPost,
                      0,
                      &xResult );

        if( xResult != pdPASS )
        {
            /* ��Ϣ����ʧ�� */
        }

        ucNumberToPost++;

        /* �ӳ� */
        crDELAY(xHandle, 1000);
    }

    /* ÿ������ʽ���������ʱ�������� */
    crEND();
}

/*
*********************************************************************************************************
*	�� �� ��: vCoRoutineLED
*	����˵��: ����ʽ����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void vCoRoutineLED(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex)
{
	/* ����ʽ�����еı���������static���͵� */
	static portBASE_TYPE xResult;
	static uint8_t ucRecieve;

	/* ÿ������ʽ����ʼ��ʱ�������� */
	crSTART( xHandle );

	for( ;; )
	{
		/* �������� */
		crQUEUE_RECEIVE( xHandle,
						 xCoRoutineQueue,
						 &ucRecieve,
						 portMAX_DELAY,
						 &xResult );

		if(xResult == pdPASS)
		{
			portENTER_CRITICAL();
			printf("����ʽ����ucRecieve = %d\r\n", ucRecieve);
			portEXIT_CRITICAL();
		}
	}

	/* ÿ������ʽ���������ʱ�������� */
	crEND();
}

/*
*********************************************************************************************************
*	�� �� ��: vFlashCoRoutine
*	����˵��: ��������ʽ����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppCoTaskCreate( void )
{
	/* �������� */
	xCoRoutineCreate(vCoRoutineLED, 0, 0 );
	
	/* �������� */
	xCoRoutineCreate(vCoRoutineLEDSyn, 1, 0 );
}

/*
*********************************************************************************************************
*	�� �� ��: vApplicationIdleHook
*	����˵��: 1) ����ʽ������ȣ�����ʽ����֮�䲻���໥��ռ�����ǿ��Ա����������ȼ���������ռ��
*             2) ���һ��Ӧ����ͬʱ���к���ʽ�������������������Щ��Ҫ������ʽ���Ⱥ������ڿ�������
*                �Ĺ��Ӻ������档
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void vApplicationIdleHook( void )
{
	vCoRoutineSchedule();
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    ��: ��
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
	/* ������ֵ�ź������״δ����ź�������ֵ��0 */
	xSemaphore = xSemaphoreCreateBinary();
	
	if(xSemaphore == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }
	
	/* ��Ϣ���г���Ϊ5��ÿ������ռ��1���ֽ� */
	xCoRoutineQueue = xQueueCreate(5, sizeof(uint8_t));	
	
	if(xCoRoutineQueue == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
