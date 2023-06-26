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




out_dir=tmp
# execute sqls with ./test_helper.sh, sqls qids is passed as parameter
execute_all_sqls(){
    mkdir -p $out_dir
    # local qids=$@
    sql_dir=$1
    # for qid in ${qids[@]}; do
    for qid in $all_sqls; do
        echo "qid: ${qid}"
    
        # dry run one time
        ./test_helper.sh -f scripts/sqls/${sql_dir}/Q${qid}.sql 2>&1 >/dev/null
        ./test_helper.sh -f scripts/sqls/${sql_dir}/Q${qid}.sql 2>&1 >/dev/null
        ./test_helper.sh -f scripts/sqls/${sql_dir}/Q${qid}.sql 2>&1 >/dev/null
        sleep 5

        # sqls="explain analyze `cat scripts/sqls/${sql_dir}/Q${qid}.sql`" 
        # psql -U postgres -d test -c "$sqls" 2>&1 | tee -a ${outputfile}
        # sleep 5

        ./test_helper.sh -o $out_dir/${qid}.out -f scripts/sqls/${sql_dir}/Q${qid}.sql 2>&1 | tee -a ${outputfile}
        # wait 1s for previous query to finish
        sleep 1
    done
}

switch_to_branch(){
    git stash
    git checkout $1
}

switch_back(){
    git checkout main
    git stash pop
}

run_plaintext_udf(){
    for branch in plain-udf plain-udf-enc-size ; do
        set -x
        switch_to_branch ${branch}
        make clean && make && make install
        switch_back
        set +x
        make load-tpch
        # redirect all output to ${outputfile}
        echo "======================" >> ${outputfile}
        echo ${branch} >> ${outputfile}
        
        out_dir=tmp/${branch}
        execute_all_sqls origin-sqls
        # ./test_helper.sh -f scripts/sqls/base-sqls/Q18.sql 2>&1 | tee -a ${outputfile}
    done
}
run_native(){
    echo "======================" >> ${outputfile}
    echo "base" >> ${outputfile}
    # checkout to main branch
    git checkout main

    make load-tpch-native

    out_dir=tmp/native
    execute_all_sqls origin-sqls
}

run_enc(){
    echo "======================" >> ${outputfile}
    echo "enc" >> ${outputfile}
    # build sim_ops
    make clean && make configure_sim && make && make install
    # run ./build/sim_ops in background

    ./build/sim_ops 666 &

    make load-tpch

    out_dir=tmp/enctext
    execute_all_sqls origin-sqls
    # ./test_helper.sh -f scripts/sqls/base-sqls/Q18.sql  2>&1 | tee -a ${outputfile}

    # send kill all process called ./build/sim_ops
    killall -9 sim_ops
}

run_benchmark(){
    # ./test_helper.sh -l -p
    out_dir=tmp/udf-kv-2
    execute_all_sqls origin-sqls

}

run_udf_kv(){
    echo "======================" >> ${outputfile}
    echo "udf_kv" >> ${outputfile}

    for scale_factor in 0.1 0.2 0.5 1 2 ; do
        echo "scale_factor: ${scale_factor}"

        git stash

        git checkout udf-kv

        make prepare-tpch WAREHOUSE=${scale_factor}
        
        git checkout main
        git stash pop 
        
        out_dir=tmp/udf-kv-${scale_factor}
        execute_all_sqls origin-sqls
    done
}

run_udf_kv

# run_benchmark