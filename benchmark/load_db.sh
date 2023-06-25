#!/bin/bash 


plain_text=false

while getopts "p" opt; do
    case $opt in
        p) 
            plain_text=true
            ;;
        \?)
            usage
            ;;
    esac
done

data_dir=tools/tmp
schema_dir=db_schemas
db_name=test
db_user=postgres
db_host=localhost
db_port=5432

if [ "$plain_text" = true ]; then
    schema_file=tpch-schema.sql
    data_dir=tools
else 
    schema_file=tpch-schema-encrypted.sql
    data_dir=tools/tmp
fi
psql -U $db_user -h $db_host -p $db_port -d $db_name -f $schema_dir/$schema_file
chmod a+rw tools/*.tbl

# tpch tables list 
tables="customer lineitem nation orders part partsupp region supplier"


# load data to postgresql database, remove last character which is '|' in every row
cat $data_dir/nation.tbl | sed 's/.$//' | psql -U $db_user -h $db_host -p $db_port -d $db_name -c "copy nation (n_nationkey, n_name, n_regionkey, n_comment) from stdin with delimiter '|';"
cat $data_dir/region.tbl | sed 's/.$//' | psql -U $db_user -h $db_host -p $db_port -d $db_name -c "copy region (r_regionkey, r_name, r_comment) from stdin with delimiter '|';"
cat $data_dir/part.tbl | sed 's/.$//' | psql -U $db_user -h $db_host -p $db_port -d $db_name -c "copy part (p_partkey, p_name, p_mfgr, p_brand, p_type, p_size, p_container, p_retailprice, p_comment) from stdin with delimiter '|';"
cat $data_dir/supplier.tbl | sed 's/.$//' | psql -U $db_user -h $db_host -p $db_port -d $db_name -c "copy supplier (s_suppkey, s_name, s_address, s_nationkey, s_phone, s_acctbal, s_comment) from stdin with delimiter '|';"
cat $data_dir/partsupp.tbl | sed 's/.$//' | psql -U $db_user -h $db_host -p $db_port -d $db_name -c "copy partsupp (ps_partkey, ps_suppkey, ps_availqty, ps_supplycost, ps_comment) from stdin with delimiter '|';"
cat $data_dir/customer.tbl | sed 's/.$//' | psql -U $db_user -h $db_host -p $db_port -d $db_name -c "copy customer (c_custkey, c_name, c_address, c_nationkey, c_phone, c_acctbal, c_mktsegment, c_comment) from stdin with delimiter '|';"
cat $data_dir/orders.tbl | sed 's/.$//' | psql -U $db_user -h $db_host -p $db_port -d $db_name -c "copy orders (o_orderkey, o_custkey, o_orderstatus, o_totalprice, o_orderdate, o_orderpriority, o_clerk, o_shippriority, o_comment) from stdin with delimiter '|';"
cat $data_dir/lineitem.tbl | sed 's/.$//' | psql -U $db_user -h $db_host -p $db_port -d $db_name -c "copy lineitem (l_orderkey, l_partkey, l_suppkey, l_linenumber, l_quantity, l_extendedprice, l_discount, l_tax, l_returnflag, l_linestatus, l_shipdate, l_commitdate, l_receiptdate, l_shipinstruct, l_shipmode, l_comment) from stdin with delimiter '|';"

# load index file 
psql -U $db_user -h $db_host -p $db_port -d $db_name -f $schema_dir/tpch-index.sql

