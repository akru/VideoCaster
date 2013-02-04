#!/bin/sh

while [ 1 ]; do
    vcap 320 240 65 $1 $2
    echo "Process down! Restarting..."
done
