#!/bin/bash
file=$1
cat $file | sed '1d' |  awk -F ',' '{ total += $2; count++} END {print total/count }' 
