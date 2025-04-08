#!/bin/bash

if [ $# != 2 ]; then 
    echo "Incorrect amount of parameters."
    exit 1
fi

FILESDIR=$1
SEARCHSTR=$2

if [ ! -d $FILESDIR ]; then
    echo "Argument 1 is not a directory."
    exit 1
fi

FILES=$(find $FILESDIR -type f | wc -l)
MATCHES=$(find $FILESDIR -type f -exec grep $SEARCHSTR {} \;| wc -l)

echo "The number of files are $FILES and the number of matching \
lines are $MATCHES."