#!/bin/bash

sql_files=$* 
# sql_files=`ls sqls`
for tag in $sql_files;
do 
    outs=`ls $tag/*.out`
    for out in $outs;
    do 
        time=`cat $out | grep "Time:" | awk '{print $2}'`
        num=`echo $out | awk -F '.' '{print $1}' | awk -F '/' '{print $2}' | sed 's/[^0-9]//g'`
        echo "$num $time"
    done 
done