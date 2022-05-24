#include <interface.hpp>
#include <tee_interface.hpp>
#include <sync.h>
#include <extension_helper.hpp>
#include <timer.hpp>
#include <stdlib.h> // at exit

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

void exit_handler(){
    TEEInvoker *invoker = TEEInvoker::getInstance();
    delete invoker;
}


TEEInvoker::TEEInvoker() {
    // print_info("get shared buffer");
    req_buffer = getSharedBuffer(sizeof (EncIntBulkRequestData));
    // print_info("buffer got");
    atexit(exit_handler);
}

Cryptor *Cryptor::cryptor_ptr = nullptr;
Cryptor::Cryptor() {
    if (sodium_init() == -1) {
        print_error("error: sodium init");
        atexit(exit_handler);
    }
    if (crypto_aead_aes256gcm_is_available() == 0) {
        print_error("error: aes256gcm unavailable"); /* Not available on this CPU */
        atexit(exit_handler);
    }
    crypto_aead_aes256gcm_keygen(this->key);
    randombytes_buf(this->nonce, sizeof(this->nonce));
}

Cryptor *Cryptor::getInstance() {
    if (cryptor_ptr == nullptr) {
        cryptor_ptr = new Cryptor();
    }
    return cryptor_ptr;
}

void Cryptor::encrypt(const uint8_t *message, uint64_t mlen, uint8_t *ciphertxt, uint64_t *clen) {
    unsigned long long result;
    crypto_aead_aes256gcm_encrypt(ciphertxt, &result,
                            message, mlen,
                            ADDITIONAL_DATA, ADDITIONAL_DATA_LEN,
                            NULL, nonce, key);
    *clen = (uint64_t)result;
}

void Cryptor::decrypt(uint8_t *message, uint64_t *mlen, const uint8_t *ciphertext, const uint64_t clen) {
    unsigned long long result;
    if (crypto_aead_aes256gcm_decrypt(message, &result,
                                    NULL,
                                    ciphertext, clen,
                                    ADDITIONAL_DATA,
                                    ADDITIONAL_DATA_LEN,
                                    nonce, key) != 0 || clen < crypto_aead_aes256gcm_ABYTES) {
        print_error("error: decrypt failed: %ul", clen);
        atexit(exit_handler);
    }
    *mlen = (uint64_t)result;
}
