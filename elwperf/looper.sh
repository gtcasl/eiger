#!/bin/bash
tool=$1
while read line           
do           
    eval $tool $line
done 
