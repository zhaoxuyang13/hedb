#!/bin/bash

sql_files=$*
# sql_files=`ls sqls`
# echo "Q, Time, Storage"
for tag in $sql_files;
do
    outs=`ls $tag/*.out`
    for out in $outs;
    do
        time=`cat $out | grep "Time:" | awk '{print $2}'`
        num=`echo $out | awk -F 'Q' '{print $2}' | awk -F '.' '{print $1}'`
        echo "Q$num"
        echo "$time" | ./calc.awk
    done
done