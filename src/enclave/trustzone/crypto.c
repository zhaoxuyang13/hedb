#include "crypto.h"

// #define DEBUG_OUTPUT

int isgraph(int c) { return (unsigned)c - 0x21 < 0x5e; }

void _print_hex(const char *what, const void *v, const unsigned long l)
{
	const unsigned char *p = v;
	unsigned long x, y = 0, z;
	printf("%s contents: \n", what);
	for (x = 0; x < l;)
	{
		printf("%02x ", p[x]);
		if (!(++x % 16) || x == l)
		{
			if ((x % 16) != 0)
			{
				z = 16 - (x % 16);
				if (z >= 8)
					printf(" ");
				for (; z != 0; --z)
				{
					printf("   ");
				}
			}
			printf(" | ");
			for (; y < x; y++)
			{
				if ((y % 8) == 0)
					printf(" ");
				if (isgraph(p[y]))
					printf("%c", p[y]);
				else
					printf(".");
			}
			printf("\n");
		}
		else if ((x % 8) == 0)
		{
			printf(" ");
		}
	}
}
static bool inited = false;

static mbedtls_gcm_context aes;

int gcm_encrypt(uint8_t *in,
				uint32_t in_sz, uint8_t *out, uint32_t *out_sz)
{
	int res = 0;
	if (!inited)
	{
		// init the context...
		mbedtls_gcm_init(&aes);
		// printf("inited. %d\n", inited);	
		// printf("location of mbedtls_gcm_setkey %x.\n", mbedtls_gcm_setkey );	
		// Set the key. This next line could have CAMELLIA or ARIA as our GCM mode cipher...
		res = mbedtls_gcm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, (const unsigned char *)enc_key, sizeof(enc_key)*8);
		inited = true;
		//printf("setkey %d\n", inited);	
	}

	uint8_t iv[IV_SIZE] = {0};
	// Initialise the GCM cipher...
	res = mbedtls_gcm_crypt_and_tag(&aes, MBEDTLS_GCM_ENCRYPT, in_sz, iv, IV_SIZE, NULL, 0,  in, out, TAG_SIZE, out + in_sz + IV_SIZE);
	// char buffer[1000];
	// printf("before start\n");	
	// res = mbedtls_gcm_starts(&aes, MBEDTLS_GCM_ENCRYPT, iv, IV_SIZE, NULL, 0 );
	// if(res != 0) {
	// 	printf("error start %d\n",res);
	// }	
	// printf("start\n");	
	// res = mbedtls_gcm_update(&aes, in_sz, in, out);
	// if(res != 0) {
	// 	printf("error update %d\n",res);
	// }
	// printf("update\n");	
	// res = mbedtls_gcm_finish(&aes, out + IV_SIZE + in_sz, TAG_SIZE);	
	// if(res != 0) {
	// 	printf("error finish %d\n",res);
	// }	
	// printf("finish\n");	
	memcpy(out + in_sz, iv, IV_SIZE); // copy iv to out.
	*out_sz = in_sz + IV_SIZE + TAG_SIZE;
	// randombytes_buf(nonce, sizeof nonce); TODO: ADD a way to get entropy

	// _print_hex("enc-txt-plain: ", (void *)in, INT32_LENGTH);
	// _print_hex("enc-iv: ", (void *)out + INT32_LENGTH, IV_SIZE);
	// _print_hex("enc-tag: ", (void *)out + INT32_LENGTH + IV_SIZE, TAG_SIZE);
	// _print_hex("enc-txt: ", (void *)out, INT32_LENGTH);
#ifdef DEBUG_OUTPUT
	printf("huk-key length %d\n", sizeof(huk_key));
	_print_hex("huk-key: ", huk_key, sizeof(huk_key));
	_print_hex("enc-txt-plain: ", (void *)in, INT32_LENGTH);
	_print_hex("enc-iv: ", (void *)hdr->iv, IV_SIZE);
	_print_hex("enc-tag: ", (void *)hdr->tag, TAG_SIZE);
	_print_hex("enc-txt: ", (void *)hdr->enc_key, INT32_LENGTH);
#endif
	return res;
}

int gcm_decrypt(uint8_t *in,
				uint32_t in_sz, uint8_t *out, uint32_t *out_sz)
{
	if (!inited)
	{
		// init the context...
		mbedtls_gcm_init(&aes);
		// Set the key. This next line could have CAMELLIA or ARIA as our GCM mode cipher...
		mbedtls_gcm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, (const unsigned char *)enc_key, strlen(enc_key) * 8);
		inited = true;
	}
	int res = 0;
	int plain_sz = in_sz - IV_SIZE - TAG_SIZE;
	res = mbedtls_gcm_auth_decrypt(&aes, plain_sz, in + plain_sz, IV_SIZE, NULL, 0, in + IV_SIZE + plain_sz, TAG_SIZE, in, out);
	return res;
}
