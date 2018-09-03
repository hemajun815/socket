#include <iostream>
#include <cstring>
#include <exception>
#include <thread>
#include <mutex>
#include <winsock2.h>

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

class ClientStartException : public std::exception
{
  public:
    const char *what() const throw()
    {
        return "Client start error!";
    }
};

class Client
{
  private:
    int m_timeout = 1000;
    static bool m_is_work;

  private:
    std::string m_host;
    int m_port;
    SOCKET m_socketfd;

  private:
    void start_client();
    static void recv_message(const SOCKET &fd);
    static void send_message(const SOCKET &fd);

  public:
    Client(const std::string &host, const int &port);
    ~Client();
    void run();
};

Client::Client(const std::string &host, const int &port) : m_host(host), m_port(port)
{
}

Client::~Client()
{
    closesocket(this->m_socketfd);
}

bool Client::m_is_work = true;

void Client::start_client()
{
    auto wVersionRequested = MAKEWORD(1, 1);
    WSADATA wsaData;
    if (WSAStartup(wVersionRequested, &wsaData))
    {
        throw ClientStartException();
    }
    if (1 != LOBYTE(wsaData.wVersion) || 1 != HIBYTE(wsaData.wVersion))
    {
        WSACleanup();
        throw ClientStartException();
    }
    this->m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->m_socketfd <= 0)
        throw ClientStartException();
    setsockopt(this->m_socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&this->m_timeout, sizeof(int)); // 发送时延
    setsockopt(this->m_socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&this->m_timeout, sizeof(int)); // 接收时延
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(sockaddr_in));
    server_addr.sin_addr.S_un.S_addr = inet_addr(this->m_host.data());
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(this->m_port);
    if (0 != connect(this->m_socketfd, (sockaddr *)&server_addr, sizeof(sockaddr)))
        throw ClientStartException();
}

void Client::recv_message(const SOCKET &fd)
{
    char *buf = new char[1024 + 1]();
    while (m_is_work)
    {
        memset(buf, 0, 1024 + 1);
        if (0 < recv(fd, buf, 1024, 0))
        {
            buf[1024] = '\0';
            std::cout << buf << std::endl;
        }
    }
}

void Client::send_message(const SOCKET &fd)
{
    std::string tmp;
    while (m_is_work)
    {
        std::cin >> tmp;
        if (0 == tmp.compare("exit"))
            m_is_work = false;
        else
            send(fd, tmp.data(), tmp.length(), 0);
    }
}

void Client::run()
{
    this->start_client();
    auto t_recv = new std::thread(Client::recv_message, this->m_socketfd);
    auto t_send = new std::thread(Client::send_message, this->m_socketfd);
    t_recv->join();
    t_send->join();
}

int arg_parse(const int &argc, const char *argv[], std::string &host, int &port)
{
    for (auto i = 1; i < argc; i++)
    {
        if (0 == strcmp("-host", argv[i]))
            host = std::string(argv[++i]);
        else if (0 == strcmp("-port", argv[i]))
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
    std::string host;
    auto port = 0;
    if (5 != argc || -1 == arg_parse(argc, argv, host, port))
    {
        std::cout << "Usage: " << argv[0] << " -host <host> -port <port>" << std::endl
                  << "\t-host: server port;" << std::endl
                  << "\t-port: server port;" << std::endl;
        return -1;
    }
    auto client = new Client(host, port);
    try
    {
        client->run();
    }
    catch (const ClientStartException &e)
    {
        std::cout << e.what() << std::endl;
    }
    delete client;
    return 0;
}
