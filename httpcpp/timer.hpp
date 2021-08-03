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

class timer
{
public:
    timer(const char* name)
        : m_name(name), m_stopped(false)
    {
        m_start = std::chrono::high_resolution_clock::now();
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
        
        std::string output = "[DEBUG] (Thread: " + std::to_string(threadID) + ") -> " + m_name + " took " + std::to_string(time) + " µs.";
        
        std::cout << output << std::endl;

        m_stopped = true;
    }
private:
    const char* m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
    bool m_stopped;
};


#endif /* timer_h */
