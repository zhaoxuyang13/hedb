select enable_debug_mode(1);
select enable_record_mode('Q1');
\timing
select
    l_returnflag,
    l_linestatus,
    sum(l_quantity) as sum_qty,
    sum(l_extendedprice) as sum_base_price,
    sum(l_extendedprice * ('1' - l_discount)) as sum_disc_price,
    sum(l_extendedprice * ('1' - l_discount) * ('1' + l_tax)) as sum_charge,
    avg(l_quantity) as avg_qty,
    avg(l_extendedprice) as avg_price,
    avg(l_discount) as avg_disc,
    count(*) as count_order
from
    lineitem
where
    l_shipdate <= pg_enc_timestamp_encrypt('1998-12-01')

group by
    l_returnflag,
    l_linestatus
order by
    l_returnflag,
    l_linestatus;
