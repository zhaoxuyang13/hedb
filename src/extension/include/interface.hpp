#pragma once 

#include <request.hpp>
#include <request_types.h>
#include <sodium.h>

#define ADDITIONAL_DATA (const unsigned char *) "123456"
#define ADDITIONAL_DATA_LEN 6

class TEEInvoker {
private:
    TEEInvoker();
    static TEEInvoker* invoker;
    void *req_buffer;
public: 
    ~TEEInvoker();
    static TEEInvoker *getInstance(){
        if (invoker == nullptr){
            invoker = new TEEInvoker;
        }
        return invoker;
    }
    /* send request, get result in req->res, get resp as return value.*/
    int sendRequest(Request *req);

};

class Cryptor {
private:
    unsigned char nonce[crypto_aead_aes256gcm_NPUBBYTES];
    unsigned char key[crypto_aead_aes256gcm_KEYBYTES];
    static Cryptor *cryptor_ptr;
    Cryptor();
public:
    ~Cryptor();
    static Cryptor *getInstance();
    void encrypt(const uint8_t *message, uint64_t mlen, uint8_t *ciphertxt, uint64_t *clen);
    void decrypt(uint8_t *message, uint64_t *mlen, const uint8_t *ciphertext, const uint64_t clen);
};

extern Cryptor *cryptor_ptr;
