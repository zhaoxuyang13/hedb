#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <cstdlib> //std::system
#include <iostream>
#include <scoped_allocator>
#include <boost/interprocess/managed_mapped_file.hpp>
// #include <kv-store.hpp>

// using std::vector, std::string, std::basic_string;
using namespace boost::interprocess;
//Define an STL compatible allocator of ints that allocates from the managed_shared_memory.
//This allocator will allow placing containers in the segment
typedef allocator<int, managed_shared_memory::segment_manager>  ShmemAllocator;
typedef std::scoped_allocator_adaptor<allocator<vector<int , ShmemAllocator>, managed_shared_memory::segment_manager>>  ShmemAllocator2D;

//Alias a vector that uses the previous STL-like allocator so that allocates
//its values from the segment
typedef vector<int, ShmemAllocator> MyVector;
typedef vector<MyVector, ShmemAllocator2D> My2DVector;

typedef allocator<char, managed_shared_memory::segment_manager> CharAllocator;
typedef basic_string<char, std::char_traits<char>, CharAllocator> MyShmString;
typedef std::scoped_allocator_adaptor<allocator<MyShmString, managed_shared_memory::segment_manager>> StringAllocator;
typedef vector<MyShmString, StringAllocator> MyShmStringVector;

typedef std::scoped_allocator_adaptor<allocator<MyShmStringVector, managed_shared_memory::segment_manager>> StringAllocator2D;
typedef vector<MyShmStringVector, StringAllocator2D> MyShmString2DVector;

const uint64_t MB = 1024 * 1024;
const uint64_t GB = 1024 * MB;

template<typename T>
using segment_allocator = allocator<T, managed_shared_memory::segment_manager>;
using void_allocator = allocator<void, managed_shared_memory::segment_manager>;

// template<typename Vt, typename Alloc>
// class VectorMap{
//     using VecAlloc = segment_allocator<Vt>;
//     using ShmVector = vector<Vt, VecAlloc>;
// public:
//     ShmVector _vec;
    
//     VectorMap(const void_allocator& alloc) :
//         _vec(alloc)
//           {}

//     uint32_t push_back(Vt& value){
//         _vec.push_back(value);
//         return _vec.size() - 1;
//     }
    
//     Vt& find(uint32_t key){
//         return _vec[key];
//     }
// };
// using IntVecMap = VectorMap<int>;

using segment_manager_t = managed_shared_memory::segment_manager;
class KVStore{

    using int_allocator = allocator<int, segment_manager_t>;
    using IntVecMap = vector<int, int_allocator> ;
    // using IntVecMapAlloc = allocator<IntVecMap, segment_manager_t> ;
    using IntVecMapAlloc = std::scoped_allocator_adaptor<allocator<IntVecMap, segment_manager_t>> ;
    using IntVecMaps = vector<IntVecMap, IntVecMapAlloc>;

    typedef allocator<char, managed_shared_memory::segment_manager> CharAllocator;
    typedef basic_string<char, std::char_traits<char>, CharAllocator> MyShmString;
    typedef std::scoped_allocator_adaptor<allocator<MyShmString, managed_shared_memory::segment_manager>> StringAllocator;
    typedef vector<MyShmString, StringAllocator> MyShmStringVector;

    typedef std::scoped_allocator_adaptor<allocator<MyShmStringVector, managed_shared_memory::segment_manager>> StringAllocator2D;
    typedef vector<MyShmStringVector, StringAllocator2D> MyShmString2DVector;

