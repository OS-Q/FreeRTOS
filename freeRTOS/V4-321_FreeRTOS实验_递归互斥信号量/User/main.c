/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 本实验主要学习FreeRTOS的递归互斥信号量
*              实验目的：
*                1. 学习FreeRTOS的递归互斥信号量
*                2. 递归互斥信号量，其实就是互斥信号量里面嵌套互斥信号量
*              实验内容：
*                3. 按下按键K1可以通过串口打印任务执行情况
*                   任务名      任务状态 优先级   剩余栈 任务序号
*                   vTaskUserIF     R       1       334     1
*                   IDLE            R       0       120     5
*                   vTaskMsgPro     B       3       458     3
*                   vTaskLED        B       2       458     2
*                   vTaskStart      B       4       490     4
*
*                   任务名       运行计数         使用率
*                   vTaskUserIF     2053            3%
*                   IDLE            49735           92%
*                   vTaskMsgPro     824             1%
*                   vTaskLED        1188            2%
*                   vTaskStart      1               <1%
*                   串口软件建议使用SecureCRT（V4光盘里面有此软件）查看打印信息。
*                    vTaskTaskUserIF 任务：按键消息处理
*                    vTaskLED 任务       ：递归互斥信号量的使用
*                    vTaskMsgPro 任务    ：递归互斥信号量的使用
*                    vTaskStart 任务     ：按键扫描
*                 4. 任务运行转态的定义如下，跟上面串口打印字母B, R, D, S对应：
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
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
static void AppObjCreate (void);

/*
**********************************************************************************************************
											变量声明
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static SemaphoreHandle_t  xRecursiveMutex = NULL;

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
					xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
					printf("=================================================\r\n");
					printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
					vTaskList((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
				
					printf("\r\n任务名       运行计数         使用率\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
					xSemaphoreGiveRecursive(xRecursiveMutex);	
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
*	功能说明: 递归互斥信号量的使用
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 2  
*********************************************************************************************************
*/
static void vTaskLED(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1000;

	/* 获取当前的系统时间 */
    xLastWakeTime = xTaskGetTickCount();
	
    while(1)
    {
       	/* 递归互斥信号量，其实就是互斥信号量里面嵌套互斥信号量 */
		xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
		{
			/* -------------------------------------------------------------------------- */
			   //假如这里是被保护的资源，第1层被保护的资源
			/* -------------------------------------------------------------------------- */
			printf("任务vTaskLED在运行，第1层被保护的资源，用户可以在这里添加被保护资源\r\n");
			
			/* 第1层被保护的资源里面嵌套被保护的资源 */
			xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
			{
				/* ---------------------------------------------------------------------- */
			    //假如这里是被保护的资源，第2层被保护的资源
				/* ---------------------------------------------------------------------- */
				printf("任务vTaskLED在运行，第2层被保护的资源，用户可以在这里添加被保护资源\r\n");
				
				/* 第2层被保护的资源里面嵌套被保护的资源 */
				xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
				{
					printf("任务vTaskLED在运行，第3层被保护的资源，用户可以在这里添加被保护资源\r\n");
					bsp_LedToggle(2);
					bsp_LedToggle(3);
				}
				xSemaphoreGiveRecursive(xRecursiveMutex);
			}
			xSemaphoreGiveRecursive(xRecursiveMutex);
		}
		xSemaphoreGiveRecursive(xRecursiveMutex);
		
		/* vTaskDelayUntil是绝对延迟，vTaskDelay是相对延迟。*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/*
*********************************************************************************************************
*	函 数 名: vTaskMsgPro
*	功能说明: 递归互斥信号量的使用
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 3  
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1500;

	/* 获取当前的系统时间 */
    xLastWakeTime = xTaskGetTickCount();
	
    while(1)
    {
		/* 递归互斥信号量，其实就是互斥信号量里面嵌套互斥信号量 */
		xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
		{
			/* -------------------------------------- */
			   //假如这里是被保护的资源，第1层被保护的资源，用户可以在这里添加被保护资源
			/* ---------------------------------------------------------------------------- */
			printf("任务vTaskMsgPro在运行，第1层被保护的资源，用户可以在这里添加被保护资源\r\n");
			
			/* 第1层被保护的资源里面嵌套被保护的资源 */
			xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
			{
				/* ------------------------------------------------------------------------ */
			    //假如这里是被保护的资源，第2层被保护的资源，用户可以在这里添加被保护资源
				/* ------------------------------------------------------------------------ */
				printf("任务vTaskMsgPro在运行，第2层被保护的资源，用户可以在这里添加被保护资源\r\n");
				
				/* 第2层被保护的资源里面嵌套被保护的资源 */
				xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
				{
					printf("任务vTaskMsgPro在运行，第3层被保护的资源，用户可以在这里添加被保护资源\r\n");
					bsp_LedToggle(1);
					bsp_LedToggle(4);
				}
				xSemaphoreGiveRecursive(xRecursiveMutex);
			}
			xSemaphoreGiveRecursive(xRecursiveMutex);	
		}
		xSemaphoreGiveRecursive(xRecursiveMutex);
		
		/* vTaskDelayUntil是绝对延迟，vTaskDelay是相对延迟。*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
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
	/* 创建递归互斥信号量 */
    xRecursiveMutex = xSemaphoreCreateRecursiveMutex();
	
	if(xRecursiveMutex == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
