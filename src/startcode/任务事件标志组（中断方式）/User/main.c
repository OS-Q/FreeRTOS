/*
*********************************************************************************************************
*
*	ģ������ : ������ģ�顣
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ��ʵ����ҪѧϰFreeRTOS������֪ͨʵ���¼���־�飨�жϷ�ʽ��
*              ʵ��Ŀ�ģ�
*                1. ѧϰFreeRTOS������֪ͨʵ���¼���־�飨�жϷ�ʽ��
*              ʵ�����ݣ�
*                2. ���°���K1����ͨ�����ڴ�ӡ����ִ�����
*                   ������      ����״̬ ���ȼ�   ʣ��ջ �������
*                   vTaskUserIF     R       1       318     1
*                   IDLE            R       0       116     5
*                   vTaskLED        B       2       484     2
*                   vTaskMsgPro     B       3       450     3
*                   vTaskStart      B       4       490     4
*
*                   ������       ���м���         ʹ����
*                   vTaskUserIF     20574           8%
*                   IDLE            227096          90%
*                   vTaskLED        1               <1%
*                   vTaskMsgPro     2277            <1%
*                   vTaskStart      1254            <1%
*                   �����������ʹ��SecureCRT��V4���������д�������鿴��ӡ��Ϣ��
*                    vTaskTaskUserIF ���񣺰�����Ϣ����
*                    vTaskLED ����       ��LED��˸
*                    vTaskMsgPro ����    ��ʹ�ú���xTaskNotifyWait���ն�ʱ���жϷ��͵���Ϣ
*                    vTaskStart ����     ������ɨ��
*                 3. ��������ת̬�Ķ������£������洮�ڴ�ӡ��ĸB, R, D, S��Ӧ��
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 4. ��ʵ��ͨ������xTaskNotify��xTaskNotifyFromISRʵ���¼���־��Ĺ��ܡ�
*                 5. K2�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro������λ0x01��
*                 6. K3�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro������λ0x02��
*
*              ע�����
*                 1. ��ʵ���Ƽ�ʹ�ô������SecureCRT��Ҫ�����ڴ�ӡЧ�������롣�������
*                    V4��������������С�
*                 2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*********************************************************************************************************
*/
#include "includes.h"

/*
**********************************************************************************************************
											�궨��
**********************************************************************************************************
*/
#define K2_BIT    0x01
#define K3_BIT    0x02

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
static void TIM_CallBack1(void);
static void TIM_CallBack2(void);

/*
**********************************************************************************************************
											��������
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;

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

				/* K2�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro������λ0x01 */
				case KEY_DOWN_K2:
					printf("K2�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro������λ0x01\r\n");
					bsp_StartHardTimer(1 ,50000, (void *)TIM_CallBack1);
					break;

				/* K3�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro������λ0x02 */
				case KEY_DOWN_K3:
					printf("K3�����£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro������λ0x02\r\n");
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
*	����˵��: ʹ�ú���xTaskNotifyWait��������vTaskTaskUserIF���͵���Ϣ
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 3
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	BaseType_t xResult;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(500); /* �������ȴ�ʱ��Ϊ500ms */
	uint32_t ulNotifiedValue;

    while(1)
    {
		/*
			��һ������ ulBitsToClearOnEntry�����ã�����ִ��ǰ����
		          notification value &= ~ulBitsToClearOnEntry
		          �򵥵�˵���ǲ���ulBitsToClearOnEntry�Ǹ�λ��1����ônotification value
		          ���Ǹ�λ�ͻᱻ���㡣

		    �ڶ������� ulBitsToClearOnExit�����ã������˳�ǰ����
				  notification value &= ~ulBitsToClearOnExit
		          �򵥵�˵���ǲ���ulBitsToClearOnEntry�Ǹ�λ��1����ônotification value
		          ���Ǹ�λ�ͻᱻ���㡣
		*/

		xResult = xTaskNotifyWait(0x00000000,       /* ����ִ��ǰ����notification value����λ */
						          0xFFFFFFFF,       /* �����˳�ǰ���notification value����λ */
						          &ulNotifiedValue, /* ����notification value������ulNotifiedValue�� */
						          xMaxBlockTime);   /* ��������ӳ�ʱ�� */

		if( xResult == pdPASS )
		{
			/* ���յ���Ϣ������Ǹ�λ������ */
			if((ulNotifiedValue & K2_BIT) != 0)
			{
				printf("���յ�K2�������º�Ķ�ʱ���ж���Ϣ, ulNotifiedValue = 0x%08x\r\n", ulNotifiedValue);

			}

			if((ulNotifiedValue & K3_BIT) != 0)
			{
				printf("���յ�K3�������º�Ķ�ʱ���ж���Ϣ, ulNotifiedValue = 0x%08x\r\n", ulNotifiedValue);
			}
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
*	�� �� ��: TIM_CallBack1��TIM_CallBack2
*	����˵��: ��ʱ���жϵĻص��������˺�����bsp_StartHardTimer�����á�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void TIM_CallBack1(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	xTaskNotifyFromISR(xHandleTaskMsgPro,     /* Ŀ������ */
			           K2_BIT,            	  /* ���ڸ���Ŀ�������notification value */
			           eSetBits,              /* eSetBits�ǲ�������֮һ������ʵ��Ŀ�������notification value��K2_BIT�Ļ���� */
	                   &xHigherPriorityTaskWoken);

	/* ���xHigherPriorityTaskWoken = pdTRUE����ô�˳��жϺ��е���ǰ������ȼ�����ִ�� */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void TIM_CallBack2(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	xTaskNotifyFromISR(xHandleTaskMsgPro,     /* Ŀ������ */
			           K3_BIT,            	  /* ���ڸ���Ŀ�������notification value */
			           eSetBits,              /* eSetBits�ǲ�������֮һ������ʵ��Ŀ�������notification value��K2_BIT�Ļ���� */
	                   &xHigherPriorityTaskWoken);

	/* ���xHigherPriorityTaskWoken = pdTRUE����ô�˳��жϺ��е���ǰ������ȼ�����ִ�� */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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

/***************************** ������www.OS-Q.comm (END OF FILE) *********************************/
