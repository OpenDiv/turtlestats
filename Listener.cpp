#include "Listener.h"
#include <iostream>
#include <fstream>

extern HWND hStaticConnection;
extern HWND hStaticControl;
extern HWND hStaticLastMsg;
extern HWND hEditChat;


std::mutex vectorOperation;
extern std::vector < std::shared_ptr<EchoWebSocket>> socketList;

Listener::Listener(boost::asio::io_context& ioc, unsigned short int port)
    : ioc(ioc), acceptor(ioc, { boost::asio::ip::make_address("127.0.0.1"), port }) {}

void Listener::asyncAccept()
{
    acceptor.async_accept(ioc, [self{ shared_from_this() }](boost::system::error_code ec, tcp::socket socket) {
        // pass socket to EchoWebSocket
        
        std::shared_ptr<EchoWebSocket> curSocket = std::make_shared<EchoWebSocket>(std::move(socket));
        curSocket->run();
        socketList.push_back(curSocket);

        SetWindowTextA(hStaticConnection, std::to_string(socketList.size()).c_str());

        self->asyncAccept();
        });
}

EchoWebSocket::EchoWebSocket(tcp::socket&& socket) : ws(std::move(socket)) {}

void EchoWebSocket::send(std::string text)
{
    ws.write(net::buffer(text));
}

void EchoWebSocket::close()
{
    boost::beast::websocket::close_reason cr;
    boost::system::error_code ec;
}

void EchoWebSocket::run()
{
    ws.async_accept([self{ shared_from_this() }](beast::error_code ec) {
        if (ec)
        {
            //cout ec.message()
            return;
        }
        self->echo();
        });
}

void EchoWebSocket::echo()
{
    ws.async_read(buffer, [self{ shared_from_this() }](beast::error_code ec,
        std::size_t bytes_transferred) {

            
            if (ec == websocket::error::closed || ec)
            {
                self->setAliveStatus(false);
                auto deadElement = std::remove_if(socketList.begin(), socketList.end(), [](const std::shared_ptr<EchoWebSocket> it) {
                    return it->wsAlive == false; 
                });
                socketList.erase(deadElement, socketList.end());
                SetWindowTextA(hStaticConnection, std::to_string(socketList.size()).c_str());
                return;
            }
            
            //  disconnected

            
            auto out = beast::buffers_to_string(self->buffer.cdata());
            out += "\r\n";
            std::wstring widestr = std::wstring(out.begin(), out.end());
            SetWindowTextA(hStaticLastMsg, out.c_str());

            SendMessage(hEditChat, EM_REPLACESEL, TRUE, (LPARAM)widestr.c_str());
            //temp log file
            std::ofstream logTurtleFile(out.substr(1, out.find("]")-1)+".txt", std::ios::app);
            if (logTurtleFile)
                logTurtleFile << out.substr(out.find("]") + 2)<<std::endl;
            logTurtleFile.close();
            //std::cout << out << std::endl;

            self->ws.async_write(self->buffer.data(), [self](beast::error_code ec, std::size_t bytes_transferred) {
                if (ec)
                {
                    //std::cout << ec.message() << "\n";
                    return;
                }
                self->buffer.consume(self->buffer.size());
                self->echo();
                });
        });
}

void EchoWebSocket::setAliveStatus(bool status)
{
    this->wsAlive = status;
}

bool EchoWebSocket::isAlive()
{
    return wsAlive;
}

