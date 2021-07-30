//
//  http_server.cpp
//  httpcpp
//
//  Created by Joe Bayer on 22/07/2021.
//

#include "http_server.h"

static bool run = true;

void on_exit(){
    run = false;
}



/*
 * Function:  send_router_view
 * --------------------
 * Namespace http_server
 *
 *  Sends a list over all routes currently stored, cant be overwritten
 *
 *
 *  std::string: response
 *
 */
std::string http_server::send_router_view()
{
    std::string response = "<HTML><body style='text-align:center;'><div style='width:30%;margin:auto;'>  <h2>HTTPccpd route view.</h2><h3>Routes that are currently available: </h3><ul style='text-align:left;'>";
    
    for(int i = 0; i < http_route_counter; i++)
    {
        response.append("<li><a href='"+routes[i]->route+"'> "+method_names[routes[i]->method]+": -> "+routes[i]->route+"</a></li>");
    }
    response.append("</ul></div></body></html>");
    
    return response;
}

/*
 * Function:  add_contentype
 * --------------------
 * Namespace response
 *
 *  Set contentype to given type
 *
 *  type: content type
 *
 *  void:
 *
 */
void response::set_contentype(const std::string& type)
{
    std::string new_content_type = "Content-Type: "+type;
    content_type = new_content_type;
}

/*
 * Function:  add_cookie
 * --------------------
 * Namespace response
 *
 *  Add a cookie with name and values to response
 *
 *  cookie_name: name of the cookie
 *  cookie_value: value of the cookie
 *
 *  void:
 *
 */
void response::add_cookie(std::string& cookie_name, std::string& cookie_value)
{
    std::string new_cookie = cookie_name +"="+ cookie_value+";";
    set_cookies.append(new_cookie);
}

/*
 * Function:  http_server
 * --------------------
 * Namespace http_server
 *
 *  Constructor for the http server
 *    Needs a http config
 *
 *  user_config: struct of http_config
 *
 *  void:
 *
 */
http_server::http_server(struct http_config user_config) : worker_pool(user_config.thread_pool_size)
{
    struct http_config* config_m = new http_config;
    config_m->port = user_config.port;
    config_m->debug = user_config.debug;
    config_m->thread_pool_size = user_config.thread_pool_size;
    
    main_header = "Server: HTTPcppd (Unix)\nContent-Security-Policy: script-src 'unsafe-inline';\n";
    
    config = config_m;
    if(config->debug)
        std::cout << "Config file has been added to the server.\n";
    
    server_socket = create_tcp_socket(config->port);
    if(server_socket == -1)
        std::cout << "Error setting up server TCP socket..\n";
    
    http_route_counter = 0;
    
    add_route("/routes", GET, &http_server::send_router_view);
    
    std::cout << "HTTP server has been created.\n";
    
    return;
}

http_server::~http_server(){
    cleanup();
}

void http_server::run(){
    
    // TODO: SETUP
    
    read_handle_loop(); /* Main accept - read event loop */
}

/*
 * Function:  open_file
 * --------------------
 * Namespace http_server
 *
 *  Read give file and return file struct
 *    Includes content buffer and content size.
 *
 *  file: file name as const string
 *
 *  struct file_s: file struct with file info
 *
 */
struct file_s* http_server::open_file(std::string& file){
    
    FILE *fp = fopen (file.c_str(), "rb");
    if ( NULL == fp ) {
        perror("FILE");
        exit(EXIT_FAILURE);
    }
    int fd = fileno(fp);

