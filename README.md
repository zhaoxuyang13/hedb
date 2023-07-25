# hedb-kv

use KV to optimize HEDB for analytic workloads.

# installation

```shell
make
make install
```

# run tpch benchmark

```shell
make prepare-tpch WAREHOUSE=0.1  # generates 0.1 warehouse data
psql -U postgres -d test # connect with psql and can execute queries.

````
