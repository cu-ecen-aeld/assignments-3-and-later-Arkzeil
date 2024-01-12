#!/bin/sh

# AELD assignment1
# need space after/before '[' and ']'
# space are required when using '!='
if [ $# != "2" ]
then
    echo "$# argements provided, expecting 2 arguments"
    exit 1
fi
# no space is allowed when using '='
filesdir=$1
searchstr=$2

if [ ! -d "$filesdir" ]
then
    echo "$filesdir is not a directory"
    exit 1
fi

# Use double quotes around variables to handle cases where they may contain spaces or special characters.
echo "The number of files are $(find "$filesdir" -type f | wc -l) and the number of matching lines are $(grep -r "$searchstr" "$filesdir" | wc -l)"  
