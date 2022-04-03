#include <kv.h>

int_map *int_buf_p = TO_C_INT(new IntMap());

EncInt IntMap::insert(int val) {
  printf("IntMap::insert");
  kv_map[counter] = val;
  uint8_t *key_in_bits = reinterpret_cast<uint8_t *>(&counter);
  return EncInt {
    0,
    0,
    *key_in_bits,
  };
}

int IntMap::find(EncInt enc_val) {
  printf("IntMap::find");
  size_t *key = reinterpret_cast<size_t *>(&enc_val.data);
  auto iter = kv_map.find(*key);
  if (iter == kv_map.end()) {
    printf("IntMap: key not found!");
    return 0;
  }
  return iter->second;
}

bool IntMap::erase(EncInt enc_val) {
  printf("IntMap::erase");
  size_t *key = reinterpret_cast<size_t *>(&enc_val.data);
  auto iter = kv_map.find(*key);
  if (iter != kv_map.end()) {
    kv_map.erase(iter);
    return true;
  }
  return false;
}

int_map *int_map_new(void) {
  return TO_C_INT(new IntMap());
}

EncInt int_map_insert(int_map *m, int val) {
  return TO_CPP_INT(m)->insert(val);
}

int int_map_find(int_map *m, EncInt enc_val) {
  return TO_CPP_INT(m)->find(enc_val);
}

bool int_map_erase(int_map *m, EncInt enc_val) {
  return TO_CPP_INT(m)->erase(enc_val);
}
