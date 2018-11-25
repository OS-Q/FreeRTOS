/*
*********************************************************************************************************
*
*	ģ������ : ������ģ�顣
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ��ʵ����ҪѧϰFreeRTOS�Ķ�̬�ڴ����(heap_4)
*              ʵ��Ŀ�ģ�
*                1. ѧϰFreeRTOS�Ķ�̬�ڴ����(heap_4)
*                2. FreeRTOS�ṩ��5���ڴ��������V4�������׵Ķ����õķ���4��heap_4�ļ���
*                   ��5�ַ�������ȱ����Թ������µ�ַ�鿴��
*                   http://www.freertos.org/a00111.html
*              ʵ�����ݣ�
*                3. ���°���K1����ͨ�����ڴ�ӡ����ִ�����
*                   ������      ����״̬ ���ȼ�   ʣ��ջ �������
*                   vTaskUserIF     R       1       334     1
*                   IDLE            R       0       120     5
*                   vTaskLED        B       2       484     2
*                   vTaskMsgPro     B       3       460     3
*                   vTaskStart      B       4       490     4
*
*                   ������       ���м���         ʹ����
*                   vTaskUserIF     3363            3%
*                   IDLE            98270           96%
*                   vTaskLED        0               <1%
*                   vTaskMsgPro     167             <1%
*                   vTaskStart      2               <1%
*                   �����������ʹ��SecureCRT��V4���������д�������鿴��ӡ��Ϣ��
*                    vTaskTaskUserIF ���񣺰�����Ϣ����
*                    vTaskLED ����       ��LED��˸
*                    vTaskMsgPro ����    ��ʹ�ú���xSemaphoreTake��������vTaskTaskUserIF���͵�ͬ���ź�
*                    vTaskStart ����     ������ɨ��
*                 4. ��������ת̬�Ķ������£������洮�ڴ�ӡ��ĸB, R, D, S��Ӧ��
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 5. K2������ ֱ�ӷ���ͬ���źŸ�����vTaskMsgPro ��
*                 6. K3�������£����ص�ǰϵͳʣ�ද̬�ڴ��С��
*                 7. ҡ��OK�����£�����1024�ֽ��ڴ棬Ȼ���ͷţ�����ӡ����ǰ��̬�ڴ��ʣ�������
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
static void AppObjCreate (void);

/*
**********************************************************************************************************
											��������
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static SemaphoreHandle_t  xSemaphore = NULL;

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
	uint8_t *pMallocMem;
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
					break;
				
				/* K3�������£����ص�ǰϵͳʣ�ද̬�ڴ��С */
				case KEY_DOWN_K3:
					printf("K2�����£���ǰ��̬�ڴ��СxPortGetFreeHeapSize() = %d\r\n", xPortGetFreeHeapSize());
					break;
				
				/* ҡ��OK�����£�����1024�ֽ��ڴ棬Ȼ���ͷţ�����ӡ����ǰ��̬�ڴ��ʣ����� */
				case JOY_DOWN_OK:
					printf("��ǰ��̬�ڴ��СxPortGetFreeHeapSize() = %d\r\n", xPortGetFreeHeapSize());
					pMallocMem = (uint8_t *)pvPortMalloc(1024);
					printf("����1024�ֽ��ڴ���СxPortGetFreeHeapSize() = %d\r\n", xPortGetFreeHeapSize());
					vPortFree(pMallocMem);
					printf("�ͷ�1024�ֽ��ڴ���СxPortGetFreeHeapSize() = %d\r\n", xPortGetFreeHeapSize());
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
	/* ��ʼ����1��������Դ����ǰ������ԴΪ0����ʱ�����ź����Ĺ��ܵ�ͬ��ֵ�ź��� */
 	xSemaphore = xSemaphoreCreateCounting(1, 0);
	
	if(xSemaphore == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
