#pragma once 
// #include "hashmap.hpp"
#include <map>
#include <functional>
#include <filesystem>
#include "defs.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
// #include "serialization.hpp"

#include <iostream>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <scoped_allocator>


using namespace boost::interprocess;

template<typename T>
using segment_allocator = allocator<T, managed_mapped_file::segment_manager>;
using void_allocator = allocator<void, managed_mapped_file::segment_manager>;
using segment_manager_t = managed_mapped_file::segment_manager;


template <typename T>
using ShmAllocator = allocator<T, segment_manager_t>;
template <typename T>
using ShmVector = vector<T, ShmAllocator<T>>;
template <typename T>
using ShmVectorAllocator = std::scoped_allocator_adaptor<allocator<ShmVector<T>, segment_manager_t>>;
template <typename T>
using Shm2DVector = vector<ShmVector<T>, ShmVectorAllocator<T>>;

using Shm2DIntVector = Shm2DVector<int>;
using Shm2DFloatVector = Shm2DVector<float>;
using Shm2DTimestampVector = Shm2DVector<TIMESTAMP>;

/* specialize for string*/
using CharAllocator = allocator<char, segment_manager_t> ;
using ShmString = basic_string<char, std::char_traits<char>, CharAllocator> ;
using ShmStringAllocator = std::scoped_allocator_adaptor<allocator<ShmString, segment_manager_t>>;
using ShmStringVector = vector<ShmString, ShmStringAllocator>;
using ShmStringVectorAllocator = std::scoped_allocator_adaptor<allocator<ShmStringVector, segment_manager_t>>;
using Shm2DStringVector = vector<ShmStringVector, ShmStringVectorAllocator>;

class KVStore{
    Shm2DIntVector int_kv;
    Shm2DFloatVector float_kv;
    Shm2DTimestampVector ts_kv;
    Shm2DStringVector str_kv;

    // template<typename T>
    // void save(const std::string& filename, T& _vec){
    //     std::ofstream file(filename);
    //     if(file.is_open()){
    //         boost::archive::text_oarchive oa(file);
    //         oa << _vec;
    //         file.close();
    //         std::cout << "Serialization complete." << std::endl;
    //     }else{
    //         std::cout << "Failed to open file for serialization." << std::endl;
    //     }
    // }


    // template<typename T>
    // void load(const std::string& filename, T& _vec){
    //     std::ifstream file(filename);
    //     if (file.is_open()){
    //         boost::archive::text_iarchive ia(file);
    //         ia >> _vec;
    //         file.close();
    //         std::cout << "Unserialization complete." << std::endl;
    //     }else{
    //         std::cout << "Failed to open file for unserialization." << std::endl;
    //     }
    // }

public:
    KVStore(uint32_t int_size, uint32_t float_size, uint32_t ts_size, uint32_t str_size, const void_allocator& alloc) : 
        int_kv(int_size + 1, alloc), float_kv(float_size + 1, alloc), ts_kv(ts_size + 1, alloc)
        , str_kv(str_size + 1, alloc)
        {
            std::cout << "KVStore init with size: " << int_size << ", " << float_size << ", " << ts_size << ", " << str_size << std::endl;
    }
    KVStore() = default;
    uint32_t push_back(uint32_t map_id, int value){
        int_kv[map_id].emplace_back(value);
        return int_kv[map_id].size() - 1;
    }
    uint32_t push_back(uint32_t map_id, float value){
        float_kv[map_id].emplace_back(value);
        return float_kv[map_id].size() - 1;
    }
    uint32_t push_back(uint32_t map_id, TIMESTAMP value){
        ts_kv[map_id].emplace_back(value);
        return ts_kv[map_id].size() - 1;
    }
    uint32_t push_back(uint32_t map_id, const char *value){
        str_kv[map_id].emplace_back(value);
        return str_kv[map_id].size() - 1;
    }

    int find_int(uint32_t map_id, uint32_t key){
        return int_kv[map_id][key];
    }
    float find_float(uint32_t map_id, uint32_t key){
        return float_kv[map_id][key];
    }
    TIMESTAMP find_ts(uint32_t map_id, uint32_t key){
        return ts_kv[map_id][key];
    }
    ShmString& find_str(uint32_t map_id, uint32_t key){
        return str_kv[map_id][key];
    }
    ShmString find_str_tmp(uint32_t key){
        return str_kv[0][key]; 
    }

    uint32_t insert_tmp(int value){
        return push_back(0, value);
    }
    uint32_t insert_tmp(float value){
        return push_back(0, value);
    }
    uint32_t insert_tmp(TIMESTAMP value){
        return push_back(0, value);
    }
    uint32_t insert_tmp(const char *value){
        return push_back(0, value);
    }

    auto getIntKV(){
        return int_kv;
    }
    auto getFloatKV(){
        return float_kv;
    }
    auto getTSKV(){
        return ts_kv;
    }
    auto getStrKV(){
        return str_kv;
    }
    // void save(std::string filename_prefix){
    //     filename_prefix = "./kv/" + filename_prefix;
    //     for(uint32_t i = 1; i < int_kv.size(); i++)
    //         save(filename_prefix + "_" + std::to_string(i) + ".int", int_kv[i]);
    //     for(uint32_t i = 1; i < float_kv.size(); i++)
    //         save(filename_prefix + "_" + std::to_string(i) + ".float", float_kv[i]);
    //     for(uint32_t i = 1; i < ts_kv.size(); i++)
    //         save(filename_prefix + "_" + std::to_string(i) + ".ts", ts_kv[i]);
    //     // for(uint32_t i = 1; i < str_kv.size(); i++)
    //     //     save(filename_prefix + "_" + std::to_string(i) + ".str", str_kv[i]);
    // }

    // void load(std::string filename_prefix){
    //     /* open directory . and list all files in directory . begin with filename */
    //     std::filesystem::path dir("./kv");
    //     for(const auto& entry : std::filesystem::directory_iterator(dir)){
    //         if(entry.path().filename().string().find(filename_prefix) == 0){
    //             /* find the file */
    //             std::string filename = entry.path().filename().string();
    //             std::string full_filename = entry.path().string();
    //             std::string suffix = filename.substr(filename.find_last_of(".") + 1);
    //             std::string map_id_str = filename.substr(filename.find_last_of("_") + 1);
    //             uint32_t map_id = std::stoi(map_id_str);
    //             if(suffix == "int"){
    //                 if(int_kv.size() < map_id + 1)
    //                     int_kv.resize(map_id + 1);
    //                 load(full_filename, int_kv[map_id]);
    //             }else if(suffix == "float"){
    //                 if(float_kv.size() < map_id + 1)
    //                     float_kv.resize(map_id + 1);
    //                 load(full_filename, float_kv[map_id]);
    //             }else if(suffix == "ts"){
    //                 if(ts_kv.size() < map_id + 1)
    //                     ts_kv.resize(map_id + 1);
    //                 load(full_filename, ts_kv[map_id]);
    //             }
    //             // else if(suffix == "str"){
    //             //     if(str_kv.size() < map_id + 1)
    //             //         str_kv.resize(map_id + 1);
    //             //     load(full_filename, str_kv[map_id]);
    //             // }
    //         }
    //     }
    // }

};
