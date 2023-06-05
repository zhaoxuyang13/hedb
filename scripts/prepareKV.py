#!/bin/python

import psycopg2
import argparse

conn = psycopg2.connect(database="test", user="postgres", host="localhost", port="5432")
cur = conn.cursor()

def getTables():
    cur.execute("select table_name from information_schema.tables where table_schema='public' and table_type='BASE TABLE';")
    return cur.fetchall()

tables = [x[0] for x in getTables()]
print (tables)

# defined a class save enc_type column info
class enc_column:
    def __init__(self, table_name, col_name, enc_type):
        self.table_name = table_name
        self.col_name = col_name
        self.enc_type = enc_type

enc_cols = []
# read schema of tables
for table in tables:
    cur.execute("select column_name, data_type from information_schema.columns where table_name='%s';" % table)
    print (table)
    print (cur.fetchall())
