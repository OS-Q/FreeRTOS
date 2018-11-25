/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 本实验主要学习FreeRTOS的任务通知实现计数信号量（中断方式）
*              实验目的：
*                1. 学习FreeRTOS的任务通知实现计数信号量（中断方式）
*              实验内容：
*                2. 按下按键K1可以通过串口打印任务执行情况
*                   任务名      任务状态 优先级   剩余栈 任务序号
*                   vTaskUserIF     R       1       334     1
*                   IDLE            R       0       120     5
*                   vTaskMsgPro     B       3       486     3
*                   vTaskLED        B       2       484     2
*                   vTaskStart      B       4       490     4
*
*                   任务名       运行计数         使用率
*                   vTaskUserIF     2021            3%
*                   IDLE            54297           95%
*                   vTaskMsgPro     1               <1%
*                   vTaskLED        0               <1%
*                   vTaskStart      283             <1%
*                   串口软件建议使用SecureCRT（V4光盘里面有此软件）查看打印信息。
*                    vTaskTaskUserIF 任务：按键消息处理
*                    vTaskLED 任务       ：LED闪烁
*                    vTaskMsgPro 任务    ：使用函数ulTaskNotifyTake接收定时器中断发生的消息
*                    vTaskStart 任务     ：按键扫描
*                 3. 任务运行转态的定义如下，跟上面串口打印字母B, R, D, S对应：
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 4. 本实验通过函数xTaskNotifyGive和ulTaskNotifyTake的实现计数信号量功能。
*                 5. 本实验计数信号量用于实现任务的同步。
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
static void AppTaskCreate (void);
static void TIM_CallBack1(void);

/*
**********************************************************************************************************
											变量声明
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;

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
				
				/* K2键按下，启动单次定时器中断，50ms后在定时器中断给任务vTaskMsgPro发消息 */
				case KEY_DOWN_K2:
					printf("K2键按下，启动单次定时器中断，50ms后在定时器中断给任务vTaskMsgPro发消息\r\n");
					bsp_StartHardTimer(1 ,50000, (void *)TIM_CallBack1);
				
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
*	功能说明: 使用函数ulTaskNotifyTake接收定时器中断发送的消息。
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 3  
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	const TickType_t xBlockTime = pdMS_TO_TICKS(500); /* 设置最大等待时间为500ms */
	uint32_t ulNotifiedValue;
	
    while(1)
    {
		ulNotifiedValue = ulTaskNotifyTake(pdFALSE,  /* 1. 此参数设置为pdFALSE，接收到的notification value减一 
											            2. 此参数设置为pdTRUE，接收到的notification value清零 */
						                   xBlockTime); /* 无限等待 */
		if( ulNotifiedValue > 0 )
        {
			/* 接收到消息 */
			printf("任务vTaskMsgPro接收到消息，ulNotifiedValue = %d\r\n", ulNotifiedValue);
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
*	函 数 名: TIM_CallBack1
*	功能说明: 定时器中断的回调函数，此函数被bsp_StartHardTimer所调用。		  			  
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void TIM_CallBack1(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	/* 
		发送任务通知, 连续发生三次，这样任务xHandleTaskMsgPro首次接收到的
	    notification value就是三。
	*/
	vTaskNotifyGiveFromISR(xHandleTaskMsgPro, &xHigherPriorityTaskWoken);
	vTaskNotifyGiveFromISR(xHandleTaskMsgPro, &xHigherPriorityTaskWoken);
	vTaskNotifyGiveFromISR(xHandleTaskMsgPro, &xHigherPriorityTaskWoken);

	/* 如果xHigherPriorityTaskWoken = pdTRUE，那么退出中断后切到当前最高优先级任务执行 */
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
