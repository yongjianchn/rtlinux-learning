# RTLINUX #

# article 1: [a realtime preemption overview](http://lwn.net/Articles/146861/)

## key point

减少kernel代码中非抢占的部分，尽量减少为了添加抢占功能而需要修改的kernel代码。rtlinux补丁充分利用了SMP功能来添加额外的抢占，减少了内核的改写。

## features

1. 抢占临界区
2. 抢占中断处理程序
3. 抢占'interrupt disable'的代码区
4. 优先级继承：kernel内部自旋锁、信号量
5. 延迟操作
6. 潜在因素减少策略

### 1. 抢占临界区

普通的自旋锁`（spinlock_t, rwlock_t)`和RCU读相关的锁（`rcu_read_lock(), rcu_read_unlock()`）是可以抢占的，信号量临界区也是可以抢占的。这意味着申请一个spinlock的时候可以阻塞，也意味着当抢占和中断有一个被禁用的时候不能申请spinlock。

那么在中断或者抢占被关闭的时候，如何申请一个锁呢？做法应该是使用`raw_spinlock_t`替代`spinlock_t`。patch中添加了一系列的用于`spin_lock()`的宏，使得`spin_lock()`作用于`raw_spinlock_t`的时候表现正常，而作用于`spinlock_t`的时候它的临界区可以被抢占。

因为临界区可以被抢占了，所以给定的临界区并非在一个cpu上执行，可能被移到另一个cpu上。所以在临界区中使用per-CPU的变量时需要处理抢占造成的可能，因为spinlock_t和rwlock_t不是正常工作的。

### 2. 抢占中断处理程序

在preempt_rt环境下，几乎所有中断处理程序运行在进程上下文，而不是中断上下文。 可以使用SA_NODELAY标识来指定某些中断使得其处理程序在中断上下文执行。比如fpu_irq, irq0, irq2， lpptest等几个中断。并且这些中断处理程序必须非常谨慎的进行操作，否则容易引起oopses和deadlock.

### 3. 抢占'interrupt disable'的代码区

首先明确就像前一段描述的那样，中断处理程序大部分运行在进程的上下文。任何和中断处理程序打交道的代码都应该准备好和别的cpu上运行的中断处理程序交互。

`spin_lockd_irqsave()`等相关原语不需要禁用抢占，因为如果中断处理程序正在运行，甚至如果它要抢占拥有spinlock_t的代码， 它将在申请spinlock_t时就被阻塞。因此临界区还是被保留的。

`local_irq_save()`仍然需要禁用抢占，因为它没有对应的锁来依赖。使用lock替代local_irq_save可以帮助减少潜在问题，但是会降低SMP性能，所以应当谨慎处置。

和`SA_NODELAY`的中断交互的代码不应该使用`local_irq_save()`，因为这不能禁用硬件中断，应该使用`raw_local_irq_save()`来替代。raw spinlocks `raw_spinlock_t, raw_rwlock_t, raw_seqlock_t`的使用也是同样的道理和原因。然而raw spinlocks和raw interrupt disabling不应该在少数低级操作的区域之外的地方使用。低级操作区域包括scheduler、architectu-redependent代码和RCU等。

### 4. 优先级继承：kernel内部自旋锁、信号量

rt程序员比较关心优先级的问题。比如低优先级a申请lock，被中优先级b抢占，高优先级c也需要申请那个锁，但是被阻塞。这个问题可能使得c无限期推迟执行，可以通过2个方式解决：抑制抢占、优先级继承。

如果抢占被抑制，则问题解决，这种策略被抢占式的内核用于spinlocks但不用于信号量。对于一些实时任务，即使对于spinlocks，抢占也不应该被抑制。

优先级继承则用于“抑制抢占”没作用的情况。其思想是高优先级的任务暂时把优先级赋给低持有lock的优先级的任务。这个思路是可传递的，如果存在更高优先级的任务d，需要任务c持有的锁，那么c和a都会持有任务d的优先级。

当然存在更加复杂的情况，但总之优先级继承阻止了优先级倒置的发生。

### 5. 延迟操作

`spin_lock()`现在可以sleep， 当抢占和中断被禁止的时候，唤醒它是非法的。在某些情况下， 通过延迟申请spinlock的操作（等到抢占再次启用）可以解决这个问题。

### 6. 潜在因素减少策略

在preemptRT的实现中为了减少调度和中断的一些潜在问题，引入了一些修改，关于x86的MMX/SSE硬件和slab分配器中使用per-CPU的变量。

## PREEMPT_RT 原语

介绍被添加的或者被明显改变的原语。

### Locking primitives

+ spinlock_t  
	临界区是可以抢占的， \_irq操作（如spin\_lock\_irqsave()）不禁用硬件中断。优先级继承策略会被使用， 一个底层的rt\_mutex被用来实现spinlock\_t, rwlock\_t, struct semaphore, struct rw\_semaphore等。

+ raw_spinlock_t  
	保留了原本的含义，不能抢占和并且在irq相关操作中真正禁用硬件中断。但是需要注意的是，应当用正常的原语，也就是说除了一些特殊情况（架构相关代码，底层调度...）raw_spinlock_t几乎不应当被使用！因为它会破坏实时性。

+ rwlock_t  
	临界区是可以抢占的， \_irq操作（如spin\_lock\_irqsave()）不禁用硬件中断。优先级继承策略会被使用。为了降低复杂性，一个task只能申请一个rwlock\_t，尽管它可能递归申请。

+ RW_LOCK_UNLOCKED(mylock)
+ raw_rwlock_t
+ seqlock_t
+ SEQLOCK_UNLOCKED(name)
+ struct semaphore
+ down_trylock()
+ struct compat_semaphore
+ struct rw_semaphore
+ struct compat_rw_semaphore

未完待续...