    struct stat finfo; /* Get file info */
    int fs = fstat(fd, &finfo);
    if(fs == -1){
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    
    off_t content_size = finfo.st_size; /* Get content sizes */

    char* content = (char*) malloc(content_size);

    size_t readf = fread(content, content_size, 1, fp); /* read file into buffer */
    if(readf != 1){
        exit(EXIT_FAILURE);
    }
    
    struct file_s* file_obj = (struct file_s*) malloc(sizeof(struct file_s)); /* create and fill file struct */
    file_obj->content = content;
    file_obj->size = content_size;

    fclose(fp);
    
    return file_obj;
}

/*
 * Function:  create_tcp_socket
 * --------------------
 * Namespace http_server
 *
 *  Create, configure and setup the servers main accept socket.
 *
 *  port: port to use for socket.
 *
 *  int: configured socket
 *
 */
int http_server::create_tcp_socket(uint32_t port)
{
    int http_socket = -1;
    if ((http_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        return -1;

    struct sockaddr_in* address = new sockaddr_in;
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons( static_cast<int>(port) );
    memset(address->sin_zero, '\0', sizeof(address->sin_zero));
    
    server_addr = address;
    
    const int opt = 1;
    if (setsockopt(http_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) /* Allow socket to re use chosen port */
        return -1;
    
    if (bind(http_socket, (struct sockaddr *)address, sizeof(struct sockaddr_in)) < 0)
        return -1;
    
    if (listen(http_socket, MAX_CONNECTIONS) < 0)
        return -1;
    
    if(config->debug)
        std::cout << "TCP socket has succesfully been configured.\n";
    
    return http_socket;
    
}

/*
 * Function:  cleanup
 * --------------------
 * Namespace http_server
 *
 *  Frees all used memory and closes socket.
 *
 */
void http_server::cleanup()
{
    delete config;
    delete server_addr;
    
    for (int i = 0; i < http_route_counter; i++)
    {
        delete routes[i];
    }
}


/*
 * Function:  read_handle_loop
 * --------------------
 * Namespace http_server
 *
 *  Accept new clients, read first packet.
 *  Forward packet to client handler.
 *
 *  void:
 *
 *  void:
 *
 */
void http_server::read_handle_loop()
{
    while(true)
    {
        int client_socket;
        int addrlen = sizeof(struct sockaddr_in);
        struct sockaddr_in* address = new sockaddr_in;
        
        if ((client_socket = accept(server_socket, (struct sockaddr *)address, (socklen_t*)&addrlen)) < 0) /* Accept new connection */
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        printf("Connection! \n");
        
        struct timeval tv;
        tv.tv_sec = 8; /* 8 second recv timout window */
        fd_set rfds;
         
        FD_ZERO(&rfds);
        FD_SET(client_socket, &rfds);
        int recVal = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
        if(recVal == 0)
            continue;
        
        char buffer[HTTP_BUFFER_SIZE] = {0};
        ssize_t valread = recv(client_socket, buffer, HTTP_BUFFER_SIZE, 0);
        buffer[HTTP_BUFFER_SIZE-1] = 0;

        if(valread <= 0){
            return;
        }
        
        std::string temp(buffer);
        
        struct http_connection* new_connection = new_http_client();
        new_connection->client_socket = client_socket;
        new_connection->client_addr = address;
        new_connection->content = temp;
        
        printf("Assigned worker!\n");
        assign_worker(new_connection);
    }
}

/*
 * Function:  assign_worker
 * --------------------
 * Namespace http_server
 *
 *  Assign a worker to current new connection.
 *
 *  connection: http_connection struct
 *
 *  void:
 *
 */
void http_server::assign_worker(struct http_connection *connection)
{
    struct thread_job* job = new thread_job;
    
    job->connection = connection;
    job->context = this;
    
    worker_pool.add_job(job);
    
}


/*
 * Function:  handle_thread_connection
 * --------------------
 * Namespace http_server
 *
 *  Entry point for a new threaded http_connection
 *
 *  connection: http_connection struct
 *
 *  void:
 *
 */
void http_server::handle_thread_connection(struct http_connection *connection)
{
    // get content vs header
    connection->header = connection->content;
    std::string delimiter = "\r\n\r\n";
    std::string content = connection->content.substr(connection->content.find(delimiter), connection->content.length());
                                                     
    connection->content = content;

    parse_connection_header(connection); /* Begin of parse pipeline */
    
    
    //TODO: TO OWN FUNCTION
    close(connection->client_socket);
    delete connection->client_addr;
    delete connection->context;
    delete connection;

}

void http_server::add_authorization(std::string u_token)
{
    use_authorization = 1;
    token = u_token;
}

bool http_server::authorize(struct http_connection* connection)
{
    std::string cookies = connection->context->cookies;
    
    std::string current_word;
    std::stringstream ss(cookies);
    
    std::getline(ss, current_word, ' ');  // Cookie:
    while(std::getline(ss, current_word, ' '))
    {
        if(current_word.find("token=") != std::string::npos)
        {
            
            std::stringstream ss(current_word);
            std::string cookie;
            
            std::getline(ss, cookie, '='); // cookie name
            std::getline(ss, cookie, '='); // cookie value
            std::getline(ss, cookie, ';'); // remove ; at the end
            
            cookie = cookie.substr(0, cookie.size()-1); //remove \r
            
            if(cookie.compare(token) == 0)
            {
                return true;
            }
            return false;
        }
        
        std::getline(ss, current_word, ' ');
    }
    return false;
    
}

void http_server::parse_method_route(struct http_connection* connection)
{
    
    std::string route_header = connection->router;
    
    std::string current_word;
    std::stringstream ss(route_header);
    
    std::getline(ss, current_word, ' '); // METHOD
    connection->context->method = get_method(current_word.c_str());
    
    std::getline(ss, current_word, ' '); // ROUTE
    connection->context->route = current_word;
    std::getline(ss, current_word, ' '); // HTTP VERSION
    
    if(use_authorization)
    {
        bool authorized = authorize(connection);
        if(!authorized)
        {
            send_error(UNAUTHORIZED, connection);
            return;
        }
        connection->context->authorized = authorized;
    }
    
    std::string response_content = handle_route(connection->context->route, connection->context->method, connection);
    // TODO: print response_content

}

/*
 * Function:  static_html
 * --------------------
 * Namespace http_server
 *
 *  Returns the content of a static html file
 *
 *  filname: string filename of html
 *
 *  std::string: file content
 *
 */
std::string http_server::static_html(std::string filname){
    
    struct file_s* file = open_file(filname);
    
    return (file->content);
}

/*
 * Function:  send_response
 * --------------------
 * Namespace http_server
 *
 *  Format and send the user given response to the current client socket.
 *
 *  connection: http_connection struct
 *  response: string response from user.
 *
 *  void:
 *
 */
void http_server::send_response(struct http_connection* connection, std::string& response){
    
    std::string header = "HTTP/1.1 200\nContent-length: "+ std::to_string(response.size()+1) + "\n";
    
    header.append(connection->res->content_type); /* Add content type to header */
    header.append("; charset=utf-8\n");
     
    if(connection->res->set_cookies.compare("") != std::string::npos){
        header.append(connection->res->set_cookies); /* Add cookies if set. */
        header.append("\n");
    }
    header.append("\n");
    
    std::string output;
    output.append(header);
    output.append(response);
    
    ssize_t n = write(connection->client_socket, output.data(), output.size()+1);
    if(n == 0){
        std::cout << "[ERROR] Write returned 0.";
    }
}

void http_server::send_error(int error_code, struct http_connection* connection)
{
    std::string response = "";
    switch (error_code) {
        case BAD_REQUEST:
            response = "HTTP/1.1 400 Bad Request\nDate: Wed, 21 Oct 2015 07:28:00 GMT\n\n";
            break;
            
        case UNAUTHORIZED:
            response = "HTTP/1.1 401 Unauthorized\nDate: Wed, 21 Oct 2015 07:28:00 GMT\n\n";
            break;
        case FORBIDDEN:
            response = "HTTP/1.1 403 Forbidden\nDate: Wed, 21 Oct 2015 07:28:00 GMT\n\n";
            break;
    }
    
    ssize_t n = write(connection->client_socket, response.data(), response.size()+1);
    if(n == 0){
        std::cout << "[ERROR] Write returned 0.";
    }
}

/*
 * Function:  prase_connection_header
 * --------------------
 * Namespace http_server
 *
 *  Parse the header of current connection
 *       Parses them on new line and puts each line to headers vector.
 *       Also get connection, host and cookies.
 *
 *  connection: http_connection struct
 *
 *  void:
 *
 */
void http_server::parse_connection_header(struct http_connection* connection)
{
    std::string current_line;
    std::stringstream ss(connection->header);

    std::getline(ss, current_line, '\n');
    connection->router = current_line; /* Router with method and route */
    
    while(std::getline(ss, current_line, '\n'))
    {
        // TODO: BETTER
        if(current_line.find("Connection: ") != std::string::npos)
        {
            connection->context->connection = current_line;
        }
        else if(current_line.find("Host: ") != std::string::npos)
        {
            connection->context->host = current_line;
        }
        else if(current_line.find("Cookie: ") != std::string::npos)
        {
            connection->context->cookies = current_line;
        }
    }
    
    parse_method_route(connection);
}

/*
 * Function:  new_http_client
 * --------------------
 * Namespace http_server
 *
 *  Malloc new http context and connection.
 *  Prepare struct for new connection.
 *
 *  void:
 *
 *  struct http_connection*: new connection struct
 *
 */
struct http_connection* http_server::new_http_client()
{
    struct http_context* context = new http_context;
    struct http_connection* connection = new http_connection;
    
    connection->context = context;
    
    return connection;
}

/*
 * Function:  handle_route
 * --------------------
 * Namespace http_server
 *
 *  Search through all routes and find given route. Call it assigned function.
 *
 *  route: Name of the route to search for.
 *  method: Enum of the access method
 *
 *  void:
 *
 */
std::string http_server::handle_route(const std::string& route, const method_et& method, struct http_connection* connection)
{
    
    for (int i = 0; i < http_route_counter; i++)
    {
        uint32_t found = routes[i]->route.compare(route);
        if(found == 0 && routes[i]->method == method)
        {
            response* new_res = new response;
            new_res->content_type = "Content-Type: text/html";
            
            request* new_req = new request;
            new_req->cookies = connection->context->cookies;
            new_req->content = connection->content;
            new_req->context = connection->context;
            new_req->connection = connection->context->connection;
            new_req->route = connection->router;
            
            connection->res = new_res;
            
            std::string response_content;
            
            switch (routes[i]->option) {
                case WITH_PARAMETER:
                    response_content = (*(routes[i]->function))(new_req, new_res); /* Invoke given route function */
                    break;
                
                case WITHOUT_PARAMETER:
                    response_content = (*(routes[i]->function_n))(); /* Invoke given route function */
                    break;
                case INTERN:
                    response_content = send_router_view(); /* Invoke given route function */
                    break;
            }
            
            send_response(connection, response_content);
            return response_content;
        }
    }
    
    
    std::string not_found = "Nothing here.";
    return not_found;
}

/*
 * Function:  add_route
 * --------------------
 * Namespace http_server
 *
 *  add a new route to the http server, adding to to the routes array.
 *
 *  route_name: Name of the route to add.
 *  method_def: Enum of the access method
 *  *user_function: function to call when route is accessed. with response and request parameters.
 *
 *  int: number of current routes.
 *
 */
int http_server::add_route(const std::string& route_name, const method_et& method_def, std::string (*user_function)(request* req, response* res) )
{
    
    struct http_route* route = new http_route;
    route->function = user_function;
    route->route = route_name;
    route->method = method_def;
    route->option = WITH_PARAMETER;
    
    routes[http_route_counter] = route;
    http_route_counter++;
    
    return http_route_counter;
    
}

int http_server::add_route(const std::string& route_name, const method_et& method_def, std::string (http_server::*user_function)() )
{
    struct http_route* route = new http_route;
    route->function_intern = user_function;
    route->route = route_name;
    route->method = method_def;
    route->option = INTERN;
    
    routes[http_route_counter] = route;
    http_route_counter++;
    
    return http_route_counter;
    
}

int http_server::add_route(const std::string& route_name, const method_et& method_def, std::string (*user_function)() )
{
    
    struct http_route* route = new http_route;
    route->function_n = user_function;
    route->route = route_name;
    route->method = method_def;
    route->option = WITHOUT_PARAMETER;
    
    routes[http_route_counter] = route;
    http_route_counter++;
    
    return http_route_counter;
    
}

/*
 * Function:  http_get_method
 * --------------------
 * Parse method string to enum
 *
 *  method_string: string of method
 *
 *  returns: method_et enum
 *
 */
method_et http_server::get_method(const char* method_string){


    if(strcmp(method_string, "GET") == 0){
        return GET;
    }

    if(strcmp(method_string, "POST") == 0){
        return POST;
    }

    if(strcmp(method_string, "HEAD") == 0){
        return HEAD;
    }

    return UNDEFINED;
}
