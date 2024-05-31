#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <codecvt>
#include <time.h>
#include <algorithm>

namespace beast = boost::beast;
namespace net = boost::asio;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

class TweakUnit
{
public:
    
private:
    const std::string label; // имя компьютера, не менять
    bool available; // если проверка на соединения провалена, компьютер становится недоступен, пока проверка не будет пройдена
    bool disconnected; // если соединение было отключено
};

class EchoWebSocket : public std::enable_shared_from_this<EchoWebSocket>
{
    websocket::stream<beast::tcp_stream> ws;
    beast::flat_buffer buffer;
    bool wsAlive = true;

public:
    EchoWebSocket(tcp::socket&& socket);

    void close();
    void send(std::string text);
    void run();
    bool isAlive();
    void setAliveStatus(bool status);
    void echo();
private:
};

using tcp = boost::asio::ip::tcp;

class Listener : public std::enable_shared_from_this<Listener>
{
    boost::asio::io_context& ioc;
    tcp::acceptor acceptor;

public:
    Listener(boost::asio::io_context& ioc, unsigned short int port);
    void asyncAccept();
};

