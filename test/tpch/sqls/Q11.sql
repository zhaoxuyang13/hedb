SET max_parallel_workers = 0;
SET max_parallel_workers_per_gather = 0;
select enable_debug_mode(1);
\timing on
select
    ps_partkey,sum(ps_supplycost * ps_availqty) as value 
from
    partsupp, supplier, nation
where
    ps_suppkey = s_suppkey and s_nationkey = n_nationkey and n_name = 'JAPAN'
group by
    ps_partkey 
having 
    sum(ps_supplycost * ps_availqty) > 
( select sum(ps_supplycost * ps_availqty) * '0.0001'
  from
    partsupp, supplier, nation 
  where
    ps_suppkey = s_suppkey 
    and s_nationkey = n_nationkey 
    and n_name = 'JAPAN')
order by value desc;
select pg_enc_print_stats(true);
