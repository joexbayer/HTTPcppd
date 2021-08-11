## Benchmarks

11.08.21

Command:
```
wrk -t12 -c400 -d30s http://127.0.0.1:5000/
```

### HTTPcppd - Hello, World!, 200threads
Result
```
Running 30s test @ http://127.0.0.1:8080/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    12.14ms   29.07ms 401.77ms   93.50%
    Req/Sec     2.59k     1.55k   17.96k    76.48%
  923920 requests in 30.10s, 182.39MB read
  Socket errors: connect 0, read 17829, write 3, timeout 0
Requests/sec:  30690.47
Transfer/sec:      6.06MB
```


### HTTPcppd - Hello, World!, 99threads - with terminal output
Result 1
```
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
### HTTPcppd - Hello, World!, 99threads - without terminal output
Result 1
```
Running 30s test @ http://127.0.0.1:8080/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    13.88ms   46.10ms 525.55ms   92.52%
    Req/Sec     3.27k     3.38k   33.51k    87.54%
  1125429 requests in 30.10s, 222.17MB read
  Socket errors: connect 0, read 69004, write 0, timeout 0
Requests/sec:  37392.01
Transfer/sec:      7.38MB
```

### HTTPcppd - Hello, World!, 20threads - without terminal output
Result 1
```
Running 30s test @ http://127.0.0.1:8080/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   238.31us  382.54us  62.52ms   99.79%
    Req/Sec    11.85k     6.18k   26.40k    54.03% xc
  2427715 requests in 30.04s, 479.26MB read
  Socket errors: connect 0, read 8142, write 0, timeout 2
Requests/sec:  80813.07
Transfer/sec:     15.95MB
```
Result 2
```
Running 30s test @ http://127.0.0.1:8080/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   256.77us    1.16ms 133.86ms   99.69%
    Req/Sec     8.96k     4.93k   19.11k    69.27%
  2407732 requests in 30.04s, 475.31MB read
  Socket errors: connect 0, read 20632, write 0, timeout 0
Requests/sec:  80143.80
Transfer/sec:     15.82MB
```
Result 3 - Run from terminal
```
Running 30s test @ http://127.0.0.1:8080/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   170.15us   34.77us   6.66ms   92.44%
    Req/Sec    11.30k     5.58k   25.46k    62.25%
  3385619 requests in 30.10s, 658.67MB read
  Socket errors: connect 155, read 1343, write 0, timeout 0
Requests/sec: 112475.80
Transfer/sec:     21.88MB
```

### ASP.NET - Web application, index.html - Hello, World!
Result
```
Running 30s test @ http://127.0.0.1:5000/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    41.94ms   95.54ms   1.98s    99.15%
    Req/Sec   391.72    162.83   800.00     61.98%
  15003 requests in 30.04s, 25.34MB read
  Socket errors: connect 0, read 1595, write 10, timeout 36
Requests/sec:    499.40
Transfer/sec:    863.63KB
```


All tests run on:
```
2,3 GHz 8-Core Intel Core i9
32 GB 2667 MHz DDR4
AMD Radeon Pro 5500M 4 GB
Intel UHD Graphics 630 1536 MB
```
