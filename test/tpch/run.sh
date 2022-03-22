#!/bin/bash
run_tag=$1

if [ ! -n "$run_tag" ] ; then
    echo "you should input run_tag to describe current run"
fi
mkdir -p $run_tag

pg_ports=54322
pg_ip=localhost

sqls=`ls sqls/*.sql`
for sql in $sqls
do 
    number=`echo $sql | awk -F "." '{print $1}' | awk -F "/" '{print $2}'`
    output_file=$run_tag/$number.out
    echo "$sql > $output_file"
    psql -U postgres -h $pg_ip -p $pg_ports -d test -f $sql > $output_file
    sleep 10s
done
