cd benchmark 
psql -U postgres -d test  -f db_schemas/tpch-schema-encrypted.sql
psql -U postgres -d test  -f db_schemas/tpch-index.sql  
java -Dlog4j.configuration=log4j.properties -jar bin/tpch.jar -b tpch -o output -s 30 --config config/tpch_config.xml --load true --execute false
cd .. 