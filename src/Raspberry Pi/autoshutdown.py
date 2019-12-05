#!/usr/bin/python
import os, time
 
while True:
	time.sleep(90) #1.5 minutes
    if '10.178' not in os.popen('ifconfig | grep 10').read():# change the number '10.178' and '10' according to wifi IP info
        os.system('sudo shutdown now')
    