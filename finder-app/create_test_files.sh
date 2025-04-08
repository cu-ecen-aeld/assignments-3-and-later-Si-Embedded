#!/bin/bash

for index in $(seq 1 10)
do
    writer.sh ./test/test$index.txt my_test$index
done