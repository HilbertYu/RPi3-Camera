#!/bin/bash 
raspivid -vf  -w 640 -h 480 -o - -t 0 -b 2000000 |nc 192.168.50.234 60000

