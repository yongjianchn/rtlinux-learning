#!/usr/bin/env python
#-*- coding: utf-8 -*-
#Filename: renameAllPatches.py
#Last modified: 2014-06-26 16:54
#Author: Yongjian Xu -- xuyongjiande@gmail.com
#Description: 

import os

mMap={'Jan':'01', \
        'Feb':'02', \
        'Mar':'03', \
        'Apr':'04', \
        'May':'05', \
        'Jun':'06', \
        'Jul':'07', \
        'Aug':'08', \
        'Sep':'09', \
        'Oct':'10', \
        'Dec':'12', \
    'Nov':'11'}
dMap={'1':'01', \
        '2':'02', \
        '3':'03', \
        '4':'04', \
        '5':'05', \
        '6':'06', \
        '7':'07', \
        '8':'08', \
        '9':'09', \
        }
notrenamed=[]
renamedNum=0
for file in os.listdir('./'):
    if '.patch' not in file:
        continue
    ####
    if 'renamed' in file:
        renamedNum += 1
        continue
    ####
    date=os.popen('grep "^Date:" %s|cut -d \' \' -f3-6' % file).read()
    if not date:
        notrenamed.append(file)
        continue
    elist=date.split(' ')
    Y=elist[2]
    if len(elist[1]) != 3:
        notrenamed.append(file)
        continue
    M=mMap[elist[1]]
    D=elist[0]
    if elist[0] in dMap:
        D=dMap[elist[0]]
    time=elist[3][:-1]# delete the lineEnd
    cmd="mv %s renamed-%s-%s-%s-%s-%s" % (file, Y, M, D, time, file)
    print cmd
    os.system(cmd)
    renamedNum += 1

print "Renamed: %d" % renamedNum
print '====='
print "Not Renamed: %d" % (len(notrenamed))
print '====='
for file in notrenamed:
    print file
