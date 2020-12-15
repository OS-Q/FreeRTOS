/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 本实验主要学习FreeRTOS的多事件等待
*              实验目的：
*                1. 学习FreeRTOS的多事件等待
*              实验内容：
*                3. 按下按键K1可以通过串口打印任务执行情况
*                   任务名      任务状态 优先级   剩余栈 任务序号
*                   vTaskUserIF     R       1       332     1
*                   IDLE            R       0       120     5
*                   vTaskStart      B       4       490     4
*                   vTaskMsgPro     B       3       456     3
*                   vTaskLED        B       2       484     2
*
*                   任务名       运行计数         使用率
*                   vTaskUserIF     52883           <1%
*                   IDLE            9862656         98%
*                   vTaskMsgPro     2839            <1%
*                   vTaskLED        1               <1%
*                   vTaskStart      49823           <1%
*                   串口软件建议使用SecureCRT（V4光盘里面有此软件）查看打印信息。
*                    vTaskTaskUserIF 任务：按键消息处理
*                    vTaskLED 任务       ：LED闪烁
*                    vTaskMsgPro 任务    ：使用函数xQueueSelectFromSet接收任务vTaskTaskUserIF发送的多个事件，信号量和消息队列
*                    vTaskStart 任务     ：按键扫描
*                 4. 任务运行转态的定义如下，跟上面串口打印字母B, R, D, S对应：
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 5. K2键按下，直接发送同步信号给任务vTaskMsgPro 。
*                 6. K3键按下，向xQueue1发送数据，任务vTaskMsgPro接收到消息后，串口打印接收到数值。
*                 7. 摇杆OK键按下，向xQueue2发送数据，任务vTaskMsgPro接收到消息后，串口打印接收到数值。
*              注意事项：
*                 1. 本实验推荐使用串口软件SecureCRT，要不串口打印效果不整齐。此软件在
*                    V4开发板光盘里面有。
*                 2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号    日期         作者            说明
*       V1.0    2015-08-19   Eric2013    1. ST固件库到V3.6.1版本
*                                        2. BSP驱动包V1.2
*                                        3. FreeRTOS版本V8.2.2
*
*	Copyright (C), 2015-2020, 安富莱www.OS-Q.comm
*
*********************************************************************************************************
*/
#include "includes.h"


/*
**********************************************************************************************************
											宏定义
**********************************************************************************************************
*/
#define QUEUE_LENGTH_1		10     /* 消息队列长度 */
#define QUEUE_LENGTH_2		10

#define BINARY_SEMAPHORE_LENGTH	1  /* 二值信号量 */

#define ITEM_SIZE_QUEUE_1	sizeof(uint32_t) /* 消息队列每个长度占用字节大小 */
#define ITEM_SIZE_QUEUE_2	sizeof(uint8_t)

