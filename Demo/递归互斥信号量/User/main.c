/*
*********************************************************************************************************
*
*	ģ������ : ������ģ�顣
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ��ʵ����ҪѧϰFreeRTOS�ĵݹ黥���ź���
*              ʵ��Ŀ�ģ�
*                1. ѧϰFreeRTOS�ĵݹ黥���ź���
*                2. �ݹ黥���ź�������ʵ���ǻ����ź�������Ƕ�׻����ź���
*              ʵ�����ݣ�
*                3. ���°���K1����ͨ�����ڴ�ӡ����ִ�����
*                   ������      ����״̬ ���ȼ�   ʣ��ջ �������
*                   vTaskUserIF     R       1       334     1
*                   IDLE            R       0       120     5
*                   vTaskMsgPro     B       3       458     3
*                   vTaskLED        B       2       458     2
*                   vTaskStart      B       4       490     4
*
*                   ������       ���м���         ʹ����
*                   vTaskUserIF     2053            3%
*                   IDLE            49735           92%
*                   vTaskMsgPro     824             1%
*                   vTaskLED        1188            2%
*                   vTaskStart      1               <1%
*                   �����������ʹ��SecureCRT��V4���������д�������鿴��ӡ��Ϣ��
*                    vTaskTaskUserIF ���񣺰�����Ϣ����
*                    vTaskLED ����       ���ݹ黥���ź�����ʹ��
*                    vTaskMsgPro ����    ���ݹ黥���ź�����ʹ��
*                    vTaskStart ����     ������ɨ��
*                 4. ��������ת̬�Ķ������£������洮�ڴ�ӡ��ĸB, R, D, S��Ӧ��
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
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
static SemaphoreHandle_t  xRecursiveMutex = NULL;

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
					xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
					printf("=================================================\r\n");
					printf("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
					vTaskList((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
				
					printf("\r\n������       ���м���         ʹ����\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
					xSemaphoreGiveRecursive(xRecursiveMutex);	
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
*	����˵��: �ݹ黥���ź�����ʹ��
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 2  
*********************************************************************************************************
*/
static void vTaskLED(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1000;

	/* ��ȡ��ǰ��ϵͳʱ�� */
    xLastWakeTime = xTaskGetTickCount();
	
    while(1)
    {
       	/* �ݹ黥���ź�������ʵ���ǻ����ź�������Ƕ�׻����ź��� */
		xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
		{
			/* -------------------------------------------------------------------------- */
			   //���������Ǳ���������Դ����1�㱻��������Դ
			/* -------------------------------------------------------------------------- */
			printf("����vTaskLED�����У���1�㱻��������Դ���û�������������ӱ�������Դ\r\n");
			
			/* ��1�㱻��������Դ����Ƕ�ױ���������Դ */
			xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
			{
				/* ---------------------------------------------------------------------- */
			    //���������Ǳ���������Դ����2�㱻��������Դ
				/* ---------------------------------------------------------------------- */
				printf("����vTaskLED�����У���2�㱻��������Դ���û�������������ӱ�������Դ\r\n");
				
				/* ��2�㱻��������Դ����Ƕ�ױ���������Դ */
				xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
				{
					printf("����vTaskLED�����У���3�㱻��������Դ���û�������������ӱ�������Դ\r\n");
					bsp_LedToggle(2);
					bsp_LedToggle(3);
				}
				xSemaphoreGiveRecursive(xRecursiveMutex);
			}
			xSemaphoreGiveRecursive(xRecursiveMutex);
		}
		xSemaphoreGiveRecursive(xRecursiveMutex);
		
		/* vTaskDelayUntil�Ǿ����ӳ٣�vTaskDelay������ӳ١�*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/*
*********************************************************************************************************
*	�� �� ��: vTaskMsgPro
*	����˵��: �ݹ黥���ź�����ʹ��
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 3  
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1500;

	/* ��ȡ��ǰ��ϵͳʱ�� */
    xLastWakeTime = xTaskGetTickCount();
	
    while(1)
    {
		/* �ݹ黥���ź�������ʵ���ǻ����ź�������Ƕ�׻����ź��� */
		xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
		{
			/* -------------------------------------- */
			   //���������Ǳ���������Դ����1�㱻��������Դ���û�������������ӱ�������Դ
			/* ---------------------------------------------------------------------------- */
			printf("����vTaskMsgPro�����У���1�㱻��������Դ���û�������������ӱ�������Դ\r\n");
			
			/* ��1�㱻��������Դ����Ƕ�ױ���������Դ */
			xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
			{
				/* ------------------------------------------------------------------------ */
			    //���������Ǳ���������Դ����2�㱻��������Դ���û�������������ӱ�������Դ
				/* ------------------------------------------------------------------------ */
				printf("����vTaskMsgPro�����У���2�㱻��������Դ���û�������������ӱ�������Դ\r\n");
				
				/* ��2�㱻��������Դ����Ƕ�ױ���������Դ */
				xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
				{
					printf("����vTaskMsgPro�����У���3�㱻��������Դ���û�������������ӱ�������Դ\r\n");
					bsp_LedToggle(1);
					bsp_LedToggle(4);
				}
				xSemaphoreGiveRecursive(xRecursiveMutex);
			}
			xSemaphoreGiveRecursive(xRecursiveMutex);	
		}
		xSemaphoreGiveRecursive(xRecursiveMutex);
		
		/* vTaskDelayUntil�Ǿ����ӳ٣�vTaskDelay������ӳ١�*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
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
	/* �����ݹ黥���ź��� */
    xRecursiveMutex = xSemaphoreCreateRecursiveMutex();
	
	if(xRecursiveMutex == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
