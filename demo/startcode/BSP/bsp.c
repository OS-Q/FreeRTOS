
#include "bsp.h"

/*******************************************************************************
**函数信息 ：
**功能描述 ：
**输入参数 ：无
**输出参数 ：无
********************************************************************************/
void bsp_Init(void)
{
	/*
		由于ST固件库的启动文件已经执行了CPU系统时钟的初始化，所以不必再次重复配置系统时钟。
		启动文件配置了CPU主时钟频率、内部Flash访问速度和可选的外部SRAM FSMC初始化。
		系统时钟缺省配置为72MHz，如果需要更改，可以修改 system_stm32f10x.c 文件
	*/
	/* 优先级分组设置为4, 优先配置好NVIC */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	bsp_InitUart(); 	/* 初始化串口 */
	bsp_InitLed(); 		/* 初始LED指示灯端口 */
	bsp_InitKey();		/* 初始化按键 */
	bsp_InitHardTimer();
}

/*----------------------- (C) COPYRIGHT 2020 www.OS-Q.comm --------------------*/