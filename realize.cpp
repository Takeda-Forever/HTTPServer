#include "function.h"

namespace WebCore {
void Server::run(std::shared_ptr<Router> router)
{
        for (;;) {
            tcp::socket socket(*io_context);
            acceptor.accept(socket);
            std::make_shared<Session>(std::move(socket), response_builder, file_handler, router)->start();
        }
}
std::string HttpResponseBuilder::make_http_response(const std::string& content, const std::string& status = "200 OK", const std::string& content_type = "text/html") const
{
        return "HTTP/1.1 " + status + "\r\n" +
               "Content-Type: " + content_type + "; charset=UTF-8" + "\r\n" +
               "Content-Length: " + std::to_string(content.size()) + "\r\n" +
               "Date: " + get_current_time() + "\r\n" +
               "\r\n" + content;
}

std::string HttpResponseBuilder::make_404_response() const
 {
    return make_http_response("<html><body><h1>404 Not Found</h1></body></html>", "404 Not Found");
}
std::string HttpResponseBuilder::get_current_time()
 {
    auto end = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    return std::string(std::ctime(&end_time));
}

std::string FileHandler::serve_file(const std::string& file_path, const std::string& content_type) const 
{
        std::ifstream file(file_path);
        if (!file) {
            return HttpResponseBuilder().make_404_response();
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return HttpResponseBuilder().make_http_response(buffer.str(), "200 OK", content_type);
}
bool FileHandler::file_exists(const std::string &file_path) const
 {
        std::ifstream file(file_path);
        return file.good();
}

void Session::start() {
    try {
        char data[1024];
        boost::system::error_code error;
        size_t length = socket.read_some(boost::asio::buffer(data), error);

        if (error == boost::asio::error::eof) return;  // Соединение закрыто
        if (error) throw boost::system::system_error(error);

        std::string request(data, length);
        std::string path;

        // Извлекаем путь из запроса
        if (request.find("GET") == 0) {
            size_t start = request.find(" ") + 1;
            size_t end = request.find(" ", start);
            path = request.substr(start, end - start);
        }

        std::string response = router->handle_request(path);
        if (response.empty()) {
            response = response_builder->make_404_response();
        }

        boost::asio::write(socket, boost::asio::buffer(response), error);
    } catch (std::exception& e) {
        std::cerr << "Exception in session: " << e.what() << std::endl;
        std::string response = response_builder->make_http_response("<html><body><h1>500 Internal Server Error</h1></body></html>", "500 Internal Server Error");
        boost::system::error_code ignored_error;
        boost::asio::write(socket, boost::asio::buffer(response), ignored_error);
    }
}

std::string Router::handle_request(const std::string& path) const
{
    if (routes.count(path)) {
        return routes.at(path)();
    }
    return "";  // Пустая строка для несуществующих маршрутов
}

void Router::add_route(const std::string& path, HandlerFunc handler) 
{
        routes[path] = handler;
}


void setup_routes(std::shared_ptr<Router> router, std::shared_ptr<FileHandler> file_handler, std::shared_ptr<HttpResponseBuilder> response_builder) {
    router->add_route("/", [file_handler, response_builder]() {
        if (file_handler->file_exists("index.html")) {
            return file_handler->serve_file("index.html", "text/html");
        }
        return response_builder->make_http_response("<html><body><h1>Welcome to My C++ Boost Web Server!</h1></body></html>");
    });

    router->add_route("/index.html", [file_handler]() {
        return file_handler->serve_file("index.html", "text/html");
    });

    router->add_route("/style.css", [file_handler, response_builder]() {
        if (file_handler->file_exists("style.css")) {
            return file_handler->serve_file("style.css", "text/css");
        }
        return response_builder->make_404_response();
    });

    // Add more routes...
    // Example: router.add_route("/file_path", [file_handler]() 
    // { 
    // return file_handler->serve_file("[file].[format]", "[fomat_type]"); 
    // });
}

}