#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "vectormap.hpp"
#include "kv-store.hpp"


using std::string, std::vector;
string data_path = "./benchmark/tools";
// string out_path = ".benchmark/data";


enum enc_type {
    None,
    EncInt,
    EncFloat,
    EncString,
    EncTimestamp,
};

const vector<string> tpch_tbls = {
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
const vector<vector<enc_type>> tpch_schema = {
    { None, EncString, None, EncString },
    { None, EncString, EncString },
    { None, EncString, EncString, EncString, EncString, EncInt, EncString, EncFloat, EncString },
    { None, EncString, EncString, None, EncString, EncFloat, EncString},
    { None, None, EncFloat, EncFloat, EncString},
    { None, EncString, EncString, None, EncString, EncFloat, EncString, EncString},
    { None, None, EncString, EncFloat, EncTimestamp, EncString, EncString, EncInt, EncString},
    { None, None, None, None, EncFloat, EncFloat, EncFloat, EncFloat, EncString, EncString, EncTimestamp, EncTimestamp, EncTimestamp, EncString, EncString, EncString}
};

// vector<vector<string>> read_tbl(string tbl_path){
//     vector<vector<string>> tbl;
//     std::ifstream infile(tbl_path);
//     string line;
//     while (std::getline(infile, line)){
//         vector<string> row;
//         std::istringstream iss(line);
//         string field;
//         while (iss >> field){
//             row.push_back(field);
//         }
//         tbl.push_back(row);
//     }
//     return tbl;
// }

uint32_t cnts[5] = {0, 0, 0, 0, 0};
uint64_t combine(uint32_t a, uint32_t b){
    uint64_t c = a;
    return c << 32 | b;
}

int transform_tbl(const string tbl_path, const string out_path, const vector<enc_type>& table_schema, KVStore& kv){
    std::ifstream infile(tbl_path);
    std::ofstream outfile(out_path);
    string line;

    std::cout << "transforming " << tbl_path << std::endl;
    // std::cout << "mapids begins at " << int_cnt << " " << float_cnt << " " << string_cnt << " " << timestamp_cnt << std::endl;
    vector<uint32_t> map_ids(table_schema.size());
    
    for(uint32_t i = 0; i < table_schema.size(); i++){
        cnts[table_schema[i]] ++;
        map_ids[i] = cnts[table_schema[i]];
    }
    while(std::getline(infile, line)){
        vector<string> row;
        // create a input stream on line, with delimiter '|'
        std::istringstream iss(line);
        string field;
        string out_line;
        while(std::getline(iss, field, '|')){
            row.push_back(field);
        }
        for(uint32_t i = 0; i < row.size(); i++){
            string field = row[i];
            uint32_t mapid = map_ids[i];
            switch (table_schema[i])
            {
            case EncInt:{
                int value = std::stoi(field);
                uint32_t key = kv.push_back(mapid, value);
                out_line += std::to_string(combine(mapid, key)) + "|";
                break;
            }
            case EncFloat:{
                float value = std::stof(field);
                uint32_t key = kv.push_back(mapid, value);
                out_line += std::to_string(combine(mapid, key)) + "|";
                break;
            }
            case EncString:{
                uint32_t key = kv.push_back(mapid, field.c_str());
                out_line += std::to_string(combine(mapid, key)) + "|";
                break;
            }
            case EncTimestamp:{
                TIMESTAMP value = std::stol(field);
                uint32_t key = kv.push_back(mapid,  value);
                out_line += std::to_string(combine(mapid, key)) + "|";
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


int main(){
    // read tpch_tbls in data_path, use filesystem lib to read files.

    /* count the number of types in schema*/
    uint32_t num[5] = {0};
    for(uint32_t i = 0; i < tpch_schema.size(); i++){
        for(uint32_t j = 0; j < tpch_schema[i].size(); j++){
            num[tpch_schema[i][j]] ++;
        }
    }
    KVStore kv(num[EncInt], num[EncFloat], num[EncString], num[EncTimestamp]);
    for(uint32_t i = 0; i < tpch_tbls.size(); i++){
        std::cout << tpch_tbls[i] << std::endl;
        string tbl_path = data_path + "/" + tpch_tbls[i];
        string out_path = data_path + "/tmp/" + tpch_tbls[i];
        int resp = transform_tbl(tbl_path, out_path, tpch_schema[i], kv);
    }
    kv.save("kv");
    return 0;
}
