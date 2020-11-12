/*
*********************************************************************************************************
*
*	ģ������ : ������ģ�顣
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ��ʵ����ҪѧϰFreeRTOS�ĵ͹���(Tickless Idleģʽ�¹ر�����ʱ��)
*              ʵ��Ŀ�ģ�
*                1. ѧϰFreeRTOS�ĵ͹���(Tickless Idleģʽ�¹ر�����ʱ��)
*                2. FreeRTOS�Դ���tickless idleģʽʹ�ñȽϼ򵥣�ֻ���û�ʹ�ܺ����ã�
*                   #define configUSE_TICKLESS_IDLE         1
*                3. �����˵���������Tickless Idleģʽ�¹ر�����ʱ�ӣ�ͨ�����ַ�ʽ��һ�����͹��ģ�
*                    ���º������ļ�FreeRTOSConfig.h�ļ���������˺궨��
*                    #define configPRE_SLEEP_PROCESSING(x)  OS_PreSleepProcessing(x)
*                    #define configPOST_SLEEP_PROCESSING(x) OS_PostSleepProcessing(x)
*                    ���ļ�port.c���溯��vPortSuppressTicksAndSleep����������������������
*                    ---------------------------------------------------------------------
*				     configPRE_SLEEP_PROCESSING( xModifiableIdleTime );
*				     if( xModifiableIdleTime > 0 )
*				     {
*					     __dsb( portSY_FULL_READ_WRITE );
*					     __wfi();
*					     __isb( portSY_FULL_READ_WRITE );
*				     }
*				     configPOST_SLEEP_PROCESSING( xExpectedIdleTime );
*                   -----------------------------------------------------------------------
*                   ͨ����������������ʵ���ڵ���__WFIָ��ǰ��ִ�н�һ���ĵ͹��Ĳ�������Ҫ���������֣�
*                   a. ����ϵͳ��Ƶ��
*             		b. �ر�����ʱ�ӡ�
*            		c. IO����Ҫ��������ֹ�������͹�������ӹ��ġ�
*                      �����IO�ڴ�������������Ϊ�ߵ�ƽ������߸���̬���룻
*                      �����IO�ڴ�������������Ϊ�͵�ƽ������߸���̬���룻
*             		��ʵ�����˹ر�����ʱ�ӵĴ���
*                   a. �ں���OS_PreSleepProcessing �ر�����ʱ��
*                   b. �ں���OS_PostSleepProcessing ��������ʱ��
*              ʵ�����ݣ�
*                4.  �����������ʹ��SecureCRT��V4���������д�������鿴��ӡ��Ϣ��
*                    vTaskTaskUserIF ���񣺰�����Ϣ����
*                    vTaskLED ����       ��ʹ�ú���xQueueReceive��������vTaskTaskUserIF���͵���Ϣ��������(xQueue2)
*                    vTaskMsgPro ����    ��ʹ�ú���xQueueReceive��������vTaskTaskUserIF���͵���Ϣ��������(xQueue1)
*                    vTaskStart ����     ������ɨ��
*                5. K2�����£���xQueue1�������ݡ�
*                6. K3�����£���xQueue2�������ݡ�
*                7. ���ڵ͹��ĵ�˵����
*                   (1) STM32F10xxx�����ֵ͹���ģʽ
*                        a. ˯��ģʽ(Cortex-M3�ں�ֹͣ�������������Cortex-M3���ĵ����裬��NVIC��ϵͳʱ
*                           ��(SysTick)����������)
*                        b. ֹͣģʽ(���е�ʱ�Ӷ���ֹͣ)
*                        c. ����ģʽ(1.8V��Դ�ر�)
*                   (2) ͨ��ָ��__WFI��������ģʽ������ͨ�������жϻ��ѡ�
*                   (3) ����ϵͳ��Ƶ���߹ر�����ʱ��Ҳ����Ч����ϵͳ���ġ�
*                8. FreeRTOS�Դ���tickless idleģʽ�ǵ��õ�ָ��__WFI��������ģʽ��
*                9. ʵ����Ŀ���Ƽ����ùٷ���ticklessģʽ��
*              ��Ƶ͹�����Ҫ�����¼��������֣�
*                1. �û���Ҫ������͵�Դ���ġ����������ʱ��Ϳ��õĻ���Դ��������ѡ��һ����ѵĵ͹���ģʽ��
*                   ����ʹ�õĵ͹��ķ�ʽ������ģʽ������ģʽ��ͣ��ģʽ��
*                2. ѡ���˵͹��ķ�ʽ����ǹرտ��Թرյ�����ʱ�ӡ�
*                3. ����ϵͳ��Ƶ��
*                4. ע��I/O��״̬��
*                   �����IO�ڴ�������������Ϊ�ߵ�ƽ������߸���̬���룻
*                   �����IO�ڴ�������������Ϊ�͵�ƽ������߸���̬���룻
*                   a. ��˯��ģʽ�£����е�I/O���Ŷ���������������ģʽʱ��״̬��
*                   b. ��ֹͣģʽ�£����е�I/O���Ŷ���������������ģʽʱ��״̬��
*                   c. �ڴ���ģʽ�£����е�I/O���Ŵ��ڸ���̬���������µ����ţ�
*                      �� ��λ����(ʼ����Ч)
*                      �� ��������Ϊ�������У׼���ʱ��TAMPER����
*                      �� ��ʹ�ܵĻ�������
*                5. ע��IO������IC�����ӡ�
*                6. ��͹��ĵ�ʱ��һ����Ҫ���ӵ������������ܱߵ��Ա߲������
*              ע�����
*                 1. ��ʵ���Ƽ�ʹ�ô������SecureCRT��Ҫ�����ڴ�ӡЧ�������롣�������
*                    V4��������������С�
*                 2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
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
					break;

				/* K2�����£���xQueue1�������� */
				case KEY_DOWN_K2:
					ucCount++;

					/* ����Ϣ���з����ݣ������Ϣ�������ˣ��ȴ�10��ʱ�ӽ��� */
					if( xQueueSend(xQueue1,
								   (void *) &ucCount,
								   (TickType_t)10) != pdPASS )
					{
						/* ����ʧ�ܣ���ʹ�ȴ���10��ʱ�ӽ��� */
						printf("K2�����£���xQueue1��������ʧ�ܣ���ʹ�ȴ���10��ʱ�ӽ���\r\n");
					}
					else
					{
						/* ���ͳɹ� */
						printf("K2�����£���xQueue1�������ݳɹ�\r\n");
					}
					break;

				/* K3�����£���xQueue2�������� */
				case KEY_DOWN_K3:
					ptMsg->ucMessageID++;
					ptMsg->ulData[0]++;;
					ptMsg->usData[0]++;

					/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
					if(xQueueSend(xQueue2,                  /* ��Ϣ���о�� */
								 (void *) &ptMsg,           /* ���ͽṹ��ָ�����ptMsg�ĵ�ַ */
								 (TickType_t)10) != pdPASS )
					{
						/* ����ʧ�ܣ���ʹ�ȴ���10��ʱ�ӽ��� */
						printf("K3�����£���xQueue2��������ʧ�ܣ���ʹ�ȴ���10��ʱ�ӽ���\r\n");
					}
					else
					{
						/* ���ͳɹ� */
						printf("K3�����£���xQueue2�������ݳɹ�\r\n");
					}


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
	/* ����10��uint8_t����Ϣ���� */
	xQueue1 = xQueueCreate(10, sizeof(uint8_t));
    if( xQueue1 == 0 )
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }

	/* ����10���洢ָ���������Ϣ���У�����CM3��32λ����һ��ָ�����ռ��4���ֽ� */
	xQueue2 = xQueueCreate(10, sizeof(struct Msg *));
    if( xQueue2 == 0 )
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }
}

