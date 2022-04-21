SET max_parallel_workers = 0;
SET max_parallel_workers_per_gather = 0;
select enable_debug_mode(1);
\timing on
select
    o_orderpriority,  count(*) as order_count
from
    orders
where
    o_orderdate >= date '1995-06-01'
and o_orderdate < date '1995-06-01' + interval '3' month
and exists (
    select * from lineitem where
    l_orderkey = o_orderkey and l_commitdate < l_receiptdate
    )
group by
    o_orderpriority
order by
    o_orderpriority;
