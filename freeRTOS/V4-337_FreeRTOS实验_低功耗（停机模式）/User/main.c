/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 本实验主要学习FreeRTOS的低功耗(停机模式)
*              实验目的：
*                1. 学习FreeRTOS的低功耗(停机模式)
*              实验内容：
*                2. 按下按键K1可以通过串口打印任务执行情况
*                   任务名      任务状态 优先级   剩余栈 任务序号
*                   vTaskUserIF     R       1       334     1
*                   IDLE            R       0       116     5
*                   vTaskLED        B       2       484     2
*                   vTaskMsgPro     B       3       484     3
*                   vTaskStart      B       4       490     4
*
*                   任务名       运行计数         使用率
*                   vTaskUserIF     2234            <1%
*                   IDLE            303991          98%
*                   vTaskLED        0               <1%
*                   vTaskMsgPro     1               <1%
*                   vTaskStart      1377            <1%
*                   串口软件建议使用SecureCRT（V4光盘里面有此软件）查看打印信息。
*                    vTaskTaskUserIF 任务：按键消息处理
*                    vTaskLED 任务       ：LED闪烁
*                    vTaskMsgPro 任务    ：LED闪烁
*                    vTaskStart 任务     ：按键扫描
*                 3. 任务运行转态的定义如下，跟上面串口打印字母B, R, D, S对应：
*                    #define tskBLOCKED_CHAR		( 'B' )
*                    #define tskREADY_CHAR		    ( 'R' )
*                    #define tskDELETED_CHAR		( 'D' )
*                    #define tskSUSPENDED_CHAR	    ( 'S' )
*                 4. 关于低功耗的停机模式说明：
*				    (1) 停止模式是在Cortex-M3的深睡眠模式基础上结合了外设的时钟控制机制，在停止模式下电压
*				       调节器可运行在正常或低功耗模式。此时在1.8V供电区域的的所有时钟都被停止，PLL、HSI和
*                      HSE的RC振荡器的功能被禁止，SRAM和寄存器内容被保留下来。
*				    (2) 在停止模式下，所有的I/O引脚都保持它们在运行模式时的状态。
*				    (3) 一定要关闭滴答定时器，实际测试发现滴答定时器中断也能唤醒停机模式。
*				    (4) 当一个中断或唤醒事件导致退出停止模式时， HSI RC振荡器被选为系统时钟。
*				    (5) 退出低功耗的停机模式后，需要重新配置使用HSE和HSE 。
*                 5. K2按键按下将系统从停机模式模式恢复。
*                 6. K3按键按下让系统进入停机模式。
*                 7. 实际项目中推荐采用官方的tickless模式。
*              设计低功耗主要从以下几方面着手：
*                 1. 用户需要根据最低电源消耗、最快速启动时间和可用的唤醒源等条件，选定一个最佳的低功耗模式。
*                    可以使用的低功耗方式有休眠模式，待机模式，停机模式。
*                 2. 选择了低功耗方式后就是关闭可以关闭的外设时钟。
*                 3. 降低系统主频。
*                 4. 注意I/O的状态。
*                    如果此IO口带上拉，请设置为高电平输出或者高阻态输入；
*                    如果此IO口带下拉，请设置为低电平输出或者高阻态输入；
*                    a. 在睡眠模式下，所有的I/O引脚都保持它们在运行模式时的状态。
*                    b. 在停止模式下，所有的I/O引脚都保持它们在运行模式时的状态。
*                    c. 在待机模式下，所有的I/O引脚处于高阻态，除了以下的引脚：
*                      ● 复位引脚(始终有效)
*                      ● 当被设置为防侵入或校准输出时的TAMPER引脚
*                      ● 被使能的唤醒引脚
*                 5. 注意IO和外设IC的连接。
*                 6. 测低功耗的时候，一定不要连接调试器，更不能边调试边测电流。
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

/*
**********************************************************************************************************
											变量声明
**********************************************************************************************************
*/
static TaskHandle_t  xHandleTaskLED = NULL;
static TaskHandle_t  xHandleTaskMsgPro = NULL;

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
*	形    参: 无
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
