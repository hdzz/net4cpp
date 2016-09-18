#pragma once

#include <functional>
#include <thread>
#include <unordered_map>
#include "std/extension.hpp"
#include "net/acceptor.hpp"

using namespace std;
using namespace std::string_literals;
using namespace std::chrono_literals;

namespace http {

    class controller
    {
    public:

        using callback = std::function<const std::string&()>;

        void response(const std::string& view)
        {
            m_callback = [&](){return view;};
        }

        void response(callback cb)
        {
            m_callback = cb;
        }

        std::string render()
        {
            return m_callback();
        }

    private:
        callback m_callback = [](){return "HTTP/1.1 404 Not Found"s;};
    };

    class server
    {
    public:

        controller& get(const std::string& path)
        {
            return m_router[path]["GET"s];
        }

        controller& head(const std::string& path)
        {
            return m_router[path]["HEAD"s];
        }

        controller& post(const std::string& path)
        {
            return m_router[path]["POST"s];
        }

        controller& put(const std::string& path)
        {
            return m_router[path]["PUT"s];
        }

        controller& patch(const std::string& path)
        {
            return m_router[path]["PATCH"s];
        }

        controller& destroy(const std::string& path)
        {
            return m_router[path]["DELETE"s];
        }

        void listen(const std::string& serice_or_port = "http"s)
        {
            auto acceptor = net::acceptor{"localhost"s, serice_or_port};
            acceptor.timeout(1h);
            while(true)
            {
                auto connection = acceptor.accept();
                auto worker = std::thread{[&](){handle(std::move(connection));}};
                worker.detach();
            }
        }

    private:

        void handle(net::endpointstream connection)
        {
            using namespace std;

            while(connection)
            {
                auto method = ""s, uri = ""s, version = ""s;
                connection >> method >> uri >> version;

                clog << method << ' ' << uri << ' ' << version;

                connection >> ws;

                while(connection && connection.peek() != '\r')
                {
                    auto name = ""s, value = ""s;
                    getline(connection, name, ':');
                    ext::trim(name);
                    getline(connection, value);
                    ext::trim(value);
                    clog << name << ": " << value << endl;
                }

                const auto content = m_router[uri][method].render();

                connection << "HTTP/1.1 200 OK"                                                   << net::crlf
                           << "Date: " << ext::to_rfc1123(chrono::system_clock::now())            << net::crlf
                           << "Server: YARESTDB/0.1"                                              << net::crlf
                           << "Access-Control-Allow-Origin: *"                                    << net::crlf
                           << "Access-Control-Allow-Methods: HEAD, GET, POST, PUT, PATCH, DELETE" << net::crlf
                           << "Content-Type: text/html; charset=UTF-8"                            << net::crlf
                           << "Content-Length: " << content.length()                              << net::crlf
                           << net::crlf
                           << (method != "HEAD"s ? content : ""s) << net::flush;
            }
        }

        using router = std::unordered_map<std::string,std::unordered_map<std::string,controller>>;

        router m_router;
    };

} // namespace http
