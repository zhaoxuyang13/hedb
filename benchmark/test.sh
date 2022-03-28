for k in 1 2 4 8 
do
	current_num=$(ls -1 results | grep "output.[0-9]*.csv" | awk '{split($0,a,"."); print a[2]}' | sort -nr | head -1)
	echo "k = ${k}, start from log number $((${current_num}+1))" >> number.log
	warehouse_setting="s#<scalefactor>1</scalefactor>#<scalefactor>${k}</scalefactor>#g"
	sed ${warehouse_setting} config/tpcc_config.xml > config/tpcc_config_s${k}.xml
	psql -U postgres -d test -f db_schemas/tpcc-schema_encrypted.sql
	java -Dlog4j.configuration=log4j.properties -jar bin/oltp.jar -b tpcc -o output -s 10 --config config/tpcc_config_s${k}.xml --load true --execute false >> number.log
	
	for j in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 18 20 24 32 48 64  
	do	
		terminal_setting="s#<terminals>1</terminals>#<terminals>${j}</terminals>#g"
		sed ${terminal_setting} config/tpcc_config_s${k}.xml > config/tpcc_config_s${k}_t${j}.xml
		for i in 1 2 3 4 5 do
			java -Dlog4j.configuration=log4j.properties -jar bin/oltp.jar -b tpcc -o output -s 20 --config config/tpcc_config_s${k}_t${j}.xml --load false --execute true
		done 
		rm config/tpcc_config_s${k}_t${j}.xml
	done
	rm config/tpcc_config_s${k}.xml
done 
