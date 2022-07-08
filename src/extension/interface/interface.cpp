#include <interface.hpp>
#include <tee_interface.hpp>
#include <sync.h>
#include <extension_helper.hpp>
#include <timer.hpp>
#include <stdlib.h> // at exit
#include <stdio.h>
#define MAX_LOG_SIZE 50000

TEEInvoker *TEEInvoker::invoker = NULL;
uint8_t log_buffer[MAX_LOG_SIZE] ;
uint64_t current_log_size = 0;
FILE *write_ptr = 0;

void flush_to_log_file(uint8_t *buffer, uint64_t size){
    if (write_ptr == 0)
    {
        write_ptr = fopen("test.binlog","wb"); 
    }
    fwrite(buffer,size,1,write_ptr);
}



TEEInvoker::~TEEInvoker() {
    freeBuffer(req_buffer);
}

int TEEInvoker::sendRequest(Request *req) {
    int resp;
    
    req->serializeTo(req_buffer);
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);
    /* TODO write barrier */
    // print_info("REQUIEST sent");

    // static myTime time;
	// static double timer = 0,duration = 0;
	// time.tic();

    req_control->status = SENT;

    /* wait for status */
    while (req_control->status != DONE)
        YIELD_PROCESSOR;
    /* TODO read barrier */

    req->copyResultFrom(req_buffer);
    resp = req_control->resp;

    /* log */
    // if(req_control->reqType != CMD_INT_DEC 
    // || req_control->reqType != CMD_INT_ENC
    // || req_control->reqType != CMD_FLOAT_ENC
    // || req_control->reqType != CMD_FLOAT_DEC
    // || req_control->reqType != CMD_STRING_ENC
    // || req_control->reqType != CMD_STRING_DEC
    // || req_control->reqType != CMD_TIMESTAMP_ENC
    // || req_control->reqType != CMD_TIMESTAMP_DEC
    // ){
    //     uint64_t req_size = req->size();
    //     flush_to_log_file((uint8_t *)req_buffer, req_size);

        // memcpy(log_buffer + current_log_size, req_buffer, req_size);
        // current_log_size += req_size;
        // if(current_log_size > MAX_LOG_SIZE - 5000){
    // }
            // current_log_size = 0;
        // }


    /* read-write barrier, no read move after this barrier, no write move before this barrier */
    req_control->status = NONE;
    
    // time.toc();
    // duration = time.getDuration() / 1000000;
    // timer += duration;
    // if(req_control->reqType == CMD_FLOAT_DEC || req_control->reqType == CMD_INT_DEC){
    //     print_info("shit");
    //     char ch[1000];	
    //     sprintf(ch, "total duration in ms: sum %f ", timer );
    //     print_info(ch);
    // }

    return resp;
}

void exit_handler(){
    print_info("EXIT handler called\n");
    TEEInvoker *invoker = TEEInvoker::getInstance();
    delete invoker;
    if(write_ptr != 0)
        fclose(write_ptr);
}


TEEInvoker::TEEInvoker() {
    // print_info("get shared buffer");
    req_buffer = getSharedBuffer(sizeof (EncIntBulkRequestData));
    // print_info("buffer got");
    atexit(exit_handler);
}
