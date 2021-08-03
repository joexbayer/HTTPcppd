//
//  thread_pool.hpp
//  httpcpp
//
//  Created by Joe Bayer on 26/07/2021.
//

#ifndef thread_pool_hpp
#define thread_pool_hpp

#include "syshead.h"
#include <queue>
#include <vector>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <unistd.h>

#define THREAD_POOL_SIZE 100

struct thread_job
{
    struct http_connection* connection;
    void* context;
};

class thread_pool
{
public:
    thread_pool(int pool_size);
    ~thread_pool();
    
    void add_job(struct thread_job* job);

private:
    
    void stop_threads();
    
    int size;
    
    std::vector<std::thread> threads;
    
    
};

#include "http_server.h"

#endif /* thread_pool_hpp */
