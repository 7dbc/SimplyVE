#!/usr/bin/env python

import subprocess
import multiprocessing
from multiprocessing import Process, Manager
import time
from matplotlib import pyplot as plt 

#three lists can communicate with other processes, store the speed results of each data point and then used to plot y-axis 
CPU_speed = multiprocessing.Manager().list()
Mem_speed = multiprocessing.Manager().list()
Net_speed = multiprocessing.Manager().list()

#three lists can communicate with other processes, store the time of each data point and then used to plot x-axis
CPU_time = multiprocessing.Manager().list()
Mem_time = multiprocessing.Manager().list()
Net_time = multiprocessing.Manager().list()

startTime = time.time()

#funtion to measure CPU speed and Memory speed
def measure(timer_minutes):
    endTime = time.time() + 60 * timer_minutes
    p1 = Process(target=Mem_measure, args=(endTime,))
    p1.start()
    p2 = Process(target=CPU_measure, args=(endTime,))
    p2.start()
    p3 = Process(target=Net_measure, args=(endTime,))
    p3.start()
    p1.join()
    p2.join()
    p3.join()

#1st subprocess to measure CPU performance
def CPU_measure(end_Time): 
    while (time.time() < end_Time): 
        #run command: use sysbench to achieve benchmark functionality
        proc1 = subprocess.Popen("sysbench --test=cpu --cpu-max-prime=10 run | grep -Eo '[0-9]+\.[0-9]+' | sed -n '3p'", stdout=subprocess.PIPE, shell=True)
        (out1, err1)= proc1.communicate()
        #store the results of above commands into the list and change bytes type to float type
        CPU_speed.append('%.2f' % float(out1.decode("utf-8")))
        #add current time to CPU_time list
        CPU_time.append('%.2f' % (time.time()-startTime))
        print ("cpu_time: ", CPU_time)
        print ("cpu_speed:", CPU_speed)

#2nd subprocess to measure memory performance
def Mem_measure(end_Time):
    while (time.time() < end_Time): 
        #run command: use sysbench to achieve benchmark functionality
        proc2 = subprocess.Popen("sysbench --test=memory --memory-block-size=2M --memory-total-size=30G run | grep -Eo '[0-9]+\.[0-9]+' | sed -n '3p'", stdout=subprocess.PIPE, shell=True)
        (out2, err2)= proc2.communicate()
        #store the results of above commands into lists respectively and change bytes type to float type
        Mem_speed.append('%.2f' % float(out2.decode("utf-8")))
        #add current time to Mem_time list
        Mem_time.append('%.2f' % (time.time()-startTime))
        print ("mem_time: ", Mem_time)
        print ("mem_speed:", Mem_speed)

#3rd subprocess to measure network performance
def Net_measure(end_Time):
    while (time.time() < end_Time): 
        #run command: use sysbench to achieve benchmark functionality
        proc3 = subprocess.Popen("iperf -c 192.168.5.68 | grep -Eo '[0-9]+\.[0-9]+' | sed -n '11p'", stdout=subprocess.PIPE, shell=True)
        (out3, err3)= proc3.communicate()
        #store the results of above commands into lists respectively and change bytes type to float type
        Net_speed.append('%.2f' % float(out3.decode("utf-8")))
        #add current time to Mem_time list
        Net_time.append('%.2f' % (time.time()-startTime))
        print ("Net_time: ", Net_time)
        print ("Net_speed:", Net_speed)     

#function to measure in A-F cases
def measureAll():
#Part A: timer 2 mins for idle
    measure(2)
#Part B: timer 3 mins for basic process
    measure(3)
#Part C: timer 2 mins for idle
    measure(2)
#Part D: timer 3 mins for intensive process
    measure(3)
#Part E: timer 2 mins for idle
    measure(2)
#Part F: timer 3 mins for special process: Encryption & Decryption, Network
    measure(3)

#plot two graphs, one for CPU speed and the other is for Memory speed
def graphPlot(): 
    #y axis: convert string to float in the two lists
    cpuList = [float(i) for i in CPU_speed]
    memList = [float(j) for j in Mem_speed]
    netList = [float(j) for j in Net_speed]

    plt.figure()
    plt.plot(CPU_time, cpuList, label='CPU speed')
    plt.xlabel('Time (sec)')
    plt.ylabel('Events per second')
    plt.xticks(rotation=40)
    plt.ylim([0, 6600000])
    plt.title('CPU Speed')
    plt.legend()
    plt.savefig('CPU.png')
    plt.show()

    plt.figure()
    plt.plot(Mem_time, memList, label='Memory speed')
    plt.xlabel('Time (sec)')
    plt.ylabel('MiB transferred per second')
    plt.xticks(rotation=40)
    plt.ylim([0, 9000])
    plt.title('Memory Speed')
    plt.legend()
    plt.savefig('Memory.png')
    plt.show()

    plt.figure()
    plt.plot(Net_time, netList, label='Network speed')
    plt.xlabel('Time (sec)')
    plt.ylabel('Bandwidth Mbits/sec')
    plt.xticks(rotation=40)
    plt.ylim([0, 100])
    plt.title('Network Speed')
    plt.legend()
    plt.savefig('Network.png')
    plt.show()

measureAll()
graphPlot()