renamed-2011-07-13-14:05:05-perf-move-irq-work-to-softirq-in-rt.patch
===

+ 修改了arch/x86/kernel/irq_work.c, 如果配置了`CONFIG_PREEMPT_RT_FULL`，则删除函数`arch_irq_work_raise()`的定义

+ 修改了kernel/irq_work.c, 如果配置了RTFULL则删除`__irq_work_run(void)`函数中的`BUG_ON(!irqs_disabled());`语句, 以及函数`irq_work_run(void)`中的`BUG_ON(!in_irq());`语句

+ 修改kernel/timer.c, 如果定义了`CONFIG_PREEMPT_RT_FULL`，则在函数`update_process_times()`中，`if (in_irq()) irq_work_run();`不会被调用。而在函数`run_timer_softirq()`中调用`irq_work_run()`。

+ 总结起来应该是irq_work_run()的调用时机发生了变化

renamed-2011-07-18-13:59:17-softirq-disable-softirq-stacks-for-rt.patch
===

+ 如果配置了RT_FULL选项，那么删除如下定义：
	+ arch/powerpc/kernel/irq.c `void do_softirq_own_stack(void)`
	+ arch/powerpc/kernel/misc_32.S `_GLOBAL(call_do_softirq)`
	+ arch/powerpc/kernel/misc_64.S `_GLOBAL(call_do_softirq)`
	+ arch/sh/kernel/irq.c `void do_softirq_own_stack(void)`
	+ arch/sparc/kernel/irq_64.c `void do_softirq_own_stack(void)`
	+ arch/x86/kernel/entry_64.S `ENTRY(do_softirq_own_stack)`
	+ arch/x86/kernel/irq_32.c `void do_softirq_own_stack(void)`

+ 在include/linux/interrupt.h中做如下修改：  
	添加`thread_do_softirq()`函数函数定义，并且规定，如果没有RT_FULL，`static inline void thread_do_softirq(void) { do_softirq(); }`， 否则`extern void thread_do_softirq(void);`， 在kernel/softirq.c 中添加了实现。

+ 总结： disable softirq stack执行方式，应该是让中断处理程序运行在进程上下文而不是中断上下文。

renamed-2011-07-24-12:11:43-kconfig-disable-a-few-options-rt.patch
===

+ 修改arch/Kconfig, mm/Kconfig, 使得OPROFILE选项和MOMMU_INITIAL_TRIM_EXCESS选项的开启的时候不能使用PREEMPT_RT_FULL

renamed-2011-07-28-10:43:51-mm-rt-kmap-atomic-scheduling.patch
===

+ arch/x86/kernel/process_32.c添加了`static void switch_kmaps(struct task_struct *prev_p, struct task_struct *next_p)`函数

+ arch/x86/mm/highmem_32.c修改了`kmap_atomic_prot()`函数
+ arch/x86/mm/iomap_32.c修改了`kmap_atomic_prot_pfn()`函数
+ include/linux/highmem.h增加了per-cpu的东西，并且修改了`kmap_atomic_idx_push()`， `kmap_atomic_idx_pop()`函数，不是太懂
+ include/linux/sched.h在highmem或x86_32情况下，增加了全局的kmap_idx, kmap_ptr[]
+ mm/highmem.c中的修改对应着前几处修改的地方，有些东西本来在这个里面的，rt把他们放到别的地方了，可能还有修改。
+ mm/memory.c修改了`pagefault_disable()`函数，添加了migrate_disable()和migrate_enable()的调用，似乎是为了保护一些操作

renamed-2011-08-11-15:31:31-peter_zijlstra-frob-pagefault_disable.patch
===

+ 修改了很多, 跟pagefault相关

```
11  arch/alpha/mm/fault.c      |    2 +-
12  arch/arm/mm/fault.c        |    2 +-
13  arch/avr32/mm/fault.c      |    3 +--
14  arch/cris/mm/fault.c       |    2 +-
15  arch/frv/mm/fault.c        |    2 +-
16  arch/ia64/mm/fault.c       |    2 +-
17  arch/m32r/mm/fault.c       |    2 +-
18  arch/m68k/mm/fault.c       |    2 +-
19  arch/microblaze/mm/fault.c |    2 +-
20  arch/mips/mm/fault.c       |    2 +-
21  arch/mn10300/mm/fault.c    |    2 +-
22  arch/parisc/mm/fault.c     |    2 +-
23  arch/powerpc/mm/fault.c    |    2 +-
24  arch/s390/mm/fault.c       |    4 ++--
25  arch/score/mm/fault.c      |    2 +-
26  arch/sh/mm/fault.c         |    2 +-
27  arch/sparc/mm/fault_32.c   |    2 +-
28  arch/sparc/mm/fault_64.c   |    2 +-
29  arch/tile/mm/fault.c       |    2 +-
30  arch/um/kernel/trap.c      |    2 +-
31  arch/x86/mm/fault.c        |    2 +-
32  arch/xtensa/mm/fault.c     |    2 +-
33  include/linux/sched.h      |   14 ++++++++++++++
34  kernel/fork.c              |    2 ++
35  24 files changed, 39 insertions(+), 24 deletions(-)
```

