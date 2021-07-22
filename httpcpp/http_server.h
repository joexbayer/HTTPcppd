//
//  http_server.h
//  httpcpp
//
//  Created by Joe Bayer on 22/07/2021.
//

#ifndef http_server_h
#define http_server_h

#define MAX_ROUTES 50
#define MAX_CONNECTIONS 10

#define HTTP_BUFFER_SIZE 8192 // 8KB

//#define CONTEXT getCurrentContext

#include "syshead.h"

typedef enum
{
    GET,
    POST,
    HEAD

} method_t ;

// CONNECTION SECTION

struct http_context
{
    /* User (Identity) TODO */
    
    char* cookies;
    char* host;
    char* connection;
    
};

struct http_connection
{
    struct http_context* context;
    char* header;
    char* content;
};

// SERVER SECTION

struct http_config /* Config struct, needs to be filled manually or through a config file */
{
    int port;
    int debug;
};

struct http_route
{
    std::string route;
    method_t method;
    void (*function)();
};

struct file_s
{
    char* content;
    off_t size;
};

class http_server /* HTTP server class */
{
    
public:
    http_server(struct http_config user_config );
    
    int add_route(const std::string& route_name, const method_t& method_def, void (*user_function)() );
    
    void run(); /* Main event handle loop, blocking. */
    
    
private:
    
    void handle_route(const std::string& route, const method_t& method);
    void read_handle_loop();
    int create_tcp_socket(uint32_t port);
    void cleanup();
    
    struct file_s* open_file(const std::string& file);
    
    struct http_config* config;
    
    struct http_route* routes[MAX_ROUTES];
    int http_route_counter;
    
    int server_socket;
    struct sockaddr_in* server_addr;

    std::string main_header;
};



#endif /* http_server_h */
