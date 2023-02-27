#!/usr/bin/env python
# -*- coding: utf-8 -*-
import argparse
import io
import os
import os.path
import sys
import datetime
import time
import math
import re
from util_py3.ssh_util import *
from util_py3.prop_util import *
from util_py3.graph_util import *
from util_py3.data_util import *

from tqdm import tqdm, trange
from pathlib import Path, PurePath

START_VM_CONFIG="scripts/config/vms.json"
DEFAULT_TPCH_CONFIG="scripts/config/baseTPCH.json"

def figure_to_config_file(figure):
    if figure == "basic":
        return DEFAULT_TPCH_CONFIG
    else:
        return DEFAULT_TPCH_CONFIG
def noPrint(args):
    pass
def startVMs(propFile = START_VM_CONFIG):
    properties = loadPropertyFile(propFile)
    
    vmScriptPath = properties['vm_scripts_path']
    # logPath = properties['logPath']
    DBVMScriptName = properties['dbms_script']
    OpsVMScriptName = properties['ops_script']
    pgPort = properties['pg_port']
    pgIp = properties['pg_ip']
    
    executeCommand("mkdir -p scripts/tmp")
    
    executeNonBlockingCommandNoOutput("cd %s && bash %s" %(vmScriptPath,OpsVMScriptName))
    executeNonBlockingCommandNoOutput("cd %s && bash %s" %(vmScriptPath,DBVMScriptName))

    
    time.sleep(10)
    
    while True:
        output = executeCommandWithOutputReturn("pg_isready  -h %s -p %s" % (pgIp, pgPort))
        # print(output)
        if b'accepting connections' in output:
            break
        time.sleep(1)
    # time.sleep(5)
    print("VM started and postgres-server is ready for connection")
 
# build the HEDB project, load corresponding schema and data.    
def prepBenchmark(propFile = DEFAULT_TPCH_CONFIG):
    properties = loadPropertyFile(propFile)
    
    homePath = properties['home_path']
    pgPort = properties['pg_port']
    pgIp = properties['pg_ip']
    experimentName = properties['experiment_name']
    executeCommand("make clean && make configure_sim && make")
    
    # start vms here, ops vm should be run after make is complete.
    startVMs()
    
    dataSize = properties['data_size']
    executeCommand("cd benchmark/tools && rm -rf *.tbl && ./dbgen -s %s" % dataSize)
    
    # edit benchmark file 
    executeCommand("cd benchmark/config && cp tpch_config.xml tmp.xml && \
            sed -i 's#<DBUrl>.*</DBUrl>#<DBUrl>jdbc:postgresql://%s:%s/test</DBUrl>#' tmp.xml" % (pgIp, pgPort))
    
    schemaName = properties['schema_name']
    indexName = properties['index_name']

    # load schema
    executeCommand("cd benchmark && psql -h %s -p %s -U postgres -d test -f db_schemas/%s" % (pgIp, pgPort, schemaName))
    executeCommand("cd benchmark && psql -h %s -p %s -U postgres -d test -f db_schemas/%s" % (pgIp, pgPort, indexName))

    # load tpch data
    executeCommand("cd benchmark && java -Dlog4j.configuration=log4j.properties -jar bin/tpch.jar -b tpch -o output -s 100 --config config/tpch_config.xml --load true --execute false")
    
    # vaccum 
    executeCommand("psql -h %s -p %s -U postgres -d test -f scripts/sqls/util_sqls/vacuum.sql" %(pgIp, pgPort))
    
    executeCommand("reset && clear")
    
    print("benchmark preparation finish")
    
