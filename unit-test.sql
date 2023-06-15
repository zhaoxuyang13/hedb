\unset ECHO

select plan(50);

select ok(pg_enc_int4_decrypt(pg_enc_int4_encrypt(1)) = 1::int4, 'enc_int4: encryption/decryption test');
select ok(pg_enc_float4_decrypt(pg_enc_float4_encrypt(1.1)) = 1.1::float4, 'enc_float4: encryption/decryption test');

select ok(1::enc_int4 = 1::enc_int4, 'enc_int4: inequality test, operator =');
select ok(0::enc_int4 != 1::enc_int4, 'enc_int4: inequality test, operator !=');
select ok(0::enc_int4 <> 1::enc_int4, 'enc_int4: inequality test, operator <>');
select ok(1::enc_int4 <= 2::enc_int4, 'enc_int4: inequality test, operator <=');
select ok(1::enc_int4 <= 1::enc_int4, 'enc_int4: inequality test, operator <=');
select ok(3::enc_int4 >= 2::enc_int4, 'enc_int4: inequality test, operator >=');
select ok(1::enc_int4 >= 1::enc_int4, 'enc_int4: inequality test, operator >=');
select ok(2::enc_int4 < 3::enc_int4, 'enc_int4: inequality test, operator <');
select ok(3::enc_int4 > 2::enc_int4, 'enc_int4: inequality test, operator >');
select ok(2::enc_int4 + 1::enc_int4 = 3::enc_int4, 'enc_int4: operator +');
select ok(2::enc_int4 - 1::enc_int4 = 1::enc_int4, 'enc_int4: operator -');
select ok(2::enc_int4 * 2::enc_int4 = 4::enc_int4, 'enc_int4: operator *');
select ok(6::enc_int4 / 2::enc_int4 = 3::enc_int4, 'enc_int4: operator /');
select ok(6::enc_int4 % 2::enc_int4 = 0::enc_int4, 'enc_int4: operator %');
select ok(2::enc_int4 ^ 2::enc_int4 = 4::enc_int4, 'enc_int4: operator ^');

select ok(1.1::enc_float4 = 1.1::enc_float4, 'enc_float4: inequality test, operator =');
select ok(0.2::enc_float4 != 1.1::enc_float4, 'enc_float4: inequality test, operator !=');
select ok(0.2::enc_float4 <> 1.1::enc_float4, 'enc_float4: inequality test, operator <>');
select ok(1.1::enc_float4 <= 2.3::enc_float4, 'enc_float4: inequality test, operator <=');
select ok(1.1::enc_float4 <= 1.1::enc_float4, 'enc_float4: inequality test, operator <=');
select ok(3.4::enc_float4 >= 2.3::enc_float4, 'enc_float4: inequality test, operator >=');
select ok(1.1::enc_float4 >= 1.1::enc_float4, 'enc_float4: inequality test, operator >=');
select ok(2.3::enc_float4 < 3.4::enc_float4, 'enc_float4: inequality test, operator <');
select ok(3.4::enc_float4 > 2.3::enc_float4, 'enc_float4: inequality test, operator >');
select ok(2.3::enc_float4 + 1.1::enc_float4 = 3.4::enc_float4, 'enc_float4: operator +');
select ok(9.9::enc_float4 / 3.3::enc_float4 = 3::enc_float4, 'enc_float4: operator /');
select ok(1.0::enc_float4 ^ 1.1::enc_float4 = 1.0::enc_float4, 'enc_float4: operator ^');

select ok('test1'::enc_text = 'test1'::enc_text, 'enc_text: inequality test, operator =');
select ok('test1'::enc_text != 'test2'::enc_text, 'enc_text: inequality test, operator !=');
select ok('test1'::enc_text <> 'test2'::enc_text, 'enc_text: inequality test, operator <>');
select ok('hello'::enc_text || 'world'::enc_text = 'helloworld'::enc_text, 'enc_text: operator ||');
select ok(substring('Thomas'::enc_text from 2::enc_int4 for 3::enc_int4) = 'hom'::enc_text, 'enc_text: operator substring');

select ok('23-11-22 00:00:01'::enc_timestamp = '23-11-22 00:00:01'::enc_timestamp, 'enc_timestamp: inequality test, operator =');
select ok('23-11-22 00:00:01'::enc_timestamp != '23-11-22 00:00:02'::enc_timestamp, 'enc_timestamp: inequality test, operator !=');
select ok('23-11-22 00:00:01'::enc_timestamp <> '23-11-22 00:00:02'::enc_timestamp, 'enc_timestamp: inequality test, operator <>');
select ok('23-11-22 00:00:01'::enc_timestamp <= '23-11-22 00:00:01'::enc_timestamp, 'enc_timestamp: inequality test, operator <=');
select ok('23-11-22 00:00:01'::enc_timestamp <= '23-11-22 00:00:03'::enc_timestamp, 'enc_timestamp: inequality test, operator <=');
select ok('23-11-22 00:00:01'::enc_timestamp >= '23-11-22 00:00:01'::enc_timestamp, 'enc_timestamp: inequality test, operator >=');
select ok('23-11-22 00:00:02'::enc_timestamp >= '23-11-22 00:00:01'::enc_timestamp, 'enc_timestamp: inequality test, operator >=');
select ok('23-11-22 00:00:01'::enc_timestamp < '23-11-22 00:00:03'::enc_timestamp, 'enc_timestamp: inequality test, operator <');
select ok('23-11-22 00:00:03'::enc_timestamp > '23-11-22 00:00:01'::enc_timestamp, 'enc_timestamp: inequality test, operator >');

DROP TABLE IF EXISTS test_table;
CREATE TABLE test_table (id int, num_i enc_int4, num_f enc_float4, str enc_text, time enc_timestamp);

INSERT INTO test_table VALUES (1, '1', '1.1', 'hello', '2020-01-01');
INSERT INTO test_table VALUES (2, '2', '2.1', 'world', '2019-01-01');
INSERT INTO test_table VALUES (3, '3', '3.1', 'from', '2018-01-01');
INSERT INTO test_table VALUES (3, '3', '3.1', 'hedb', '2017-01-01');

SELECT results_eq(
   'select SUM(num_i) from test_table',
    $$VALUES (9::enc_int4)$$,
    'enc_int4: SUM function '
);

SELECT results_eq(
   'select MIN(num_i) from test_table',
    $$VALUES (1::enc_int4)$$,
    'enc_int4: MIN function '
);

SELECT results_eq(
   'select MAX(num_i) from test_table',
    $$VALUES (3::enc_int4)$$,
    'enc_int4: MAX function '
);

SELECT results_eq(
   'select AVG(num_i) from test_table',
    $$VALUES (2::enc_int4)$$,
    'enc_int4: AVG function (with rounding)'
);

SELECT results_eq(
   'select SUM(num_f) from test_table',
    $$VALUES (9.4::enc_float4)$$,
    'enc_float4: SUM function'
);

SELECT results_eq(
   'select AVG(num_f) from test_table',
    $$VALUES (2.35::enc_float4)$$,
    'enc_float4: AVG function'
);

SELECT results_eq(
   'select str from test_table where str like ''%db%''',
    $$VALUES ('hedb'::enc_text)$$,
    'enc_text: LIKE function'
);

DROP TABLE IF EXISTS test_table;

select * from finish();

