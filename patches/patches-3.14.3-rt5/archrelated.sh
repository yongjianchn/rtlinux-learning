#!/bin/bash
#Filename: archrelated.sh
#Last modified: 2014-06-26 14:17
#Author: Yongjian Xu -- xuyongjiande@gmail.com
#Description: 

echo ==============
echo archrelated
echo ==============
grep "+++ " *|grep -v grep|grep 'arch'|cut -d '.' -f 1|uniq|tee /tmp/arch
archN=`grep "+++ " *|grep -v grep|grep 'arch'|cut -d '.' -f 1|uniq|wc -l`
echo "totally: $archN"

echo ==============
echo noarch
echo ==============
grep "+++ " *|grep -v 'arch'|cut -d '.' -f 1|uniq|tee /tmp/noarch
noarchN=`grep "+++ " *|grep -v 'arch'|cut -d '.' -f 1|uniq|uniq|wc -l`
echo "totally: $noarchN"

echo ==============
echo "arch && noarch"
echo ==============
cat /tmp/arch /tmp/noarch |sort|uniq -d
sameN=`cat /tmp/arch /tmp/noarch |sort|uniq -d|wc -l`
echo "totally: $sameN"
rm /tmp/arch /tmp/noarch
