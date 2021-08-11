//
//  http_server.cpp
//  httpcpp
//
//  Created by Joe Bayer on 22/07/2021.
//

#include "http_server.h"

static bool run = true;
struct http_cache* start_cache = nullptr;

static std::mutex cache_mutex;

void on_exit()
{
    run = false;
}


struct file_s* http_cache::find(const std::string &filename, struct http_cache *cache)
{
    if(cache == nullptr)
    {
        return nullptr;
    }
    if(cache->filename.compare(filename) != std::string::npos)
    {
        std::cout << "Found Cache for: " << filename << std::endl;
        return cache->file;
    }
    return http_cache::find(filename, cache->next);
}

void http_cache::add(struct file_s *u_file, const std::string &filename)
{
    if(start_cache == nullptr)
    {
        cache_mutex.lock();
        struct http_cache* new_cache = new http_cache;
        new_cache->file = u_file;
        new_cache->filename = filename;
        new_cache->next = nullptr;
        start_cache = new_cache;
        std::cout << "New Cache for: " << filename << std::endl;
        cache_mutex.unlock();
        return;
    }
    add_recursive(u_file, filename, start_cache);
    
}
void http_cache::add_recursive(struct file_s* u_file, const std::string& filename, struct http_cache* next)
{
    if(next == nullptr)
    {
        cache_mutex.lock();
        struct http_cache* new_cache = new http_cache;
        new_cache->file = u_file;
        new_cache->filename = filename;
        new_cache->next = nullptr;
        next = new_cache;
        
        std::cout << "New Cache for: " << filename << std::endl;
        cache_mutex.unlock();
        return;
    }
    add_recursive(u_file, filename, next->next);
}

void http_cache::free_cache(struct http_cache* start)
{
    cache_mutex.lock();
    if(start == nullptr)
        return;
    free(start->file->content);
    delete start->file;
    cache_mutex.unlock();
    free_cache(start->next);
    delete start;
    start = nullptr;
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
    std::string response =
        "<HTML>                                                                                                     \
            <head>                                                                                                  \
            </head>                                                                                                 \
            <nav class='navbar navbar-light bg-light'>                                                              \
            <a class='navbar-brand' href='#'>                                                                       \
                <img src='favicon.ico' width='30' height='30' class='d-inline-block align-top'>                     \
                HTTPccpd - route view.                                                                              \
            </a>                                                                                                    \
        </nav>                                                                                                      \
        <body style='text-align:center;'><div style='width:60%;margin:auto;'>                                       \
            <h3>Routes that are currently available: </h3>                                                          \
            <table style='width:70%;margin:auto;'>                                                                     \
                <thead>                                                                                             \
                    <tr>                                                                                            \
                        <th>Method</th>                                                                             \
                        <th>Route</th>                                                                              \
                        <th>Parameters</th>                                                                         \
                        <th>Action</th>                                                                             \
                    </tr>                                                                                           \
                </thead>                                                                                            \
                <tbody>";
    
    for(int i = 0; i < http_route_counter; i++)
    {
        std::string param;
        std::string raw_input = routes[i]->params;
        std::string input = raw_input.substr(1, routes[i]->params.size()-2); /* Remove [ ] */
        input.erase(remove_if(input.begin(), input.end(), isspace), input.end()); /* Remove spaces */
        std::stringstream ss(input);
        
        response.append("<tr><form method=\""+method_names[routes[i]->method]+"\" action=\""+routes[i]->route+"\"><td>   \
                        "+method_names[routes[i]->method]+"</td><td>"+routes[i]->route+" </td><td>");
        if(routes[i]->has_params)
        {
            while(getline(ss, param, ','))
            {
                response.append("<input style='width:50%;' class='input-group col-xs-2' placeholder='"+param+"' type='text' name='"+param+"'>");
            }
        }
        
        response.append("</td><td><button class='btn btn-primary'>Send</button></td></form></tr>");
    }
    response.append("</tbody></table></div></body></html>");
    
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
    content_type = std::move(new_content_type); /* Move instead of assignment copy */
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
    std::string new_cookie = " "+cookie_name +"="+ cookie_value+";";
    set_cookies.append(new_cookie);
}

