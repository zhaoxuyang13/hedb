#include <iostream>
#include <fstream>
#include <filesystem>
#include "kv.hpp"
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <vector>
#include <string> // for parsing files and strings
#include "timestamp_parse.hpp"

// extern "C" {
// #include <postgres.h>
// #include <fmgr.h>
// #include <utils/array.h>
// #include <utils/arrayaccess.h>
// #include <utils/builtins.h>
// #include <utils/numeric.h>
// #include <datatype/timestamp.h>
// #include <utils/datetime.h>
// #include <utils/timestamp.h>
// }

std::string data_path = "./benchmark/tools";

enum enc_type {
    None,
    EncInt,
    EncFloat,
    EncString,
    EncTimestamp,
};

const std::vector<std::string> tpch_tbls = {
    "nation.tbl",
    "region.tbl",
    "part.tbl",
    "supplier.tbl",
    "partsupp.tbl",
    "customer.tbl",
    "orders.tbl",
    "lineitem.tbl"
};

// all the keys in schema are None
const std::vector<std::vector<enc_type>> tpch_schema = {
    { None, EncString, None, EncString },
    { None, EncString, EncString },
    { None, EncString, EncString, EncString, EncString, EncInt, EncString, EncFloat, EncString },
    { None, EncString, EncString, None, EncString, EncFloat, EncString},
    { None, None, EncFloat, EncFloat, EncString},
    { None, EncString, EncString, None, EncString, EncFloat, EncString, EncString},
    { None, None, EncString, EncFloat, EncTimestamp, EncString, EncString, EncInt, EncString},
    { None, None, None, None, EncFloat, EncFloat, EncFloat, EncFloat, EncString, EncString, EncTimestamp, EncTimestamp, EncTimestamp, EncString, EncString, EncString}
};

uint32_t cnts[5] = {0, 0, 0, 0, 0};
uint64_t combine(uint32_t a, uint32_t b){
    uint64_t c = a;
    return c << 32 | b;
}


Timestamp parseTimestamp(const char* str)
{

    Timestamp result;
    char workbuf[MAXDATELEN + MAXDATEFIELDS];
    char* field[MAXDATEFIELDS];
    int ftype[MAXDATEFIELDS];
    int dterr;
    int nf;
    int tz;
    int dtype;
    fsec_t fsec;
    struct pg_tm tt, *tm_ptr = &tt;
    char buf[MAXDATELEN + 1];
    char src_byte[TIMESTAMP_LENGTH];
    int resp;

    dterr = ParseDateTime(str, workbuf, sizeof(workbuf), field, ftype, MAXDATEFIELDS, &nf);

    assert(nf == 1);

    if (dterr == 0)
        dterr = DecodeDateTime(field, ftype, nf, &dtype, tm_ptr, &fsec, &tz);

    assert(dtype == 2);
    tm2timestamp(tm_ptr, fsec, NULL, &result);


    return result;
}

#include <chrono>

// TIMESTAMP parseTimestamp(const char* str)
// {
//     // parse the string to postgresql TIMESTAMP, 
//     // string format: YYYY-MM-DD, TIMESTAMP is microsecond from 1970-01-01
//     // return the TIMESTAMP

//     std::tm timeinfo = {};
//     std::istringstream ss(str);

//     // Extract year, month, and day from the string
//     if (!(ss >> std::get_time(&timeinfo, "%Y-%m-%d"))) {
//         std::cerr << "Error parsing the timestamp string" << std::endl;
//         return 0;
//     }

//     // Convert tm to time_point
//     std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(std::mktime(&timeinfo));

//     // Calculate the number of microseconds since 2000-01-01
//     auto us = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()) - std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(946684800));
//     return us.count();
// }





int transform_tbl(const std::string tbl_path, const std::string out_path, const std::vector<enc_type>& table_schema, KVStore *kv){
    std::ifstream infile(tbl_path);
    std::ofstream outfile(out_path);
    std::string line;

    std::cout << "transforming " << tbl_path << std::endl;
    // std::cout << "mapids begins at " << int_cnt << " " << float_cnt << " " << string_cnt << " " << timestamp_cnt << std::endl;
    std::vector<uint32_t> map_ids(table_schema.size());
    
    for(uint32_t i = 0; i < table_schema.size(); i++){
        cnts[table_schema[i]] ++;
        map_ids[i] = cnts[table_schema[i]];
    }
    while(std::getline(infile, line)){
        std::vector<std::string> row;
        // create a input stream on line, with delimiter '|'
        std::istringstream iss(line);
        std::string field;
        std::string out_line;
        while(std::getline(iss, field, '|')){
            row.push_back(field);
        }
        for(uint32_t i = 0; i < row.size(); i++){
            std::string field = row[i];
            uint32_t mapid = map_ids[i];
            switch (table_schema[i])
            {
            case EncInt:{
                int value = std::stoi(field);
                uint32_t key = kv->push_back(mapid, value);
                out_line += FLAG_CHAR + std::to_string(combine(mapid, key)) + "|";
                break;
            }
            case EncFloat:{
                float value = std::stof(field);
                uint32_t key = kv->push_back(mapid, value);
                out_line += FLAG_CHAR + std::to_string(combine(mapid, key)) + "|";
                break;
            }
            case EncString:{
                uint32_t key = kv->push_back(mapid, field.c_str());
                out_line += FLAG_CHAR + std::to_string(combine(mapid, key)) + "|";
                break;
            }
            case EncTimestamp:{
                TIMESTAMP value = parseTimestamp(field.c_str());
                // printf("timestamp: %llu\n", value);
                uint32_t key = kv->push_back(mapid,  value);
                out_line += FLAG_CHAR + std::to_string(combine(mapid, key)) + "|";
                break;
            }
            case None:{
                out_line += field + "|";
                break;
            }
            default:
                break;
            }
        }
        outfile << out_line << std::endl;
    }
    outfile.close();
    std::cout << "save to " << out_path << std::endl; 
    return 0;
}

using namespace boost::interprocess;
using void_allocator = allocator<void, managed_shared_memory::segment_manager>;


const uint64_t MB = 1024 * 1024;
const uint64_t GB = 1024 * MB;

const char * MMAP_FILENAME = "./kv.mmap";
int main(){
    // read tpch_tbls in data_path, use filesystem lib to read files.

    /* count the number of types in schema*/
    uint32_t num[5] = {0};
    for(uint32_t i = 0; i < tpch_schema.size(); i++){
        for(uint32_t j = 0; j < tpch_schema[i].size(); j++){
            num[tpch_schema[i][j]] ++;
        }
    }
    
    managed_mapped_file segment(open_or_create, MMAP_FILENAME, 4 * GB);
    void_allocator void_alloc_inst(segment.get_segment_manager());
    segment.destroy<KVStore>("kv");
    KVStore *kv = segment.construct<KVStore>("kv")(num[EncInt], num[EncFloat], num[EncTimestamp], num[EncString], void_alloc_inst);
    // KVStore kv(num[EncInt], num[EncFloat], num[EncTimestamp], num[EncString], void_alloc_inst);
    for(uint32_t i = 0; i < tpch_tbls.size(); i++){
        std::cout << tpch_tbls[i] << std::endl;
        std::string tbl_path = data_path + "/" + tpch_tbls[i];
        std::string out_path = data_path + "/tmp/" + tpch_tbls[i];
        int resp = transform_tbl(tbl_path, out_path, tpch_schema[i], kv);
    }
    segment.flush();
    // kv.save("kv");
    return 0;
}
