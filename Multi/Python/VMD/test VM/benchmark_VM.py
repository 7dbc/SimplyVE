#!/usr/bin/env python

import subprocess
import multiprocessing
from multiprocessing import Process, Manager
import time
from matplotlib import pyplot as plt 

#two lists can communicate with other processes, store the average speed of each stage and then used to plot graphs
CPU_speed=multiprocessing.Manager().list()
Mem_speed=multiprocessing.Manager().list()

#funtion to measure CPU speed and Memory speed
def measure(timer_minutes):
    endTime = time.time() + 5 * timer_minutes
    p1 = Process(target=Mem_measure, args=(endTime,))
    p1.start()
    p2 = Process(target=CPU_measure, args=(endTime,))
    p2.start()
    p1.join()
    p2.join()

#1st subprocess to measure CPU performance
def CPU_measure(end_Time): 
    #list to store the temporary CPU data
    CPU = []
    sum1 = 0
    while (time.time() < end_Time): 
        #run command: use sysbench to achieve benchmark functionality
        proc1 = subprocess.Popen("sysbench --test=cpu --cpu-max-prime=10 run | grep -Eo '[0-9]+\.[0-9]+' | sed -n '3p'", stdout=subprocess.PIPE, shell=True)
        (out1, err1)= proc1.communicate()
        #store the results of above commands into the list and change bytes type to float type
        CPU.append(float(out1.decode("utf-8")))
        print ("CPU: ", float(out1.decode("utf-8")))

    #sum up and then get the average of CPU speed
    for num1 in CPU:
        sum1 = sum1 + num1
    average_CPU = sum1 / len(CPU)
    CPU_speed.append('%.2f' % average_CPU)
    print (CPU_speed)
    print ("cpu speed(events per second): ", '%.2f' % average_CPU)

#2nd subprocess to measure memory performance
def Mem_measure(end_Time):
    #list to store the temporary memory data  
    mem = []
    sum2 = 0
    while (time.time() < end_Time): 
        #run command: use sysbench to achieve benchmark functionality
        proc2 = subprocess.Popen("sysbench --test=memory --memory-block-size=1M --memory-total-size=10G run | grep -Eo '[0-9]+\.[0-9]+' | sed -n '3p'", stdout=subprocess.PIPE, shell=True)
        (out2, err2)= proc2.communicate()
        #store the results of above commands into lists respectively and change bytes type to float type
        mem.append(float(out2.decode("utf-8")))
        print ("Memory: ", float(out2.decode("utf-8")))

    #sum up and then get the average of Memory speed
    for num2 in mem:
        sum2 = sum2 + num2
    average_Mem = sum2 / len(mem)
    Mem_speed.append('%.2f' % average_Mem)
    print (Mem_speed)
    print ("Memory speed: ", '%.2f' % average_Mem, "MiB/sec")

#function to measure in A-F cases
def measureAll():
#Part A: timer 2 mins for idle
    measure(2)
#Part B: timer 3 mins for basic VM
    measure(3)
#Part C: timer 2 mins for idle
    measure(2)
#Part D: timer 3 mins for intensive VM
    measure(3)
#Part E: timer 2 mins for idle
    measure(2)
#Part F: timer 3 mins for special VM: Encryption & Decryption, Network
    measure(3)

#plot two graphs, one for CPU speed and the other is for Memory speed
def graphPlot():
    #x axis scale: at the 2nd min, 5th min, 7th min, 10th min, 12th min, 15th min
    x_axis = [2, 5, 7, 10, 12, 15] 
    #y axis: convert string to float in the two lists
    cpuList = [float(i) for i in CPU_speed]
    memList = [float(j) for j in Mem_speed]

    fig = plt.figure()
    ax1 = fig.add_subplot(1, 2, 1)
    ax2 = fig.add_subplot(1, 2, 2)
    ax1.plot(x_axis, cpuList, label='CPU speed')
    ax2.plot(x_axis, memList, label='Memory speed')
    ax1.set_xlabel('Time (min)')
    ax1.set_ylabel('Events per second')
    ax1.set_title('CPU Speed')
    ax1.legend()
    ax2.set_xlabel('Time (min)')
    ax2.set_ylabel('MiB transferred per second')
    ax2.set_title('Memory Speed')
    ax2.legend()
    plt.show()

measureAll()
graphPlot()