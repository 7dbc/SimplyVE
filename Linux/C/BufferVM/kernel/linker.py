#!/usr/bin/python3

import sys
import math
from subprocess import call

#print ('Number of arguments:', len(sys.argv), 'arguments.')
#print('\n'.join(map(str,sys.argv)))

c_flags = ""
link_flags = ""
lib_flags = ""
objects = ""
target = ""
linker_script = ""

def page_align(x):
    return int(math.ceil(x / 4096.0)) * 4096

def link(map):
    command = "gcc " + objects + " " + c_flags + " " + link_flags + \
              " -T script.ld" + (" -Xlinker -Map=sections.map " if map else " ") + \
              lib_flags + \
              " -o " + ("/dev/null" if map else target)

    call(command, shell=True)

for i in range(1,(len(sys.argv)-1)//2*2+1,2):
    if sys.argv[i] == '-c':
        c_flags = sys.argv[i+1]
    elif sys.argv[i] == '-l':
        link_flags = sys.argv[i+1]
    elif sys.argv[i] == '-L':
        lib_flags = sys.argv[i+1]
    elif sys.argv[i] == '-o':
        objects = sys.argv[i+1]
    elif sys.argv[i] == '-t':
        target = sys.argv[i+1]
    elif sys.argv[i] == '-s':
        linker_script = sys.argv[i+1]

call("cpp -x c -undef -USECOND_LINK " + linker_script + " -o script.ld && sed -i '/^\s*#/d' script.ld", shell=True)
link(True)

#process map - this is fairly grim but it gets the job done right? (the whole fact I need to do this is a hack anyway)

min_page_addr = 2**64 - 1
max_page_addr = 0

with open('sections.map') as f:
    stage = 0
    for line in f:
        line = line.rstrip()
        if stage == 0:
            if line == "Linker script and memory map":
                stage = 1
        elif stage == 1:
            if line == "":
                stage = 2
        elif stage == 2:
            if line.startswith("LOAD "):
                stage = 3
        elif stage == 3:
            if not line.startswith("LOAD "):
                stage = 4
        if stage == 4:
            if line.startswith("OUTPUT("):
                break;
            if not line.startswith(" ") and line != "":
                fields = line.strip().split()
                if len(fields) >= 3:
                    start = int(fields[1], 16)
                    size = int(fields[2], 16)

                    min_page_addr = min(min_page_addr, start)
                    max_page_addr = max(max_page_addr, start + size)

binary_size = page_align(abs(min_page_addr - max_page_addr))

call("cpp -x c -undef -DSECOND_LINK -DBINARY_SIZE=" + hex(binary_size) + " " + linker_script + " script.ld && sed -i '/^\s*#/d' script.ld", shell=True)
link(False)