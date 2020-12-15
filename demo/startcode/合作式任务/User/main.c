
#include "includes.h"


static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void AppCoTaskCreate(void);
static void AppTaskCreate (void);
static void AppObjCreate (void);
static void vCoRoutineLED(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex);

static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static SemaphoreHandle_t  xSemaphore = NULL;


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
static void vCoRoutineLED(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex)
{
	/* 合作式任务中的变量必须是static类型的 */
	static const portTickType xTimeToDelay[2] = {100, 100};

	/* 每个合作式任务开始的时候必须调用 */
	crSTART( xHandle );

	for( ;; )
	{
		/* 通过uxIndex来区分合作式任务 */
		if(uxIndex == 0)
		{
			bsp_LedToggle(1);
		}
		else if(uxIndex == 1)
		{
			bsp_LedToggle(2);
		}

		crDELAY(xHandle, xTimeToDelay[uxIndex]);
	}

	/* 每个合作式任务结束的时候必须调用 */
	crEND();
}
/*******************************************************************************
**函数信息 ：
**功能描述 ：
**输入参数 ：无
**输出参数 ：无
********************************************************************************/
static void AppCoTaskCreate( void )
{
	uint8_t uxIndex;

	/* 创建两个合作式任务，优先级都是0 */
	for( uxIndex = 0; uxIndex < 2; uxIndex++ )
	{
		xCoRoutineCreate(vCoRoutineLED, 0, uxIndex);
	}
}

/*******************************************************************************
**函数信息 ：
**功能描述 ：
**输入参数 ：无
**输出参数 ：无
********************************************************************************/
void vApplicationIdleHook( void )
{
	vCoRoutineSchedule();
}

/*******************************************************************************
**函数信息 ：
**功能描述 ：
**输入参数 ：无
**输出参数 ：无
********************************************************************************/
static void AppObjCreate (void)
{
	/* 创建二值信号量，首次创建信号量计数值是0 */
	xSemaphore = xSemaphoreCreateBinary();

	if(xSemaphore == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
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

	/* 创建合作式任务 */
	AppCoTaskCreate();

	/* 创建任务通信机制 */
	AppObjCreate();

    /* 启动调度，开始执行任务 */
    vTaskStartScheduler();

	/* 如果系统正常启动是不会运行到这里的，运行到这里极有可能是空闲任务heap空间不足造成创建失败 */
	while(1);
}

/*----------------------- (C) COPYRIGHT 2020 www.OS-Q.comm --------------------*/
