SET max_parallel_workers = 0;
SET max_parallel_workers_per_gather = 0;
select enable_debug_mode(1);
\timing on
select
    sum(l_extendedprice*l_discount) as revenue 
from
    lineitem
where
    l_shipdate >= date '1995-01-01' 
and l_shipdate < date '1995-01-01' + interval '1' year 
and l_discount between '0.07' and '0.09' 
and l_quantity < '24';
select pg_enc_print_stats(true);
