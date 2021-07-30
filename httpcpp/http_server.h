//
//  http_server.h
//  httpcpp
//
//  Created by Joe Bayer on 22/07/2021.
//

#ifndef http_server_h
#define http_server_h

#include "syshead.h"
#include "thread_pool.hpp"

#define MAX_ROUTES 50
#define MAX_CONNECTIONS 10
#define HTTP_BUFFER_SIZE 8192 // 8KB

// ********** HTTP pipeline stages **********
#define HTTP_ACCEPTED_CLIENT 0x01
#define HTTP_ACCEPTED_REQUEST 0x02
#define HTTP_ASSIGNED_WORKER 0x03


// ********** HTTP error codes **********
#define BAD_REQUEST 400
#define UNAUTHORIZED 401
#define PAYMENT_REQUIRED 402
#define FORBIDDEN 403
#define NOT_FOUND 404

typedef enum /* Enum for HTTP access method */
{
    GET,
    POST,
    HEAD,
    UNDEFINED

} method_et ;
static std::string method_names[4] = {"GET", "POST", "HEAD", "UNDEFINED"};

enum function_option /* Function option for variable parameters */
{
    WITH_PARAMETER,
    WITHOUT_PARAMETER,
    INTERN
};


typedef struct request /* http request given to user defined function */
{
    std::string cookies;
    std::string content;
    std::string route;
    std::string connection;
    
    struct http_context* context;
    
} request;


typedef struct response /* http request given to user defined function */
{
    std::string content_type;
    std::string set_cookies;
    
    void set_contentype(const std::string& type);
    void add_cookie(std::string& cookie_name, std::string& cookie_value);
    
} response;


// CONNECTION SECTION
struct http_context
{
    /* User (Identity) TODO */
    
    std::string cookies;
    std::string host;
    std::string connection;
    
    bool authorized;
    
    method_et method;
    std::string route;
};

struct http_connection
{
    struct http_context* context;
    std::string header;
    std::string content;
    std::string router;
    
    // user response
    response* res;
    
    // socket
    int client_socket;
    struct sockaddr_in* client_addr;
    
};

// SERVER SECTION

struct http_config /* Config struct, needs to be filled manually or through a config file */
{
    int port;
    int debug;
    int thread_pool_size;
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
    ~http_server();
    
    int add_route(const std::string& route_name, const method_et& method_def, std::string (*user_function)(request* req, response* res) );
    int add_route(const std::string& route_name, const method_et& method_def, std::string (*user_function)());
    int add_route(const std::string& route_name, const method_et& method_def, std::string (http_server::*user_function)());
    
    void add_authorization(std::string token);
    void run(); /* Main event handle loop, blocking. */
    void handle_thread_connection(struct http_connection* connection);
    
    static std::string static_html(std::string filname);
    
private:
    void read_handle_loop();
    int create_tcp_socket(uint32_t port);
    void cleanup();
    
    //pipeline
    void parse_connection_header(struct http_connection* connection);
    void parse_method_route(struct http_connection* connection);
    bool authorize(struct http_connection* connection);
    std::string handle_route(const std::string& route, const method_et& method, struct http_connection* connection);
    void send_response(struct http_connection* connection, std::string& response);
    void send_error(int error_code, struct http_connection* connection);
    
    static struct file_s* open_file(std::string& file);
    
    struct http_connection* new_http_client();
    method_et get_method(const char* method_string);
    void assign_worker(struct http_connection* connection);
    std::string send_router_view();

private:
    struct http_config* config;
    struct http_route* routes[MAX_ROUTES];
    int http_route_counter;
    
    int server_socket;
    struct sockaddr_in* server_addr;
    
    int use_authorization;
    std::string token;
    
    thread_pool worker_pool;
    
    std::string main_header;
};

struct http_route
{
    std::string route;
    method_et method;
    
    enum function_option option;
    std::string (*function)(request* req, response* res); // WITH PARAMETER
    std::string (*function_n)(); // WIHTOUT PARAMETER
    std::string (http_server::*function_intern)(); // INTERNAL FUNCTION
};

#endif /* http_server_h */
