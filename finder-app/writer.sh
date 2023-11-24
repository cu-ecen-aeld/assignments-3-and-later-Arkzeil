#!/bin/bash

# AELD assignment1
# need space after/before '[' and ']'
# space are required when using '!='
if [ $# != "2" ]
then
    echo "$# argements provided, expecting 2 arguments"
    exit 1
fi
# no space allowed when using '='
writefile=$1
writestr=$2
# get directory of 'writefile'
directory=$(dirname "$writefile")

if [ ! -d "$directory" ]
then
    #echo "$directory is not a directory"
    mkdir -p "$directory"
fi

# Use double quotes around variables to handle cases where they may contain spaces or special characters.
echo ""$writestr"" > $writefile
