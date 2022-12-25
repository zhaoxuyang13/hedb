#!/bin/bash
target=$1
run_tag=$2

if [ ! -n "$run_tag" ] ; then
    echo "you should input run_tag to describe current run"
fi

mkdir -p $run_tag

PG_LOCAL=1
pg_ports=5432
pg_ip=localhost

sqls=`ls $target/*.sql`
for sql in $sqls
do
    number=`echo $sql | awk -F "." '{print $1}' | awk -F "/" '{print $3}'`
    output_file=$run_tag/$number.out

    psql -U postgres -d testnative -h $pg_ip -p $pg_ports -f $sql > $output_file
    querytime=`cat $output_file | grep "Time:" | awk '{print $2}'`
    echo $querytime
    sleep 3s
done
