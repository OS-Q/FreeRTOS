/*
*********************************************************************************************************
*
*	ģ������ : ������ģ�顣
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ��ʵ����ҪѧϰFreeRTOS�ĵ͹���(Tickless Idleģʽ)
*              ʵ��Ŀ�ģ�
*                1. ѧϰFreeRTOS�ĵ͹���(Tickless Idleģʽ)
*                2. FreeRTOS�Դ���tickless idleģʽʹ�ñȽϼ򵥣�ֻ���û�ʹ��
*                   �����ã�
*                   #define configUSE_TICKLESS_IDLE         1
*                3. Ϊ�˴�ӡϵͳ��Ϣ��������ʼ��һ����ʱ���жϣ����ȸ��ڵδ�ʱ���жϣ�
*                   ÿ50us��һ�Σ�ʵ��ʹ��Tickless Idleģʽ��ز�Ҫ��ʹ��������ܣ����
*                   ���ܽ�����ϵͳ��Ϣ��ӡ��
*              ʵ�����ݣ�
*                2. ���°���K1����ͨ�����ڴ�ӡ����ִ�����
*                   ������      ����״̬ ���ȼ�   ʣ��ջ �������
*                   vTaskUserIF     R       1       332     1
*                   IDLE            R       0       120     5
*                   vTaskMsgPro     B       3       458     3
*                   vTaskLED        B       2       458     2
*                   vTaskStart      B       4       490     4
*
*                   ������       ���м���         ʹ����
*                   vTaskUserIF     5363            2%
*                   IDLE            254937          97%
*                   vTaskMsgPro     865             <1%
*                   vTaskLED        1234            <1%
*                   vTaskStart      2               <1%
*                   �����������ʹ��SecureCRT��V4���������д�������鿴��ӡ��Ϣ��
*                    vTaskTaskUserIF ���񣺰�����Ϣ����
*                    vTaskLED ����       ��ʹ�ú���xQueueReceive��������vTaskTaskUserIF���͵���Ϣ��������(xQueue2)
*                    vTaskMsgPro ����    ��ʹ�ú���xQueueReceive��������vTaskTaskUserIF���͵���Ϣ��������(xQueue1)
*                    vTaskStart ����     ������ɨ��
*                 3. ��������ת̬�Ķ������£������洮�ڴ�ӡ��ĸB, R, D, S��Ӧ��
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 4. K2�����£���xQueue1�������ݡ�
*                 5. K3�����£���xQueue2�������ݡ�
*                 6. ���ڵ͹��ĵ�˵����
*                   (1) STM32F10xxx�����ֵ͹���ģʽ
*                        a. ˯��ģʽ(Cortex-M3�ں�ֹͣ�������������Cortex-M3���ĵ����裬��NVIC��ϵͳʱ
*                           ��(SysTick)����������)
*                        b. ֹͣģʽ(���е�ʱ�Ӷ���ֹͣ)
*                        c. ����ģʽ(1.8V��Դ�ر�)
*                   (2) ͨ��ָ��__WFI��������ģʽ������ͨ�������жϻ��ѡ�
*                   (3) ����ϵͳ��Ƶ���߹ر�����ʱ��Ҳ����Ч����ϵͳ���ġ�
*                 7. FreeRTOS�Դ���tickless idleģʽ�ǵ��õ�ָ��__WFI��������ģʽ��
*                 8. ʵ����Ŀ���Ƽ����ùٷ���ticklessģʽ��
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

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
