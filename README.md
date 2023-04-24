# HEDB

![Status](https://img.shields.io/badge/Version-Experimental-green.svg)
[![License: MIT](https://img.shields.io/badge/License-Mulan-brightgreenn.svg)](http://license.coscl.org.cn/MulanPubL-2.0)

HEDB's design removes the trade-off between security and maintenance. Its dual-mode design accomplishes two goals: 1) achieving interface security by preventig illegal invocations to UDFs in the execution mode, and 2) allowing DBA common maintenance tasks by replaying legal invocations in the maintenance mode.

Currently, HEDB supports PostgreSQL and TPC-H workloads.

- [HEDB](#hedb)
  - [Paper](#paper)
  - [Artifact summary](#artifact-summary)
  - [Artifact check-list](#artifact-check-list)
  - [Supported platform](#supported-platform)
  - [Repo structure](#repo-structure)
  - [Environment setup on host (Optional)](#environment-setup-on-host-optional)
    - [Prerequisites](#prerequisites)
    - [Create the extension](#create-the-extension)
    - [Build \& install the extension](#build--install-the-extension)
  - [Experiments](#experiments)
    - [Prepare](#prepare)
    - [Kick-off functional](#kick-off-functional)
      - [A kick-off functional experiment](#a-kick-off-functional-experiment)
    - [Claims](#claims)
    - [One push-button to run all experiments](#one-push-button-to-run-all-experiments)
    - [Experiment 1: End-to-end performance (1.5 hours)](#experiment-1-end-to-end-performance-15-hours)
    - [Experiment 2: Record overhead (40 mins)](#experiment-2-record-overhead-40-mins)
    - [Experiment 3: Replay overhead (50 mins)](#experiment-3-replay-overhead-50-mins)
    - [Experiment 4: Anonymized log generation time (XXX mins)](#experiment-4-anonymized-log-generation-time-xxx-mins)
  - [Limitations](#limitations)
  - [Contacts](#contacts)

## Paper

* [Encrypted Databases Made Secure Yet Maintainable](https://www.usenix.org/conference/osdi23/presentation/li)<br>
Mingyu Li, Xuyang Zhao, Le Chen, Cheng Tan, Huorong Li, Sheng Wang, Zeyu Mi, Yubin Xia, Feifei Li, Haibo Chen<br>
The 17th USENIX Symposium on Operating Systems Design and Implementation (OSDI ‘23)

## Artifact summary

This artifact contains the implementation of HEDB and scripts for reproducing the main results of this work.

## Artifact check-list

- Code link: <https://github.com/XXX>
- OS Version: Ubuntu 20.04
- Linux kernel version: >= 5.4
- Python version: >= 3.9
- Metrics: latency
- Expected runtime: see the documents or runtime logs for each experiment.

## Supported platform

HEDB requires an ARM server that supports S-EL2, a hardware virtualization technology in ARM TrustZone.

To reproduce HEDB functionality, you can run FVP on either an ARM or x86 platform.

To reproduce HEDB performance, we strongly recommend you to run HEDB using two VMs on the ARM server.

If you have trouble applying an cloudlab account, please contact us for assistance.

## Repo structure

```
Github Repo Root
├── Makefile
├── benchmark       # Data loader and data generator.
│   ├── README.md
│   ├── bin         # Executable jar files for loading data in database.
│   ├── config      # Configuration files for running the executable jar files.
│   ├── db_schemas  # Sql files for loading tpch and tpcc schema.
│   └── tools       # Data generator.
├── config.mk
├── klee_scripts # Scripts for running klee.
├── scripts      # Scripts for the experiments.
│   ├── config              # Json files for running different experiments.
│   ├── desenitize_test.sh  
│   ├── eval_AE_time.sh     # The script for running the experiments and getting the running time.
│   ├── experiment_steps.py 
│   ├── fig                 # The scripts for plotting the figure.
│   ├── figures             # Output figures of the experiments.
│   ├── run_experiment.py   # The script for selecting and run an experiment.
│   ├── run_test.sh         # The script for running a series of Sqls.
│   ├── sqls                # Sql files for different experiments.
│   ├── tmp                 # Immediate files of the experiments.
│   ├── type-ii-edb-breach.py
│   └── util_py3            # Utils used by the scripts.
└── src          # HEDB code base
```

## Environment setup on host (Optional)

All experiments can be performed by logging into our cluster, as showed in [Experiments/Prepare](#prepare). While you can also follow the following instructions to simulate them on the host.

### Prerequisites

Install Postgresql
```sh
sudo apt-get install postgresql postgresql-server-dev-all
```

### Create the extension
```sh
psql -U postgres -p 5432 -h localhost
```

```sql
CREATE USER test WITH PASSWORD 'password';
CREATE database test;
\c test;
CREATE extension encdb;
```

### Build & install the extension
```sh
make configure_sim
make
sudo make install
```

## Experiments

Unless otherwise specified, all HEDB's experiments run on top of a Kunpeng 96-core ARMv8.2 CPU machine with two VMs. Each experiment runs 3 times to warm up by default.

### Prepare

1. Please login to our cluster (ubuntu@192.168.122.158) using the key attached in HotCRP. We have
   set up all necessary environments to run all the experiments.

2. Each experiment will generate a figure in the `./figure` directory of this
   repository. You can fetch generated figures to your email. You may wish to
   use `scp` to send figures to your computer, but the gateway blocks this.

**Please be noted that different evaluators cannot run experiments at the same time. You may check whether other evaluators are running the experiments before you run the artifact.**

### Kick-off functional

#### A kick-off functional experiment

**Step 1:**
Login to the ARM Server.

**Step 2:**
In the terminal, run the following commands to setup environment for executing SQL query using HEDB.

```shell
python3 ./scripts/run_experiment.py --setup --config kickoff

```

**Output:**

```
(... a lot of information ...)
benchmark preparation finish
```

**Step 3:**
Execute a simple SQL query using HEDB,

```
psql -U postgres -d test -c '\timing on' -f scripts/sqls/timestamp-sqls/Q1.sql
```

**Output:**
```shell
(... Query Result, press 'q' to quit ...)
Timing is on.
Time: 0.748 ms
Timing is off.
```

### Claims
- **(C1)** Figure 4: Type-II EBD's runtime overhead varies amongst TPC-H 22 queries. HEDB's optimizations speeds Type-II EDB.
- **(C2)** Figure 5 (a): HEBD's record overhead is low and acceptable.
- **(C3)** Figure 5 (b): HEBD's replay overhead is much faster than UDF-based replay.
- **(C4)** Figure 5 (c): HEDB's optimizations boost the anonymized log generation time.

### One push-button to run all experiments

To further ease the evaluation, you can run the following script which automatically runs all experiments.

**Step 1: Run all experiments (XXX hours)**

```shell
cd ~/osdi23-artifact/HEDB/script/
bash ./oneforall.sh
```

**Step 2: Email figures to your computer**

```shell
cd ~/osdi23-artifact/HEDB/script/
bash ./fetch.sh xxx@gmail.com
```

After fetching all these figures, you can verify the abovementioned**Major
Claims** by referring to the **Expected results** and **Important notes** below.

Note that, you can also run each experiment by yourself with the steps described below.

### Experiment 1: End-to-end performance (1.5 hours)

This experiment runs HEDB and three optimization strategies (i.e., O1-Parallel Decryption, O2-
Order-revealing Encryption, and O3-Expression Evaluation) TPC-H. The output reports each
system's latency normalized to the SOTA---ARM-version StealthDB as the baseline. 

**Command to run:**

```shell
cd ~/osdi23-artifact/HEDB/script/
bash ./fig4.sh
```

**Output:**

- `fig4.pdf`

**Expected results:**

- O1 can improve all queries and reduce around 15% end-to-end query execution time on average (matching **C1**).
- O2 makes the Q1’s overhead decreased by around 50% (matching **C1**).
- O3 optimizes Q1’s to decrease its overhead by around 10% (matching **C1**).

**Important notes of Experiment 1:**

- The reproduced results may not exactly match the results presented in the paper due to the noise nature of system such as scheduling, disk I/O, etc.

### Experiment 2: Record overhead (40 mins)

This experiment runs HEDB with log recording enabled, for replaying and debugging later on.

**Command to run:**

```shell
cd ~/osdi23-artifact/HEDB/script/
bash ./fig5a.sh
```

**Output:**

- `figure5a.pdf`.

**Expected results:**

- HEDB's record incurs overhead no more than 10% (matching **C3**).

**Important notes of Experiment 2:**

- Owing to the randomness, the results may not exactly match the origial.

### Experiment 3: Replay overhead (50 mins)

This experiment runs HEDB and re-executes TPC-H queries by calling UDF and replaying logs. The less time cost the better, which saves the DBA’s time and effort.

**Command to run:**

```shell
cd ~/osdi23-artifact/HEDB/script/
bash ./fig5b.sh
```

**Output:**
- `figure5b.pdf`.

**Expected results:**

- HEDB’s log-based replay is faster than UDF-based replay (by honestly calling UDF), effectively saving the DBA’s time and effort (matching **C3**).
- HEDB’s replay still incurs 5x slowdown compared with the vanilla without encryption (matching **C3**).

### Experiment 4: Anonymized log generation time (XXX mins)

This experiment translates the log recorded in Experiment 2 into an anonymized form. It firsr uses KLEE to collect the path constraint, and then uses masking rule constraint to generate new inputs.

**Command to run:**

```shell
cd ~/osdi23-artifact/HEDB/script/
bash ./fig5c.sh
```

**Output:**

- `figure5c.pdf`.

**Expected results:**

- Baseline of KLEE/Z3 estimation is very long (matching **C4**).
- HEDB's optimization achieves 10x to 200x (matching **C9**). 

**Important notes of Experiment 4:**

- We do not run KLEE/Z3 using the log, as it takes too long. Instead, we measure the cost for one invocation and time it with the invocation frequency. We measure the optimized KLEE/Z3 using real machine.
- KLEE does not support floating-point numbers, hence Q18 is not supported.
- Owing to the randomness, the results may not exactly match the origial.

## Limitations

This repo is a research prototype rather than a production-ready system. Its current implementation has two main limitations.

1. HEDB relies on determistic record-replay to reproduce the DBMS bugs, hence falling short in providing read-write workloads such as TPC-C.
2. HEDB depends on KLEE to reproduce the UDF bugs. The official version of KLEE cannot support floating-point numbers. HEDB inherits this limitation.

## Contacts

- Mingyu Li: maxulle [at] sjtu [dot] edu [dot] cn.
- Xuyang Zhao: brokenbones [at] sjtu [doc] edu [doc] cn
- Le Chen: cen-le [at] sjtu [dot] edu [dot] cn