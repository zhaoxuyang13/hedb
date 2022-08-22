// #include <thread>
#include <extension_helper.hpp>
#include <unistd.h>
#include <defs.h>
#include <sys/shm.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <sim_interface.hpp>
#include <sys/types.h>
#include "request_types.h"
/* --------------------------------------------- */

void forkOpsProcess(){
    int keyid = getpid(); // keyid is parrent pid
    pid_t pid = fork();
    if(pid == 0){ // child
        char keyid_str[64];
        sprintf(keyid_str, "%d", keyid);
        
        char ch[100];
        sprintf(ch, "keyid_str: %s", keyid_str);
        print_info(ch);
     
        char *argv[ ]={"sim_ops", keyid_str, NULL};   
        char *envp[ ]={"", NULL};
        execve("/usr/local/lib/hedb/sim_ops",argv, envp);
    }
}

void *getSharedBuffer(size_t size)
{
    void *shm_buffer = NULL;
    int shmid;
    key_t key = (key_t) getpid();
    char ch[100];
    sprintf(ch, "shm keyid is server pid %d", key);
    print_info(ch);
    forkOpsProcess();
    // 1. create shared memory
    shmid = shmget(key, size, 0644 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed\n");
        exit(EXIT_FAILURE);
    }
    // 2. attach shared memory
    shm_buffer = shmat(shmid, NULL, 0);
    if (shm_buffer == (void *)-1) {
        perror("shmat failed\n");
        exit(EXIT_FAILURE);
    }
    return shm_buffer;
}
void freeBuffer(void *buffer)
{
    BaseRequest *req = (BaseRequest *)buffer;
    req->status = EXIT;
    // 4. detach shared memory
    if (shmdt(buffer) == -1) {
        perror("shmdt failed\n");
        exit(EXIT_FAILURE);
    }
}