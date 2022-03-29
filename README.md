### Usage 
1. 安装 Postgresql 

```shell
sudo apt-get install postgresql postgresql-server-dev-all

or 

build from source. https://www.postgresql.org/docs/current/install-short.html
```

1. 安装 SGX driver, sdk, 
2. 编译安装arm edb

```shell
make configure_sgx
make 
sudo make install 
```

4. 安装encdb 插件 

```bash
psql -U postgres
```

```psql
CREATE USER test WITH PASSWORD 'password';
CREATE database test;
\c test
CREATE extension encdb;
SELECT pg_enc_int4_encrypt(1) + pg_enc_int4_encrypt(2);
SELECT pg_enc_int4_decrypt(pg_enc_int4_encrypt(1) + pg_enc_int4_encrypt(2));

```

5. TPCC benchmark （事务型）

修改配置文件 benchmark/config/tpcc_config.xml中的DBUrl, username, password 为当前本机postgres的配置

定义warehouse（数据量）和terminal（并发量）

```bash
make load-tpcc
cd benchmark 
java -Dlog4j.configuration=log4j.properties -jar bin/oltp.jar -b tpcc -o output -s 100 --config config/tpcc_config.xml --load false --execute true
# 注意这里 -s 后面的数字和config.xml里面定义的time一样。
# 输出结果到benchmark/results目录，
```

6. TPCH benchmark （分析型）

```bash
benchmark/tool/dbgen -s 2 # 指定warehouse大小
make load-tpch
cd benchmark 
java -Dlog4j.configuration=log4j.properties -jar bin/tpch.jar -b tpch -o output -s 10 --config config/tpch_config.xml --load false --execute true
# 和上面运行方式差不多
```

整体是参考stealthdb写的，可以参考
https://github.com/cryptograph/stealthdb/tree/master/
https://github.com/cryptograph/stealthdb/tree/master/benchmark

### 代码结构
- Enclave : TEE中的代码
  - 子目录SGX, TZ分别是对应不同平台的代码
  - 其余是通用代码逻辑
- extension：PG插件代码
  - xxx.sql 声明四个加密类型和对应类型的各种函数
  - enc_xxx.cpp 定义了每个函数（按照postgres的规范写的）
  - Interface目录：调用TEE的一层接口
    - 子目录SGX，TZ对应了调用不同平台的代码
  - include 头文件
    - enc_type和request_type定义了加密类型的数据结构，和传输的请求的数据结构。
    - 使用C是因为trustzone的ta只能使用C编写，如果使用class的话没法兼容。（其实这一块实现的不好，为了贪图c++的方便，除了trustzone都想用c++，结果代码里c和c++混合在一起，代码不太优雅，但是能用就行）

项目使用cmake + Makefile来编译（更方便的管理各种依赖）


### Notes

- SGX enclave sign key should be provided by user in production.