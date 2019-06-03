# [FreeRTOS](https://github.com/OS-Q/FreeRTOS) 

[![sites](OS-Q/OS-Q.png)](http://www.os-q.com)

#### 归属实时系统：[RTQ](https://github.com/OS-Q/RTQ)
#### 关于系统架构：[OS-Q](https://github.com/OS-Q/OS-Q)

### [FreeRTOS简介](https://github.com/OS-Q/FreeRTOS/wiki)

在OS-Q实施中，需要对[Q1](https://github.com/OS-Q/Q1) 和 [Q2](https://github.com/OS-Q/Q2) 的设备实时驱动，在一定程度上需要借助系统进行任务调度。

[FreeRTOS](https://www.freertos.org/) 是在嵌入式平台中能与Linux相匹敌的操作系统，它特别适用于开发物联网终端设备。

FreeRTOS缺少Linux功能，比如设备驱动程序、用户帐户以及高级的网络和内存管理。然而，它占用的资源比Linux少得多，更不用说与VxWorks这样的主流实时操作系统相比了，它还提供开源GPL许可证。FreeRTOS可以在内存不到0.5KB、ROM为5-10KB的设备上运行，不过与TCP/IP架构结合使用更为常见，它更像是24KB内存和60KB闪存。

* [Demo例程](Demo/) 

### [OS-Q : Operation System for edge devices](http://www.OS-Q.com/FreeRTOS)