
#include "includes.h"


static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void AppTaskCreate (void);
static void TIM_CallBack1(void);


static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;

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
	const TickType_t xBlockTime = pdMS_TO_TICKS(500); /* 设置最大等待时间为500ms */
	uint32_t ulNotifiedValue;
    while(1)
    {
		ulNotifiedValue = ulTaskNotifyTake(pdFALSE,  	/* 1. 此参数设置为pdFALSE，接收到的notification value减一 2. 此参数设置为pdTRUE，接收到的notification value清零 */
						                   xBlockTime); /* 无限等待 */
		if( ulNotifiedValue > 0 )
        {
			/* 接收到消息 */
			printf("任务vTaskMsgPro接收到消息，ulNotifiedValue = %d\r\n", ulNotifiedValue);
        }
        else
        {
			bsp_LedToggle(1);
			bsp_LedToggle(4);
        }
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

/*----------------------- (C) COPYRIGHT 2020 www.OS-Q.comm --------------------*/
