/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 本实验主要学习FreeRTOS的动态内存管理(heap_4)
*              实验目的：
*                1. 学习FreeRTOS的动态内存管理(heap_4)
*                2. FreeRTOS提供了5中内存管理方法，V4板子配套的都是用的方法4，heap_4文件。
*                   这5种方法的优缺点可以官网如下地址查看：
*                   http://www.freertos.org/a00111.html
*              实验内容：
*                3. 按下按键K1可以通过串口打印任务执行情况
*                   任务名      任务状态 优先级   剩余栈 任务序号
*                   vTaskUserIF     R       1       334     1
*                   IDLE            R       0       120     5
*                   vTaskLED        B       2       484     2
*                   vTaskMsgPro     B       3       460     3
*                   vTaskStart      B       4       490     4
*
*                   任务名       运行计数         使用率
*                   vTaskUserIF     3363            3%
*                   IDLE            98270           96%
*                   vTaskLED        0               <1%
*                   vTaskMsgPro     167             <1%
*                   vTaskStart      2               <1%
*                   串口软件建议使用SecureCRT（V4光盘里面有此软件）查看打印信息。
*                    vTaskTaskUserIF 任务：按键消息处理
*                    vTaskLED 任务       ：LED闪烁
*                    vTaskMsgPro 任务    ：使用函数xSemaphoreTake接收任务vTaskTaskUserIF发送的同步信号
*                    vTaskStart 任务     ：按键扫描
*                 4. 任务运行转态的定义如下，跟上面串口打印字母B, R, D, S对应：
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 5. K2键按下 直接发送同步信号给任务vTaskMsgPro 。
*                 6. K3按键按下，返回当前系统剩余动态内存大小。
*                 7. 摇杆OK键按下，申请1024字节内存，然后释放，并打印申请前后动态内存的剩余情况。
*
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
static SemaphoreHandle_t  xSemaphore = NULL;

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
	uint8_t *pMallocMem;
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

				/* K3按键按下，返回当前系统剩余动态内存大小 */
				case KEY_DOWN_K3:
					printf("K2键按下，当前动态内存大小xPortGetFreeHeapSize() = %d\r\n", xPortGetFreeHeapSize());
					break;

				/* 摇杆OK键按下，申请1024字节内存，然后释放，并打印申请前后动态内存的剩余情况 */
				case JOY_DOWN_OK:
					printf("当前动态内存大小xPortGetFreeHeapSize() = %d\r\n", xPortGetFreeHeapSize());
					pMallocMem = (uint8_t *)pvPortMalloc(1024);
					printf("申请1024字节内存后大小xPortGetFreeHeapSize() = %d\r\n", xPortGetFreeHeapSize());
					vPortFree(pMallocMem);
					printf("释放1024字节内存后大小xPortGetFreeHeapSize() = %d\r\n", xPortGetFreeHeapSize());
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
	/* 初始化有1个可用资源，当前可用资源为0，此时计数信号量的功能等同二值信号量 */
 	xSemaphore = xSemaphoreCreateCounting(1, 0);

	if(xSemaphore == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
}

/***************************** 安富莱www.OS-Q.comm (END OF FILE) *********************************/
