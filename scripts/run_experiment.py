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

from tqdm import tqdm, trange
from pathlib import Path, PurePath

from experiment_steps import * 

START_VM_CONFIG="scripts/config/vms.json"
DEFAULT_TPCH_CONFIG="scripts/config/baseTPCH.json"
RECORD_TPCH_CONFIG="scripts/config/recordTPCH.json"
REPLAY_TPCH_CONFIG="scripts/config/replayTPCH.json"

def avg(lst):
    return sum( [float(i) for i in lst]  ) / len(lst)

# return data is [ {"query" : queryId1, "times": [t0,t1,t2 ...]}, 
#                  {"query" : queryId2, "times": [t0,t1,t2 ...]}, 
#                  {"query" : queryId3, "times": [t0,t1,t2 ...]}, 
#                ... ]
def transformToList(data):
    return [ avg(data[i]['times'][1:]) for i in range(1, 23)] # skip first test result as warming up
    

def run_record_steps():
    startVMs()
    
    prepBenchmark(RECORD_TPCH_CONFIG)
    record_data = runBenchmark(RECORD_TPCH_CONFIG)
    
    prepBenchmark(DEFAULT_TPCH_CONFIG)
    baseline_data = runBenchmark(DEFAULT_TPCH_CONFIG)
    
    data = {
        'Query' : list( range(1, 23) ),
        'ARM-version StealthDB': transformToList(baseline_data),
        'w/ Record': transformToList(record_data), 
    }
    writer = pd.ExcelWriter('scripts/tmp/record.xlsx', engine='openpyxl')
    
    df = pd.DataFrame(data)
    df.to_excel(writer, sheet_name='s=1_vm')
    
    graphData(RECORD_TPCH_CONFIG)
    
    
def run_replay_steps():
    pass

def run_default_steps():
    pass

def run_figure_steps(figure):
    if figure == 'fig2' or figure == 'record':
        run_record_steps()
    elif figure == 'fig3' or figure == ' replay':
        run_replay_steps()
    else :
        run_default_steps()

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
