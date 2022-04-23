#pragma once 
#include <enc_types.h>
#include <sgx/enclave.hpp>

#ifdef __cplusplus
#include <unordered_map>

template<typename PlainType, typename EncType>
class BufferMap {
 private:
  std::unordered_map<uint64_t, PlainType> kv_map {};
  uint64_t counter = 0;
 public:
  BufferMap() = default;
  ~BufferMap() = default;
  EncType insert(PlainType val);
  PlainType find(EncType enc_val);
  bool erase(EncType enc_val);
};

#else
#include <stdbool.h>
#endif // __cpluscplus

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct int_map_ int_map;
typedef struct float_map_ float_map;

/* ================== INT ================== */
int_map *int_map_new(void);
EncInt int_map_insert(int_map *m, int val);
int int_map_find(int_map *m, EncInt enc_val);
bool int_map_erase(int_map *m, EncInt enc_val);
/* ================= FLOAT ================= */
float_map *float_map_new(void);
EncFloat float_map_insert(float_map *m, float val);
float float_map_find(float_map *m, EncFloat enc_val);
bool float_map_erase(float_map *m, EncFloat enc_val);

// rdtsc
uint64_t rdtsc();

/* GLOBAL int and float buffers */
extern int_map *int_buf_p;
extern float_map *float_buf_p;

/* GLOBAL calc counters */
extern int int_comp_count;
extern int int_calc_count;
extern int int_bulk_count;
extern int int_bulk_dec_count;
extern int float_comp_count;
extern int float_calc_count;
extern int float_bulk_count;
extern int float_bulk_dec_count;
extern int timestamp_comp_count;
extern int text_comp_count;
extern int text_like_count;
extern int text_cat_count;
extern int text_substr_count;


#ifdef __cplusplus
}
#endif

EncFloat decode_bytes(uint64_t token);
uint64_t encode_bytes(EncFloat *enc);
