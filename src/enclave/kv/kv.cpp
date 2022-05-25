#include <kv.h>
#include <fstream>

int_map *int_buf_p = (int_map *)(new BufferMap(4));
float_map *float_buf_p = (float_map *)(new BufferMap(4));
time_map *time_buf_p = (time_map *)(new BufferMap(8));
text_map *text_buf_p = (text_map *)(new BufferMap(1024));

int BufferMap::insert(uint8_t *data) {
  sgx_sha256_hash_t key;
  sgx_status_t hash_status = sgx_sha256_msg(data, data_length, &key);
  std::string key_str = std::string(reinterpret_cast<char *>(key));
  auto iter = kv_map.find(key_str);
  if (iter == kv_map.end())
    kv_map[key_str] = 1;
  else
    kv_map[key_str] = iter->second + 1;
  printf("Value is: %d", kv_map[key_str]);
  return kv_map[key_str];
}

bool BufferMap::find(uint8_t *data) {
  sgx_sha256_hash_t key;
  sgx_status_t hash_status = sgx_sha256_msg(data, data_length, &key);
  std::string key_str = std::string(reinterpret_cast<char *>(key));
  auto iter = kv_map.find(key_str);
  if (iter == kv_map.end())
    return false;
  return true;
}

void BufferMap::dump() {
  // printf("DUMP");
  std::string result = "";
  for (auto iter: kv_map) {
    result = result + std::to_string(iter.second) + ',';
  }
  printf("Len: %d, %s", result.length(), result.c_str());
  return;
}

int int_map_insert(int_map *m, uint8_t *data) {
  return ((BufferMap *)m)->insert(data);
}

void int_map_dump(int_map *m) {
  return ((BufferMap *)m)->dump();
}

int float_map_insert(float_map *m, uint8_t *data) {
  return ((BufferMap *)m)->insert(data);
}

void float_map_dump(float_map *m) {
  return ((BufferMap *)m)->dump();
}

int time_map_insert(time_map *m, uint8_t *data) {
  return ((BufferMap *)m)->insert(data);
}

void time_map_dump(time_map *m) {
  return ((BufferMap *)m)->dump();
}

int text_map_insert(text_map *m, uint8_t *data) {
  return ((BufferMap *)m)->insert(data);
}

void text_map_dump(text_map *m) {
  return ((BufferMap *)m)->dump();
}

uint64_t rdtsc(){
  unsigned int lo,hi;
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t)hi << 32) | lo;
}
