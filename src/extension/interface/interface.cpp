#include <interface.hpp>
#include <sync.h>


TEEInvoker::~TEEInvoker() {
    free(req_buffer);
}

int TEEInvoker::sendRequest(Request *req) {
    int resp;

    req->serializeTo(req_buffer);
    BaseRequest *req_control = static_cast<BaseRequest *>(req_buffer);
    /* TODO write barrier */

    req_control->status = SENT;

    /* wait for status */
     while (req_control->status != DONE)
         YIELD_PROCESSOR;
     /* TODO read barrier */

     req->copyResultFrom(req_buffer);
     resp = req_control->resp;

     /* read-write barrier, no read move after this barrier, no write move before this barrier */
     req_control->status = NONE;
     return resp;
}

TEEInvoker::TEEInvoker() {
    req_buffer = (void *)malloc(sizeof (EncIntBulkRequestData));
}
