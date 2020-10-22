#!/usr/bin/env python

import subprocess
import time
import matplotlib.pyplot as plt
import numpy as np

CPU_time = []
CPU_speed = []

startTime = time.time()

while (time.time() < startTime + 60 * 12): 
    #run command: use sysbench to achieve benchmark functionality
    proc1 = subprocess.Popen("sysbench --test=cpu --cpu-max-prime=10 run | grep -Eo '[0-9]+\.[0-9]+' | sed -n '3p'", stdout=subprocess.PIPE, shell=True)
    (out1, err1)= proc1.communicate()
    #store the results of above commands into the list and change bytes type to float type
    CPU_speed.append('%.2f' % float(out1.decode("utf-8")))
    #add current time to CPU_time list
    CPU_time.append('%.2f' % (time.time()-startTime))
    print ("cpu_time: ", CPU_time)
    print ("cpu_speed:", CPU_speed)

def graphPlot():
    list1 = [float(i) for i in CPU_speed]
    cpuList = np.array(list1)
    cpuTime = np.array(CPU_time)
    fig,ax = plt.subplots(1)
    # Make your plot, set your labels
    plt.plot(cpuTime, cpuList)
    plt.xlabel('Time')
    plt.ylabel('Events per second')
    #plt.xticks(rotation=40)
    plt.title("CPU Speed")
    plt.autoscale(enable=True, axis=u'both', tight=False)
    ax.set_xticklabels([])
    plt.savefig("NEW_CPU3(1).png", dpi=500)
    plt.show()

graphPlot()

