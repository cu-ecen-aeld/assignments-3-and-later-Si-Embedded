#!/bin/bash

if [ $# != 2 ]; then 
    echo "Incorrect amount of parameters."
    exit 1
fi

WRITEFILE=$1
WRITESTR=$2

mkdir -p $(dirname $WRITEFILE) && touch $WRITEFILE

if [ $? -ne 0 ]; then
    echo "Failed to create and write the file."
    exit 1
fi

echo $WRITESTR > $WRITEFILE