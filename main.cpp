// ----------------------------------------------------------------
// Copyright (c) 2024 Takeda-Forever
// All rights reserved.
// Http request responeser is free software: you can 
// redistribute it and/or modify it for need.
// ----------------------------------------------------------------

/*

How it works:
 - This code uses boost.asio library to establish a TCP connection to a server.
 - The server's IP address and port are provided as command line arguments.
 - The client sends a HTTP GET request to the server.
 - The server responds with the HTTP headers and body.
 - The client prints out the response.

Compile and run the code:
 g++ -o http_server main.cpp -lboost_asio -lptread
 or 
 cl main.cpp /EHsc /I /path/to/boost-library
*/



#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "function.h"
#include "lib.h"
#include "pch.h"

int main(int argc, char** argv) {
std::cin.tie(NULL); 
std::ios_base::sync_with_stdio(false);


    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1; 
    }
    short port = std::stoi(argv[1]);

    try {
        auto file_handler = std::make_shared<WebCore::FileHandler>();
        auto response_builder = std::make_shared<WebCore::HttpResponseBuilder>();
        WebCore::Server server(port, response_builder, file_handler);
        auto router = std::make_shared<WebCore::Router>();
        setup_routes(router, file_handler, response_builder);

        server.run(router);
    } catch (std::exception& e) {
        std::cerr << "Server exception: " << e.what() << std::endl;
    }

    return 0;
}