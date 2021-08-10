# HTTP C++

### HTTP RESTful API with routes and serving static files.

[WRK](https://github.com/wg/wrk) performance test on localhost:
```
wrk -t12 -c400 -d30s http://127.0.0.1:8080/
----------------------------------------------
Running 30s test @ http://127.0.0.1:8080/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     7.01ms   42.74ms 902.12ms   98.89%
    Req/Sec     2.56k     1.61k    7.75k    60.88%
  897665 requests in 30.10s, 177.20MB read
  Socket errors: connect 155, read 1931, write 2, timeout 0
Requests/sec:  29818.32
Transfer/sec:      5.89MB
```

#### How to use: 

#### 1.(Optional) Create a config for your http server

You can explicit declare memebers:
```c+
/* Config file for server */
http_config config;
config.port = 8080;
config.thread_pool_size = 99;
config.favicon = "favicon.ico";
config.log_level = VERBOSE;
```
or use the constructor
```c++
http_config config(PORT, THREADS, FAVICON, LOG);
```

#### 2. Create the http server app

Either with or without config:
```c++
/* Create new http server based on config. */
http_server app(config);
or
http_server app();
```

#### 3. Create routes

To create routes you need to define a route, method and function. Best practice is to use a lambda.
(Function needs to take in a request and response pointer.)
```c++
 /* GET Route example with static file */
app.route("/", GET, [](request* req, response* res){

    res->send("Hello World!");

});
```

To define parameters put a ? after the route and then define parameters between two brackets.
You can access parameters by using the request->params map.
To access headers use the request->headers map.
```c++
/* GET Route example json response and params */
app.route("/json?[age]", GET, [](request* req, response* res){

    res->set_contentype("application/json");
    /* Use params and return value in json. */
    res->send("{name: \""+req->headers["Host"]+"\", age: \"" + req->params["age"] + "\"}");

});
```

Special responses:
```c++
res->send(req->context->send_router_view()); /* Sends a detailed UI for all current routes */

res->set_contentype("application/json"); /* Sents response content type, default is text/html */

res->add_cookie(req->params["username"], req->params["password"]); /* Add a response cookie */

res->send(req->context->stats()); /* Send server stats as JSON */ 

res->redirect("/"); /* Redirect to given route */

res->send(http_server::static_html("index.html")); /* Sends given HTML file as response */
```
Example of ```req->context->stats()```, data collected during runtime.
```javascript
{
 "Stats": {
  "127.0.0.1": {
    "GET / HTTP/1.1": "2",
    "GET /favicon.ico HTTP/1.1": "3",
    "GET /json?age= HTTP/1.1": "3",
    "GET /json?age=21 HTTP/1.1": "1",
    "GET /routes HTTP/1.1": "1",
    "GET /routes? HTTP/1.1": "2",
    "GET /stats? HTTP/1.1": "3",
    "POST /example HTTP/1.1": "1",
    "Type": "Client"
  },
  "Authorized requests": "16",
  "Responses": "11",
  "Status" : "running", 
  "Using threads" : "2", 
  "Total threads" : "99" 
 }
}
```

#### 4. Add an authentication token (OPTIONAL) and run the server
```c++
/* Adding cookie token authentication. */
app.authentication("secret-token"); 

/* Start server */
app.run()
```
