//
//  main.cpp
//  HTTPcppd
//
//  Created by Joe Bayer on 22/07/2021.
//

#include <iostream>
#include "http_server.h"

int main()
{
    /* Scope for http server */
    {
        /* Config file for server */
        http_config config; /* http_config config(PORT, DEBUG, THREADS, FAVICON, LOG); */
        config.port = 8080;
        config.thread_pool_size = 99;
        config.favicon = "favicon.ico";
        config.log_level = VERBOSE;
        
        /* Create new http server based on config. */
        http_server app(config);
        
        /* GET Route example with static file */
        app.route("/", GET, [](request* req, response* res){
            
            res->send(http_server::static_html("index.html"));
            
        });
        
        /* GET Route example with pre defined function */
        app.route("/routes", GET, [](request* req, response* res){
            
            res->send(req->context->send_router_view());
            
        });
        
        /* GET Route example json response and params */
        app.route("/json?[age]", GET, [](request* req, response* res){
            
            res->set_contentype("application/json");
            /* Use params and headers and return value in json. */
            res->send("{\"Host\": \""+req->headers["Host"]+"\",\"IP\": \""+req->client_ip+"\", \"age\": \"" + req->params["age"] + "\"}");
            
        });
        
        /* GET Route example json stats resposne */
        app.route("/stats", GET, [](request* req, response* res){
            
            res->set_contentype("application/json");
            /* Return static stats method for http_server */
            res->send(req->context->stats());
            
        });
        
        /* POST example with params and redirect */
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
    
    return 0;
}
