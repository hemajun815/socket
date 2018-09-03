#include <iostream>
#include <unordered_map>
#include <queue>
#include <exception>
#include <algorithm>
#include <winsock2.h>

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

class ServerStartException : public std::exception
{
  public:
    const char *what() const throw()
    {
        return "Server start error!";
    }
};

class Server
{
  private:
    timeval m_select_tv = {2, 0};
    int m_num_max_clients = 100;
    int m_timeout = 1000;

  private:
    int m_port;
    SOCKET m_socketfd;
    std::unordered_map<SOCKET, std::string> *m_clients;
    std::unordered_map<SOCKET, std::queue<std::string>> *m_messages;

  private:
    void start_server();
    void accept_new_connection();
    void recv_message(const SOCKET &fd);
    void send_message(const SOCKET &fd);
    void disconnect_connection(const SOCKET &fd);

  public:
    Server(const int &port);
    ~Server();
    void select_loop();
};

Server::Server(const int &port) : m_port(port)
{
    this->m_clients = new std::unordered_map<SOCKET, std::string>();
    this->m_messages = new std::unordered_map<SOCKET, std::queue<std::string>>();
}

Server::~Server()
{
    if (this->m_clients)
    {
        this->m_clients->clear();
        delete this->m_clients;
    }
    if (this->m_messages)
    {
        this->m_messages->clear();
        delete this->m_messages;
    }
}

void Server::start_server()
{
    auto w_version_requested = MAKEWORD(1, 1);
    WSADATA wsa_data;
    if (0 != WSAStartup(w_version_requested, &wsa_data))
    {
        throw ServerStartException();
    }
    if (1 != LOBYTE(wsa_data.wVersion) || 1 != HIBYTE(wsa_data.wVersion))
    {
        WSACleanup();
        throw ServerStartException();
    }
    this->m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->m_socketfd <= 0)
        throw ServerStartException();
    const auto reuse_addr = true;
    setsockopt(this->m_socketfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse_addr, sizeof(bool)); // 地址重用
    setsockopt(this->m_socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&this->m_timeout, sizeof(int));    // 发送时延
    setsockopt(this->m_socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&this->m_timeout, sizeof(int));    // 接收时延
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(sockaddr_in));
    server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(this->m_port);
    if (0 != bind(this->m_socketfd, (sockaddr *)&server_addr, sizeof(sockaddr)))
    {
        WSACleanup();
        throw ServerStartException();
    }
    if (0 != listen(this->m_socketfd, this->m_num_max_clients))
    {
        WSACleanup();
        throw ServerStartException();
    }
}

void Server::accept_new_connection()
{
    sockaddr_in addr_client;
    auto len = sizeof(sockaddr_in);
    memset(&addr_client, 0, len);
    SOCKET fd = accept(this->m_socketfd, (SOCKADDR *)&addr_client, (int *)&len);
    if (0 < fd)
    {
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&this->m_timeout, sizeof(int)); // 发送时延
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&this->m_timeout, sizeof(int)); // 接收时延
        this->m_clients->erase(fd);
        this->m_clients->insert(std::make_pair<>(fd, inet_ntoa(addr_client.sin_addr)));
        this->m_messages->erase(fd);
        this->m_messages->insert(std::make_pair<>(fd, std::queue<std::string>()));
        std::cout << "Accepted a connection of " << inet_ntoa(addr_client.sin_addr) << std::endl;
    }
}

void Server::recv_message(const SOCKET &fd)
{
    const auto buf_size = 1024;
    char *buf = new char[buf_size]();
    auto len_data = recv(fd, buf, buf_size, 0);
    if (len_data > 0) // 收到消息，加入消息队列。
    {
        buf[len_data] = '\0';
        std::string mess = this->m_clients->at(fd) + " | " + std::string(buf);
        for (auto it = this->m_clients->begin(); it != this->m_clients->end(); it++)
            this->m_messages->at(it->first).push(mess);
    }
}

void Server::send_message(const SOCKET &fd)
{
    if (!this->m_messages->at(fd).empty())
    {
        auto mess = this->m_messages->at(fd).back();
        this->m_messages->at(fd).pop();
        send(fd, mess.data(), mess.length(), 0);
    }
}

void Server::disconnect_connection(const SOCKET &fd)
{
    closesocket(fd);
    this->m_clients->erase(fd);
    this->m_messages->erase(fd);
    std::cout << this->m_clients->at(fd) << " disconnected." << std::endl;
}

void Server::select_loop()
{
    this->start_server();
    fd_set rfds, wfds, efds;
    while (true)
    {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_ZERO(&efds);
        FD_SET(this->m_socketfd, &rfds);
        for (auto it = this->m_clients->begin(); it != this->m_clients->end(); it++)
        {
            FD_SET(it->first, &rfds);
            FD_SET(it->first, &wfds);
            FD_SET(it->first, &efds);
        }
        switch (select(this->m_clients->size() + 1, &rfds, &wfds, &efds, &this->m_select_tv))
        {
        case 0:
            continue;
        case SOCKET_ERROR:
            std::cout << "Select Error(" << WSAGetLastError() << ")." << std::endl;
            break;
        default:
            if (FD_ISSET(this->m_socketfd, &rfds))
                this->accept_new_connection();
            for (auto it = this->m_clients->begin(); it != this->m_clients->end(); it++)
            {
                auto fd = it->first;
                if (FD_ISSET(fd, &rfds))
                    this->recv_message(fd);
                if (FD_ISSET(fd, &wfds))
                    this->send_message(fd);
                if (FD_ISSET(fd, &efds))
                    this->disconnect_connection(fd);
            }
        }
    }
}

int arg_parse(const int &argc, const char *argv[], int &port)
{
    for (auto i = 1; i < argc; i++)
    {
        if (0 == strcmp("-port", argv[i]))
            port = atoi(argv[++i]);
        else
        {
            std::cout << "Cannot parse the paremeter: " << argv[i] << "." << std::endl;
            return -1;
        }
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    auto port = 0;
    if (3 != argc || -1 == arg_parse(argc, argv, port))
    {
        std::cout << "Usage: " << argv[0] << " -port <port>" << std::endl
                  << "\t-port: server port;" << std::endl;
        return -1;
    }
    auto server = new Server(port);
    try
    {
        server->select_loop();
    }
    catch (const ServerStartException &e)
    {
        std::cout << e.what() << std::endl;
    }
    delete server;
    return 0;
}
