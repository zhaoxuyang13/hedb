#pragma once 
#include <enc_types.h>
#include <sgx/enclave.hpp>

#define CACHE_SIZE 11

#ifdef __cplusplus
#include <functional>
#include <vector>
#include <utility>

extern int count_total;
extern int count_found;

constexpr bool operator==(const EncFloat &lhs, const EncFloat &rhs);

struct enc_hash {
  template<typename EncType>
  std::size_t operator() (const EncType &enc_val) const {
    // return abs((enc_val.data[0] + enc_val.data[1] << 4 + enc_val.data[2] << 8 + enc_val.data[3] << 12) % CACHE_SIZE);
    size_t hash = 1;
    for (size_t i = 0; i < 4; ++i) {
      hash = hash * 31 + enc_val.data[0];
    }
    return abs(hash % CACHE_SIZE);
  }
};

template<typename PlainType, typename EncType>
class BufferMap {
 private:
  std::vector<std::pair<EncType, PlainType>> arr_map {};
 public:
  BufferMap() {
    this->arr_map.reserve(CACHE_SIZE);
  };
  ~BufferMap() = default;
  void insert(const EncType *enc_val, const PlainType *plain_val);
  PlainType find(const EncType *enc_val, bool *found);
};

#else
#include <stdbool.h>
#endif // __cplusplus

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef struct buffer_map_ buffer_map;

/* ================= FLOAT ================= */
void float_map_insert(buffer_map *m, const EncFloat *enc_val, const float *plain_val);
float float_map_find(buffer_map *m, const EncFloat *enc_val, bool *found);
/* ================= TEXT ================= */
void text_map_insert(buffer_map *m, const EncCStr *enc_val, const Str *plain_val);
Str text_map_find(buffer_map *m, const EncCStr *enc_val, bool *found);

/* ================ Global ================= */
extern buffer_map *f_map_p;
extern buffer_map *t_map_p;

#ifdef __cplusplus
}
#endif // __cplusplus

EncFloat decode_bytes(uint64_t token);
uint64_t encode_bytes(EncFloat *enc);