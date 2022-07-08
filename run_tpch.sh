
RUN_TIME=60
cd benchmark 
time_setting="s#<time>30</time>#<time>${RUN_TIME}</time>#g"
# echo "${time_setting}"
sed "${time_setting}" config/tpch_config.xml > config/tpch_config_tmp.xml

echo "java -Dlog4j.configuration=log4j.properties -jar bin/tpch.jar -b tpch -o output -s $RUN_TIME --config config/tpch_config_tmp.xml --load false --execute true" 
java -Dlog4j.configuration=log4j.properties -jar bin/tpch.jar -b tpch -o output -s $RUN_TIME --config config/tpch_config_tmp.xml --load false --execute true

rm config/tpch_config_tmp.xml
cd .. 