def runBenchmark(propFile = DEFAULT_TPCH_CONFIG):
        
    properties = loadPropertyFile(propFile)
    # Username for ssh-ing.
    # username = properties['username']
    # Name of the experiment that will be run
    experimentName = properties['experiment_name']
    
    pgPort = properties['pg_port']
    pgIp = properties['pg_ip']
    
    # mkdir tmp for the experiment.
    tmpResultPath =  Path("scripts/tmp/") / experimentName
    executeCommand("mkdir -p %s" % tmpResultPath)
     
    #
    sqlsPath = properties['sqls_path']
    results = []
    for i in range(1,23):
        queryName = "Q%d.sql" % i
        queryTimes = []
        for j in range(4):
            output = executeCommandWithOutputReturn("psql -h %s -p %s -U postgres -d test -f %s/%s" %(pgIp, pgPort, sqlsPath, queryName))

            # print(queryNazme)
            # print(output)
            time_match = re.match(r'.*Time: ([0-9\.]*) ms.*', str(output))
            assert(time_match is not None)
            queryTime = time_match.group(1)
            queryTimes.append(queryTime)
            time.sleep(5)
        
        # print(queryTime)
        results.append({
            "queryName": queryName, 
            "queryTime": queryTimes[1] ## need to compute average
        })
    print(results)
    dataFile = tmpResultPath
    saveData(results, "scripts/tmp", experimentName)

def graphData(propFile = DEFAULT_TPCH_CONFIG):
   
    properties = loadPropertyFile(propFile)
    figure = properties["figure"]
    homePath = Path(properties["home_path"])
    
    scriptPath = homePath / 'scripts'
    paperFigDir = scriptPath / 'figures' / 'paper'
    artifactFigDir = scriptPath / 'figures' / 'artifact'
    cmpFigDir = scriptPath / 'figures' 
    outFile = scriptPath / 'tmp' 
    # add elif statement here add new pictures. 
    if figure == "basic":
        figName = 'basic.pdf'
        title = "Figure basic"
        script = Path("fig/basic_graph.py")
        paperDataFile =  paperFigDir / 'basic.csv'
    else:
        print("Unsupported figure:", figure)
        return    
        
    
    
    # graph in paper
    cmd = f'python3 {script} -l -t "Paper {title}" {paperDataFile} {paperFigDir / figName}'
    executeCommand(cmd)
    # graph in AE
    cmd = f'python3 {script} -l -t "Artifact Evaluation" {outFile} {artifactFigDir / figName}'
    executeCommand(cmd)
    # pdfjam 
    cmd = f"pdfjam --landscape --nup 2x1 {artifactFigDir / figName} {paperFigDir / figName} --outfile {cmpFigDir / figName}"
    executeCommand(cmd)

def cleanupExperiment():
    cmd = r"ps aux | grep ./qemu-system-aarch64 | awk '{print $2}' | head -n -1 | xargs kill -9"
    executeCommand(cmd)
    print("qemu vm shut down")
    
# def runLogAnonymize(propFile):
    
#     return


def main():
    
    parser = argparse.ArgumentParser(description='Run experiment.')
    parser.add_argument('-s', '--setup', action='store_true',
                        help='setup instances (default: false)')
    parser.add_argument('-r', '--run', action='store_true',
                        help='run experiment (default: false)')
    # parser.add_argument('--summarize', action='store_true',
    #                     help='summarize experiment results (default: false)')
    parser.add_argument('-g', '--graph', action='store_true',
                        help='graph experiment results (default: false)')
    parser.add_argument('-c', '--cleanup', action='store_true',
                        help='cleanup instances(default: false)')
    parser.add_argument('-f', '--figure', help='figure to reproduce')
    parser.add_argument('--config', help='config file for experiment')
    args = parser.parse_args()
    # prepBenchmark()
    # exit()
    
    if not args.cleanup and not args.figure and not args.config:
        parser.print_help()
        parser.exit()

    if  not args.setup and not args.run and not args.graph and not args.cleanup:
        parser.print_help()
        parser.exit()


    if args.figure:
        config_file = figure_to_config_file(args.figure)
    else:
        config_file = args.config
    if args.setup:
        print("Setting up... (starting VMs)")
        prepBenchmark()
    if args.run:
        print("Running experiment...")
        runBenchmark()
        graphData(config_file)
    # if args.summarize:
    #     summarizeData(config_file)
    if args.graph:
        graphData(config_file)
    if args.cleanup:
        print("Cleaning up... shutting down vms, clean up temporary dirs")
        cleanupExperiment()


if __name__ == '__main__':
    main()
