#!/usr/bin/env python

import subprocess
import time
import matplotlib.pyplot as plt
import numpy as np

Mem_time = []
Mem_speed = []

startTime = time.time()

while (time.time() < startTime + 60 * 12):
    #run command: use sysbench to achieve benchmark functionality
    proc2 = subprocess.Popen("sysbench --test=memory --memory-block-size=2M --memory-total-size=30G run | grep -Eo '[0-9]+\.[0-9]+' | sed -n '3p'", stdout=subprocess.PIPE, shell=True)
    (out2, err2)= proc2.communicate()
    #store the results of above commands into lists respectively and change bytes type to float type
    Mem_speed.append('%.2f' % float(out2.decode("utf-8")))
    #add current time to Mem_time list
    Mem_time.append('%.2f' % (time.time()-startTime))
    print ("mem_time: ", Mem_time)
    print ("mem_speed:", Mem_speed)

def graphPlot():
    list1 = [float(j) for j in Mem_speed]
    memList = np.array(list1)
    memTime = np.array(Mem_time)
    fig,ax = plt.subplots(1)
    # Make your plot, set your labels
    plt.plot(memTime, memList)
    plt.ylabel('MiB transferred per second')
    plt.xlabel('Time')
    #plt.xticks(rotation=40)
    plt.title("Memory Speed")
    plt.autoscale(enable=True, axis=u'both', tight=False)
    ax.set_xticklabels([])
    plt.savefig("NEW_Memory.png", dpi=500)
    plt.show()

graphPlot()
