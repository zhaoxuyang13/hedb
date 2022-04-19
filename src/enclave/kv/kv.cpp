#include <kv.h>

int_map *int_buf_p = (int_map *)(new BufferMap<int, EncInt>());
float_map *float_buf_p = (float_map *)(new BufferMap<float, EncFloat>());

template<typename PlainType, typename EncType>
EncType BufferMap<PlainType, EncType>::insert(PlainType val) {
  printf("BufferMap::insert, key: %d val: %f", counter, val);
  kv_map[counter] = val;
  uint8_t *key_in_bits = reinterpret_cast<uint8_t *>(&counter);
  // set IV, data to 0
  EncType ret {
    {0},
    {0},
    {0},
  };
  // store uint64_t key in tag
  memcpy(ret.tag, key_in_bits, sizeof(uint64_t));
  counter++;
  return ret;
}

template<typename PlainType, typename EncType>
PlainType BufferMap<PlainType, EncType>::find(EncType enc_val) {
  uint64_t *key = reinterpret_cast<uint64_t *>(&enc_val.tag);
  auto iter = kv_map.find(*key);
  if (iter == kv_map.end()) {
    printf("BufferMap: key not found! key: %d", *key);
    return 0;
  }
  // DELETE kv after find
  printf("BufferMap::find, key: %d, val: %f", *key, iter->second);
  // int ret = iter->second;
  // kv_map.erase(iter);
  // return ret;
  return iter->second;
}

template<typename PlainType, typename EncType>
bool BufferMap<PlainType, EncType>::erase(EncType enc_val) {
  uint64_t *key = reinterpret_cast<uint64_t *>(&enc_val.tag);
  printf("BufferMap::erase key: %d", *key);
  auto iter = kv_map.find(*key);
  if (iter != kv_map.end()) {
    kv_map.erase(iter);
    return true;
  }
  return false;
}

int_map *int_map_new(void) {
  return (int_map *)(new BufferMap<int, EncInt>());
}

EncInt int_map_insert(int_map *m, int val) {
  return ((BufferMap<int, EncInt> *)m)->insert(val);
}

int int_map_find(int_map *m, EncInt enc_val) {
  return ((BufferMap<int, EncInt> *)m)->find(enc_val);
}

bool int_map_erase(int_map *m, EncInt enc_val) {
  return ((BufferMap<int, EncInt> *)m)->erase(enc_val);
}

float_map *float_map_new(void) {
  return (float_map *)(new BufferMap<float, EncFloat>());
}

EncFloat float_map_insert(float_map *m, float val) {
  return ((BufferMap<float, EncFloat> *)m)->insert(val);
}
float float_map_find(float_map *m, EncFloat enc_val) {
  return ((BufferMap<float, EncFloat> *)m)->find(enc_val);
}
bool float_map_erase(float_map *m, EncFloat enc_val) {
  return ((BufferMap<float, EncFloat> *)m)->erase(enc_val);
}
