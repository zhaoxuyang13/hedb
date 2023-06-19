#pragma once 
// #include "hashmap.hpp"
#include <map>
#include <functional>
#include <string>
#include <vector>
#include <filesystem>
#include "vectormap.hpp"
#include "defs.h"
#include <shared_mutex>
#include <mutex>

typedef std::shared_mutex Lock;
typedef std::unique_lock<Lock>  WriteLock;
typedef std::shared_lock<Lock>  ReadLock;

class KVStore{
    /* xx_kv[index] is hashmap for map_id = index + 1, index = map_id - 1 */
    std::vector<VectorMap<int>> int_kv;
    std::vector<VectorMap<float>> float_kv;
    std::vector<VectorMap<TIMESTAMP>> ts_kv;
    std::vector<VectorMap<std::string>> str_kv;
    // Lock int_lock, float_lock, ts_lock, str_lock;
public:
    KVStore(uint32_t int_size, uint32_t float_size, uint32_t str_size, uint32_t ts_size) : 
        int_kv(int_size + 1), float_kv(float_size + 1), ts_kv(ts_size + 1), str_kv(str_size + 1) {
            std::cout << "KVStore init with size: " << int_size << ", " << float_size << ", " << ts_size << ", " << str_size << std::endl;
        }
    uint32_t push_back(uint32_t map_id, int value){
        return int_kv[map_id].push_back(value);
    }
    uint32_t push_back(uint32_t map_id, float value){
        return float_kv[map_id].push_back(value);
    }
    uint32_t push_back(uint32_t map_id, TIMESTAMP value){
        return ts_kv[map_id].push_back(value);
    }
    uint32_t push_back(uint32_t map_id, const char *value){
        return str_kv[map_id].push_back(value);
    }

    int find_int(uint32_t map_id, uint32_t key){
        return int_kv[map_id].find(key);
    }
    float find_float(uint32_t map_id, uint32_t key){
        
        return float_kv[map_id].find(key);
    }
    TIMESTAMP find_ts(uint32_t map_id, uint32_t key){
        return ts_kv[map_id].find(key);
    }
    std::string& find_str(uint32_t map_id, uint32_t key){
        return str_kv[map_id].find(key);
    }
    std::string find_str_tmp(uint32_t key){
        return str_kv[0].find_tmp(key);
    }

    uint32_t insert_tmp(int value){
        // WriteLock r_lock(int_lock);
        return int_kv[0].push_back(value);
    }
    uint32_t insert_tmp(float value){
        // WriteLock r_lock(float_lock);
        return float_kv[0].push_back(value);
    }
    uint32_t insert_tmp(TIMESTAMP value){
        // WriteLock r_lock(ts_lock);
        return ts_kv[0].push_back(value);
    }
    uint32_t insert_tmp(char *value){
        // WriteLock r_lock(str_lock);
        return str_kv[0].push_back(value);
    }

    void save(std::string filename_prefix){
        filename_prefix = "./kv/" + filename_prefix;
        for(uint32_t i = 1; i < int_kv.size(); i++)
            int_kv[i].save(filename_prefix + "_" + std::to_string(i) + ".int");
        for(uint32_t i = 1; i < float_kv.size(); i++)
            float_kv[i].save(filename_prefix + "_" + std::to_string(i) + ".float");
        for(uint32_t i = 1; i < ts_kv.size(); i++)
            ts_kv[i].save(filename_prefix + "_" + std::to_string(i) + ".ts");
        for(uint32_t i = 1; i < str_kv.size(); i++)
            str_kv[i].save(filename_prefix + "_" + std::to_string(i) + ".str");
    }

    void load(std::string filename_prefix){
        /* open directory . and list all files in directory . begin with filename */
        std::filesystem::path dir("./kv");
        for(const auto& entry : std::filesystem::directory_iterator(dir)){
            if(entry.path().filename().string().find(filename_prefix) == 0){
                /* find the file */
                std::string filename = entry.path().filename().string();
                std::string full_filename = entry.path().string();
                std::string suffix = filename.substr(filename.find_last_of(".") + 1);
                std::string map_id_str = filename.substr(filename.find_last_of("_") + 1);
                uint32_t map_id = std::stoi(map_id_str);
                if(suffix == "int"){
                    if(int_kv.size() < map_id + 1)
                        int_kv.resize(map_id + 1);
                    int_kv[map_id].load(full_filename);
                }else if(suffix == "float"){
                    if(float_kv.size() < map_id + 1)
                        float_kv.resize(map_id + 1);
                    float_kv[map_id].load(full_filename);
                }else if(suffix == "ts"){
                    if(ts_kv.size() < map_id + 1)
                        ts_kv.resize(map_id + 1);
                    ts_kv[map_id].load(full_filename);
                }
                else if(suffix == "str"){
                    if(str_kv.size() < map_id + 1)
                        str_kv.resize(map_id + 1);
                    str_kv[map_id].load(full_filename);
                }
            }
        }
    }

};
