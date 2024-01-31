#! /bin/bash

target=aesdsocket

case $1 in
    #start aesdsocket in daemon mode
    start) ./$target -d
        ;;
    # send a SIGTERM signal(default is 15 actually) to aesdsocket child process
    stop) pkill -15 $target
        ;;
esac