renamed-2011-12-14-01:03:49-cpumask-disable-offstack-on-rt.patch
===

+ CPUMASK_OFFSTACK 要求 PREEMPT_RT_FULL关闭，也就是PREEMPT_RT_FULL要求CPUMASK_OFFSTACK关闭

renamed-2013-02-13-11:03:11-arm-enable-highmem-for-rt.patch
===

根据总的patch的分析，之前作者在arm上为了rt的实现关闭了highmem，这个patch用于兼容highmem和rt

+ 修改了arch/arm/include/asm/switch_to.h, 为了在`switch_to(prev, next, last)`过程中调用`switch_kmaps(prev, next)`函数。此函数是rt定义在arch/arm/mm/highmem.c里面的。

+ 修改了arch/arm/mm/highmem.c里面的`kmap_atomic()`和`__kunmap_atomic()`， `kmap_atomic_pfn()` `kmap_atomic_to_page()`, 定义了上一行描述的`switch_kmaps()`函数

+ 修改了include/linux/highmem.h 增加`#include <linux/sched.h>`, 因为linux/sched.h中新增了一些全局的东西[参考: renamed-2011-08-11-15:31:31-peter_zijlstra-frob-pagefault_disable.patch]。

renamed-2014-01-31-14:20:31-irq_work-allow-certain-work-in-hard-irq-context.patch
===

+ arch/arm/kernel/smp.c arch/powerpc/kernel/time.c arch/sparc/kernel/pcr.c几个arch相关的地方，删除了`arch_irq_work_raise()`函数, 根据后续分析，此函数在kernel/irq_work.c中也有定义，这里应该是统一使用了那个地方的定义。

+ include/linux/irq_work.h

	新增一个宏`#define IRQ_WORK_HARD_IRQ 8UL`

+ kernel/irq_work.c

	修改`irq_work_queue()`函数， `__irq_work_run()`函数, 跟per-cpu的变量有关，细节不了解

+ kernel/time/tick-sched.c

	> ```
	> 142  static DEFINE_PER_CPU(struct irq_work, nohz_full_kick_work) = {
	> 143  |  .func = nohz_full_kick_work_func,
	> 144 +|  .flags = IRQ_WORK_HARD_IRQ, //add this line
	> 145  };
	> ```
+ kernel/timer.c

	之前的一个patch在rt模式下不会在`update_process_times()`中调用`irq_work_run()`， 现在可以调用了。

+ 总结： 硬件中断相关的一些内容， 此外发现很多时候，rt的开发是先关闭某个功能(不调用一些函数)，然后在实现了一些相关的东西之后，又启用那些功能。所以某个patch独立出来不一定能理解，需要按照时间线来理解。或者直接看最后的大patch，不过局部性信息就没有小patch突出了。

renamed-notime-oleg-signal-rt-fix.patch
===

+ 修改了x86的arch/x86/include/asm/signal.h, arch/x86/kernel/signal.c, 两处地方功能完成了一个修改：在`do_notify_resume()`函数中加入

	> ```
	> 66 +#ifdef ARCH_RT_DELAYS_SIGNAL_SEND
	> 67 +|  if (unlikely(current->forced_info.si_signo)) {
	> 68 +|  |   struct task_struct *t = current;
	> 69 +|  |   force_sig_info(t->forced_info.si_signo,|&t->forced_info, t);
	> 70 +|  |   t->forced_info.si_signo = 0;
	> 71 +|  }
	> 72 +#endif
	> ```

+ include/linux/sched.h 和kernel/signal.c两个文件的修改， 把函数`force_sig_info()`重命名为`do_force_sig_info()`， 而`force_sig_info()`则由rt来重新实现（如果配置rt则调用新的，否则调用旧的）。
