SET max_parallel_workers = 0;
SET max_parallel_workers_per_gather = 0;
select enable_debug_mode(1);
\timing on
select
    l_shipmode,
    sum(case 
        when o_orderpriority ='1-URGENT' or o_orderpriority ='2-HIGH'
        then 1 else 0 end) as high_line_count,
    sum(case 
        when o_orderpriority <> '1-URGENT'
        and o_orderpriority <> '2-HIGH' 
        then 1 else 0 end) as low_line_count
from
    orders,lineitem
where
    o_orderkey = l_orderkey
    and l_shipmode in ('TRUCK', 'REG AIR')
    and l_commitdate < l_receiptdate
    and l_shipdate < l_commitdate
    and l_receiptdate >= date '1995-01-01'
    and l_receiptdate < date '1995-01-01' + interval '1' year
group by l_shipmode
order by l_shipmode;
select pg_enc_print_stats(true);
