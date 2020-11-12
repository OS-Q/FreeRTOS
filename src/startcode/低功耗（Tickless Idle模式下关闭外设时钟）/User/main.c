/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 本实验主要学习FreeRTOS的低功耗(Tickless Idle模式下关闭外设时钟)
*              实验目的：
*                1. 学习FreeRTOS的低功耗(Tickless Idle模式下关闭外设时钟)
*                2. FreeRTOS自带的tickless idle模式使用比较简单，只需用户使能宏配置：
*                   #define configUSE_TICKLESS_IDLE         1
*                3. 下面简单说明下如何在Tickless Idle模式下关闭外设时钟，通过这种方式进一步降低功耗：
*                    如下函数在文件FreeRTOSConfig.h文件里面进行了宏定义
*                    #define configPRE_SLEEP_PROCESSING(x)  OS_PreSleepProcessing(x)
*                    #define configPOST_SLEEP_PROCESSING(x) OS_PostSleepProcessing(x)
*                    在文件port.c里面函数vPortSuppressTicksAndSleep调用了上面这两个函数：
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
*                   通过这两个函数可以实现在调用__WFI指令前后执行进一步的低功耗操作，主要有以下三种：
*                   a. 降低系统主频。
*             		b. 关闭外设时钟。
*            		c. IO引脚要做处理，防止拉电流和灌电流增加功耗。
*                      如果此IO口带上拉，请设置为高电平输出或者高阻态输入；
*                      如果此IO口带下拉，请设置为低电平输出或者高阻态输入；
*             		本实验做了关闭外设时钟的处理。
*                   a. 在函数OS_PreSleepProcessing 关闭外设时钟
*                   b. 在函数OS_PostSleepProcessing 开启外设时钟
*              实验内容：
*                4.  串口软件建议使用SecureCRT（V4光盘里面有此软件）查看打印信息。
*                    vTaskTaskUserIF 任务：按键消息处理
*                    vTaskLED 任务       ：使用函数xQueueReceive接收任务vTaskTaskUserIF发送的消息队列数据(xQueue2)
*                    vTaskMsgPro 任务    ：使用函数xQueueReceive接收任务vTaskTaskUserIF发送的消息队列数据(xQueue1)
*                    vTaskStart 任务     ：按键扫描
*                5. K2键按下，向xQueue1发送数据。
*                6. K3键按下，向xQueue2发送数据。
*                7. 关于低功耗的说明：
*                   (1) STM32F10xxx有三种低功耗模式
*                        a. 睡眠模式(Cortex-M3内核停止，所有外设包括Cortex-M3核心的外设，如NVIC、系统时
*                           钟(SysTick)等仍在运行)
*                        b. 停止模式(所有的时钟都已停止)
*                        c. 待机模式(1.8V电源关闭)
*                   (2) 通过指令__WFI进入休眠模式，可以通过任意中断唤醒。
*                   (3) 降低系统主频或者关闭外设时钟也可有效降低系统功耗。
*                8. FreeRTOS自带的tickless idle模式是调用的指令__WFI进入休眠模式。
*                9. 实际项目中推荐采用官方的tickless模式。
*              设计低功耗主要从以下几方面着手：
*                1. 用户需要根据最低电源消耗、最快速启动时间和可用的唤醒源等条件，选定一个最佳的低功耗模式。
*                   可以使用的低功耗方式有休眠模式，待机模式，停机模式。
*                2. 选择了低功耗方式后就是关闭可以关闭的外设时钟。
*                3. 降低系统主频。
*                4. 注意I/O的状态。
*                   如果此IO口带上拉，请设置为高电平输出或者高阻态输入；
*                   如果此IO口带下拉，请设置为低电平输出或者高阻态输入；
*                   a. 在睡眠模式下，所有的I/O引脚都保持它们在运行模式时的状态。
*                   b. 在停止模式下，所有的I/O引脚都保持它们在运行模式时的状态。
*                   c. 在待机模式下，所有的I/O引脚处于高阻态，除了以下的引脚：
*                      ● 复位引脚(始终有效)
*                      ● 当被设置为防侵入或校准输出时的TAMPER引脚
*                      ● 被使能的唤醒引脚
*                5. 注意IO和外设IC的连接。
*                6. 测低功耗的时候，一定不要连接调试器，更不能边调试边测电流。
*              注意事项：
*                 1. 本实验推荐使用串口软件SecureCRT，要不串口打印效果不整齐。此软件在
*                    V4开发板光盘里面有。
*                 2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*********************************************************************************************************
*/
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
					break;

				/* K2键按下，向xQueue1发送数据 */
				case KEY_DOWN_K2:
					ucCount++;

					/* 向消息队列发数据，如果消息队列满了，等待10个时钟节拍 */
					if( xQueueSend(xQueue1,
								   (void *) &ucCount,
								   (TickType_t)10) != pdPASS )
					{
						/* 发送失败，即使等待了10个时钟节拍 */
						printf("K2键按下，向xQueue1发送数据失败，即使等待了10个时钟节拍\r\n");
					}
					else
					{
						/* 发送成功 */
						printf("K2键按下，向xQueue1发送数据成功\r\n");
					}
					break;

				/* K3键按下，向xQueue2发送数据 */
				case KEY_DOWN_K3:
					ptMsg->ucMessageID++;
					ptMsg->ulData[0]++;;
					ptMsg->usData[0]++;

					/* 使用消息队列实现指针变量的传递 */
					if(xQueueSend(xQueue2,                  /* 消息队列句柄 */
								 (void *) &ptMsg,           /* 发送结构体指针变量ptMsg的地址 */
								 (TickType_t)10) != pdPASS )
					{
						/* 发送失败，即使等待了10个时钟节拍 */
						printf("K3键按下，向xQueue2发送数据失败，即使等待了10个时钟节拍\r\n");
					}
					else
					{
						/* 发送成功 */
						printf("K3键按下，向xQueue2发送数据成功\r\n");
					}


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
	/* 创建10个uint8_t型消息队列 */
	xQueue1 = xQueueCreate(10, sizeof(uint8_t));
    if( xQueue1 == 0 )
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }

	/* 创建10个存储指针变量的消息队列，由于CM3是32位机，一个指针变量占用4个字节 */
	xQueue2 = xQueueCreate(10, sizeof(struct Msg *));
    if( xQueue2 == 0 )
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
}

/*
*********************************************************************************************************
*	函 数 名: OS_PreSleepProcessing
*	功能说明: 下面的函数在文件FreeRTOSConfig.h文件里面进行了宏定义：
*              #define configPRE_SLEEP_PROCESSING(x)  OS_PreSleepProcessing(x)
*              #define configPOST_SLEEP_PROCESSING(x) OS_PostSleepProcessing(x)
*              在文件port.c里面函数vPortSuppressTicksAndSleep调用了上面这两个函数：
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
*             通过这两个函数可以实现在调用__WFI指令前后执行进一步的低功耗操作，主要有以下三种：
*             1. 降低系统主频。
*             2. 关闭外设时钟。
*             3. IO引脚要做处理，防止拉电流和灌电流增加功耗。
*                如果此IO口带上拉，请设置为高电平输出或者高阻态输入；
*                如果此IO口带下拉，请设置为低电平输出或者高阻态输入；
*             下面的函数做了关闭外设时钟的处理。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void OS_PreSleepProcessing(uint32_t vParameters)
{
	/* 关闭时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, DISABLE);
}

void OS_PostSleepProcessing(uint32_t vParameters)
{
	/* 重新开启时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
}

/***************************** 安富莱www.OS-Q.comm (END OF FILE) *********************************/
