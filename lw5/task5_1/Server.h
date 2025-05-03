#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include <mutex>

using boost::asio::ip::tcp;

class Server
{
public:
    explicit Server(unsigned short port) : m_port(port),
                                           m_acceptor(m_ioContext, tcp::endpoint(tcp::v4(), port)) {}

    void Run()
    {
        std::cout << "Server running on port " << m_port << "\n";
        AcceptClient();
        m_ioContext.run();
    }

    [[nodiscard]] std::vector<tcp::socket*>& GetClients() const
    {
        static std::vector<tcp::socket*> clientRefs;
        clientRefs.clear();
        for (const auto& sock : m_clients)
        {
            clientRefs.push_back(sock.get());
        }
        return clientRefs;
    }

private:
    void AcceptClient()
    {
        auto socket = std::make_unique<tcp::socket>(m_ioContext);
        m_acceptor.async_accept(
            *socket, [this, socketPtr = socket.get()](const boost::system::error_code& error) mutable {
                if (!error)
                {
                    std::lock_guard lock(m_mutex);
                    std::cout << "Client connected: " << socketPtr->remote_endpoint().address() << "\n";
                    m_clients.push_back(std::move(socketPtr));
                }
                AcceptClient(); // рекурсивно принимать следующих
            });
    }

    unsigned short m_port;
    boost::asio::io_context m_ioContext;
    tcp::acceptor m_acceptor;
    std::vector<std::unique_ptr<tcp::socket>> m_clients;
    std::mutex m_mutex;
};
