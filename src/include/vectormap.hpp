#pragma once 
#include <iostream>
#include <fstream>
#include <string>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#include "serialization.hpp"

using boost::interprocess::string;
using boost::interprocess::vector;


template<typename Vt>
class VectorMap{
    vector<Vt> _vec;

public:
    VectorMap(){ }
    
    void insert(uint32_t key, Vt& value){
        // assert(key == _vec.size());
        // _vec.push_back(value);
        // return ;
        /* */
        if(_vec.size() < key + 1){
            _vec.resize(key + 1);
        }
        _vec[key] = value;
    }
    uint32_t push_back(Vt& value){
        _vec.push_back(value);
        return _vec.size() - 1;
    }
    
    Vt& find(uint32_t key){
        return _vec[key];
    }
    Vt find_tmp(uint32_t key){
        return _vec[key];
    }

    void save(const std::string& filename){
        std::ofstream file(filename);
        if(file.is_open()){
            boost::archive::text_oarchive oa(file);
            oa << _vec;
            file.close();
            std::cout << "Serialization complete." << std::endl;
        }else{
            std::cout << "Failed to open file for serialization." << std::endl;
        }
    }

    void load(const std::string& filename){
        std::ifstream file(filename);
        if (file.is_open()){
            boost::archive::text_iarchive ia(file);
            ia >> _vec;
            file.close();
            std::cout << "Unserialization complete." << std::endl;
        }else{
            std::cout << "Failed to open file for unserialization." << std::endl;
        }
    }

    
};

// template specialization for std::string
template<>
class VectorMap<string>{
    vector<string> _vec;
public: 
    VectorMap(){ 
    }
    void insert(uint32_t key, char* value){
        if(_vec.size() < key + 1){
            _vec.resize(key + 1);
        }
        _vec[key] = string(value);
    }

    uint32_t push_back(const char* value){
        _vec.emplace_back(string(value));
        return _vec.size() - 1;
    }
    string& find(uint32_t key){
        return _vec[key];
    }
    string find_tmp(uint32_t key){
        return _vec[key];
    }

    void save(const std::string& filename){
        std::ofstream file(filename);
        if(file.is_open()){
            boost::archive::text_oarchive oa(file);
            oa << _vec;
            file.close();
            std::cout << "Serialization complete." << std::endl;
        }else{
            std::cout << "Failed to open file for serialization." << std::endl;
        }
    }

    void load(const std::string& filename){
        std::ifstream file(filename);
        if (file.is_open()){
            boost::archive::text_iarchive ia(file);
            ia >> _vec;
            file.close();
            std::cout << "Unserialization complete." << std::endl;
        }else{
            std::cout << "Failed to open file for unserialization." << std::endl;
        }
    }

};