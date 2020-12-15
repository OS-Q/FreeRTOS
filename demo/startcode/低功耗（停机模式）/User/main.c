
#include "includes.h"



static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void AppTaskCreate (void);

static TaskHandle_t  xHandleTaskLED = NULL;
static TaskHandle_t  xHandleTaskMsgPro = NULL;


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
					printf("=================================================\r\n");
					printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
					vTaskList((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);

					printf("\r\n任务名       运行计数         使用率\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
					break;

				 /* K3键按下 进入到停机模式 */
				case KEY_DOWN_K3:
					printf("K3按键按下，系统进入待机模式，按K2按键将唤醒\r\n");
					/*
					1. 停止模式是在Cortex-M3的深睡眠模式基础上结合了外设的时钟控制机制，在停止模式下电压
						调节器可运行在正常或低功耗模式。此时在1.8V供电区域的的所有时钟都被停止， PLL、 HSI和
						HSE的RC振荡器的功能被禁止， SRAM和寄存器内容被保留下来。
					2. 在停止模式下，所有的I/O引脚都保持它们在运行模式时的状态。
					3. 一定要关闭滴答定时器，实际测试发现滴答定时器中断也能唤醒停机模式。
					*/
					SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  /* 关闭滴答定时器 */
					PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);
					portENTER_CRITICAL();

					/*
					1、当一个中断或唤醒事件导致退出停止模式时， HSI RC振荡器被选为系统时钟。
					2、退出低功耗的停机模式后，需要重新配置使用HSE和HSE
					*/
					SystemInit();
					SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* 使能滴答定时器 */
					portEXIT_CRITICAL();
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
	const TickType_t xFrequency = 500;

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

/*******************************************************************************
**函数信息 ：
**功能描述 ：
**输入参数 ：无
**输出参数 ：无
********************************************************************************/
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
static void vTaskStart(void *pvParameters)
{
    while(1)
    {
		/* 按键扫描 */
		bsp_KeyScan();
        vTaskDelay(10);
    }
}
/*******************************************************************************
**函数信息 ：
**功能描述 ：
**输入参数 ：无
**输出参数 ：无
********************************************************************************/
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
/*----------------------- (C) COPYRIGHT 2020 www.OS-Q.comm --------------------*/
