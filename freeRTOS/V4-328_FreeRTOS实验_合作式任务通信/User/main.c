/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 本实验主要学习FreeRTOS的合作式任务通讯
*              实验目的：
*                1. 学习FreeRTOS的合作式任务通讯
*              实验内容：
*                2. 按下按键K1可以通过串口打印任务执行情况
*                   任务名      任务状态 优先级   剩余栈 任务序号
*                   vTaskUserIF     R       1       334     1
*                   IDLE            R       0       86      5
*                   vTaskLED        B       2       484     2
*                   vTaskMsgPro     B       3       480     3
*                   vTaskStart      B       4       490     4
*
*                   任务名       运行计数         使用率
*                   vTaskUserIF     1240            <1%
*                   IDLE            238159          98%
*                   vTaskLED        1               <1%
*                   vTaskMsgPro     40              <1%
*                   vTaskStart      1207            <1%
*                   串口软件建议使用SecureCRT（V4光盘里面有此软件）查看打印信息。
*                    vTaskTaskUserIF 任务：按键消息处理
*                    vTaskLED 任务       ：LED闪烁
*                    vTaskMsgPro 任务    ：使用函数xSemaphoreTake接收任务vTaskTaskUserIF发送的同步信号
*                    vTaskStart 任务     ：按键扫描
*                 3. 任务运行转态的定义如下，跟上面串口打印字母B, R, D, S对应：
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 4. K2键按下 直接发送同步信号给任务vTaskMsgPro 。
*                 5. 合作式任务调度，合作式任务之间不能相互抢占，但是可以被其它高优先级的任务抢占。
*                 6. 如果一个应用中同时含有合作式任务和其它类型任务，那些需要将合作式调度函数放在空
*                    闲任务的钩子函数里面。
*                 7. 本实验演示合作式任务vCoRoutineLEDSyn给合作式任务vCoRoutineLED发送消息队列数据。
*
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
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
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
static void AppCoTaskCreate(void);
static void AppTaskCreate (void);
static void AppObjCreate (void);
static void vCoRoutineLED(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex);
static void vCoRoutineLEDSyn(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex);

/*
**********************************************************************************************************
											变量声明
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static SemaphoreHandle_t  xSemaphore = NULL;
static xQueueHandle xCoRoutineQueue = NULL;

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
	
	/* 创建合作式任务 */
	AppCoTaskCreate();
	
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
				
				/* K2键按下 直接发送同步信号给任务vTaskMsgPro */
				case KEY_DOWN_K2:
					printf("K2键按下，直接发送同步信号给任务vTaskMsgPro\r\n");
					xSemaphoreGive(xSemaphore);
				
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
		bsp_LedToggle(1);
		bsp_LedToggle(2);

		/* vTaskDelayUntil是绝对延迟，vTaskDelay是相对延迟。*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/*
*********************************************************************************************************
*	函 数 名: vTaskMsgPro
*	功能说明: 使用函数xSemaphoreTake接收任务vTaskTaskUserIF发送的同步信号
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 3  
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	BaseType_t xResult;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(300); /* 设置最大等待时间为300ms */
	
    while(1)
    {
		xResult = xSemaphoreTake(xSemaphore, (TickType_t)xMaxBlockTime);
		
		if(xResult == pdTRUE)
		{
			/* 接收到同步信号 */
			printf("接收到同步信号\r\n");
		}
		else
		{
			/* 超时 */
			bsp_LedToggle(3);
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
*	函 数 名: vCoRoutineLEDSyn
*	功能说明: 合作式任务
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void vCoRoutineLEDSyn(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex)
{
	 /* 合作式任务中的变量必须是static类型的 */
	 static uint8_t ucNumberToPost = 0;
	 static portBASE_TYPE xResult;

    /* 每个合作式任务开始的时候必须调用 */
    crSTART( xHandle );

    for( ;; )
    {
        crQUEUE_SEND( xHandle,
                      xCoRoutineQueue,
                      &ucNumberToPost,
                      0,
                      &xResult );

        if( xResult != pdPASS )
        {
            /* 消息发送失败 */
        }

        ucNumberToPost++;

        /* 延迟 */
        crDELAY(xHandle, 1000);
    }

    /* 每个合作式任务结束的时候必须调用 */
    crEND();
}

/*
*********************************************************************************************************
*	函 数 名: vCoRoutineLED
*	功能说明: 合作式任务
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void vCoRoutineLED(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex)
{
	/* 合作式任务中的变量必须是static类型的 */
	static portBASE_TYPE xResult;
	static uint8_t ucRecieve;

	/* 每个合作式任务开始的时候必须调用 */
	crSTART( xHandle );

	for( ;; )
	{
		/* 接收数据 */
		crQUEUE_RECEIVE( xHandle,
						 xCoRoutineQueue,
						 &ucRecieve,
						 portMAX_DELAY,
						 &xResult );

		if(xResult == pdPASS)
		{
			portENTER_CRITICAL();
			printf("合作式任务ucRecieve = %d\r\n", ucRecieve);
			portEXIT_CRITICAL();
		}
	}

	/* 每个合作式任务结束的时候必须调用 */
	crEND();
}

/*
*********************************************************************************************************
*	函 数 名: vFlashCoRoutine
*	功能说明: 创建合作式任务
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppCoTaskCreate( void )
{
	/* 接受数据 */
	xCoRoutineCreate(vCoRoutineLED, 0, 0 );
	
	/* 发送数据 */
	xCoRoutineCreate(vCoRoutineLEDSyn, 1, 0 );
}

/*
*********************************************************************************************************
*	函 数 名: vApplicationIdleHook
*	功能说明: 1) 合作式任务调度，合作式任务之间不能相互抢占，但是可以被其它高优先级的任务抢占。
*             2) 如果一个应用中同时含有合作式任务和其它类型任务，那些需要将合作式调度函数放在空闲任务
*                的钩子函数里面。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void vApplicationIdleHook( void )
{
	vCoRoutineSchedule();
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
	
	/* 消息队列长度为5，每个长度占用1个字节 */
	xCoRoutineQueue = xQueueCreate(5, sizeof(uint8_t));	
	
	if(xCoRoutineQueue == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
