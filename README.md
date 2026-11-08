# c-http-server

A minimal, single-threaded HTTP file server written in C using BSD sockets.

I built this to understand what actually happens between a browser and a web server. I wrote the socket handling and request parsing by hand, and checked the server's behaviour packet by packet in Wireshark.

## What it does

- Listens for TCP connections on port **8080**
- Accepts one client at a time and reads the HTTP request
- Parses the request line (method, path, version)
- Serves the requested file from disk, or returns an error response if it can't be found

## Build and run

Requirements: a C compiler (`gcc` or `clang`) on macOS or Linux.

```bash
gcc -Wall -Wextra -o server server.c
./server
```

Then open <http://localhost:8080/index.html> in a browser, or request a file from the command line:

```bash
printf 'GET /index.html HTTP/1.0\r\n\r\n' | nc localhost 8080
```

## How I tested it

- **netcat:** sent hand-written and deliberately malformed requests to check how the server parses requests and handles errors.
- **Wireshark:** captured traffic on the loopback interface to trace each connection: the TCP three-way handshake, the HTTP request and response, and the connection teardown.
- **Packet loss:** simulated dropped packets on loopback and watched TCP retransmit the lost segments. The server code never saw the loss, because the kernel's TCP stack handled it.

## What I learned

The most useful lesson was seeing where my code ends and the operating system begins. The handshake completes before `accept()` returns, and retransmissions happen without the program knowing. My server only ever sees a reliable byte stream. Checking what is actually on the wire, not just what the program prints, is now part of how I debug.

## What I'd improve

- **Concurrency:** because the server is single-threaded, one slow client blocks everyone else. I'd redesign it around `poll()` or a thread pool.
- **Automated tests:** testing was manual with netcat. I'd add a test suite that runs on every commit through CI.
- **HTTP coverage:** add more status codes, MIME types, and protection against path traversal (e.g. `GET /../secret`).