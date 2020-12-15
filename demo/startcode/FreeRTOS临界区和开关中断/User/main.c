/*
*                   临界区解释：
*					 代码的临界短也称为临界区，指处理时不可分割的代码。一旦这部分代码开始执行，则不允许
*                    任何中断打入。为确保临界段代码的执行不被中断，在进入临界段之前必须关中断，而临界段
*                    代码执行完后，要立即开中断。
*                   （1）本实验通过如下两个函数实现进入临界段和推出临界段
*					      taskENTER_CRITICAL();    进入临界区
*					        临界区代码
*					      taskEXIT_CRITICAL();     推出临界区
*
*                    （2）通过如下两个函数实现开关中断，注意，关闭了中断也就关闭了任务的调度：
*		                  taskDISABLE_INTERRUPTS();  关闭中断
*						    防止被中断打断的代码
*                         taskENABLE_INTERRUPTS();  打开中断
*
*                    （3）临界区设置和开关中断的区别。
*                         临界区设置里面也有开关中断操作的，并且支持开关中断的嵌套使用，而单纯的开关
*                         中断操作是不能够嵌套使用的，比如下面：
*                         void FunctionA()
*                         {
*	                          taskDISABLE_INTERRUPTS();  关闭中断
*	                          FunctionB(); 调用函数B
*	                          FunctionC(); 调用函数C
*	                          taskENABLE_INTERRUPTS();  打开中断
*                         }
*
*                         void FunctionB()
*                         {
*	                          taskDISABLE_INTERRUPTS();  关闭中断
*	                          代码
*	                          taskENABLE_INTERRUPTS();  打开中断
*                         }
*                        工程中调用了FunctionA就会出现执行完FunctionB后中断被打开的情况，此时
*                        FunctionC将不被保护了。
*       本实验通过临界区设置和开关中断的功能，实现printf函数的多任务调用。
*********************************************************************************************************
*/
#include "includes.h"


static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void AppTaskCreate (void);


static TaskHandle_t xHandleTaskLED = NULL;

/*******************************************************************************
**函数信息 ：
**功能描述 ：
**输入参数 ：无
**输出参数 ：无
********************************************************************************/
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

/*******************************************************************************
**函数信息 ：
**功能描述 ：
**输入参数 ：无
**输出参数 ：无
********************************************************************************/
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
                    /* 进入临界区 */
					taskENTER_CRITICAL();
					printf("=================================================\r\n");
					printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
					vTaskList((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);

					printf("\r\n任务名       运行计数         使用率\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
					/* 退出临界区 */
					taskEXIT_CRITICAL();
					break;

				/* 其他的键值不处理 */
				default:
					break;
			}
		}

		vTaskDelay(10);
	}
}

/*******************************************************************************
**函数信息 ：
**功能描述 ：
**输入参数 ：无
**输出参数 ：无
********************************************************************************/
static void vTaskLED(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 200;

	/* 获取当前的系统时间 */
    xLastWakeTime = xTaskGetTickCount();

    while(1)
    {
		/* 进入临界区 */
		taskENTER_CRITICAL();
		printf("任务vTaskLED正在运行\r\n");
		/* 退出临界区 */
		taskEXIT_CRITICAL();
		bsp_LedToggle(2);
		bsp_LedToggle(3);

		/* vTaskDelayUntil是绝对延迟，vTaskDelay是相对延迟。*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/*******************************************************************************
**函数信息 ：
**功能描述 ：
**输入参数 ：无
**输出参数 ：无
********************************************************************************/
static void vTaskMsgPro(void *pvParameters)
{
    while(1)
    {
		/* 关闭中断 */
		taskDISABLE_INTERRUPTS();
		printf("任务vTaskMsgPro正在运行\r\n");
		/* 打开中断 */
		taskENABLE_INTERRUPTS();

		bsp_LedToggle(1);
		bsp_LedToggle(4);
		vTaskDelay(300);
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
                    NULL );          /* 任务句柄  */


	xTaskCreate(    vTaskStart,     /* 任务函数  */
                    "vTaskStart",   /* 任务名    */
                    512,            /* stack大小，单位word，也就是4字节 */
                    NULL,           /* 任务参数  */
                    4,              /* 任务优先级*/
                    NULL );         /* 任务句柄  */
}

/*----------------------- (C) COPYRIGHT 2020 www.OS-Q.comm --------------------*/