    IntVecMaps _intVecMaps;
    MyShmString2DVector _stringVecMaps;
public:    
    KVStore(const void_allocator& alloc) :
          _intVecMaps(alloc), _stringVecMaps(alloc)
          {}
    uint32_t push_back(uint32_t map_id, int value){
        int cur_size = _intVecMaps.size();
        while(map_id >= cur_size){
            _intVecMaps.emplace_back();
            cur_size++;
        }
        // if(map_id >= cur_size){
        //     _intVecMaps.resize(map_id + 1);
        // }
        _intVecMaps[map_id].push_back(value);
        return _intVecMaps[map_id].size() - 1;
    }
    int find_int(uint32_t map_id, uint32_t key){
        return _intVecMaps[map_id][key];
    }
    uint32_t push_back(uint32_t map_id, const char* value){
        int cur_size = _stringVecMaps.size();
        // if(map_id >= cur_size){
        //     _stringVecMaps.resize(map_id + 1);
        // }
        while(map_id >= cur_size){
            _stringVecMaps.emplace_back();
            cur_size++;
        }
        _stringVecMaps[map_id].emplace_back(value);
        return _stringVecMaps[map_id].size() - 1;
    }
    const char* find_string(uint32_t map_id, uint32_t key){
        return _stringVecMaps[map_id][key].c_str();
    }
};



//Main function. For parent process argc == 1, for child process argc == 2
int main(int argc, char *argv[])
{
     
  if(argc == 1){ //Parent process
    std::cout << "parent" << std::endl;
    //Remove shared memory on construction and destruction
    
    //Create a new segment with given name and size
    managed_mapped_file segment(open_or_create, "MySharedMemory", 100 * MB);

    segment.destroy<KVStore>("kvStore");
    void_allocator void_alloc_inst(segment.get_segment_manager());

    KVStore *kvStore = segment.construct<KVStore>("kvStore")(void_alloc_inst);
    for (int i = 0; i < 10;++i) {
        for(int j = 0; j < i; j ++){
            auto id = kvStore->push_back(i, j);
            auto id2 = kvStore->push_back(i, ("str " + std::to_string(j)).c_str());
            std::cout << id << ":" << id2 << ",";

        }
        std::cout << std::endl;
    }


    // const ShmemAllocator vec_int_alloc (segment.get_segment_manager());
    // const ShmemAllocator2D vec_int_alloc2d (segment.get_segment_manager());
    // MyVector *vec_int = segment.construct<MyVector>("intVec")(vec_int_alloc);
    // My2DVector *vec_int2d = segment.construct<My2DVector>("intVec2d")(vec_int_alloc2d);



    // Initialize shared memory STL-compatible allocator
    // const CharAllocator char_alloc_inst (segment.get_segment_manager());
    // const StringAllocator alloc_inst (segment.get_segment_manager());
    // const StringAllocator2D alloc_inst2d (segment.get_segment_manager());
    // //Construct a vector named "MyVector" in shared memory with argument alloc_inst
    // // MyShmStringVector *myvector = segment.construct<MyShmStringVector>("MyVector")(alloc_inst);
    // MyShmString2DVector *myvector = segment.construct<MyShmString2DVector>("MyVector")(alloc_inst2d);

    // for (int i = 0; i < 10;++i) {
    //     myvector->emplace_back();
    //     for (int j = 0; j < 10; ++j) {
    //         myvector->back().emplace_back(("hello" + std::to_string(i)).c_str());
    //     }
    // }
    //Launch child process
    std::string s(argv[0]); 
    s += " child ";
    if(0 != std::system(s.c_str()))
      return 1;

    //Check child has destroyed the vector
    if(segment.find<MyShmString2DVector>("MyVector").first)
        return 1;
    }else{ //Child process
        //Open the managed segment
        std::cout << "child" << std::endl;
        managed_mapped_file segment(open_only, "MySharedMemory");

        KVStore *kvStore = segment.find<KVStore>("kvStore").first;
        
        for(int i = 0; i < 10; ++i){
            for(int j = 0; j < i ; ++j){
                std::cout << kvStore->find_int(i, j) << ",";
                std::cout << kvStore->find_string(i, j) << ",";
            }
            std::cout << std::endl;
        }

        //Find the vector using the c-string name
        // MyShmString2DVector *myvector = segment.find<MyShmString2DVector>("MyVector").first;

        //Use vector in reverse order
        // for (auto &vec : *myvector){
        //     // std::cout << vec << std::endl;
        //     for (auto i:vec)
        //         std::cout << i << ", ";
        //     std::cout << std::endl;
        // }
        //When done, destroy the vector from the segment
        // segment.destroy<KVStore>("kvStore");
    }

  return 0;
};
