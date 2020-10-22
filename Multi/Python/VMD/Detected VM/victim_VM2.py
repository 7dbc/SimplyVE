#!/usr/bin/env python

import time
import hashlib
import os
import simplecrypt as sc
from cryptography.fernet import Fernet

def helloWorld(duration):
    endTime = time.time() + duration
    while (time.time() < endTime): 
        print("Hello world!")
    print("---------------------------Basic process completed---------------------------")

def intense(duration):
    """Simple password cracking function (as an alternative to CLI-based programs)"""
    endTime = time.time() + duration
    while (time.time() < endTime): 
        message = "my deep dark secret".encode()
        # generate a key
        key = Fernet.generate_key()
        # encrypt messages
        f = Fernet(key)
        encrypted = f.encrypt(message)
        # decrypt messages
        decrypted = f.decrypt(encrypted)
    print("---------------------------Crypto process completed---------------------------")
        
def specialProcess(duration):
    """Continuously downloads data"""
    endTime = time.time() + duration
    while (time.time() < endTime): 
        os.system("curl http://ipv4.download.thinkbroadband.com/1GB.zip --output some.file")
    print("-------------------------Downloading process completed-------------------------")

def videoStreaming():
    os.system("vlc --play-and-exit https://www.youtube.com/watch?v=XCbcDBm2lqo")
    print("-----------------------Video streaming process completed-----------------------")


#Part A: timer 1 min for idle
print ("1st idle: 1 minutes")
time.sleep(60)

#Part B: timer 2 mins for basic process
print ("---------------------------Basic process starts--------------------------")
helloWorld(120)

#Part C: timer 1 min for idle
print ("2nd idle: 1 minutes")
time.sleep(60)

#Part D: timer 2 mins for intensive process
print ("---------------------------Crypto process starts--------------------------")
intense(120)

#Part E: timer 1 min for idle
print ("3rd idle: 1 minutes")
time.sleep(60)

#Part F: timer 2 mins for video streaming
print ("-----------------------Video streaming process starts---------------------")
videoStreaming()

#Part G: timer 1 min for idle
print ("3rd idle: 1 minutes")
time.sleep(60)

#Part H: timer 2 mins for special process: Downloading
print ("-------------------------Downloading process starts-----------------------")
specialProcess(120)

print ("ALL DONE!!!")