#pragma once 
#include <enc_types.h>
#include <sgx/enclave.hpp>

#define CACHE_SIZE 11

#ifdef __cplusplus
#include <unordered_map>
#include <functional>
#include <array>
#include <utility>

struct enc_hash {
  template<typename EncType>
  std::size_t operator() (const EncType &enc_val) const {
    return (enc_val.data[0] + enc_val.data[1] << 4 + enc_val.data[2] << 8 + enc_val.data[3] << 12) % 11;
  }
};

template<typename PlainType, typename EncType>
class BufferMap {
 private:
  // std::unordered_map<EncType, PlainType, enc_hash> kv_map {};
  std::array<std::pair<EncType, PlainType>, CACHE_SIZE> arr_map {};
 public:
  BufferMap() = default;
  ~BufferMap() = default;
  void insert(const EncType *enc_val, const PlainType plain_val);
  PlainType find(const EncType *enc_val, bool *found);
};

constexpr bool operator==(const EncFloat &lhs, const EncFloat &rhs);

#else
#include <stdbool.h>
#endif // __cplusplus

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef struct buffer_map_ buffer_map;

/* ================= FLOAT ================= */
void float_map_insert(buffer_map *m, const EncFloat *enc_val, const float plain_val);
float float_map_find(buffer_map *m, const EncFloat *enc_val, bool *found);

/* ================ Global ================= */
extern buffer_map *f_map_p;

#ifdef __cplusplus
}
#endif // __cplusplus

EncFloat decode_bytes(uint64_t token);
uint64_t encode_bytes(EncFloat *enc);