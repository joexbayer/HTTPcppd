//
//  main.cpp
//  HTTPcppd
//
//  Created by Joe Bayer on 22/07/2021.
//

#include <iostream>
#include "http_server.h"

int main(int argc, const char * argv[])
{
    
    {
        /* Config file for server */
        http_config config; /* http_config config(8080, 1, 10, "favicon.ico"); */
        config.port = 8080;
        config.debug = 1;
        config.thread_pool_size = 10;
        config.favicon = "favicon.ico";
        
        /* Create new http server based on config. */
        http_server app(config);
        
        /* Route example without request and response parameters */
        app.route("/", GET, [](){
            return http_server::static_html("index.html");
        });
        
        /* Route example with request and reponse parameters */
        app.route("/json", GET, [](request* req, response* res){
            
            res->set_contentype("application/json");
            
            std::string body = "{name: \"joe\", age: \"" + req->params["age"] + "\"}";
            
            return body;
        });
        
        /* Adding cookie token authentication. */
        app.authentication("secret-token");
        
        /* Start server */
        app.run();
    }
    std::cin.get();
    
    return 0;
}
