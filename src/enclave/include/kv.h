#pragma once 
#include <enc_types.h>
#include <sgx/enclave.hpp>
#define TO_CPP_INT(a) (reinterpret_cast<IntMap *>(a))
#define TO_C_INT(a)   (reinterpret_cast<int_map *>(a))

#ifdef __cplusplus
#include <map>

class IntMap {
 private:
  std::map<uint64_t, int> kv_map {};
  uint64_t counter = 0;
 public:
  IntMap() = default;
  ~IntMap() = default;
  EncInt insert(int val);
  int find(EncInt enc_val);
  bool erase(EncInt enc_val);
};

#else
#include <stdbool.h>
#endif // __cpluscplus

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct int_map_ int_map;

int_map *int_map_new(void);
EncInt int_map_insert(int_map *m, int val);
int int_map_find(int_map *m, EncInt enc_val);
bool int_map_erase(int_map *m, EncInt enc_val);

/* GLOBAL int and float buffer */
extern int_map *int_buf_p;
extern int_map *float_buf_p;

#ifdef __cplusplus
}
#endif


EncFloat decode_bytes(uint64_t token);
uint64_t encode_bytes(EncFloat *enc);
