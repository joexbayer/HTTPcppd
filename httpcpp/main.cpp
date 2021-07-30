//
//  main.cpp
//  httpcpp
//
//  Created by Joe Bayer on 22/07/2021.
//

#include <iostream>
#include "http_server.h"

std::string home(){
    
    return http_server::static_html("index.html");
}

std::string json(){
    
    std::string response = "{name: \"joe\", age: \"21\"}";
    
    return response;
}


int main(int argc, const char * argv[]) {

    struct http_config config;
    config.port = 8080;
    config.debug = 1;
    config.thread_pool_size = 10;
    
    http_server server(config);
    server.add_route("/", GET, &home);
    server.add_route("/json", GET, &json);
    server.add_authorization("secret-token");
    
    server.run();
    
    return 0;
}
