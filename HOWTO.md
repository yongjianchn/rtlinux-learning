如何编译、安装和测试PREEMPT-RT内核
==================================

下载RT补丁和内核
-------------------------------------------------------------------------------

* 登录此网站[projects-rt](https://www.kernel.org/pub/linux/kernel/projects/rt/)，寻找目标patch
	* 如patch-3.10.58-rt62.patch.xz
* 下载rt patch对应版本的kernel。[kernel代码下载](https://www.kernel.org/pub/linux/kernel/)
	* 如：linux-3.10.58.tar.xz
* 解压并打补丁

	```
	tar xavf linux-3.10.58.tar.xz
	cd linux-3.10.58
	xzcat ../patch-3.10.58-rt62.patch.xz | patch -p1
	```

编译安装新的kernel
-------------------------------------------------------------------------------

### 内核的配置

```
# 拷贝host上的config到内核源码目录下,命名为.config
cp /boot/config-xxx .config
yes '' | make oldconfig
make menuconfig
```

NOTE: 如果要编译preempt-rt内核，需要make menuconfig的时候把CONFIG_PREEMPT_RT_FULL和HIGH_RES_TIMERS选项选中。

PREEMPT_RT_FULL为例:

```
# 在menuconfig的界面，按'/'键，出现搜索界面，输入“preempt_rt_full”，出现搜索结果，然后按出现的数字序号（这里按1）跳转到相应配置
# 按enter键进入选择列表，选中fully preemptible kernel（不同的内核版本，名称可能不同，应该是列表最后一项）。
```

### centos上编译安装kernel

**方法1**

```
make -j4
make modules -j4
sudo make modules_install
sudo make install
```

**方法2**

```
# 生成rpm包：
make rpm-pkg -j4
# You can found the rpm in ~/rpmbuld/RPMS/

# 安装rpm包：
rpm -ivh --force ~/rpmbuild/RPMS/kernel-xxx.rpm

# 制作initrd
# $version 是相应内核的版本， 应当存在/lib/modules/$version这个文件夹
mkinitrd /boot/initrd-$version.img $version

# 修改grub配置
sudo vi /etc/grub2.cfg
# 参考已有的menuentry添加一个新的entry, 修改其中的内核文件和initrd文件为编译和制作出的kernel和initrd
# $kernel is /boot/vmlinuz-xxx  $initrd is /boot/initrd-xxx.img

# 更新grub启动项
sudo grub2-mkconfig

# 重启
sudo reboot # NOW you can use the new kernel
```

测试实时性能
-------------------------------------------------------------------------------

**下载和编译工具**

```
git clone git://git.kernel.org/pub/scm/linux/kernel/git/clrkwllms/rt-tests.git
cd rt-tests && make （可能报错缺失numa，自行安装）
```

**测试**

* 开一个bash，运行sudo ./cyclictest -a -t -n -p99(参数可自行修改，参数含义，自行看help)
* 另开一个bash, 运行sudo ./hackbench -g 100 -l 10000. 
* 回到第一个bash可以看到运行cyclictest的运行延迟（延迟越低，实时性越好）。
