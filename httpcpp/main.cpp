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
    /* Scope for http server */
    {
        /* Config file for server */
        http_config config; /* http_config config(PORT, DEBUG, THREADS, FAVICON); */
        config.port = 8080;
        config.debug = 1;
        config.thread_pool_size = 10;
        config.favicon = "favicon.ico";
        
        /* Create new http server based on config. */
        http_server app(config);
        
        /* Route example with static file */
        app.route("/", GET, [](request* req, response* res){
            
            res->send(http_server::static_html("index.html"));
            
        });
        
        /* Route example json response and params */
        app.route("/json?[age, name]", GET, [](request* req, response* res){
            
            res->set_contentype("application/json");
            /* Use params and return value in json. */
            res->send("{name: \"joe\", age: \"" + req->params["age"] + "\"}");
            
        });
        
        /* Route example with redirect */
        app.route("/redirect", GET, [](request* req, response* res){
            
            res->redirect("/");
            
        });
        
        /* POST / GET example */
        app.route("/example", GET, [](request* req, response* res){
            res->send("<form method=\"POST\" action=\"/example\"> \
                          <input placeholder=\"Username\" type=\"text\" name=\"username\"> \
                          <input placeholder=\"Password\" type=\"password\" name=\"password\">  \
                          <button>Log in</button> \
                      </form>");
        });
        app.route("/example?[username, password]", POST, [](request* req, response* res){
            
            res->add_cookie(req->params["username"], req->params["password"]);
            
            res->redirect("/");
        });
        
        /* Adding cookie token authentication. */
        app.authentication("secret-token");
        
        /* Start server */
        app.run();
    }
    std::cin.get();
    
    
    //curl -i -X POST -H 'Content-Type: application/x-www-form-urlencoded' -H 'Cookie: token=secret-token' -d '{"name": "New item", "year": "2009"}' 127.0.0.1:8080/json
    
    return 0;
}
