
#include "includes.h"


#define BIT_0	(1 << 0)
#define BIT_1	(1 << 1)
#define BIT_ALL (BIT_0 | BIT_1)

static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void AppTaskCreate (void);
static void AppObjCreate (void);


static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static EventGroupHandle_t xCreatedEventGroup = NULL;

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

	/* 创建任务通信机制 */
	AppObjCreate();

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
	EventBits_t uxBits;

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

				/* K2键按下，直接发送事件标志给任务vTaskMsgPro，设置bit0 */
				case KEY_DOWN_K2:
					/* 设置事件标志组的bit0 */
					uxBits = xEventGroupSetBits(xCreatedEventGroup, BIT_0);
					if((uxBits & BIT_0) != 0)
					{
						printf("K2键按下，事件标志的bit0被设置\r\n");
					}
					else
					{
						printf("K2键按下，事件标志的bit0被清除，说明任务vTaskMsgPro已经接受到bit0和bit1被设置的情况\r\n");
					}
					break;

				/* K3键按下，直接发送事件标志给任务vTaskMsgPro，设置bit1 */
				case KEY_DOWN_K3:
					/* 设置事件标志组的bit1 */
					uxBits = xEventGroupSetBits(xCreatedEventGroup, BIT_1);
					if((uxBits & BIT_1) != 0)
					{
						printf("K3键按下，事件标志的bit1被设置\r\n");
					}
					else
					{
						printf("K3键按下，事件标志的bit1被清除，说明任务vTaskMsgPro已经接受到bit0和bit1被设置的情况\r\n");
					}
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
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* 最大延迟100ms */

    while(1)
    {
		/* 等K2按键按下设置bit0和K3按键按下设置bit1 */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* 事件标志组句柄 */
							         BIT_ALL,            /* 等待bit0和bit1被设置 */
							         pdTRUE,             /* 退出前bit0和bit1被清除，这里是bit0和bit1都被设置才表示“退出”*/
							         pdTRUE,             /* 设置为pdTRUE表示等待bit1和bit0都被设置*/
							         xTicksToWait); 	 /* 等待延迟时间 */

		if((uxBits & BIT_ALL) == BIT_ALL)
		{
			/* 接收到bit1和bit0都被设置的消息 */
			printf("接收到bit0和bit1都被设置的消息\r\n");
		}
		else
		{
			/* 超时，另外注意仅接收到一个按键按下的消息时，变量uxBits的相应bit也是被设置的 */
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

/*******************************************************************************
**函数信息 ：
**功能描述 ：
**输入参数 ：无
**输出参数 ：无
********************************************************************************/
static void AppObjCreate (void)
{
	/* 创建事件标志组 */
	xCreatedEventGroup = xEventGroupCreate();

	if(xCreatedEventGroup == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
}

/*----------------------- (C) COPYRIGHT 2020 www.OS-Q.comm --------------------*/
