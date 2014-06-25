# Getting started

## 什么是real-time？

real-time的应用在某事件触发到其做出回应之间有deadline。为了满足deadline的需求，需要使用RTOS（real-time os），这样的话，最慢反应时间可以根据应用和环境被计算出来。

典型的RTOS使用优先级， 这样一个任务的等待时间就只依赖于优先级更高或者同等的任务，其他的任务可以被忽略。而正常的OS（如正常的linux）的一个任务的延迟则依赖系统所有正常运行的任务，很难确定有一个某一个应用在deadline前做出反应，因为**preemption**(抢占)可能被关闭，并且关闭的时间也不确定。

## linux2.6上的实时性能

通常情况下，linux只在一些情况下允许一个进程进行抢占操作。

+ CPU 在user-mode
+ 系统调用或者中断返回到user-space
+ kernel代码被阻塞，或者明确放弃（explicitly yield）

这样的话，即使是高优先级的线程也不能抢占正在运行的kernel代码，必须等到kernel代码明确放弃控制权。

2.6内核配置了`CONFIG_PREEMPT_VOLUNTARY`来引入对长反应时间的检测，使得kernel可以自行放弃控制权，并运行高优先级的任务。它减少了程序反应时间过长的可能，却没有完全消除。不像`CONFIG_PREEMPT`对系统 的影响那么大。

`CONFIG_PREEMPT`的选项则导致所有内核代码不在spinlock的保护之中，中断处理程序将有非自愿的被高优先级内核线程抢占。在这个选项下，最坏的情况，任务反应时间降低到个位数的毫秒级别，如果一个实时linux程序需要延迟小于这个量级，那么应该使用`CONFIG_PREEMPT_RT`补丁。

## rt补丁如何工作？

rt补丁将内核变成完全抢占式的，主要做了下列事情：

+ 修改了lock原语的实现，使得可以抢占
+ 被`spinlock_t, rwlock_t等`保护的临界区可以抢占， `raw_spinlock_t, raw_rwlock_t等`保留不能抢占的特性
+ 优先级继承priority inheritance的实现
+ 中断处理程序变成可被抢占的内核线程， 软中断被视为内核线程上下文， 就像用户空间的进程一样拥有一个task_struct。
+ 原本的linux定时器的API拥有了更高的分辨率，使得用户空间POSIX timer也拥有了更高高分辨率。

## rt补丁支持什么架构

x86 x86_64 ARM MIPS Power等架构的系统都有成功的例子，但是需要说明的是rt补丁的可用并非只跟cpu架构有关，因为这不止跟指令集有关，还跟cpu或者cpu支撑芯片以及设备驱动能提供高分辨率的定时器有关。所以即使都是arm芯片也不一定都能同样使用rt，可能存在某一个拥有更高的延迟或者性能不好的情况。具体测试过的平台，参考[官方说明](https://rt.wiki.kernel.org/index.php/CONFIG_PREEMPT_RT_Patch#Platforms_Tested_and_in_Use_with_CONFIG_PREEMPT_RT)

## 如何使用rt-patch

+ 下载patch和对应的vanilla kernel
+ 打patch
+ 配置内核CONFIG_PREEMPT_RT_FULL等选项（各版本不同）
