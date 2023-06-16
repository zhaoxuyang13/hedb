wget https://github.com/theory/pgtap/releases/download/v1.2.0/pgTAP-1.2.0.zip
unzip pgTAP-1.2.0.zip

cd pgTAP-1.2.0
make && sudo make install
sudo apt install libtap-parser-sourcehandler-pgtap-perl

pg_prove -U postgres unit-test.sql
