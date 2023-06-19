// #include <iostream>
// #include "kv-store.hpp"
// // #include <boost/interprocess/managed_shared_memory.hpp>


// int main(){

//     KVStore kv;
//     kv.load("kv");



// }
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <cstdlib> //std::system
#include <iostream>
#include <scoped_allocator>


using namespace boost::interprocess;
//Define an STL compatible allocator of ints that allocates from the managed_shared_memory.
//This allocator will allow placing containers in the segment
// typedef allocator<int, managed_shared_memory::segment_manager>  ShmemAllocator;
// typedef std::scoped_allocator_adaptor<allocator<vector<int , ShmemAllocator>, managed_shared_memory::segment_manager>>  ShmemAllocator2D;

//Alias a vector that uses the previous STL-like allocator so that allocates
//its values from the segment
// typedef vector<int, ShmemAllocator> MyVector;
// typedef vector<MyVector, ShmemAllocator2D> My2DVector;

typedef allocator<char, managed_shared_memory::segment_manager> CharAllocator;
typedef basic_string<char, std::char_traits<char>, CharAllocator> MyShmString;
typedef allocator<MyShmString, managed_shared_memory::segment_manager> StringAllocator;
typedef vector<MyShmString, StringAllocator> MyShmStringVector;

// typedef for 2d vector of string 
typedef std::scoped_allocator_adaptor<allocator<MyShmStringVector, managed_shared_memory::segment_manager>> StringAllocator2D;
typedef vector<MyShmStringVector, StringAllocator2D> MyShmString2DVector;

const uint64_t MB = 1024 * 1024;
const uint64_t GB = 1024 * MB;

//Main function. For parent process argc == 1, for child process argc == 2
int main(int argc, char *argv[])
{
  if(argc == 1){ //Parent process
    std::cout << "parent" << std::endl;
    //Remove shared memory on construction and destruction
    struct shm_remove
    {
      shm_remove() { shared_memory_object::remove("MySharedMemory"); }
      ~shm_remove(){ shared_memory_object::remove("MySharedMemory"); }
    } remover;
    
    //Create a new segment with given name and size
    managed_shared_memory segment(create_only, "MySharedMemory", 2 * GB);

    //Initialize shared memory STL-compatible allocator
    const CharAllocator char_alloc_inst (segment.get_segment_manager());
    const StringAllocator alloc_inst (segment.get_segment_manager());
    const StringAllocator2D alloc_inst2d (segment.get_segment_manager());
    //Construct a vector named "MyVector" in shared memory with argument alloc_inst
    // MyShmStringVector *myvector = segment.construct<MyShmStringVector>("MyVector")(alloc_inst);
    MyShmString2DVector *myvector = segment.construct<MyShmString2DVector>("MyVector")(alloc_inst2d);

    for (int i = 0; i < 10;++i) {
        myvector->emplace_back();
        for (int j = 0; j < 10; ++j) {
            myvector->back().emplace_back(("hello" + std::to_string(i)).c_str(), char_alloc_inst);
        }
    }
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
        managed_shared_memory segment(open_only, "MySharedMemory");

        //Find the vector using the c-string name
        MyShmString2DVector *myvector = segment.find<MyShmString2DVector>("MyVector").first;

        //Use vector in reverse order
        for (auto &vec : *myvector){
            // std::cout << vec << std::endl;
            for (auto i:vec)
                std::cout << i << ", ";
            std::cout << std::endl;
        }
        //When done, destroy the vector from the segment
        segment.destroy<MyShmString2DVector>("MyVector");
    }

  return 0;
};