#define COMBINED_LENGTH (QUEUE_LENGTH_1 + QUEUE_LENGTH_2 + BINARY_SEMAPHORE_LENGTH) /* 添加到Queue Set的总长度 */

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
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static SemaphoreHandle_t  xSemaphore = NULL;
static xQueueSetHandle xQueueSet = NULL;
static xQueueHandle xQueue1 = NULL;
static xQueueHandle xQueue2 = NULL;

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

				/* K2键按下 直接发送同步信号给任务vTaskMsgPro */
				case KEY_DOWN_K2:
					printf("K2键按下，直接发送同步信号给任务vTaskMsgPro\r\n");
					xSemaphoreGive(xSemaphore);
					break;

				/* K3键按下，向xQueue1发送数据 */
				case KEY_DOWN_K3:
					ulCount++;

					/* 向消息队列发数据，如果消息队列满了，等待10个时钟节拍 */
					if( xQueueSend(xQueue1,
								   (void *) &ulCount,
								   (TickType_t)10) != pdPASS )
					{
						/* 发送失败，即使等待了10个时钟节拍 */
						printf("K3键按下，向xQueue1发送数据失败，即使等待了10个时钟节拍\r\n");
					}
					else
					{
						/* 发送成功 */
						printf("K3键按下，向xQueue1发送数据成功\r\n");
					}
					break;

				/* 摇杆OK键按下，向xQueue1发送数据 */
				case JOY_DOWN_OK:
					ucCount++;

					/* 向消息队列发数据，如果消息队列满了，等待10个时钟节拍 */
					if( xQueueSend(xQueue2,
								   (void *) &ucCount,
								   (TickType_t)10) != pdPASS )
					{
						/* 发送失败，即使等待了10个时钟节拍 */
						printf("摇杆OK键按下，向xQueue1发送数据失败，即使等待了10个时钟节拍\r\n");
					}
					else
					{
						/* 发送成功 */
						printf("摇杆OK键按下，向xQueue1发送数据成功\r\n");
					}
					break;

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
*	功能说明: LED闪烁
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 2
*********************************************************************************************************
*/
static void vTaskLED(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 200;

	/* 获取当前的系统时间 */
    xLastWakeTime = xTaskGetTickCount();

    while(1)
    {
       	bsp_LedToggle(2);
		bsp_LedToggle(3);

		/* vTaskDelayUntil是绝对延迟，vTaskDelay是相对延迟。*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/*
*********************************************************************************************************
*	函 数 名: vTaskMsgPro
*	功能说明: 使用函数xQueueSelectFromSet接收任务vTaskTaskUserIF发送的多个事件，信号量和消息队列
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 3
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	QueueSetMemberHandle_t xActivatedMember;
	uint32_t ulQueueMsgValue;
	uint8_t  ucQueueMsgValue;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(300); /* 设置最大等待时间为300ms等同300/portTICK_RATE_MS */

    while(1)
    {
		/* 多事件等待，等待二值信号量和消息队列 */
        xActivatedMember = xQueueSelectFromSet(xQueueSet, xMaxBlockTime);

        /* 根据xActivatedMember判断接受到的消息类型 */
        if(xActivatedMember == xQueue1)
        {
			/* 消息队列1接收到消息 */
            xQueueReceive(xActivatedMember, &ulQueueMsgValue, 0);
			printf("消息队列1接收到消息 ulQueueMsgValue = %d\r\n", ulQueueMsgValue);
        }
        else if(xActivatedMember == xQueue2)
        {
			/* 消息队列2接收到消息 */
            xQueueReceive(xActivatedMember, &ucQueueMsgValue, 0);
			printf("消息队列2接收到消息 ucQueueMsgValue = %d\r\n", ucQueueMsgValue);
        }
        else if(xActivatedMember == xSemaphore)
        {
			/* 接收到信号量 */
			xSemaphoreTake(xActivatedMember, 0);
			printf("接收到信号量\r\n");
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
*	形    参：无
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


	xTaskCreate(    vTaskLED,    /* 任务函数  */
                    "vTaskLED",  /* 任务名    */
                    512,         /* stack大小，单位word，也就是4字节 */
                    NULL,        /* 任务参数  */
                    2,           /* 任务优先级*/
                    &xHandleTaskLED );   /* 任务句柄  */

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

/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通信机制
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
	/* 创建二值信号量，首次创建信号量计数值是0 */
	xSemaphore = xSemaphoreCreateBinary();

	if(xSemaphore == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }

	/* 创建QueuSet */
    xQueueSet = xQueueCreateSet(COMBINED_LENGTH);

	if(xQueueSet == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }

    /* 创建消息队列 */
    xQueue1 = xQueueCreate(QUEUE_LENGTH_1, ITEM_SIZE_QUEUE_1);

	if(xQueue1 == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }

    xQueue2 = xQueueCreate(QUEUE_LENGTH_2, ITEM_SIZE_QUEUE_2);

	if(xQueue2 == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }

    /* 添加到queue set时，消息队列和信号量必须为空*/
    /* 添加消息队列和信号量到Queue Set */
    xQueueAddToSet(xQueue1, xQueueSet);
    xQueueAddToSet(xQueue2, xQueueSet);
    xQueueAddToSet(xSemaphore, xQueueSet);
}

/***************************** 安富莱www.OS-Q.comm (END OF FILE) *********************************/
