#pragma once 
#include <unistd.h> // pid
#include <string>
#include <fcntl.h> //open
#include <sys/mman.h> //mmap


class Recorder{
    int write_fd;
    std::string filename;
    /* variables to operate write_fd */
    const int DATA_LENGTH = 16 * 1024 * 1024UL;
    unsigned long file_length;
    unsigned long file_cursor;
    char *write_addr;
    /* return mmaped write buffer from write_fd,
    extened the file if file size is not large enough */    
    char *get_write_buffer(unsigned long length);

    Recorder(std::string filename = ""): 
     file_length(0), file_cursor(0), write_addr(nullptr), 
     write_fd(0), filename(filename){}
    
    ~Recorder(){
        close(write_fd);   
    }

public:
    static Recorder& getInstance(const std::string& filename = ""){
        static Recorder recorder(filename);
        return recorder;
    }
    Recorder(const Recorder& recorder) = delete;
    Recorder & operator=(const Recorder& recorder) = delete;

    void record(void *request_buffer);
};


/* use different Recorder subclass to represent different rr strategy*/





