SET max_parallel_workers = 0;
SET max_parallel_workers_per_gather = 0;
select enable_debug_mode(1);
\timing on
select
    c_custkey, c_name, 
    sum(l_extendedprice * ('1' - l_discount)) as revenue,
    c_acctbal, n_name, c_address, c_phone, c_comment 
from
    customer, orders, lineitem, nation
where
c_custkey = o_custkey
and l_orderkey = o_orderkey
and o_orderdate >= date '1993-08-01' 
and o_orderdate < date '1993-08-01' + interval '3' month 
and l_returnflag = 'R' 
and c_nationkey = n_nationkey
group by
    c_custkey,c_name,c_acctbal,c_phone,n_name,c_address,c_comment
order by
    revenue desc;
