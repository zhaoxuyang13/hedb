SET max_parallel_workers = 0;
SET max_parallel_workers_per_gather = 0;
select enable_debug_mode(1);
\timing on
select
    sum(l_extendedprice) / '7.0' as avg_yearly
from
    lineitem, part
where
    p_partkey = l_partkey
    and p_brand = 'Brand#15' 
    and p_container = 'JUMBO CASE'
    and l_quantity < (
        select
            '0.2' * avg(l_quantity)
        from
            lineitem
        where
            l_partkey = p_partkey
    );
select pg_enc_print_stats(true);
