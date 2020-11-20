
#include "includes.h"



static TaskHandle_t xHandleTaskLED = NULL;


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

				/* K2������ �������vTaskLED�����ȼ� */
				case KEY_DOWN_K2:
					printf("K2�����£�����vTaskLED�����ȼ���: %d\r\n", uxTaskPriorityGet(xHandleTaskLED));
					break;

				/* K3������ ������vTaskLED�����ȼ���2����Ϊ3 */
				case KEY_DOWN_K3:
					printf("K3�����£�������vTaskLED�����ȼ���2����Ϊ3\r\n");
					vTaskPrioritySet(xHandleTaskLED, 3);
					break;

				/* ҡ��UP�����£���������vTaskLED */
				case JOY_DOWN_U:
					printf("ҡ��UP�����£���������vTaskLED\r\n");
					vTaskSuspend(xHandleTaskLED);
					break;

				/* ҡ��DOWN�����£��ָ�����vTaskLED */
				case JOY_DOWN_D:
					printf("ҡ��DOWN�����£��ָ�����vTaskLED\r\n");
					vTaskResume(xHandleTaskLED);
					break;

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
static void vTaskMsgPro(void *pvParameters)
{
	portTickType xDelay, xNextTime;

	/* ��ȡ��һ������ִ�е�ʱ�� */
	xNextTime = xTaskGetTickCount () + 300;

    while(1)
    {
		bsp_LedToggle(1);
		bsp_LedToggle(4);

		/* ��vTaskDelayʵ��vTaskDelayUntil() */
		xDelay = xNextTime - xTaskGetTickCount();
		xNextTime += 300;

		if(xDelay <= 300)
		{
			vTaskDelay(xDelay);
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
                    NULL );          /* ������  */


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

/*----------------------- (C) COPYRIGHT 2020 www.OS-Q.comm --------------------*/
