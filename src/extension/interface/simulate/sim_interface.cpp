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
#include "ops_server.h"
#include <cassert>
#include <errno.h>
/* --------------------------------------------- */
/* this load barrier is only for arm */
#ifdef __aarch64__
	#define LOAD_BARRIER asm volatile("dsb ld" ::: "memory")
	#define STORE_BARRIER asm volatile("dsb st" ::: "memory")
#elif __x86_64
	#define LOAD_BARRIER ;
	#define STORE_BARRIER ;
#endif


void forkOpsProcess(){
    int keyid = getpid(); // keyid is parrent pid
    pid_t pid = fork();
    if(pid == 0){ // child
        char keyid_str[64];
        sprintf(keyid_str, "%d", keyid);
        
        // char ch[100];
        // sprintf(ch, "keyid_str: %s", keyid_str);
        // print_info(ch);
     
        char *argv[ ]={"sim_ops", keyid_str, NULL};   
        char *envp[ ]={"", NULL};
        execve("/usr/local/lib/hedb/sim_ops",argv, envp);
    }
}


static int posix_shmid;
static void *posix_shm_addr = NULL;
// static void posix_shm_exit_handler(){

// }

static void *get_shmem_posix(size_t size){
	// 1. create shared memory
	int key = 666;
    posix_shmid = shmget(key, size, 0666 | IPC_CREAT);
    if (posix_shmid == -1){
        char ch[100];
        sprintf(ch, "shmget failed %d errno\n", errno);
        print_info(ch);
        exit(EXIT_FAILURE);
    }
    // 2. attach shared memory
    posix_shm_addr = shmat(posix_shmid, NULL, 0);
    if (posix_shm_addr == (void *)-1) {
        perror("shmat failed\n");
        exit(EXIT_FAILURE);
    }
	// atexit(posix_shm_exit_handler);
	return posix_shm_addr;
}

#define SHM_SIZE (16*1024*1024)
#define META_REQ_SIZE 1024
#define REQ_REGION_SIZE 1024*1024
#define REGION_N_OFFSET(n) (META_REQ_SIZE + REQ_REGION_SIZE * n)

static void *shm_addr;
static int shm_id;
void *getSharedBuffer(size_t size)
{

    shm_addr = get_shmem_posix(SHM_SIZE);
    OpsServer *ops_server = (OpsServer *)shm_addr;
    print_info("get shmem\n");

    spin_lock(&ops_server->lock);
    assert(ops_server->status == SHM_NONE); 

    print_info("lock got\n");
    ops_server->status = SHM_GET;
    while(ops_server->status != SHM_DONE){
        YIELD_PROCESSOR;
    }
    LOAD_BARRIER;
    shm_id = ops_server->ret_id;
    void *buffer = shm_addr + REGION_N_OFFSET(shm_id);

    char ch[100];
    sprintf(ch, "server done %d. shm_addr %p, buffer %p\n", ops_server->ret_id, shm_addr, buffer);
    print_info(ch);

    STORE_BARRIER;
    ops_server->status = SHM_NONE;
    spin_unlock(&ops_server->lock);

    return buffer;
}
void freeBuffer(void *buffer)
{
    OpsServer *ops_server = (OpsServer *)shm_addr;
    spin_lock(&ops_server->lock);
    assert(ops_server->status == SHM_NONE);  

    ops_server->free_id = shm_id; // free shm_id buffer
    STORE_BARRIER;
    ops_server->status = SHM_FREE;
    while(ops_server->status != SHM_DONE){
        YIELD_PROCESSOR;
    }
    ops_server->status = SHM_NONE;
    spin_unlock(&ops_server->lock);
}