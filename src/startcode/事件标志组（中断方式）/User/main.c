/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 本实验主要学习FreeRTOS的事件标志组（中断方式）
*              实验目的：
*                1. 学习FreeRTOS的事件标志组（中断方式）
*              实验内容：
*                2. 按下按键K1可以通过串口打印任务执行情况
*                   任务名      任务状态 优先级   剩余栈 任务序号
*                   vTaskUserIF     R       1       334     1
*                   IDLE            R       0       120     5
*                   vTaskLED        B       2       484     2
*                   vTaskMsgPro     B       3       476     3
*                   vTaskStart      B       4       490     4
*                   Tmr Svc         B       5       234     6
*
*                   任务名       运行计数         使用率
*                   vTaskUserIF     2445            1%
*                   IDLE            211291          98%
*                   vTaskLED        0               <1%
*                   vTaskMsgPro     0               <1%
*                   vTaskStart      1065            <1%
*                   Tmr Svc         1               <1%
*                   串口软件建议使用SecureCRT（V4光盘里面有此软件）查看打印信息。
*                    vTaskTaskUserIF 任务：按键消息处理
*                    vTaskLED 任务       ：LED闪烁
*                    vTaskMsgPro 任务    ：使用函数xEventGroupWaitBits接收定时器中断的事件标志
*                    vTaskStart 任务     ：按键扫描
*                 3. 任务运行转态的定义如下，跟上面串口打印字母B, R, D, S对应：
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 4. K2键按下，启动单次定时器中断，50ms后在定时器中断给任务vTaskMsgPro发送事件标志，设置bit0。
*                 5. K3键按下，启动单次定时器中断，50ms后在定时器中断给任务vTaskMsgPro发送事件标志，设置bit1。
*                 6. 任务vTaskMsgPro只有接收到bit0和bit1都被设置了才执行串口打印消息。
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
#define BIT_0	(1 << 0)
#define BIT_1	(1 << 1)
#define BIT_ALL (BIT_0 | BIT_1)

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
static void TIM_CallBack1(void);
static void TIM_CallBack2(void);

/*
**********************************************************************************************************
											变量声明
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static EventGroupHandle_t xCreatedEventGroup = NULL;

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

				/* K2键按下，启动单次定时器中断，50ms后在定时器中断给任务vTaskMsgPro发送事件标志 */
				case KEY_DOWN_K2:
					printf("K2键按下，启动单次定时器中断，50ms后在定时器中断给任务vTaskMsgPro发送事件标志\r\n");
					bsp_StartHardTimer(1 ,50000, (void *)TIM_CallBack1);
					break;

				/* K3键按下，启动单次定时器中断，50ms后在定时器中断给任务vTaskMsgPro发送事件标志 */
				case KEY_DOWN_K3:
					printf("K3键按下，启动单次定时器中断，50ms后在定时器中断给任务vTaskMsgPro发送事件标志\r\n");
					bsp_StartHardTimer(2 ,50000, (void *)TIM_CallBack2);
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
*	功能说明: 使用函数xEventGroupWaitBits接收定时器中断发送的事件标志
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 3
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* 最大延迟100ms */

    while(1)
    {
		/* 等K2按键按下设置bit0和K3按键按下设置bit1 */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* 事件标志组句柄 */
							         BIT_ALL,            /* 等待bit0和bit1被设置 */
							         pdTRUE,             /* 退出前bit0和bit1被清除 */
							         pdTRUE,             /* 设置为pdTRUE表示等待bit1和bit0都被设置*/
							         xTicksToWait); 	 /* 等待延迟时间 */

		if((uxBits & BIT_ALL) == BIT_ALL)
		{
			/* 接收到bit1和bit0都被设置的消息 */
			printf("接收到bit0和bit1都被设置的消息\r\n");
		}
		else
		{
			/* 超时，另外注意仅接收到一个按键按下的消息时，变量uxBits的相应bit也是被设置的 */
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
*	函 数 名: TIM_CallBack1和TIM_CallBack2
*	功能说明: 定时器中断的回调函数，此函数被bsp_StartHardTimer所调用。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void TIM_CallBack1(void)
{
	BaseType_t xResult;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	/* 向任务vTaskMsgPro发送事件标志 */
	xResult = xEventGroupSetBitsFromISR(xCreatedEventGroup, /* 事件标志组句柄 */
									    BIT_0 ,             /* 设置bit0 */
									    &xHigherPriorityTaskWoken );

	/* 消息被成功发出 */
	if( xResult != pdFAIL )
	{
		/* 如果xHigherPriorityTaskWoken = pdTRUE，那么退出中断后切到当前最高优先级任务执行 */
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

static void TIM_CallBack2(void)
{
	BaseType_t xResult;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	/* 向任务vTaskMsgPro发送事件标志 */
	xResult = xEventGroupSetBitsFromISR(xCreatedEventGroup, /* 事件标志组句柄 */
									    BIT_1,              /* 设置bit1 */
									    &xHigherPriorityTaskWoken );

	/* 消息被成功发出 */
	if( xResult != pdFAIL )
	{
		/* 如果xHigherPriorityTaskWoken = pdTRUE，那么退出中断后切到当前最高优先级任务执行 */
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
	/* 创建事件标志组 */
	xCreatedEventGroup = xEventGroupCreate();

	if(xCreatedEventGroup == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
}

/***************************** 安富莱www.OS-Q.comm (END OF FILE) *********************************/
