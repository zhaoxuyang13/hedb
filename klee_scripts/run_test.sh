#!/bin/bash

rm -rf desen
mkdir desen
for i in {1..22}
do 
    LOG_FILE=~/001-log/Q${i}.log
    OUT_FILE=desen/Q${i}-desen.log
    /usr/bin/time -v ../klee_scripts/desenitize.sh ${LOG_FILE} ${OUT_FILE} 2>&1 >> output.log
done