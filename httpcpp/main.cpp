//
//  main.cpp
//  httpcpp
//
//  Created by Joe Bayer on 22/07/2021.
//

#include <iostream>
#include "http_server.h"


void home(){
    std::cout << "Hello, World!\n";
}

int main(int argc, const char * argv[]) {

    struct http_config config;
    config.port = 8080;
    config.debug = 1;
    
    
    http_server server(config);
    
    server.add_route("/", GET, &home);
    
    server.run();
    
    return 0;
}
