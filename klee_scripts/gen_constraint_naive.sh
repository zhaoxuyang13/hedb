#!/bin/bash

KTEST_FILE_DIR=$1

SMT_OUTPUT_DIR=smt2-files


ROOT_DIR=${PWD}/../
SCRIPT_DIR=${ROOT_DIR}/klee_scripts
BUILD_DIR=${ROOT_DIR}/build
# SMT2_FILENAME=${BUILD_DIR}/tmp/${OUTPUT_TMP_ID}-constraints.smt2

# set -o xtrace

# call klee to generate smt2 constraints 
# mkdir -p ${OUTPUT_TMP_DIR}

# gen_constraint (pid.constraint) 

# KLEE=/home/klee/klee_src/build/bin/klee
KLEE=klee

# ${SCRIPT_DIR}/run_klee.sh ${BUILD_DIR}/whole.bc ops ${BUILD_DIR}/${KTEST_FILENAME} > /dev/null 2>&1
rm -rf klee-output-tmp-*
NUM=`ls ktests | wc -l`
BEGIN=0
STEP=10000
# echo total ${NUM} ktests, run with BATCH SIZE ${STEP}
for file in `ls ktests-tmp`
do
    ${KLEE} \
        --write-smt2s \
        --entry-point=ops_wrapper \
        --warnings-only-to-file --output-stats=0 \
        --posix-runtime --libc=uclibc \
        --use-forked-solver=0 \
        --seed-file=${KTEST_FILE_DIR}/${file} \
        --named-seed-matching --only-replay-seeds \
        --always-output-seeds=0 --use-branch-cache=0 \
        ${BUILD_DIR}/whole.bc  
done





    # --seed-file=${BUILD_DIR}/0-gen.ktest \
    # --seed-file=${BUILD_DIR}/1-gen.ktest \

# mv ${OUTPUT_TMP_DIR}/test000001.smt2 ${SMT2_FILENAME}
# mv ${OUTPUT_TMP_DIR}/test000001.smt2 ${SMT2_FILENAME}
# mv klee-last/test000001.ktest ${BUILD_DIR}/basic.ktest # equivalent output file generated by klee
# mv klee-last/test000001.smt2 $SMT2_FILENAME

# rm -rf ${OUTPUT_TMP_DIR}

# solve smt constraints
# ITERS=1
# ${SCRIPT_DIR}/run_z3.py ${SMT2_FILENAME} ${ITERS} >> ${OUTPUT_FILENAME}

