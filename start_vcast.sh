#!/bin/sh

while [ 1 ]; do
    vcast 320 240 65 $1 $2
    echo "Process down! Restarting..."
done
