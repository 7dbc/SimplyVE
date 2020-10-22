#!/usr/bin/env python

import subprocess
import time
import matplotlib.pyplot as plt
import numpy as np

Net_time = []
Net_speed = []

startTime = time.time()

while (time.time() < startTime + 60 * 12):
    #run command: use sysbench to achieve benchmark functionality
    proc3 = subprocess.Popen("iperf -c 192.168.5.68 | grep -Eo '[0-9]+\.[0-9]+' | sed -n '11p'", stdout=subprocess.PIPE, shell=True)
    (out3, err3)= proc3.communicate()
    #store the results of above commands into lists respectively and change bytes type to float type
    Net_speed.append('%.2f' % float(out3.decode("utf-8")))
    #add current time to Mem_time list
    Net_time.append('%.2f' % (time.time()-startTime))
    print ("Net_time: ", Net_time)
    print ("Net_speed:", Net_speed)    

def graphPlot():
    list1 = [float(j) for j in Net_speed]
    netList = np.array(list1)
    netTime = np.array(Net_time)
    fig,ax = plt.subplots(1)
	# Make your plot, set your labels
    plt.plot(netTime, netList)
    plt.ylabel('Bandwidth Mbits/sec')
    plt.xlabel('Time')
    #plt.xticks(rotation=40)
    plt.title("Network Speed")
    plt.autoscale(enable=True, axis=u'both', tight=False)
    ax.set_xticklabels([])
    plt.savefig("NEW_Network.png", dpi=500)
    plt.show()

graphPlot()
