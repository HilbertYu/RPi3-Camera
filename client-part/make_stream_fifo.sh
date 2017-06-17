#!/bin/bash

rm -fv ./fifo
nc -l 0.0.0.0 60000 > fifo
