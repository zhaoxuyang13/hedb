SET max_parallel_workers = 0;
SET max_parallel_workers_per_gather = 0;
select enable_debug_mode(1);
\timing on
select
    c_count, count(*) as custdist 
from
    (select
        c_custkey,count(o_orderkey)
    from
        customer left outer join orders on
        c_custkey = o_custkey and o_comment not like '%special%packages%'
    group by c_custkey
    )as c_orders (c_custkey, c_count)
group by
    c_count 
order by
    custdist desc,c_count desc;
select pg_enc_print_stats(true);
