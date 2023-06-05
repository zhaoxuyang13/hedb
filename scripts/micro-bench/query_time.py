#!/bin/python


import argparse



def main():
   
   
    # 1. parse arguments 
    parser = argparse.ArgumentParser(description='Run experiment.')
        # run experiment to generate figure x
    parser.add_argument('-s', '--scale', help="scale factor")
        # run single steps of experiment by hand.
    parser.add_argument('-s', '--setup', action='store_true',
                        help='setup instances (default: false)')
  

    
    if not args.cleanup and not args.figure and not (args.config and args.setup):
        parser.print_help()
        parser.exit()


if __name__ == '__main__':
    main()
