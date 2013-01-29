#!/bin/sh

while [ 1 ]; do
    vcap $1 $2
    echo "Process down! Restarting..."
done
