
#include "includes.h"


/*
**********************************************************************************************************
											函数声明
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
											变量声明
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

MSG_T   g_tMsg; /* 定义一个结构体用于消息队列 */

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
	/* 硬件初始化初始化 */
	bsp_Init();

	/* 初始化一个定时器中断，精度高于滴答定时器中断，这样才可以获得准确的系统信息 */
	vSetupSysInfoTest();

	/* 创建任务 */
	AppTaskCreate();

	/* 创建任务通信机制 */
	AppObjCreate();

    /* 启动调度，开始执行任务 */
    vTaskStartScheduler();

	/* 如果系统正常启动是不会运行到这里的，运行到这里极有可能是空闲任务heap空间不足造成创建失败 */
	while(1);
}

/*
*********************************************************************************************************
*	函 数 名: vTaskTaskUserIF
*	功能说明: 按键消息处理
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 1  (数值越小优先级越低，这个跟uCOS相反)
*********************************************************************************************************
*/
static void vTaskTaskUserIF(void *pvParameters)
{
	MSG_T   *ptMsg;
	uint8_t ucCount = 0;
	uint8_t ucKeyCode;
	uint8_t pcWriteBuffer[500];

	/* 初始化结构体指针 */
	ptMsg = &g_tMsg;

	/* 初始化数组 */
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
				/* K1键按下 打印任务执行情况 */
				case KEY_DOWN_K1:
					printf("=================================================\r\n");
					printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
					vTaskList((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);

					printf("\r\n任务名       运行计数         使用率\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
					break;

				/* K2键按下，向xQueue1发送数据 */
				case KEY_DOWN_K2:
					ucCount++;
					/* 向消息队列发数据，即使消息队列已经满了 */
					xQueueOverwrite(xQueue1, (void *) &ucCount);
					printf("K2键按下，向xQueue1发送数据，即使消息队列已经满了\r\n");
					break;

				/* K3键按下，向xQueue2发送数据 */
				case KEY_DOWN_K3:
					ptMsg->ucMessageID++;
					ptMsg->ulData[0]++;;
					ptMsg->usData[0]++;

					/* 使用消息队列实现指针变量的传递 */
					xQueueOverwrite(xQueue2, (void *) &ptMsg);
					printf("K3键按下，向xQueue2发送数据，即使消息队列已经满了\r\n");

				/* 其他的键值不处理 */
				default:
					break;
			}
		}

		vTaskDelay(10);
	}
}

/*
*********************************************************************************************************
*	函 数 名: vTaskLED
*	功能说明: 使用函数xQueueReceive接收任务vTaskTaskUserIF发送的消息队列数据
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 2
*********************************************************************************************************
*/
static void vTaskLED(void *pvParameters)
{
	MSG_T *ptMsg;
	BaseType_t xResult;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200); /* 设置最大等待时间为200ms */

    while(1)
    {
		xResult = xQueueReceive(xQueue2,                   /* 消息队列句柄 */
		                        (void *)&ptMsg,  		   /* 这里获取的是结构体的地址 */
		                        (TickType_t)xMaxBlockTime);/* 设置阻塞时间 */


		if(xResult == pdPASS)
		{
			/* 成功接收，并通过串口将数据打印出来 */
			printf("接收到消息队列数据ptMsg->ucMessageID = %d\r\n", ptMsg->ucMessageID);
			printf("接收到消息队列数据ptMsg->ulData[0] = %d\r\n", ptMsg->ulData[0]);
			printf("接收到消息队列数据ptMsg->usData[0] = %d\r\n", ptMsg->usData[0]);
		}
		else
		{
			/* 超时 */
			bsp_LedToggle(2);
			bsp_LedToggle(3);
		}
    }
}

/*
*********************************************************************************************************
*	函 数 名: vTaskMsgPro
*	功能说明: 使用函数xQueueReceive接收任务vTaskTaskUserIF发送的消息队列数据
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 3
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	BaseType_t xResult;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(300); /* 设置最大等待时间为300ms */
	uint8_t ucQueueMsgValue;

    while(1)
    {
		xResult = xQueueReceive(xQueue1,                   /* 消息队列句柄 */
		                        (void *)&ucQueueMsgValue,  /* 存储接收到的数据到变量ucQueueMsgValue中 */
		                        (TickType_t)xMaxBlockTime);/* 设置阻塞时间 */

		if(xResult == pdPASS)
		{
			/* 成功接收，并通过串口将数据打印出来 */
			printf("接收到消息队列数据ucQueueMsgValue = %d\r\n", ucQueueMsgValue);
		}
		else
		{
			/* 超时 */
			bsp_LedToggle(1);
			bsp_LedToggle(4);
		}
    }
}

/*
*********************************************************************************************************
*	函 数 名: vTaskStart
*	功能说明: 启动任务，也就是最高优先级任务。
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 4
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
    while(1)
    {
		/* 按键扫描 */
		bsp_KeyScan();
        vTaskDelay(10);
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
    xTaskCreate(    vTaskTaskUserIF,   /* 任务函数  */
                    "vTaskUserIF",     /* 任务名    */
                    512,               /* stack大小，单位word，也就是4字节 */
                    NULL,              /* 任务参数  */
                    1,                 /* 任务优先级*/
                    NULL );            /* 任务句柄  */


	xTaskCreate(    vTaskLED,        	/* 任务函数  */
                    "vTaskLED",      	/* 任务名    */
                    512,             	/* stack大小，单位word，也就是4字节 */
                    NULL,            	/* 任务参数  */
                    2,                  /* 任务优先级*/
                    &xHandleTaskLED );  /* 任务句柄  */

	xTaskCreate(    vTaskMsgPro,     /* 任务函数  */
                    "vTaskMsgPro",   /* 任务名    */
                    512,             /* stack大小，单位word，也就是4字节 */
                    NULL,            /* 任务参数  */
                    3,               /* 任务优先级*/
                    &xHandleTaskMsgPro );  /* 任务句柄  */


	xTaskCreate(    vTaskStart,     /* 任务函数  */
                    "vTaskStart",   /* 任务名    */
                    512,            /* stack大小，单位word，也就是4字节 */
                    NULL,           /* 任务参数  */
                    4,              /* 任务优先级*/
                    NULL );         /* 任务句柄  */
}

/*******************************************************************************
**函数信息 ：
**功能描述 ：
**输入参数 ：无
**输出参数 ：无
********************************************************************************/
static void AppObjCreate (void)
{
	/* 创建1个uint8_t型消息队列 */
	xQueue1 = xQueueCreate(1, sizeof(uint8_t));
    if( xQueue1 == 0 )
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }

	/* 创建1个存储指针变量的消息队列，由于CM3是32位机，一个指针变量占用4个字节 */
	xQueue2 = xQueueCreate(1, sizeof(struct Msg *));
    if( xQueue2 == 0 )
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
}

/*----------------------- (C) COPYRIGHT 2020 www.OS-Q.comm --------------------*/
