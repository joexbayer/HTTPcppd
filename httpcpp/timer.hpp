//
//  timer.hpp
//  httpcpp
//
//  Created by Joe Bayer on 03/08/2021.
//

#ifndef timer_hpp
#define timer_hpp

#include <string>
#include <chrono>

#include "logger.hpp"

class timer
{
public:
    timer(const char* name, logger* log)
        : m_name(name), m_stopped(false)
    {
        m_start = std::chrono::high_resolution_clock::now();
        log_p = log;
    }

    ~timer()
    {
        if (!m_stopped)
            stop();
    }

    void stop()
    {
        auto endTimepoint = std::chrono::high_resolution_clock::now();

        long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start).time_since_epoch().count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
        
        auto time = end - start;

        size_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
        
        std::string output = "(Thread: " + std::to_string(threadID) + ") -> " + m_name + " took " + std::to_string(time) + " µs.\n";
        
        log_p->log(output, VERBOSE);

        m_stopped = true;
    }
private:
    const char* m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
    bool m_stopped;
    
    logger* log_p;
};


#endif /* timer_h */
