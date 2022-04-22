SET max_parallel_workers = 0;
SET max_parallel_workers_per_gather = 0;
select enable_debug_mode(1);
\timing on
select
    '100.00' * sum(case
        when p_type like 'PROMO%' 
        then l_extendedprice*('1' - l_discount) 
        else '0' end) / sum(l_extendedprice * ('1' - l_discount))
    as promo_revenue
from
    lineitem, part
where
    l_partkey = p_partkey
    and l_shipdate >= date '1995-06-01' 
    and l_shipdate < date '1995-06-01' + interval '1' month;
select pg_enc_print_stats(true);
