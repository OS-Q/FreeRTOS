/*
*********************************************************************************************************
*
*	ģ������ : ������ģ�顣
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ��ʵ����ҪѧϰFreeRTOS�Ķ��¼��ȴ�
*              ʵ��Ŀ�ģ�
*                1. ѧϰFreeRTOS�Ķ��¼��ȴ�
*              ʵ�����ݣ�
*                3. ���°���K1����ͨ�����ڴ�ӡ����ִ�����
*                   ������      ����״̬ ���ȼ�   ʣ��ջ �������
*                   vTaskUserIF     R       1       332     1
*                   IDLE            R       0       120     5
*                   vTaskStart      B       4       490     4
*                   vTaskMsgPro     B       3       456     3
*                   vTaskLED        B       2       484     2
*
*                   ������       ���м���         ʹ����
*                   vTaskUserIF     52883           <1%
*                   IDLE            9862656         98%
*                   vTaskMsgPro     2839            <1%
*                   vTaskLED        1               <1%
*                   vTaskStart      49823           <1%
*                   �����������ʹ��SecureCRT��V4���������д�������鿴��ӡ��Ϣ��
*                    vTaskTaskUserIF ���񣺰�����Ϣ����
*                    vTaskLED ����       ��LED��˸
*                    vTaskMsgPro ����    ��ʹ�ú���xQueueSelectFromSet��������vTaskTaskUserIF���͵Ķ���¼����ź�������Ϣ����
*                    vTaskStart ����     ������ɨ��
*                 4. ��������ת̬�Ķ������£������洮�ڴ�ӡ��ĸB, R, D, S��Ӧ��
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 5. K2�����£�ֱ�ӷ���ͬ���źŸ�����vTaskMsgPro ��
*                 6. K3�����£���xQueue1�������ݣ�����vTaskMsgPro���յ���Ϣ�󣬴��ڴ�ӡ���յ���ֵ��
*                 7. ҡ��OK�����£���xQueue2�������ݣ�����vTaskMsgPro���յ���Ϣ�󣬴��ڴ�ӡ���յ���ֵ��
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
#define QUEUE_LENGTH_1		10     /* ��Ϣ���г��� */
#define QUEUE_LENGTH_2		10

#define BINARY_SEMAPHORE_LENGTH	1  /* ��ֵ�ź��� */

#define ITEM_SIZE_QUEUE_1	sizeof(uint32_t) /* ��Ϣ����ÿ������ռ���ֽڴ�С */
#define ITEM_SIZE_QUEUE_2	sizeof(uint8_t)

#define COMBINED_LENGTH (QUEUE_LENGTH_1 + QUEUE_LENGTH_2 + BINARY_SEMAPHORE_LENGTH) /* ��ӵ�Queue Set���ܳ��� */

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
static xQueueSetHandle xQueueSet = NULL;
static xQueueHandle xQueue1 = NULL;
static xQueueHandle xQueue2 = NULL;

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
	uint32_t ulCount = 0;
	uint8_t ucCount = 0;
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

				/* K3�����£���xQueue1�������� */
				case KEY_DOWN_K3:
					ulCount++;

					/* ����Ϣ���з����ݣ������Ϣ�������ˣ��ȴ�10��ʱ�ӽ��� */
					if( xQueueSend(xQueue1,
								   (void *) &ulCount,
								   (TickType_t)10) != pdPASS )
					{
						/* ����ʧ�ܣ���ʹ�ȴ���10��ʱ�ӽ��� */
						printf("K3�����£���xQueue1��������ʧ�ܣ���ʹ�ȴ���10��ʱ�ӽ���\r\n");
					}
					else
					{
						/* ���ͳɹ� */
						printf("K3�����£���xQueue1�������ݳɹ�\r\n");
					}
					break;

				/* ҡ��OK�����£���xQueue1�������� */
				case JOY_DOWN_OK:
					ucCount++;

					/* ����Ϣ���з����ݣ������Ϣ�������ˣ��ȴ�10��ʱ�ӽ��� */
					if( xQueueSend(xQueue2,
								   (void *) &ucCount,
								   (TickType_t)10) != pdPASS )
					{
						/* ����ʧ�ܣ���ʹ�ȴ���10��ʱ�ӽ��� */
						printf("ҡ��OK�����£���xQueue1��������ʧ�ܣ���ʹ�ȴ���10��ʱ�ӽ���\r\n");
					}
					else
					{
						/* ���ͳɹ� */
						printf("ҡ��OK�����£���xQueue1�������ݳɹ�\r\n");
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
*	����˵��: ʹ�ú���xQueueSelectFromSet��������vTaskTaskUserIF���͵Ķ���¼����ź�������Ϣ����
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 3
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	QueueSetMemberHandle_t xActivatedMember;
	uint32_t ulQueueMsgValue;
	uint8_t  ucQueueMsgValue;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(300); /* �������ȴ�ʱ��Ϊ300ms��ͬ300/portTICK_RATE_MS */

    while(1)
    {
		/* ���¼��ȴ����ȴ���ֵ�ź�������Ϣ���� */
        xActivatedMember = xQueueSelectFromSet(xQueueSet, xMaxBlockTime);

        /* ����xActivatedMember�жϽ��ܵ�����Ϣ���� */
        if(xActivatedMember == xQueue1)
        {
			/* ��Ϣ����1���յ���Ϣ */
            xQueueReceive(xActivatedMember, &ulQueueMsgValue, 0);
			printf("��Ϣ����1���յ���Ϣ ulQueueMsgValue = %d\r\n", ulQueueMsgValue);
        }
        else if(xActivatedMember == xQueue2)
        {
			/* ��Ϣ����2���յ���Ϣ */
            xQueueReceive(xActivatedMember, &ucQueueMsgValue, 0);
			printf("��Ϣ����2���յ���Ϣ ucQueueMsgValue = %d\r\n", ucQueueMsgValue);
        }
        else if(xActivatedMember == xSemaphore)
        {
			/* ���յ��ź��� */
			xSemaphoreTake(xActivatedMember, 0);
			printf("���յ��ź���\r\n");
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
	/* ������ֵ�ź������״δ����ź�������ֵ��0 */
	xSemaphore = xSemaphoreCreateBinary();

	if(xSemaphore == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }

	/* ����QueuSet */
    xQueueSet = xQueueCreateSet(COMBINED_LENGTH);

	if(xQueueSet == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }

    /* ������Ϣ���� */
    xQueue1 = xQueueCreate(QUEUE_LENGTH_1, ITEM_SIZE_QUEUE_1);

	if(xQueue1 == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }

    xQueue2 = xQueueCreate(QUEUE_LENGTH_2, ITEM_SIZE_QUEUE_2);

	if(xQueue2 == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }

    /* ��ӵ�queue setʱ����Ϣ���к��ź�������Ϊ��*/
    /* �����Ϣ���к��ź�����Queue Set */
    xQueueAddToSet(xQueue1, xQueueSet);
    xQueueAddToSet(xQueue2, xQueueSet);
    xQueueAddToSet(xSemaphore, xQueueSet);
}

/***************************** ������www.OS-Q.comm (END OF FILE) *********************************/
