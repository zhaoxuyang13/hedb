#!/usr/bin/env python
# -*- coding: utf-8 -*-
import argparse
import io
import os
import os.path
from util_py3.ssh_util import *
from util_py3.prop_util import *
from util_py3.graph_util import *
from util_py3.data_util import *
import pandas as pd
import pickle
from tqdm import tqdm, trange
from pathlib import Path, PurePath

from experiment_steps import * 

START_VM_CONFIG="scripts/config/vms.json"
DEFAULT_TPCH_CONFIG="scripts/config/baseTPCH.json"
RECORD_TPCH_CONFIG="scripts/config/recordTPCH.json"
REPLAY_TPCH_CONFIG="scripts/config/replayTPCH.json"
NATIVE_TPCH_CONFIG="scripts/config/nativeTPCH.json"
EXP_TPCH_CONFIG="scripts/config/expTPCH.json"
OPT_FIG_CONFIG="scripts/config/optimizedFigure.json"

def avg(lst):
    return sum( [float(i) for i in lst]  ) / len(lst)

# return data is [ {"query" : queryId1, "times": [t0,t1,t2 ...]}, 
#                  {"query" : queryId2, "times": [t0,t1,t2 ...]}, 
#                  {"query" : queryId3, "times": [t0,t1,t2 ...]}, 
#                ... ]
def transformToList(data):
    return [ avg(data[i]['times'][1:]) for i in range(0, 22)] # skip first test result as warming up
    

def run_record_steps():
    compileCodes()
    startVMs()
    
    prepBenchmark(DEFAULT_TPCH_CONFIG)
    
    record_data = runBenchmark(RECORD_TPCH_CONFIG)
    
    baseline_data = runBenchmark(DEFAULT_TPCH_CONFIG)
    
    data = {
        'Query' : list( range(1, 23) ),
        'ARM-version StealthDB': transformToList(baseline_data),
        'w/ Record': transformToList(record_data), 
    }
    df = pd.DataFrame(data)
    with pd.ExcelWriter('scripts/tmp/record.xlsx', engine='openpyxl') as writer:
        df.to_excel(writer, sheet_name='s=1_vm')
    
    # writer = pd.ExcelWriter('scripts/tmp/record.xlsx', engine='openpyxl')
    
    # df = pd.DataFrame(data)
    # df.to_excel(writer, sheet_name='s=1_vm')
    # writer.close()
    
    graphData(RECORD_TPCH_CONFIG)
    
    
def run_replay_steps():
    compileCodes()
    startVMs()
    
    prepBenchmark(RECORD_TPCH_CONFIG)
    
    # replay
    print("generate replay data")
    runBenchmark(RECORD_TPCH_CONFIG)    # generate the record files
    replay_data = runBenchmark(REPLAY_TPCH_CONFIG)

    # UDF-based
    print("generate UDF-based data")
    udfbase_data = runBenchmark(DEFAULT_TPCH_CONFIG)
    
    # vanilla
    print("generate vanilla data\n")
    prepBenchmark(NATIVE_TPCH_CONFIG)
    vanilla_data = runBenchmark(NATIVE_TPCH_CONFIG)
    
    data = {
        'Query' : list( range(1, 23) ),
        'Vanilla (w/o encryption)': transformToList(vanilla_data),
        'Log-based replay': transformToList(replay_data), 
        'UDF-based replay': transformToList(udfbase_data),
    }
    df = pd.DataFrame(data)
    with pd.ExcelWriter('scripts/tmp/replay.xlsx', engine='openpyxl') as writer:
        df.to_excel(writer, sheet_name='s=1_vm_replay')
    
    graphData(REPLAY_TPCH_CONFIG)


def run_default_steps():
    compileCodes()
    startVMs()
    
    prepBenchmark(NATIVE_TPCH_CONFIG)
    
    # vanilla
    print("generate vanilla data")
    vanilla_data = runBenchmark(NATIVE_TPCH_CONFIG)
    
    prepBenchmark(DEFAULT_TPCH_CONFIG)
    
    # simply encrypted
    print("generate ARM-version StealthDB data")
    udfbase_data = runBenchmark(DEFAULT_TPCH_CONFIG)

    # exp
    print("generate exp optimization data")
    exp_data = runBenchmark(EXP_TPCH_CONFIG)
    
    # order-revealing
    print("generate order optimization data")
    generateOrder()
    order_data = runBenchmark(DEFAULT_TPCH_CONFIG)
    
    # recompile the code to enable parallel enc/dec
    cleanupExperiment()
    
    compileCodes(parallel=True)
    startVMs()
    
    prepBenchmark(DEFAULT_TPCH_CONFIG)
    
    # parallel
    print("parallel enabled")
    parallel_data = runBenchmark(DEFAULT_TPCH_CONFIG)
    
    # all opt
    print("generate all optimization data")
    generateOrder()
    opt_data = runBenchmark(EXP_TPCH_CONFIG)
    
    data = {
        'Query' : list( range(1, 23) ),
        'Native': transformToList(vanilla_data),
        'ARM-version StealthDB': transformToList(udfbase_data),
        'w/ O1(parallel)': transformToList(parallel_data),
        'w/ O2(order)': transformToList(order_data),
        'w/ O3(expression)': transformToList(exp_data),
        'w/ HEDB\'s optimization': transformToList(opt_data)
        
    }
    df = pd.DataFrame(data)
    with pd.ExcelWriter('scripts/tmp/optimization.xlsx', engine='openpyxl') as writer:
        df.to_excel(writer, sheet_name='s=1_vm')
    
    graphData(OPT_FIG_CONFIG)
    

def run_figure_steps(figure):
    if figure == 'fig2' or figure == 'record': 
        run_record_steps()
    elif figure == 'fig3' or figure == 'replay':
        run_replay_steps()
    elif figure == 'base':
        run_default_steps()
    else :
        print("enter a fig name")
        pass

def main():
   
   
    # 1. parse arguments 
    parser = argparse.ArgumentParser(description='Run experiment.')
        # run experiment to generate figure x
    parser.add_argument('-f', '--figure', help='figure to reproduce')

        # run single steps of experiment by hand.
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
    parser.add_argument('--config', help='config file for experiment')
    args = parser.parse_args()

    
    # if not args.cleanup and not args.figure and not args.config:
    #     parser.print_help()
    #     parser.exit()



    if args.figure:
        run_figure_steps(args.figure)
        return
    if args.config:
        config_file = args.config
  
    if args.setup:
        print("Setting up... (starting VMs)")
        prepBenchmark(config_file)
    if args.run:
        print("Running experiment...")
        runBenchmark(config_file)
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
