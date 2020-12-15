
#include "includes.h"


static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void AppTaskCreate (void);
static void TIM_CallBack1(void);


static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;

/*******************************************************************************
**������Ϣ ��
**�������� ��
**������� ����
**������� ����
********************************************************************************/
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

/*******************************************************************************
**������Ϣ ��
**�������� ��
**������� ����
**������� ����
********************************************************************************/
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

/*******************************************************************************
**������Ϣ ��
**�������� ��
**������� ����
**������� ����
********************************************************************************/
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

/*******************************************************************************
**������Ϣ ��
**�������� ��
**������� ����
**������� ����
********************************************************************************/
static void vTaskMsgPro(void *pvParameters)
{
	const TickType_t xBlockTime = pdMS_TO_TICKS(500); /* �������ȴ�ʱ��Ϊ500ms */
	uint32_t ulNotifiedValue;
    while(1)
    {
		ulNotifiedValue = ulTaskNotifyTake(pdFALSE,  	/* 1. �˲�������ΪpdFALSE�����յ���notification value��һ 2. �˲�������ΪpdTRUE�����յ���notification value���� */
						                   xBlockTime); /* ���޵ȴ� */
		if( ulNotifiedValue > 0 )
        {
			/* ���յ���Ϣ */
			printf("����vTaskMsgPro���յ���Ϣ��ulNotifiedValue = %d\r\n", ulNotifiedValue);
        }
        else
        {
			bsp_LedToggle(1);
			bsp_LedToggle(4);
        }
    }
}

/*******************************************************************************
**������Ϣ ��
**�������� ��
**������� ����
**������� ����
********************************************************************************/
static void vTaskStart(void *pvParameters)
{
    while(1)
    {
		/* ����ɨ�� */
		bsp_KeyScan();
        vTaskDelay(10);
    }
}

/*******************************************************************************
**������Ϣ ��
**�������� ��
**������� ����
**������� ����
********************************************************************************/
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

/*******************************************************************************
**������Ϣ ��
**�������� ��
**������� ����
**������� ����
********************************************************************************/
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

/*----------------------- (C) COPYRIGHT 2020 www.OS-Q.comm --------------------*/
