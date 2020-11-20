
#include "includes.h"

static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void AppTaskCreate (void);

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
	uint8_t ucCount = 0;
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

				/* K2键按下，发送邮箱数据给任务vTaskMsgPro */
				case KEY_DOWN_K2:
					printf("K2键按下，发送邮箱数据给任务vTaskMsgPro，覆盖方式\r\n");
					xTaskNotify(xHandleTaskMsgPro,      /* 目标任务 */
								ucCount++,              /* 发送数据 */
								eSetValueWithOverwrite);/* 如果目标任务没有及时接收，上次的数据会被覆盖 */
					break;

				/* K3键按下 直接发送设置位0x02给任务vTaskMsgPro */
				case KEY_DOWN_K3:
					printf("K3键按下，发送邮箱数据给任务vTaskMsgPro，非覆盖方式\r\n");
				    /* 非覆盖模式的数据发送 */
					if(xTaskNotify(xHandleTaskMsgPro, ucCount++, eSetValueWithoutOverwrite) == pdPASS)
					{
						/* 目标任务的notification value被更新 */
						printf("任务vTaskMsgPro的notification value被更新\r\n");
					}
					else
					{
						/*  目标任务的notification value未更新，这种情况是上次上次的数据被没有接收，不能进行覆盖 */
						printf("任务vTaskMsgPro的notification value未被更新\r\n");
					}


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
	BaseType_t xResult;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(500); /* 设置最大等待时间为500ms */
	uint32_t ulNotifiedValue;
    while(1)
    {
		/*
			第一个参数 ulBitsToClearOnEntry的作用（函数执行前）：
		          notification value &= ~ulBitsToClearOnEntry
		          简单的说就是参数ulBitsToClearOnEntry那个位是1，那么notification value
		          的那个位就会被清零。

		    第二个参数 ulBitsToClearOnExit的作用（函数退出前）：
				  notification value &= ~ulBitsToClearOnExit
		          简单的说就是参数ulBitsToClearOnEntry那个位是1，那么notification value
		          的那个位就会被清零。

			采用函数xTaskNotifyWait实现类似消息邮箱的功能，变量ulNotifiedValue是接收到的数据
		*/

		xResult = xTaskNotifyWait(0x00000000,       /* 函数执行前保留notification value所有位 */
						          0xFFFFFFFF,       /* 函数退出前清除notification value所有位 */
						          &ulNotifiedValue, /* 保存notification value到变量ulNotifiedValue中 */
						          xMaxBlockTime);   /* 最大允许延迟时间 */

		if(xResult == pdPASS)
		{
			printf("接收到消息邮箱数据ulNotifiedValue = %d\r\n", ulNotifiedValue);
		}
		else
		{
			/* 超时 */
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
