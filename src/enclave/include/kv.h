#pragma once 
#include <enc_types.h>
#include <sgx/enclave.hpp>

#ifdef __cplusplus
#include <map>
#include <string>

/**
 * @brief std::map<key, val>
 * key: SHA256 value of EncType.data
 * val: number of accesses
 */
class BufferMap {
 private:
  std::map<std::string, int> kv_map {};
  int data_length;
 public:
  BufferMap(int data_length): data_length(data_length) {};
  ~BufferMap() = default;
  int insert(uint8_t *data);
  bool find(uint8_t *data);
  void dump();
};

#else
#include <stdbool.h>
#endif // __cpluscplus

#ifdef __cplusplus
extern "C"
{
#endif

uint64_t rdtsc();

typedef struct int_map_ int_map;
typedef struct float_map_ float_map;
typedef struct time_map_ time_map;
typedef struct text_map_ text_map;

int int_map_insert(int_map *m, uint8_t *data);
void int_map_dump(int_map *m);
int float_map_insert(float_map *m, uint8_t *data);
void float_map_dump(float_map *m);
int time_map_insert(time_map *m, uint8_t *data);
void time_map_dump(time_map *m);
int text_map_insert(text_map *m, uint8_t *data);
void text_map_dump(text_map *m);

/* GLOBAL int and float buffers */
extern int_map *int_buf_p;
extern float_map *float_buf_p;
extern time_map *time_buf_p;
extern text_map *text_buf_p;

#ifdef __cplusplus
}
#endif

EncFloat decode_bytes(uint64_t token);
uint64_t encode_bytes(EncFloat *enc);
