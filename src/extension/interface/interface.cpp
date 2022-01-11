#include <interface.hpp>
#include <tee_interface.hpp>
#include <sync.h>
#include <extension_helper.hpp>
#include <timer.hpp>
TEEInvoker *TEEInvoker::invoker = NULL;
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

TEEInvoker::TEEInvoker() {
    req_buffer = getSharedBuffer(sizeof (EncIntBulkRequestData));
}
