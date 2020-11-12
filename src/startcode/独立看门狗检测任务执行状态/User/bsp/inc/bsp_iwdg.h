/*
*********************************************************************************************************
*
*	模块名称 : 独立看门狗驱动
*	文件名称 : bsp_iwdg.c
*	版    本 : V1.0
*	说    明 : IWDG驱动程序
*	修改记录 :
*		版本号    日期        作者       说明
*		V1.0    2015-08-10   Eric2013   正式发布
*
*	Copyright (C), 2015-2020, 安富莱www.OS-Q.comm
*
*********************************************************************************************************
*/

#ifndef _BSP_IWDG_H
#define _BSP_IWDG_H



/** 描述    : 独立看门狗初始化*/
void bsp_InitIwdg(uint32_t _ulIWDGTime);


/** 描述    : 喂独立看门狗*/
void IWDG_Feed(void);

#endif
