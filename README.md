### Usage 

1. 安装 Postgresql 

```shell
sudo apt-get install postgresql postgresql-server-dev-all

or 

build from source. https://www.postgresql.org/docs/current/install-short.html
```

2. 安装OP-TEE  [ref]( https://optee.readthedocs.io/en/latest/building/gits/build.html)

   1. Install prerequisite

      ```bash
      sudo apt-get install android-tools-adb android-tools-fastboot autoconf \
              automake bc bison build-essential ccache codespell \
              cscope curl device-tree-compiler expect flex ftp-upload gdisk iasl \
              libattr1-dev libcap-dev libcap-ng-dev \
              libfdt-dev libftdi-dev libglib2.0-dev libgmp-dev libhidapi-dev \
              libmpc-dev libncurses5-dev libpixman-1-dev libssl-dev libtool make \
              mtools netcat ninja-build python3-crypto \
              python3-pycryptodome python3-pyelftools python3-serial \
              rsync unzip uuid-dev xdg-utils xterm xz-utils zlib1g-dev
      ```

   2. Install REPO

      ```bash
      mkdir ~/bin
      PATH=~/bin:$PATH
      curl https://storage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
      chmod a+x ~/bin/repo
      git config --global user.name "Your Name" #repo need git name/email config.
      git config --global user.email "you@example.com" 
      ```

   3. Get source code 

      ``` bash
      mkdir <work-dir>
      cd <work-dir>
      repo init -u https://github.com/OP-TEE/manifest.git -m qemu_v8.xml
      repo sync
      # apply our own code.
      mkdir -p share/edb
      tar -xzf edb.tar.gz -C share/edb
      cp share/edb/patch/qemu_v8.mk build/qemu_v8.mk
      mkdir -p img
      mv ramdisk img/ramdisk
      ```

   4. build and run OP-TEE

      ```bash
      cd build
      make toolchains
      make \
      	QEMU_VIRTFS_ENABLE=y \
        QEMU_USERNET_ENABLE=y \  # 用于共享文件夹
        CFG_WITH_PAGER=y \  
      	run
      ```

3. 编译安装EDB扩展（In Qemu ）

   会弹出两个窗口，一个是secure ，一个是normal，在qemu侧输入c开始执行。

   在normal侧输入root登陆

   ```bash
   mkdir mnt
   mount /dev/vda mnt # 挂载准备的包含postgres的img
   ./mnt/mnt.sh # chroot
   ./init.sh 
   cd edb
   make configure_tz #选择trustzone作为TEE
   make build 
   sudo make install
   ```

4. 安装encdb 插件  (在Host机器上)

```bash
# qemu里，已经通过ssh将pg端口转发到host上54322端口
psql -U postgres -p 54322 -h localhost
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

```bash
cd benchmark 
java -Dlog4j.configuration=log4j.properties -jar bin/oltp.jar -b tpcc -o output -s 100 --config config/tpcc_config.xml --load true --execute false
java -Dlog4j.configuration=log4j.properties -jar bin/oltp.jar -b tpcc -o output -s 100 --config config/tpcc_config.xml --load false --execute true
```

6. TPCH benchmark （分析型）

```bash
cd benchmark 
./tool/dbgen -s 2 # 指定warehouse大小
java -Dlog4j.configuration=log4j.properties -jar bin/tpch.jar -b tpch -o output -s 10 --config config/tpch_config.xml --load true --execute false
java -Dlog4j.configuration=log4j.properties -jar bin/tpch.jar -b tpch -o output -s 10 --config config/tpch_config.xml --load false --execute true
```