void response::redirect(std::string url)
{
    redirect_ = 1;
    redirect_url = std::move(url);
}

void response::send(std::string content)
{
    response_data = content;
}

http_config::http_config(int u_port, int u_threads,std::string u_favicon, enum log_level level)
{
    port = u_port;
    thread_pool_size = u_threads;
    favicon = u_favicon;
    log_level = level;
}
http_config::http_config(){}

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
    config_m->favicon = user_config.favicon;
    config_m->thread_pool_size = user_config.thread_pool_size;
    config_m->log_level = user_config.log_level;
    
    main_header = "Server: HTTPcppd (Unix)\nContent-Security-Policy: script-src 'unsafe-inline';\n";
    
    config = config_m;
    setup();
}

http_server::http_server() : worker_pool(10)
{
    struct http_config* config_m = new http_config;
    config_m->port = 8080;
    config_m->thread_pool_size = 10;
    config_m->favicon = "favicon.ico";
    config_m->log_level = WARNING;
    
    config = config_m;
    setup();
}


/*
 * Function:  setup
 * --------------------
 * Namespace http_server
 *
 *  Setup for server on start
 *
 *  void:
 *
 *  void:
 *
 */
void http_server::setup()
{
    log.log_level(config->log_level); /* Set log level for logger */
    
    log.log("Config file has been added to the server.\n", VERBOSE);
    
    server_socket = create_tcp_socket(config->port);
    if(server_socket == -1)
        log.log("Error setting up server TCP socket..\n", ERROR);
    
    http_route_counter = 0;
    
    std::cout << "HTTP server has been started.\n";
}

http_server::~http_server()
{
    cleanup();
}

std::string http_server::stats()
{
    timer t("stats to json", &log);
    return log.to_json(config);
}

void http_server::run()
{
    
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
struct file_s* http_server::open_file(std::string& file)
{
    
    FILE *fp = fopen (file.c_str(), "rb");
    if ( NULL == fp ) {
        perror("FILE");
        exit(EXIT_FAILURE);
    }
    int fd = fileno(fp);

    struct stat finfo; /* Get file info */
    int fs = fstat(fd, &finfo);
    if(fs == -1)
    {
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    
    off_t content_size = finfo.st_size; /* Get content sizes */

    char* content = (char*) malloc(content_size);

    size_t readf = fread(content, content_size, 1, fp); /* read file into buffer */
    if(readf != 1)
    {
        exit(EXIT_FAILURE);
    }
    
    content[content_size] = 0;
    struct file_s* file_obj = new file_s;/* create and fill file struct */
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
    timer t("create_tcp_socket", &log);
    
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
    
    log.log("TCP socket has succesfully been configured.\n", VERBOSE);
    
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
    http_cache::free_cache(start_cache);
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
    int requests = 10;
    while(true)
    {
        int client_socket;
        int addrlen = sizeof(struct sockaddr_in);
        struct sockaddr_in* address = new sockaddr_in;
        
        if ((client_socket = accept(server_socket, (struct sockaddr *)address, (socklen_t*)&addrlen)) < 0) /* Accept new connection */
        {
            log.log("Accept on socket failed!", WARNING);
            continue;
        }
        
        struct http_connection* new_connection = new_http_client();
        new_connection->client_socket = client_socket;
        new_connection->client_addr = std::move(address);
        
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address->sin_addr), str, INET_ADDRSTRLEN);
        new_connection->context->client_ip = std::string(str);
        
        assign_worker(new_connection);
        
        requests--;
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
    job->connection->stage = HTTP_ACCEPTED_CLIENT;
    
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
    int alive = 1;
    while(alive)
    {
        alive = 0;
        struct timeval tv;
        tv.tv_sec = 8; /* 8 second recv timout window */
        fd_set rfds;
         
        FD_ZERO(&rfds);
        FD_SET(connection->client_socket, &rfds);
        int recVal = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
        if(recVal == 0)
        {
            break;
        }
        
        char buffer[HTTP_BUFFER_SIZE] = {0};
        ssize_t valread = recv(connection->client_socket, buffer, HTTP_BUFFER_SIZE, 0);
        buffer[HTTP_BUFFER_SIZE-1] = 0;

        if(valread <= 0)
        {
            break;
        }
    
        timer t("handle_thread_connection", &log);
    
        //TODO: BETTER
        connection->content = std::string(buffer);
        connection->stage = HTTP_ACCEPTED_REQUEST;
        
        // get content vs header
        connection->header = connection->content;
        std::string delimiter = "\r\n\r\n";
        std::string content = connection->content.substr(connection->content.find(delimiter), connection->content.length());
        delimiter.clear();
                                                         
        connection->content = std::move(content); /* Move because content is no longer needed */
        content.clear();
        
        t.~timer();
        pipeline_handler(connection); /* Begin of parse pipeline */
    
        // end of pipeline
        if(connection->context->keep_alive)
            alive = 1;
    }
    close_connection(connection);
}

void http_server::close_connection(struct http_connection* connection)
{
    close(connection->client_socket);
    //delete connection->client_addr;
    //delete connection->context; /* New context on keep alive? */
    delete connection;
}

void http_server::authentication(std::string u_token)
{
    use_authentication = 1;
    token = u_token;
}

/*
 * Function:  authenticate
 * --------------------
 * Namespace http_server
 *
 *  Checks if current connection contains a cookie with correct token.
 *
 *  connection: http_connection struct
 *
 *  bool: autheticated
 *
 */
bool http_server::authenticate(struct http_connection* connection)
{
    
    timer t("authenticate", &log);
    
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
            
            if(cookie.compare(token) != std::string::npos)
            {
                return true;
            }
            return false;
        }
        
        std::getline(ss, current_word, ' ');
    }
    return false;
    
}


