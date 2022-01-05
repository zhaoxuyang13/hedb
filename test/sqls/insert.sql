drop table test;
drop extension encdb cascade;
create extension encdb;
create table test(a enc_float4, b enc_text, c enc_text);
insert into test(a,b,c)
            select random()*(10^5)::integer,substr(md5(random()::text), 0, 25)::varchar,substr(md5(random()::text), 0, 25)::varchar
            from generate_series(1,600000);
\timing
explain analyze
select sum(a) from test;
