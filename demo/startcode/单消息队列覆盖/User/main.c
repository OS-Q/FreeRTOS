
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
static TaskHandle_t  xHandleTaskLED = NULL;
static TaskHandle_t  xHandleTaskMsgPro = NULL;
static QueueHandle_t xQueue1 = NULL, xQueue2 = NULL;

typedef struct Msg
{
	uint8_t  ucMessageID;
	uint16_t usData[2];
	uint32_t ulData[2];
}MSG_T;

MSG_T   g_tMsg; /* ����һ���ṹ��������Ϣ���� */

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
	MSG_T   *ptMsg;
	uint8_t ucCount = 0;
	uint8_t ucKeyCode;
	uint8_t pcWriteBuffer[500];

	/* ��ʼ���ṹ��ָ�� */
	ptMsg = &g_tMsg;

	/* ��ʼ������ */
	ptMsg->ucMessageID = 0;
	ptMsg->ulData[0] = 0;
	ptMsg->usData[0] = 0;

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

				/* K2�����£���xQueue1�������� */
				case KEY_DOWN_K2:
					ucCount++;
					/* ����Ϣ���з����ݣ���ʹ��Ϣ�����Ѿ����� */
					xQueueOverwrite(xQueue1, (void *) &ucCount);
					printf("K2�����£���xQueue1�������ݣ���ʹ��Ϣ�����Ѿ�����\r\n");
					break;

				/* K3�����£���xQueue2�������� */
				case KEY_DOWN_K3:
					ptMsg->ucMessageID++;
					ptMsg->ulData[0]++;;
					ptMsg->usData[0]++;

					/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
					xQueueOverwrite(xQueue2, (void *) &ptMsg);
					printf("K3�����£���xQueue2�������ݣ���ʹ��Ϣ�����Ѿ�����\r\n");

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
*	����˵��: ʹ�ú���xQueueReceive��������vTaskTaskUserIF���͵���Ϣ��������
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 2
*********************************************************************************************************
*/
static void vTaskLED(void *pvParameters)
{
	MSG_T *ptMsg;
	BaseType_t xResult;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200); /* �������ȴ�ʱ��Ϊ200ms */

    while(1)
    {
		xResult = xQueueReceive(xQueue2,                   /* ��Ϣ���о�� */
		                        (void *)&ptMsg,  		   /* �����ȡ���ǽṹ��ĵ�ַ */
		                        (TickType_t)xMaxBlockTime);/* ��������ʱ�� */


		if(xResult == pdPASS)
		{
			/* �ɹ����գ���ͨ�����ڽ����ݴ�ӡ���� */
			printf("���յ���Ϣ��������ptMsg->ucMessageID = %d\r\n", ptMsg->ucMessageID);
			printf("���յ���Ϣ��������ptMsg->ulData[0] = %d\r\n", ptMsg->ulData[0]);
			printf("���յ���Ϣ��������ptMsg->usData[0] = %d\r\n", ptMsg->usData[0]);
		}
		else
		{
			/* ��ʱ */
			bsp_LedToggle(2);
			bsp_LedToggle(3);
		}
    }
}

/*
*********************************************************************************************************
*	�� �� ��: vTaskMsgPro
*	����˵��: ʹ�ú���xQueueReceive��������vTaskTaskUserIF���͵���Ϣ��������
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 3
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	BaseType_t xResult;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(300); /* �������ȴ�ʱ��Ϊ300ms */
	uint8_t ucQueueMsgValue;

    while(1)
    {
		xResult = xQueueReceive(xQueue1,                   /* ��Ϣ���о�� */
		                        (void *)&ucQueueMsgValue,  /* �洢���յ������ݵ�����ucQueueMsgValue�� */
		                        (TickType_t)xMaxBlockTime);/* ��������ʱ�� */

		if(xResult == pdPASS)
		{
			/* �ɹ����գ���ͨ�����ڽ����ݴ�ӡ���� */
			printf("���յ���Ϣ��������ucQueueMsgValue = %d\r\n", ucQueueMsgValue);
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


	xTaskCreate(    vTaskLED,        	/* ������  */
                    "vTaskLED",      	/* ������    */
                    512,             	/* stack��С����λword��Ҳ����4�ֽ� */
                    NULL,            	/* �������  */
                    2,                  /* �������ȼ�*/
                    &xHandleTaskLED );  /* ������  */

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

/*******************************************************************************
**������Ϣ ��
**�������� ��
**������� ����
**������� ����
********************************************************************************/
static void AppObjCreate (void)
{
	/* ����1��uint8_t����Ϣ���� */
	xQueue1 = xQueueCreate(1, sizeof(uint8_t));
    if( xQueue1 == 0 )
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }

	/* ����1���洢ָ���������Ϣ���У�����CM3��32λ����һ��ָ�����ռ��4���ֽ� */
	xQueue2 = xQueueCreate(1, sizeof(struct Msg *));
    if( xQueue2 == 0 )
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }
}

/*----------------------- (C) COPYRIGHT 2020 www.OS-Q.comm --------------------*/
