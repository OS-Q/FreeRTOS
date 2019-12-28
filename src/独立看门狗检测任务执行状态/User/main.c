/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 本实验主要学习独立看门狗检测FreeRTOS任务执行状态
*              实验目的：
*                1. 学习独立看门狗检测FreeRTOS任务执行状态
*              实验内容：
*                2. 按下按键K1可以通过串口打印任务执行情况
*                   任务名      任务状态 优先级   剩余栈 任务序号
*                   vTaskUserIF     R       1       334     1
*                   IDLE            R       0       120     6
*                   vTaskStart      B       5       466     5
*                   vTaskLED        B       2       484     2
*                   vTaskMsgPro     B       3       474     3
*                   vTaskScan       B       4       490     4
*
*                   任务名       运行计数         使用率
*                   vTaskUserIF     2834            2%
*                   IDLE            97914           96%
*                   vTaskStart      560             <1%
*                   vTaskLED        0               <1%
*                   vTaskMsgPro     6               <1%
*                   vTaskScan       508             <1%
*                   串口软件建议使用SecureCRT（V4光盘里面有此软件）查看打印信息。
*                    vTaskTaskUserIF 任务：按键消息处理
*                    vTaskLED 任务       ：LED闪烁
*                    vTaskMsgPro 任务    ：LED闪烁
*                    vTaskScan 任务      ：按键扫描
*                    vTaskStart 任务     ：等待所有任务发事件标志过来
*                 3. 任务运行转态的定义如下，跟上面串口打印字母B, R, D, S对应：
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                4. 看门狗监测任务执行状态说明：
*                   (1). 设置看门狗复位时间是10s，如果10s内不喂狗系统复位。
*                   (2). 使用事件标志组，在最高优先级任务中等待其它所有用户任务发来的事
*                        件标志，如果所有任务都发来了事件标志，那么就执行喂狗程序，如果
*                        有一个任务10s内没有发来事件标志，那么系统会被复位。
*                   (3). 简单的说就是为了检测任务的执行转态，我们设置每个任务10s内必须发一
*                        次事件标志以此来表示任务在执行。如果10s内有一个任务没有发来消息
*                        ，系统会被复位。
*                   (4). 等待事件标志的任务：	
*		                 uxBits = xEventGroupWaitBits(xCreatedEventGroup, 
*							                          TASK_BIT_ALL,       
*							                          pdTRUE,             
*							                          pdTRUE,            
*							                          xTicksToWait); 	             
*				    其它四个发送事件标志的任务：
*		            xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_0);
*		            xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);
*		            xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);
*		            xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_3);
*                3. K2按键按下后将任务vTaskTaskUserIF延迟20s后执行，从而实现看门狗复位。
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
											宏定义
**********************************************************************************************************
*/
#define TASK_BIT_0	 (1 << 0)
#define TASK_BIT_1	 (1 << 1)
#define TASK_BIT_2	 (1 << 2)
#define TASK_BIT_3	 (1 << 3)
#define TASK_BIT_ALL (TASK_BIT_0 | TASK_BIT_1 | TASK_BIT_2 | TASK_BIT_3)

/*
**********************************************************************************************************
											函数声明
**********************************************************************************************************
*/
static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskScan(void *pvParameters);
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
	const TickType_t xTicksToWait = 20000 / portTICK_PERIOD_MS; /* 最大延迟20s */

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
				
				/* K2按键按下，让vTaskTaskUserIF任务延迟20s，以实现看门狗复位情况 */
				case KEY_DOWN_K2:
					printf("K2按键按下，让vTaskTaskUserIF任务延迟20s，以实现看门狗复位情况\r\n");
					vTaskDelay(xTicksToWait);
					break;

				/* 其他的键值不处理 */
				default:                     
					break;
			}
		}
		
		/* 发送事件标志，表示任务正常运行 */
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_0);
		
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
	const TickType_t xFrequency = 500;

	/* 获取当前的系统时间 */
    xLastWakeTime = xTaskGetTickCount();
	
    while(1)
    {
       	bsp_LedToggle(2);
		bsp_LedToggle(3);
		
		/* 发送事件标志，表示任务正常运行 */
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);
		
		/* vTaskDelayUntil是绝对延迟，vTaskDelay是相对延迟。*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/*
*********************************************************************************************************
*	函 数 名: vTaskMsgPro
*	功能说明: LED闪烁
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 3  
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1000;

	/* 获取当前的系统时间 */
    xLastWakeTime = xTaskGetTickCount();
	
    while(1)
    {
       	bsp_LedToggle(1);
		bsp_LedToggle(4);
		
		/* 发送事件标志，表示任务正常运行 */
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);
		
		/* vTaskDelayUntil是绝对延迟，vTaskDelay是相对延迟。*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/*
*********************************************************************************************************
*	函 数 名: vTaskScan
*	功能说明: 按键扫描
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 4  
*********************************************************************************************************
*/
static void vTaskScan(void *pvParameters)
{
    while(1)
    {
		/* 按键扫描 */
		bsp_KeyScan();
		
		/* 发送事件标志，表示任务正常运行 */
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_3);
        vTaskDelay(10);
    }
}

/*
*********************************************************************************************************
*	函 数 名: vTaskStart
*	功能说明: 启动任务，也就是最高优先级任务。
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 5  
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* 最大延迟100ms */
	
	/* 
	  开始执行启动任务主函数前使能独立看门狗。
	  设置LSI是128分频，下面函数参数范围0-0xFFF，分别代表最小值3.2ms和最大值13107.2ms
	  下面设置的是10s，如果10s内没有喂狗，系统复位。
	*/
	bsp_InitIwdg(0xC35);
	
	/* 打印系统开机状态，方便查看系统是否复位 */
	printf("=====================================================\r\n");
	printf("=系统开机执行\r\n");
	printf("=====================================================\r\n");
	
    while(1)
    {
		/* 等待所有任务发来事件标志 */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* 事件标志组句柄 */
							         TASK_BIT_ALL,       /* 等待TASK_BIT_ALL被设置 */
							         pdTRUE,             /* 退出前TASK_BIT_ALL被清除，这里是TASK_BIT_ALL都被设置才表示“退出”*/
							         pdTRUE,             /* 设置为pdTRUE表示等待TASK_BIT_ALL都被设置*/
							         xTicksToWait); 	 /* 等待延迟时间 */
		
		if((uxBits & TASK_BIT_ALL) == TASK_BIT_ALL)
		{
			IWDG_Feed();
			printf("五个用户任务都正常运行\r\n");
		}
	    else
		{
			/* 基本是每xTicksToWait进来一次 */
			/* 通过变量uxBits简单的可以在此处检测那个任务长期没有发来运行标志 */
		}
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
	
	
	xTaskCreate(    vTaskScan,      /* 任务函数  */
                    "vTaskScan",    /* 任务名    */
                    512,            /* stack大小，单位word，也就是4字节 */
                    NULL,           /* 任务参数  */
                    4,              /* 任务优先级*/
                    NULL );         /* 任务句柄  */
					
	xTaskCreate(    vTaskStart,     /* 任务函数  */
                    "vTaskStart",   /* 任务名    */
                    512,            /* stack大小，单位word，也就是4字节 */
                    NULL,           /* 任务参数  */
                    5,              /* 任务优先级*/
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