/*
 * Function:  parse_method_route
 * --------------------
 * Namespace http_server
 *
 *  parses current connection router, gets route, method and http version.
 *
 *  connection: http_connection struct
 *
 *  void:
 *
 */
void http_server::parse_method_route(struct http_connection* connection)
{
    timer t("parse_method_route", &log);
    
    connection->stage = HTTP_PARSE_METHOD;
    
    std::string route_header = connection->router;
    
    std::string current_word;
    std::stringstream ss(route_header);
    
    std::getline(ss, current_word, ' '); // METHOD
    connection->context->method = get_method(current_word.c_str());
    
    std::getline(ss, current_word, ' '); // ROUTE
    
    std::stringstream ss_route_(current_word);
    std::getline(ss_route_, current_word, '?');
    
    connection->context->route = current_word;
    std::getline(ss, current_word, ' '); // HTTP VERSION
    
    if(use_authentication == 1)
    {
        t.~timer();
        bool authenticated = authenticate(connection);
        if(!authenticated)
        {
            log.count("Unauthorized requests");
            log.log("Request was not autheticated!\n", WARNING);;
            send_error(UNAUTHORIZED, connection);
            return;
        }
        log.count("Authorized requests");
        connection->context->authorized = authenticated;
    }
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
std::string http_server::static_html(std::string filname)
{
    struct file_s* file = http_cache::find(filname, start_cache);
    if(file != nullptr)
    {
        std::string return_string(file->content);

        return return_string;
    }
    
    struct file_s* file_new = open_file(filname);
    
    http_cache::add(file_new, filname);
    
    std::string return_string(file_new->content);
    
    //free(file->content);
    //delete file;

    return return_string;
}

/* UNDER CONSTRUCTION */
void http_server::pipeline_handler(struct http_connection* connection)
{
    timer res("Response", &log);
    switch (connection->stage) {
        case HTTP_ACCEPTED_REQUEST:
            // pipeline
            parse_connection_header(connection);
            parse_router(connection);
            parse_method_route(connection);
            handle_route(connection->context->route, connection->context->method, connection);
            
            break;
        default:
            break;
    }
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
void http_server::send_response(struct http_connection* connection)
{
    
    timer t("send_response", &log);
    
    connection->stage = HTTP_SEND_RESPONSE;
    
    std::string header = "HTTP/1.1 "+ std::to_string(connection->res->status) +"\nContent-length: "+ std::to_string(connection->res->response_data.size()) + "\n";
    
    header.append(connection->res->content_type); /* Add content type to header */
    header.append("; charset=utf-8\n");
    header.append(main_header);
    
    if(connection->context->keep_alive)
        header.append("Connection: keep-alive\n");
     
    if(connection->res->set_cookies.compare("Set-Cookie: ") != std::string::npos)
    {
        header.append(connection->res->set_cookies); /* Add cookies if set. */
        header.append("\n");
    }
    header.append("\n");
    
    std::string output;
    output.append(header);
    output.append(connection->res->response_data);
    
    ssize_t n = write(connection->client_socket, output.data(), output.size());
    if(n == 0)
    {
        log.log("Write returned 0!", WARNING);
    }
    
    /* Causes errors, maybe on incomplete requests? */
    //delete connection->res;
}

/*
 * Function:  send_favicon
 * --------------------
 * Namespace http_server
 *
 *  Sends favicon, mainly for google chrome
 *
 *  connection: http_connection struct
 *
 *  void:
 *
 */
void http_server::send_favicon(struct http_connection* connection)
{
    timer t("send_favicon", &log);
    
    struct file_s* file = http_cache::find(config->favicon, start_cache);
    if(file == nullptr)
    {
        file = open_file(config->favicon);
        http_cache::add(file, config->favicon);
    }
    
    std::string header = "HTTP/1.1 200\nContent-length: "+ std::to_string(file->size) + "\n";
    header.append("Content-Type: image/gif\n\n");
    
    char* response = (char*) malloc(file->size+header.size());
    memcpy(response, header.c_str(), header.size());
    memcpy(response+header.size(), file->content, file->size);
    
    ssize_t n = write(connection->client_socket, response, header.size()+file->size);
    if(n == 0)
    {
        log.log("Write returned 0!", WARNING);
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
    
    ssize_t n = write(connection->client_socket, response.data(), response.size()+2);
    if(n == 0)
    {
        log.log("Write returned 0!", WARNING);
    }
}

void http_server::send_redirect(std::string url, struct http_connection* connection)
{
    std::string response = "HTTP/1.1 301 Moved Permanently\nConnection: Close\nLocation: "+url+"\n";
    if(connection->res->set_cookies.compare("Set-Cookie: ") != std::string::npos)
    {
        response.append(connection->res->set_cookies);
        response.append(response+ "\n");
    }
    response.append(response+ "\n");
    ssize_t n = write(connection->client_socket, response.data(), response.size()+1);
    if(n == 0)
    {
        log.log("Write returned 0!", WARNING);
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
    
    timer t("parse_connection_header", &log);
    
    connection->stage = HTTP_PARSE_HEADER;
    
    std::string current_line;
    std::stringstream ss(connection->header);

    std::getline(ss, current_line, '\n');
    connection->router = current_line; /* Router with method and route */
    current_line.erase(std::remove(current_line.begin(), current_line.end(), '\r'), current_line.end());
    
    //std::cout << connection->context->client_ip << " -> "<< connection->router;
    log.count(connection->context->client_ip, current_line);
    
    t.~timer();
    timer t_h("parsing headers.", &log);
    while(std::getline(ss, current_line, '\n'))
    {
        if(current_line.find("Connection: ") != std::string::npos)
        {
            connection->context->connection = current_line;
            if(current_line.find("keep-alive") != std::string::npos)
            {
                connection->context->keep_alive = true;
            }
        }
        else if(current_line.find("Cookie: ") != std::string::npos)
        {
            connection->context->cookies = current_line;
        }
        else if(current_line.find("Content-Type: ") != std::string::npos)
        {
            if(current_line.find("application/x-www-form-urlencoded") != std::string::npos)
            {
                parse_post_params(connection);
            }
        }
        std::string header_edit;
        std::stringstream ss_header(current_line);

        std::getline(ss_header, header_edit, ' ');
        std::string header_name = header_edit.substr(0, header_edit.size()-1);
        std::getline(ss_header, header_edit, ' ');
        connection->context->headers[header_name] = header_edit.substr(0, header_edit.size()-1);
        
    }
}

void http_server::parse_post_params(struct http_connection* connection)
{
    
    std::string params = connection->content.substr(4, connection->content.size());
    
    parse_params(connection, params);
}

void http_server::parse_router(struct http_connection* connection)
{
    timer t("parse_router", &log);
    
    connection->stage = HTTP_PARSE_ROUTER;
    
    std::string header_params;
    std::stringstream ss_params(connection->router);
    if(connection->router.find("?") != std::string::npos)
    {
        
        std::string header_params;
        std::stringstream ss_params(connection->router);

        std::getline(ss_params, header_params, ' ');
        std::getline(ss_params, header_params, ' ');
        
        std::stringstream ss_params_2(header_params);
        
        std::getline(ss_params_2, header_params, '?');
        std::getline(ss_params_2, header_params, '?'); /* Params in the form, name=value&name2=value2 */
        
        t.~timer();
        parse_params(connection, header_params);
    }
}

void http_server::parse_params(struct http_connection* connection, std::string& raw_params)
{
    timer t("parse_params", &log);
    std::string header_params;
    
    if(raw_params.find("&") != std::string::npos)
    {
        std::stringstream ss_params_many(raw_params);
        while(std::getline(ss_params_many, header_params, '&'))
        {
            std::stringstream ss_params_many_value(header_params);
            std::getline(ss_params_many_value, header_params, '=');
            std::string name = header_params;
            std::getline(ss_params_many_value, header_params, '=');
            std::string value = header_params;
            
            connection->context->params[name] = value;
        }
        return;
    }
    
    std::stringstream ss_params_many_value(raw_params);
    std::getline(ss_params_many_value, header_params, '=');
    std::string name = header_params;
    std::getline(ss_params_many_value, header_params, '=');
    std::string value = header_params;

    connection->context->params[name] = value;
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
    timer t("handle_route", &log);
    
    connection->stage = HTTP_PARSE_METHOD;
    
    /* Check for favicon.ico */
    uint32_t found = route.compare("/favicon.ico");
    if(found == 0 && method == GET)
    {
        t.~timer();
        send_favicon(connection);
        return "";
    }
    
    for (int i = 0; i < http_route_counter; i++)
    {
        uint32_t found = routes[i]->route.compare(route);
        if(found == 0 && routes[i]->method == method)
        {
            response* new_res = new response;
            
            connection->res = new_res;
            connection->context->context = this;
            
            std::string response_content;
            
            switch (routes[i]->option) {
                case USER_DEFINED:
                    timer t_user("user function", &log);
                     (*(routes[i]->function))(connection->context, new_res); /* Invoke given route function */
                    t_user.~timer();
                    break;
            }
            
            t.~timer();
            if(connection->res->redirect_)
            {
                send_redirect(connection->res->redirect_url, connection);
                //delete new_res;
                return response_content;
                
            }
            log.count("Responses");
            send_response(connection);
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
int http_server::route(const std::string& route_name, const method_et& method_def, void (*user_function)(request* req, response* res) )
{
    
    struct http_route* route = new http_route;
    route->function = user_function;
    if(route_name.find("?") != std::string::npos)
    {
        std::string params;
        std::stringstream ss_params(route_name);
        
        std::getline(ss_params, params, '?');
        route->route = params;
        std::getline(ss_params, params, '?');
        route->params = params;
        route->has_params = 1;
    }
    else
    {
        route->params = "[ ]";
        route->route = route_name;
        route->has_params = 0;
    }
    route->method = method_def;
    route->option = USER_DEFINED;
    
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
method_et http_server::get_method(const char* method_string)
{


    if(strcmp(method_string, "GET") == 0)
    {
        return GET;
    }

    if(strcmp(method_string, "POST") == 0)
    {
        return POST;
    }

    if(strcmp(method_string, "HEAD") == 0)
    {
        return HEAD;
    }

    return UNDEFINED;
}
