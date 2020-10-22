#!/usr/bin/env python

import argparse
import time
import multiprocessing
import random
import hashlib
import sys
import os
import subprocess
import simplecrypt as sc

class CommStruct:
    def __init__(self, ptr, time):
        self.ptr = ptr
        self.time = time

class BackgroundProcess:
    """Background Process class that takes a command and forks it 
    into a separate process"""
    def __init__(self, processName):
        self.processName = processName
        self.id = hex(random.getrandbits(128))
        self.alive = True
        
        if os.path.exists('.' + self.id):
            os.remove('.' + self.id)
        
    def start(self):
        """Parses and starts passed command"""

        if type(self.processName) != type("string"):
            string = str(self.processName).split(' ')[1]
            print("Starting command '%s'" % string)
            self.proc = multiprocessing.Process(target=self.processName, args=(self.id,))
            self.proc.start()
        else:
            print("Starting command '%s'" % self.processName)
            
            self.proc = multiprocessing.Process(target=self.callProc)
            self.proc.start()

    def callProc(self):
        """Wrapper process for CLI commands, as you cannot pass required args to subprocess.call()"""
        
        subprocess.call(self.processName, shell=True)
        f = open('.' + self.id, 'w')
        f.write("dead")
        f.close()
        print("Process has died")
    
    def isAlive(self):
        """Returns bool of whether the proc is dead or not"""
        self.proc.join(timeout=0)
        if self.proc.is_alive():
            return 1
        return 0

def specialProcess(procnum):
    """Continuously encrypts and decrypts data until the endtime is hit"""
    password = "superDuperPassword"
    data = "Secret something"
    enc = sc.encrypt(password, data)
    sc.decrypt(password, enc)
    print("Completed encrypt/decrypt routines")

def helloWorld(self):
    print("Hello world!")

def intense(catch):
    """Simple password cracking function (as an alternative to CLI-based programs)"""

    # hash for SuperPassword
    target = "52101400a06b0d716b0092edf68c492b" 

    # Password file
    f = open("passes.txt", 'rb')
    #f = open("/usr/share/wordlists/password/rockyou.txt", 'rb')
    data = b""
    ctr = 0
    end = False
    print("[+] Loading wordlist into memory")
    try:
        data = f.read()
        data = data.split(b'\n')
    except UnicodeDecodeError:
        pass

    ctr = 0
    f.close()
    print("[+] Passwords loaded: %s" % len(data))

    print("[+] Breaking password...")
    for password in data:
        password = password.strip(b"\n")
        hashobj = hashlib.md5()
        hashobj.update(password)
        guess = hashobj.hexdigest()
        if guess == target:
            print("[+] Password identified: %s" % password.decode())
            return 0
        if ctr % 10000 == 0:
            print("Current index: %d, Password: %s, Hash: %s" % (ctr, password, guess))
        ctr +=1

    print("[-] No password identified for hash: %s" % target)
    print("\tPasswords tried: %d" % ctr)
    return 1

def main():
    parser = argparse.ArgumentParser(description="Synchronizes executions across multiple machines")
    parser.add_argument('type', help="Define the execution profile")
    
    args = parser.parse_args()

    comms = []

    if args.type == "basic":
        print("[+] Starting 'basic' VM command list...")
        comms = [CommStruct(helloWorld, 3*60)]
    elif args.type == "max":
        print("[+] Starting 'max' VM command list...")
        comms = [CommStruct(intense, 3*60)]
    elif args.type == "vary":
        print("[+] Starting 'vary' VM command list...")
        comms = [CommStruct(specialProcess, 2*60), 
            CommStruct("curl http://ipv4.download.thinkbroadband.com/5MB.zip --output some.file", 2*60)]
            #CommStruct("rm file.iso", 0)
    else:
        print("[-] Illegal type argument: '%s'" % args.type)
        sys.exit(1)

    no_limit = False
    end_time = 0
    for process in comms:
        proc = BackgroundProcess(process.ptr)
        if process.time == 0:
            no_limit = True
        else:
            end_time = time.time() + process.time
        while (time.time() < end_time): 
            proc.start()

main()