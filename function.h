#include "lib.h"
#include "pch.h"

namespace WebCore{

class Router {
public:
    using HandlerFunc = std::function<std::string()>;

    void add_route(const std::string&, HandlerFunc);

    std::string handle_request(const std::string&) const;

private:
    std::unordered_map<std::string, HandlerFunc> routes_;
};

// Interface for making HTTP requests
class IHttpResponseBuilder {
public:
    virtual ~IHttpResponseBuilder() = default;
    virtual std::string make_http_response(const std::string& content, const std::string& status = "200 OK", const std::string& content_type = "text/html") const = 0;
    virtual std::string make_404_response() const = 0;
};


// Realization IHttpResponseBuilder
class HttpResponseBuilder : public IHttpResponseBuilder {
public:
    std::string make_http_response(const std::string&, const std::string&, const std::string&) const override;

    std::string make_404_response() const override;

private:
    static std::string get_current_time();
};

// Interface for handling File Manager
class IFileHandler {
public:
    virtual ~IFileHandler() = default;
    virtual std::string serve_file(const std::string& file_path, const std::string& content_type) const = 0;
    virtual bool file_exists(const std::string& file_path) const = 0;
};

// Realization IFileHandler
class FileHandler : public IFileHandler {
public:
    std::string serve_file(const std::string&, const std::string&) const override;

    bool file_exists(const std::string&) const override;
};

// Interface for handling Sessinos
class ISession {
public:
    virtual ~ISession() = default;
    virtual void start() = 0;
};

// Realization ISession
class Session : public ISession {
public:
    Session(
    tcp::socket socket_, 
    std::shared_ptr<IHttpResponseBuilder> response_builder_, 
    std::shared_ptr<IFileHandler> file_handler_,
    std::shared_ptr<Router> router_)
        : 
        socket(std::move(socket_)), 
        router(std::move(router_)),
        response_builder(std::move(response_builder_)), 
        file_handler(std::move(file_handler_)) {};

    void start() override;

private:
    std::shared_ptr<Router> router;
    tcp::socket socket;
    std::shared_ptr<IHttpResponseBuilder> response_builder;
    std::shared_ptr<IFileHandler> file_handler;
};

// Class for Server handling
class Server {
public:
    Server(
    short port_, 
    std::shared_ptr<IHttpResponseBuilder> response_builder_, 
    std::shared_ptr<IFileHandler> file_handler_)
        :
        io_context(std::make_shared<boost::asio::io_context>()), 
        acceptor(*io_context, tcp::endpoint(tcp::v4(), port_)), 
        response_builder(std::move(response_builder_)), 
        file_handler(std::move(file_handler_)) {}

    void run(std::shared_ptr<Router>);

private:

    
    std::shared_ptr<boost::asio::io_context> io_context;
    tcp::acceptor acceptor;
    std::shared_ptr<IHttpResponseBuilder> response_builder;
    std::shared_ptr<IFileHandler> file_handler;
};

void setup_routes(std::shared_ptr<Router>, std::shared_ptr<FileHandler>, std::shared_ptr<HttpResponseBuilder>);

}