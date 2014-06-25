RT PREEMPT HOWTO
================

## Install

+ Download

```
wget ftp://ftp.kernel.org/pub/linux/kernel/v3.x/linux-3.14.3.tar.xz
wget https://www.kernel.org/pub/linux/kernel/projects/rt/3.14/patch-3.14.3-rt5.patch.xz
```

+ Apply patch

```
tar xavf linux-3.14.3.tar.xz
cd linux-3.14.4
xzcat ../patch-3.14.3.rt5.patch.xz | patch -p1
```

+ Configuration and compilation

```
make menuconfig
```

搜索并配置下面2项开关为开启状态

```
HIGH_RES_TIMERS
PREEMPT_RT_FULL
```

然后正常编译安装内核

## run

### check the kernel

+ version

```
uname -a
```

+ process list

```
ps ax #这里可以看到有很多softirq
```

+ interrupts

```
cat /proc/interrupts #这里可以看到，跟vanilla kernel的输出比，多了一列额外的信息。类似\[........N/  0\].

I (IRQ_INPROGRESS) - IRQ handler active
D (IRQ_DISABLED) - IRQ disabled
P (IRQ_PENDING) - IRQ pending
R (IRQ_REPLAY) - IRQ has been replayed but not acked yet
A (IRQ_AUTODETECT) - IRQ is being autodetected
W (IRQ_WAITING) - IRQ not yet seen - for autodetection
L (IRQ_LEVEL) - IRQ level-triggered
M (IRQ_MASKED) - IRQ masked - shouldn't be seen again
N (IRQ_NODELAY) - IRQ must run immediately
```

### change priority of a Thread

```
chrt -f -p $PRIO $PID # 修改pid的优先级
chrt -p $PID # 显示pid的优先级信息
```

## real-time programming

pass...
