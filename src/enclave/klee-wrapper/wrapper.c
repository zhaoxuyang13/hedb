#include <stdio.h>
#include <defs.h>
#include <klee/klee.h>

#include "prefix_udf.h"
#define CMD_PREFIX_OVERLAP 1001



int main(int argc, char *argv[]){

    if(argc <= 2){
        printf("Usage: %s [CMD_ID] [OP1] ...[OPN] \n \
      to generate input with the same control flow\n \
        ", argv[0]);
        exit(0);
    }
    int op = argv[0];
    
    switch (op)
    {
    case CMD_PREFIX_OVERLAP:{
        prefix_range a,b;
        klee_make_symbolic(&a,sizeof(a), 'a');
        klee_make_symbolic(&b,sizeof(b), 'b');
        bool is_overlap = pr_overlaps(a,b);
        printf("ouput: %d\n", is_overlap);
        break;
    }
    
    default:
        break;
    }
    
}