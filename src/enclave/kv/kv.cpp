#include <kv.h>

buffer_map *f_map_p = (buffer_map *)(new BufferMap<float, EncFloat>());
buffer_map *t_map_p = (buffer_map *)(new BufferMap<Str, EncCStr>());

constexpr bool operator==(const EncFloat &lhs, const EncFloat &rhs) {
  return memcmp(&lhs, &rhs, sizeof(EncFloat)) == 0;
}

template<typename PlainType, typename EncType>
void BufferMap<PlainType, EncType>::insert(const EncType *enc_val, const PlainType *plain_val) {
  size_t bucket = enc_hash()(*enc_val);
  arr_map[bucket] = std::pair<EncType, PlainType>(*enc_val, *plain_val);
}

template<typename PlainType, typename EncType>
PlainType BufferMap<PlainType, EncType>::find(const EncType *enc_val, bool *found) {
  size_t bucket = enc_hash()(*enc_val);
  if (memcmp(enc_val, &arr_map[bucket].first, sizeof(EncType)) == 0) {
    *found = true;
    return arr_map[bucket].second;
  }
  *found = false;
  return PlainType{};
}

void float_map_insert(buffer_map *m, const EncFloat *enc_val, const float *plain_val) {
  ((BufferMap<float, EncFloat> *)m)->insert(enc_val, plain_val);
}

float float_map_find(buffer_map *m, const EncFloat *enc_val, bool *found) {
  return ((BufferMap<float, EncFloat> *)m)->find(enc_val, found);
}

void text_map_insert(buffer_map *m, const EncCStr *enc_val, const Str *plain_val) {
  ((BufferMap<Str, EncCStr> *)m)->insert(enc_val, plain_val);
}

Str text_map_find(buffer_map *m, const EncCStr *enc_val, bool *found) {
  return ((BufferMap<Str, EncCStr> *)m)->find(enc_val, found);
}
