#!/bin/bash

LOG_FILE=$1 # Qx.log
DESEN_FILE=$2 # Qx-desen.log

ROOT_DIR=${PWD}/..
SCRIPT_DIR=${ROOT_DIR}/klee_scripts
BUILD_DIR=${ROOT_DIR}/build

# decrypt and transform logfile to ktest files into directory ${BUILD}/ktests
rm -rf ktest
${BUILD_DIR}/desenitizer ${LOG_FILE}

# use klee to generate constraints(SMT2 files) into directory ${BUILD}/klee-output-tmp
rm -rf klee-output-tmp
${SCRIPT_DIR}/gen_constraint.sh ${BUILD_DIR}/ktests

# use z3 to solve constraints and output to file
rm ${DESEN_FILE}
${SCRIPT_DIR}/run_z3.py ${BUILD_DIR}/klee-output-tmp ${DESEN_FILE}



