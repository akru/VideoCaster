#!/bin/sh

while [ 1 ]; do
    vcast /dev/video0 2 320 240 80 $1 $2
    echo "Process down! Restarting..."
done
