drop table test;
drop extension encdb cascade;
create extension encdb;
create table test(a float4, b VARCHAR(50), c VARCHAR(50));
insert into test(a,b,c)
            select random()*(10^5)::integer,substr(md5(random()::text), 0, 25)::enc_text,substr(md5(random()::text), 0, 25)::enc_text
            from generate_series(1,600000);
\timing
explain analyze
select sum(a) from test;
