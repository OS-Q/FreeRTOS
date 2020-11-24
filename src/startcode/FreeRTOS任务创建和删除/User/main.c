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
    while(1)
    {
		bsp_LedToggle(2);
		bsp_LedToggle(3);
        vTaskDelay(200);
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

				/* K2������ ɾ������vTaskLED */
				case KEY_DOWN_K2:
					printf("K2�����£�ɾ������vTaskLED\r\n");
					if(xHandleTaskLED != NULL)
					{
						vTaskDelete(xHandleTaskLED);
						xHandleTaskLED = NULL;
					}
					break;

				/* K3������ ���´�������vTaskLED */
				case KEY_DOWN_K3:
					printf("K3�����£����´�������vTaskLED\r\n");
					if(xHandleTaskLED == NULL)
					{
						xTaskCreate(    vTaskLED,            /* ������  */
										"vTaskLED",          /* ������    */
										512,                 /* stack��С����λword��Ҳ����4�ֽ� */
										NULL,                /* �������  */
										2,                   /* �������ȼ�*/
										&xHandleTaskLED );   /* ������  */
					}
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
    while(1)
    {
		bsp_LedToggle(1);
		bsp_LedToggle(4);
        vTaskDelay(300);
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
