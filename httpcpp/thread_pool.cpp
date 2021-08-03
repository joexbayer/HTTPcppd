//
//  thread_pool.cpp
//  httpcpp
//
//  Created by Joe Bayer on 26/07/2021.
//

#include "thread_pool.hpp"

// threading basic setup
static std::mutex queue_mutex;

static std::mutex active_threads_mutex;

int active_threads = 0;
int total_threads = 0;

static std::condition_variable condition;
static std::queue<thread_job*> job_queue;

static bool run = true;

static std::vector<std::thread> threads;

static void thread_loop()
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [](){return run == false || !job_queue.empty();});
            
            if(!run)
                break;
            
            struct thread_job* job = job_queue.front();
            job_queue.pop();
            
            queue_mutex.unlock(); /* Unlock queue, TODO: put in scope? */
    
            active_threads_mutex.lock();
            active_threads++;
            active_threads_mutex.unlock();
            
            http_server* server_context = static_cast<http_server*>(job->context);
            
            if(server_context->config->debug)
            {
                size_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
                std::string out = "[DEBUG] Assigned Job to thread ("+std::to_string(threadID)+"). Using "+std::to_string(active_threads) + "/" + std::to_string(total_threads) +" threads.";
                std::cout << out << std::endl;
            }
        
            server_context->handle_thread_connection(job->connection);
        
            active_threads_mutex.lock();
            active_threads--;
            active_threads_mutex.unlock();
            
            delete job;
            
            if(server_context->config->debug)
            {
                size_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
                std::string out = "[DEBUG] Job finished on thread ("+std::to_string(threadID)+"). Using "+std::to_string(active_threads) + "/" + std::to_string(total_threads) +".";
                std::cout << out << std::endl;
            }
        }
    }
    
    std::cout << std::this_thread::get_id() << std::endl;
}



/*
 * Function:  thread_pool
 * --------------------
 * Namespace thread_pool
 *
 *  Constructor for the thread pool
 *
 *  pool_size: size of current thread pool
 *
 *  void:
 *
 */
thread_pool::thread_pool(int pool_size)
{
    
    if(pool_size > THREAD_POOL_SIZE)
    {
        pool_size = THREAD_POOL_SIZE;
    }
    size = pool_size;
    total_threads = pool_size;
    
    for (int i = 0; i < pool_size; i++)
    {
        
        threads.push_back(std::thread(thread_loop));
    }
}

thread_pool::~thread_pool()
{
    stop_threads();
}

void thread_pool::stop_threads()
{
    run = false;
    condition.notify_all();
    
    for (std::thread &j : threads)
    {
        j.join();
    }
}


/*
 * Function:  add_job
 * --------------------
 * Namespace thread_pool
 *
 *  Adds a job to the current queue and notifies a waiting thread.
 *
 *  job: thread_job object to queue.
 *
 *  void:
 *
 */
void thread_pool::add_job(struct thread_job* job)
{
    
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        job_queue.push(job);
    }
    
    condition.notify_one();
    
}

