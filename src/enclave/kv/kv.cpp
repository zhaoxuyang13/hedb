#include <kv.h>

int_map *int_buf_p = TO_C_INT(new IntMap());
int_map *float_buf_p = TO_C_INT(new IntMap());

EncInt IntMap::insert(int val) {
  // printf("IntMap::insert, key: %d val: %d", counter, val);
  kv_map[counter] = val;
  uint8_t *key_in_bits = reinterpret_cast<uint8_t *>(&counter);
  // set IV, data to 0
  EncInt ret {
    {0},
    {0},
    {0},
  };
  // store uint64_t key in tag
  memcpy(ret.tag, key_in_bits, INT32_LENGTH);
  counter++;
  return ret;
}

int IntMap::find(EncInt enc_val) {
  uint64_t *key = reinterpret_cast<uint64_t *>(&enc_val.tag);
  // printf("IntMap::find, key: %d", *key);
  auto iter = kv_map.find(*key);
  if (iter == kv_map.end()) {
    printf("IntMap: key not found!");
    return 0;
  }
  // DELETE kv after find
  // int ret = iter->second;
  // kv_map.erase(iter);
  // return ret;
  return iter->second;
}

bool IntMap::erase(EncInt enc_val) {
  uint64_t *key = reinterpret_cast<uint64_t *>(&enc_val.tag);
  // printf("IntMap::erase key: %d", *key);
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
