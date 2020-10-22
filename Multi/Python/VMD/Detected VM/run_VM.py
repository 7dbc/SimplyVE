import os
import time

#Part A: timer 2 mins for idle
print ("1st idle: 2 minutes")
time.sleep(120)

#Part B: timer 3 mins for basic process
os.system("python3 VM1.py basic")

#Part C: timer 2 mins for idle
print ("2nd idle: 2 minutes")
time.sleep(120)

#Part D: timer 3 mins for intensive process
os.system("python3 VM1.py max")

#Part E: timer 2 mins for idle
print ("3rd idle: 2 minutes")
time.sleep(120)

#Part F: timer 3 mins for special process: Encryption & Decryption, Network
os.system("python3 VM1.py vary")
