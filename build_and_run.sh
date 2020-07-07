#!/bin/bash

g++ -g tracker.cpp Common/MemoryCommon.cpp DolphinProcess/DolphinAccessor.cpp DolphinProcess/Linux/LinuxDolphinProcess.cpp MemoryWatch/MemWatchEntry.cpp -lncurses -o tracker
sudo setcap cap_sys_ptrace=eip /home/ethan/mp-term-tracker/tracker
./tracker
