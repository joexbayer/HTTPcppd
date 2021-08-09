//
//  logger.hpp
//  httpcpp
//
//  Created by Joe Bayer on 06/08/2021.
//

#ifndef logger_h
#define logger_h

#include "syshead.h"

enum log_level {WARNING, ERROR, VERBOSE};

class logger
{
public:
    void count(const std::string& name)
    {
        stats[name] = stats[name]+1;
    }
    void count(const std::string& group, const std::string& name)
    {
        stats_grouped[group][name] = stats_grouped[group][name] + 1;
    }
    
    void log_level(enum log_level l)
    {
        current_level = l;
    }
    
    void print()
    {
        for(std::map<std::string,int>::iterator it = stats.begin(); it != stats.end(); ++it) {
          std::cout << it->first << ": " << it->second << std::endl;
        }
    }
    
    std::string to_json(struct http_config* config)
    {
        std::string json = "{\n\t\"Stats\": [{\n";
        
        for(std::map <std::string, std::map <std::string, int>>::iterator it_group = stats_grouped.begin(); it_group != stats_grouped.end(); ++it_group) {
            
            std::string json_group_title = "\t\t\""+ it_group->first +"\": [{\n";
            
            for(std::map<std::string,int>::iterator it = it_group->second.begin(); it != it_group->second.end(); ++it) {
                
                std::string json_inner = "\t\t\t\t\""+  it->first+"\": \""+std::to_string(it->second) +"\",\n";
                json_group_title.append(json_inner);
            }
            std::string json_inner = "\t\t\t\t\"Type\": \"Client\"\n";
            json_group_title.append(json_inner);
            
            std::string json_group_title_end = "\t\t}],\n";
            json_group_title.append(json_group_title_end);
            json.append(json_group_title);
        }
        
        for(std::map<std::string,int>::iterator it = stats.begin(); it != stats.end(); ++it) {
            
            std::string json_inner = "\t\t\""+  it->first+"\": \""+std::to_string(it->second) +"\",\n";
            json.append(json_inner);
        }
        std::string json_inner = "\t\t\"Status\" : \"running\", \n";
        json_inner.append("\t\t\"Using threads\" : \""+std::to_string(active_threads)+"\", \n");
        json_inner.append("\t\t\"Total threads\" : \""+std::to_string(total_threads)+"\" \n");
        json.append(json_inner);
        
        json.append("\t}]\n}");
        
        return json;
    }
    
    void log(const std::string& message, enum log_level level)
    {
        switch (level) {
            case VERBOSE:
                if(current_level >= level)
                    std::cout << "[VERBOSE] "+message;
                break;
            case WARNING:
                count("Warnings");
                if(current_level >= level)
                    std::cout << "[WARNING] "+message;
                break;
                
            case ERROR:
                count("Errors");
                if(current_level >= level)
                    std::cout << "[ERROR] "+message;
                break;
                
            default:
                break;
        }
        
    }
    
private:
    std::map <std::string, int> stats;
    std::map <std::string, std::map <std::string, int>> stats_grouped;
    
    enum log_level current_level;
};

#include "http_server.h"
#include "thread_pool.hpp"

#endif /* logger_h */
