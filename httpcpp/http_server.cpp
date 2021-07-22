//
//  http_server.cpp
//  httpcpp
//
//  Created by Joe Bayer on 22/07/2021.
//

#include "http_server.h"

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
http_server::http_server(struct http_config user_config)
{
    
    struct http_config* config_m = (struct http_config*) malloc( sizeof(struct http_config) );
    config_m->port = user_config.port;
    config_m->debug = user_config.debug;
    
    config = config_m;
    if(config->debug)
        std::cout << "Config file has been added to the server.\n";
    
    server_socket = create_tcp_socket(config->port);
    if(server_socket == -1)
        std::cout << "Error setting up server TCP socket..\n";
    
    http_route_counter = 0;
    
    std::cout << "HTTP server has been created.\n";
    
    return;
}


void http_server::run(){
    
    // TODO: SETUP
    
    read_handle_loop(); /* Main accept / read event loop */
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

    struct sockaddr_in* address = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
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
    free(config);
    free(server_addr);
    
    for (int i = 0; i < http_route_counter; i++)
    {
        free(routes[i]);
    }
    
    if(config->debug)
        std::cout << "All used memory has been freed.\n";
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
    int socket;
    int addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in* address = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
    
    if ((socket = accept(server_socket, (struct sockaddr *)address, (socklen_t*)&addrlen)) < 0) /* Accept new connection */
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    char buffer[HTTP_BUFFER_SIZE] = {0};
    ssize_t valread = recv(socket, buffer, HTTP_BUFFER_SIZE, 0);
    buffer[HTTP_BUFFER_SIZE-1] = 0;

    if(valread <= 0){
        return;
    }

    printf("%s\n", buffer);
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
void http_server::handle_route(const std::string& route, const method_t& method)
{
    
    for (int i = 0; i < http_route_counter; i++)
    {
        uint32_t found = routes[i]->route.compare("/");
        if(found == 0 && routes[i]->method == method)
        {
            (*(routes[i]->function))(); /* Invoke given route function */
            return;
        }
    }

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
 *  *user_function: function to call when route is accessed.
 *
 *  int: number of current routes.
 *
 */
int http_server::add_route(const std::string& route_name, const method_t& method_def, void (*user_function)() )
{
    
    struct http_route* route = (struct http_route* ) malloc(sizeof(struct http_route));
    route->function = user_function;
    route->route = route_name;
    route->method = method_def;
    
    routes[http_route_counter] = route;
    http_route_counter++;
    
    handle_route("/", method_def);
    
    return http_route_counter;
    
}
