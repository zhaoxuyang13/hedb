#/bin/bash

usage() {
    echo "Usage: $0 [-o outputfile] [-s schema_file]"
    echo "  -o output filename"
    exit 1
} 

while getopts "o:s:" opt; do
    case $opt in
        o) 
            outputfile=$OPTARG
            ;;
        s)
            schema_file=$OPTARG
            ;;
        \?)
            usage
            ;;
    esac
done



# bash function to load data
load_data() {
    ./test_helper.sh -l
}

# all sqls is qid from 1 to 22
all_sqls=$(seq 1 22)
echo all_sqls = $all_sqls

# execute sqls with ./test_helper.sh, sqls qids is passed as parameter
execute_all_sqls(){
    # local qids=$@
    sql_dir=$1
    # for qid in ${qids[@]}; do
    for qid in $all_sqls; do
        echo "qid: ${qid}"
        # dry run one time
        ./test_helper.sh -f scripts/sqls/${sql_dir}/Q${qid}.sql 2>&1 >/dev/null
        sleep 2
        
        ./test_helper.sh -f scripts/sqls/${sql_dir}/Q${qid}.sql 2>&1 | tee -a ${outputfile}
        # wait 2s for previous query to finish
        sleep 2
    done
}

for branch in plaintext-udf plaintext-udf-enc-size plaintext-udf-float32; do
    git checkout ${branch} -f
    make clean && make && make install
    git checkout main -f
    ./test_helper.sh -l
    # redirect all output to ${outputfile}
    echo "======================" >> ${outputfile}
    echo ${branch} >> ${outputfile}
    
    execute_all_sqls base-sqls
    # ./test_helper.sh -f scripts/sqls/base-sqls/Q18.sql 2>&1 | tee -a ${outputfile}
done

echo "======================" >> ${outputfile}
echo "base" >> ${outputfile}
# checkout to main branch
git checkout main -f

./test_helper.sh -l -p

execute_all_sqls origin-sqls


echo "======================" >> ${outputfile}
echo "enc" >> ${outputfile}
# build sim_ops
make clean && make configure_sim && make && make install
# run ./build/sim_ops in background

./build/sim_ops 666 &

./test_helper.sh -l 

execute_all_sqls base-sqls
# ./test_helper.sh -f scripts/sqls/base-sqls/Q18.sql  2>&1 | tee -a ${outputfile}

# send kill all process called ./build/sim_ops
killall -9 sim_ops
