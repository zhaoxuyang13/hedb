#!/bin/bash

usage() {
    echo "Usage: $0 [-l] [-w warehouse] [-s schema] [-v] [-h host] [-p port] [-d database] [-U user] [-f file] [-c command]"
    echo "  -l  load benchmark data"
    echo "  -v  vacuum full of all tables"
    echo "  -h  print this message"
    exit 1
} 

loading=false
vacuuming=true
warehouse=0
schema=tpch
host=localhost
port=5432
database=test
user=postgres


while getopts "hlvpw:s:f:c:" opt; do
    case $opt in
        p) 
            plaintext=true
            ;;
        l)
            loading=true
            ;;
        v)
            vacuuming=true
            ;;
        w) 
            warehouse=$OPTARG
            ;;
        s)
            schema=$OPTARG
            ;;
        f)
            file=$OPTARG
            ;;
        c)
            command=$OPTARG
            ;;
        
        h)
            usage
            ;;
        \?)
            usage
            ;;
    esac
done

if [ "$schema" == "tpch" ] ; then
    if [ "$plaintext" = true ] ; then
        schema_file="db_schemas/tpch-schema.sql"
    else
        schema_file="db_schemas/tpch-schema-encrypted.sql"
    fi
    index_file="db_schemas/tpch-index.sql"
    config_file="config/tpch_config.xml"
    java_args="-jar bin/tpch.jar -b tpch"
    if [ `expr $warehouse \> 0` -ne 0 ] ; then
        echo "generate test data"
        cd benchmark/tools
        echo "./dbgen -vf -s $warehouse"
        ./dbgen -vf -s $warehouse
        cd -
    fi
fi

# if loading data, dont execute any query
if [ "$loading" = true ] ; then
    echo "loading data"
    cd benchmark 
    echo "psql -h ${host} -p ${port} -U ${user} -d ${database} -f ${schema_file}"
    psql -h ${host} -p ${port} -U ${user} -d ${database} -f ${schema_file}
    psql -h ${host} -p ${port} -U ${user} -d ${database} -f ${index_file}
    sed -i "s#<DBUrl>.*</DBUrl>#<DBUrl>jdbc:postgresql://${host}:${port}/${database}</DBUrl>#" ${config_file}
    echo "java -Dlog4j.configuration=log4j.properties ${java_args} -o output -s 100 --config ${config_file} --load true --execute false"
    java -Dlog4j.configuration=log4j.properties ${java_args} -o output -s 100 --config ${config_file} --load true --execute false
    if [ "$vacuuming" = true ] ; then
        echo "load complete, vacuuming"
        psql -h ${host} -p ${port} -U ${user} -d ${database} -c "vacuum full"
    fi
    exit 0
fi

# print the command executed
set -x

psql -U postgres -d test -c "\o /dev/null" \
                        -c "\timing" \
                        -f $file
                        # -c "set max_parallel_workers=0;" \

                         