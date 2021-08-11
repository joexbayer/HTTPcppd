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
#include "timer.hpp"
#include "logger.hpp"

#define MAX_ROUTES 50
#define MAX_CONNECTIONS 10
#define HTTP_BUFFER_SIZE 8192 // 8KB

// ********** HTTP pipeline stages **********
enum http_pipeline_stages {
    
    HTTP_ACCEPTED_CLIENT,
    HTTP_ACCEPTED_REQUEST,
    HTTP_PARSE_HEADER,
    HTTP_PARSE_ROUTER,
    HTTP_PARSE_METHOD,
    HTTP_HANDLE_ROUTE,
    HTTP_SEND_RESPONSE
    
};

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
    USER_DEFINED
};


typedef struct response /* http request given to user defined function */
{
    std::string content_type;
    std::string set_cookies;
    std::string response_data;
    
    int status;
    
    std::string redirect_url;
    int redirect_;
    
    response()
    {
        content_type = "Content-Type: text/html";
        status = 200;
        redirect_ = 0;
        set_cookies = "Set-Cookie: ";
    }
    
    void send(std::string content);
    void redirect(std::string url);
    void set_contentype(const std::string& type);
    void add_cookie(std::string& cookie_name, std::string& cookie_value);
    
} response;


// CONNECTION SECTION
typedef struct http_context request;

struct http_cache
{
    struct file_s* file;
    std::string filename;
    
    struct http_cache* next;
    
    static struct file_s* find(const std::string& filename, struct http_cache* cach);
    static void add(struct file_s* u_file, const std::string& filename);
    static void add_recursive(struct file_s* u_file, const std::string& filename, struct http_cache* next);
};

static struct http_cache* start_cache = nullptr;

struct http_connection
{
    struct http_context* context;
    std::string header;
    std::string content;
    std::string router;
    std::string params_string;
    
    response* res;
    
    enum http_pipeline_stages stage;
    
    int client_socket;
    struct sockaddr_in* client_addr;
    
};

// SERVER SECTION

typedef struct http_config /* Config struct, needs to be filled manually or through a config file */
{
    int port;
    int thread_pool_size;
    std::string favicon;
    enum log_level log_level;
    
    http_config(int u_port, int u_threads,std::string u_favicon, enum log_level level);
    http_config();
    
} http_config;


struct file_s
{
    char* content;
    off_t size;
};

class http_server /* HTTP server class */
{
    
public:
    http_server(struct http_config user_config );
    http_server(); /* Default constructor, logging off, 10 threads, port 8080  */
    ~http_server();
    
    int route(const std::string& route_name, const method_et& method_def, void (*user_function)(request* req, response* res) );
    
    void authentication(std::string token);
    void run(); /* Main event handle loop, blocking. */
    void handle_thread_connection(struct http_connection* connection);
    
    static std::string static_html(std::string filname);
    
    std::string stats();
    std::string send_router_view();
    
public:
    struct http_config* config;
    
private:
    void read_handle_loop();
    int create_tcp_socket(uint32_t port);
    void cleanup();
    
    void setup();
    
    //pipeline
    void pipeline_handler(struct http_connection* connection);
    
    void parse_connection_header(struct http_connection* connection);
    void parse_method_route(struct http_connection* connection);
    void parse_router(struct http_connection* connection);
    bool authenticate(struct http_connection* connection);
    std::string handle_route(const std::string& route, const method_et& method, struct http_connection* connection);
    void send_response(struct http_connection* connection);
    void send_error(int error_code, struct http_connection* connection);
    void send_redirect(std::string url, struct http_connection* connection);
    void parse_post_params(struct http_connection* connection);
    void parse_params(struct http_connection* connection, std::string& raw_params);
    
    void close_connection(struct http_connection* connection); /* Close client connection. */
    
    static struct file_s* open_file(std::string& file);
    
    struct http_connection* new_http_client();
    method_et get_method(const char* method_string);
    void assign_worker(struct http_connection* connection);

    void send_favicon(struct http_connection* connection);

private:
    struct http_route* routes[MAX_ROUTES];
    int http_route_counter;
    
    std::string favicon;
    
    int server_socket;
    struct sockaddr_in* server_addr;
    
    int use_authentication;
    std::string token;
    
    thread_pool worker_pool;
    logger log;
    
    std::string main_header;
};

typedef struct http_context
{
    /* User (Identity) TODO */
    
    std::string cookies;
    std::string connection;
    
    http_server* context;
    
    std::map <std::string, std::string> params;
    std::map <std::string, std::string> headers;
    
    bool authorized;
    bool keep_alive;
    
    std::string client_ip;
    
    method_et method;
    std::string route;
} request;

struct http_route
{
    std::string route;
    method_et method;
    
    std::string params;
    int has_params;
    
    enum function_option option;
    void (*function)(request* req, response* res); // WITH PARAMETER
};



#endif /* http_server_h */
