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
 g++ -o http_server main.cpp -lboost_asio -lpthread
 or 
 cl main.cpp /EHsc /I /path/to/boost-library
*/



#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <string>
#include <chrono>
#include <ctime>



using boost::asio::ip::tcp;

std::string getIpAddr()
{
    #include <winsock2.h> 
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    WSADATA wsa_Data;
    int wsa_ReturnCode = WSAStartup(0x202, &wsa_Data);
    // Get the local hostname
    char szHostName[255];
    gethostname(szHostName, 255);
    struct hostent* host_entry;
    host_entry = gethostbyname(szHostName);
    char* szLocalIP;
    szLocalIP = inet_ntoa(*(struct in_addr*)*host_entry->h_addr_list);
    return std::string(szLocalIP);
    WSACleanup();
}

std::string get_time() 
{
    auto start = std::chrono::system_clock::now();
    // Some computation here
    auto end = std::chrono::system_clock::now();
 
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
 
    return (std::string)std::ctime(&end_time);
}

// Function to generate HTTP response
std::string make_http_response(const std::string& content, const std::string& status = "200 OK", const std::string& content_type = "text/html") {
    return "HTTP/1.1 " + status + "\r\n" +
           "Location: "+ getIpAddr() + "/" + "index.html" + "\r\n" +
           "Content-Type: " + content_type + "; charset=UTF-8"+"\r\n" +
           "Content-Length: " + std::to_string(content.size()) + "\r\n" +
           "Data: " + get_time() + "\r\n" +
           "\r\n" + content;
}

// Function to generate a 404 Not Found response
std::string make_404_response() {
    std::string content = "<html><body><h1>404 Not Found</h1></body></html>";
    return make_http_response(content, "404 Not Found");
}

// Function to serve static files (like .css)
std::string serve_file(const std::string& file_path, const std::string& content_type) {
    std::ifstream file(file_path);
    if (!file) {
        return make_404_response();  // Return 404 if file not found
    }

    std::stringstream buffer;
    buffer << file.rdbuf();  // Read file into stringstream
    std::string content = buffer.str();

    return make_http_response(content, "200 OK", content_type);  // Serve the file with the correct Content-Type
}

bool detect_server_file(const std::string& file_path)
{
    std::ifstream file(file_path);
    if (!file) 
        return false;  // Return 404 if file not found

    return true;

}

void session(tcp::socket socket) {
    try {
        char data[1024];
        boost::system::error_code error;

        // Read the HTTP request
        size_t length = socket.read_some(boost::asio::buffer(data), error);

        if (error == boost::asio::error::eof) {
            return;  // Connection closed cleanly
        } else if (error) {
            throw boost::system::system_error(error);
        }

        // Basic request parsing (for demonstration, this is very simple)
        std::string request(data, length);
        std::string response;

        if (request.find("GET / ") != std::string::npos || request.find("GET /index.html") != std::string::npos) {
            // Serve the main page
            if(!detect_server_file("index.html")){
                std::string content = "<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\"></head>"
                                      "<body><h1>Welcome to My C++ Boost Web Server!</h1></body></html>";
                response = make_http_response(content);
            }else
                response = serve_file("index.html", "text/html");

        } else if (request.find("GET /style.css") != std::string::npos) {
            // Serve the CSS file
            if(!detect_server_file("style.css"))
                response = make_404_response();  // Return 404 if file not found
            else
                response = serve_file("style.css", "text/css");
        } else {
            // If the resource is not found, return 404 Not Found
            response = make_404_response();
        }

        // Send the HTTP response
        boost::asio::write(socket, boost::asio::buffer(response), error);

    } catch (std::exception& e) {
        std::cerr << "Exception in session: " << e.what() << std::endl;

        // Return a 500 Internal Server Error response
        std::string response = make_http_response("<html><body><h1>500 Internal Server Error</h1></body></html>", "500 Internal Server Error");
        boost::system::error_code ignored_error;
        boost::asio::write(socket, boost::asio::buffer(response), ignored_error);
    }
}

void server(std::shared_ptr<boost::asio::io_context> io_context, short port) {
    tcp::acceptor acceptor(*io_context, tcp::endpoint(tcp::v4(), port));

    for (;;) {
        tcp::socket socket(*io_context);
        acceptor.accept(socket);
        std::thread(session, std::move(socket)).detach();
    }
}

int main() {
    try {
        auto io_context = std::make_shared<boost::asio::io_context>();
        server(io_context, 8080);  // Listening on port 8080
    }
    catch (std::exception& e) {
        std::cerr << "Server exception: " << e.what() << std::endl;
    }

    return 0;
}