/*
*********************************************************************************************************
*	�� �� ��: OS_PreSleepProcessing
*	����˵��: ����ĺ������ļ�FreeRTOSConfig.h�ļ���������˺궨�壺
*              #define configPRE_SLEEP_PROCESSING(x)  OS_PreSleepProcessing(x)
*              #define configPOST_SLEEP_PROCESSING(x) OS_PostSleepProcessing(x)
*              ���ļ�port.c���溯��vPortSuppressTicksAndSleep����������������������
*              ---------------------------------------------------------------------
*				configPRE_SLEEP_PROCESSING( xModifiableIdleTime );
*				if( xModifiableIdleTime > 0 )
*				{
*					__dsb( portSY_FULL_READ_WRITE );
*					__wfi();
*					__isb( portSY_FULL_READ_WRITE );
*				}
*				configPOST_SLEEP_PROCESSING( xExpectedIdleTime );
*             -----------------------------------------------------------------------
*             ͨ����������������ʵ���ڵ���__WFIָ��ǰ��ִ�н�һ���ĵ͹��Ĳ�������Ҫ���������֣�
*             1. ����ϵͳ��Ƶ��
*             2. �ر�����ʱ�ӡ�
*             3. IO����Ҫ��������ֹ�������͹�������ӹ��ġ�
*                �����IO�ڴ�������������Ϊ�ߵ�ƽ������߸���̬���룻
*                �����IO�ڴ�������������Ϊ�͵�ƽ������߸���̬���룻
*             ����ĺ������˹ر�����ʱ�ӵĴ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void OS_PreSleepProcessing(uint32_t vParameters)
{
	/* �ر�ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, DISABLE);
}

void OS_PostSleepProcessing(uint32_t vParameters)
{
	/* ���¿���ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
}

/***************************** ������www.OS-Q.comm (END OF FILE) *********************************/
