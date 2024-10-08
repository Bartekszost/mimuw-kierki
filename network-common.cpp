#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdexcept>
#include <deque>
#include <memory>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "network-common.h"

static std::pair<std::string, int> get_host_and_port_info(struct addrinfo *p)
{
    char ip_str[INET6_ADDRSTRLEN];
    void *addr;
    int port;

    if (p->ai_family == AF_INET)
    {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        addr = &(ipv4->sin_addr);
        port = ntohs(ipv4->sin_port);
    }
    else
    {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
        addr = &(ipv6->sin6_addr);
        port = ntohs(ipv6->sin6_port);
    }

    inet_ntop(p->ai_family, addr, ip_str, sizeof ip_str);

    return {ip_str, port};
}

static struct addrinfo *get_local_addr_info(int sockfd)
{
    struct sockaddr_storage ss;
    socklen_t len = sizeof ss;

    if (getsockname(sockfd, (struct sockaddr *)&ss, &len) == -1)
        throw std::runtime_error(strerror(errno));

    struct addrinfo *res = new addrinfo;
    memset(res, 0, sizeof(struct addrinfo));

    if (ss.ss_family == AF_INET)
    {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)&ss;
        res->ai_family = AF_INET;
        res->ai_addrlen = sizeof(struct sockaddr_in);
        res->ai_addr = (struct sockaddr *)new sockaddr_in;
        memcpy(res->ai_addr, ipv4, sizeof(struct sockaddr_in));
    }
    else
    {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&ss;
        res->ai_family = AF_INET6;
        res->ai_addrlen = sizeof(struct sockaddr_in6);
        res->ai_addr = (struct sockaddr *)new sockaddr_in6;
        memcpy(res->ai_addr, ipv6, sizeof(struct sockaddr_in6));
    }

    return res;
}

uint16_t read_port(const char *port)
{
    char *end;
    long port_number = strtol(port, &end, 10);
    if (*end != '\0' || port_number < 0 || port_number > UINT16_MAX)
        throw std::invalid_argument("Invalid port number");
    return port_number;
}

long long get_current_time_in_millis()
{
    auto epoch = std::chrono::system_clock::now().time_since_epoch();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
    return duration_ms;
}

std::string current_time_to_string()
{
    auto now = std::chrono::system_clock::now();
    auto itt = std::chrono::system_clock::to_time_t(now);
    std::tm bt = *std::localtime(&itt);

    auto duration_ms = get_current_time_in_millis();
    auto ms = duration_ms % 1000;

    std::stringstream ss;
    ss << std::put_time(&bt, "%FT%T")
       << '.' << std::setfill('0') << std::setw(3) << ms;

    return ss.str();
}

Socket::Socket(const char *host, uint16_t port, IPVersion ip_version, bool verbose)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));

    switch (ip_version)
    {
    case IPVersion::IPv4:
        hints.ai_family = AF_INET;
        break;
    case IPVersion::IPv6:
        hints.ai_family = AF_INET6;
        break;
    default:
        hints.ai_family = AF_UNSPEC;
    }

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int status = getaddrinfo(host, std::to_string(port).c_str(), &hints, &res);
    if (status != 0)
        throw std::runtime_error(gai_strerror(status));

    this->verbose = verbose;
    closed = false;
    all_messages_received = false;
    all_messages_received = true;

    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next)
    {
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd == -1)
            continue;

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) != -1)
            break;

        close(socket_fd);
    }

    if (p == NULL)
        throw std::runtime_error("Failed to connect");

    std::tie(sender_ip, sender_port) = get_host_and_port_info(p);

    freeaddrinfo(res);

    res = get_local_addr_info(socket_fd);

    std::tie(receiver_ip, receiver_port) = get_host_and_port_info(res);

    freeaddrinfo(res);

    read_queue = std::deque<char>();
    write_queue = std::deque<char>();

    if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error(strerror(errno));
}

Socket::Socket(uint16_t port, bool verbose)
{
    this->verbose = verbose;
    closed = false;
    all_messages_received = false;
    all_messages_sent = false;

    struct addrinfo hints, *res, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    int status = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &res);
    if (status != 0)
        throw std::runtime_error(gai_strerror(status));

    for (p = res; p != NULL; p = p->ai_next)
    {
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd == -1)
            continue;

        if (p->ai_family == AF_INET6)
        {
            int off = 0;
            if (setsockopt(socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof(off)) == -1)
            {
                close(socket_fd);
                continue;
            }
        }

        if (bind(socket_fd, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(socket_fd);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        throw std::runtime_error("Failed to bind");
    }

    addrinfo *local_addr_info = get_local_addr_info(socket_fd);
    std::tie(sender_ip, sender_port) = get_host_and_port_info(local_addr_info);
    freeaddrinfo(res);
    freeaddrinfo(local_addr_info);

    if (listen(socket_fd, 5) < 0)
    {
        throw std::runtime_error("Listen failed");
    }

    std::cerr << "Listening on " << sender_ip << ':' << sender_port << '\n';
}

Socket::Socket(int socket_fd, const std::string &sender_ip, uint16_t sender_port, const std::string &receiver_ip, uint16_t receiver_port, bool verbose)
{
    this->socket_fd = socket_fd;
    this->sender_ip = sender_ip;
    this->sender_port = sender_port;
    this->receiver_ip = receiver_ip;
    this->receiver_port = receiver_port;
    this->verbose = verbose;
    closed = false;
    all_messages_received = false;
    all_messages_sent = false;
    read_queue = std::deque<char>();
    write_queue = std::deque<char>();

    if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error(strerror(errno));
}

Socket::~Socket()
{
    close(socket_fd);
}

std::shared_ptr<Socket> Socket::accept_connection()
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    int new_socket_fd = accept(socket_fd, (struct sockaddr *)&their_addr, &addr_size);
    if (new_socket_fd == -1)
        throw std::runtime_error(strerror(errno));

    struct addrinfo *res = new addrinfo;
    memset(res, 0, sizeof(struct addrinfo));

    if (their_addr.ss_family == AF_INET)
    {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)&their_addr;
        res->ai_family = AF_INET;
        res->ai_addrlen = sizeof(struct sockaddr_in);
        res->ai_addr = (struct sockaddr *)new sockaddr_in;
        memcpy(res->ai_addr, ipv4, sizeof(struct sockaddr_in));
    }
    else
    {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&their_addr;
        res->ai_family = AF_INET6;
        res->ai_addrlen = sizeof(struct sockaddr_in6);
        res->ai_addr = (struct sockaddr *)new sockaddr_in6;
        memcpy(res->ai_addr, ipv6, sizeof(struct sockaddr_in6));
    }

    std::string sender_ip, receiver_ip;
    uint16_t sender_port, receiver_port;

    std::tie(sender_ip, sender_port) = get_host_and_port_info(res);

    res = get_local_addr_info(new_socket_fd);

    std::tie(receiver_ip, receiver_port) = get_host_and_port_info(res);

    freeaddrinfo(res);

    std::cerr << "Accepted connection from " << sender_ip << ':' << sender_port << '\n';

    return std::make_shared<Socket>(new_socket_fd, sender_ip, sender_port, receiver_ip, receiver_port, verbose);
}

void Socket::send(const std::string &message)
{
    write_queue.insert(write_queue.end(), message.begin(), message.end());
    if (verbose)
    {
        std::cout << '['
                  << receiver_ip
                  << ':'
                  << receiver_port
                  << ','
                  << sender_ip
                  << ':'
                  << sender_port
                  << ','
                  << current_time_to_string()
                  << "] "
                  << message;
    }
}

std::string Socket::extract_message()
{
    std::string message;
    auto it = read_queue.begin();
    while (it != read_queue.end() && message.size() < MAX_MESSAGE_SIZE)
    {
        message += *it;
        if (*it == '\n')
            break;
        ++it;
    }

    if (!message.empty() && (message.size() == MAX_MESSAGE_SIZE || message.back() == '\n'))
    {
        read_queue.erase(read_queue.begin(), it + 1);
    }
    else
    {
        if (closed)
            all_messages_received = true;
        message.clear();
    }

    if (verbose && !message.empty())
        std::cout << '['
                  << sender_ip
                  << ':'
                  << sender_port
                  << ','
                  << receiver_ip
                  << ':'
                  << receiver_port
                  << ','
                  << current_time_to_string()
                  << "] "
                  << message;

    return message;
}

void Socket::handle_write()
{
    if (closed && all_messages_sent)
        return;

    if (write_queue.empty())
    {
        if (closed)
            all_messages_sent = true;
        return;
    }

    std::vector<char> buffer(write_queue.begin(), write_queue.end());
    ssize_t bytes_sent = ::send(socket_fd, buffer.data(), buffer.size(), 0);
    if (bytes_sent > 0)
        write_queue.erase(write_queue.begin(), write_queue.begin() + bytes_sent);
    else if (bytes_sent == 0)
        closed = true;
    else if (bytes_sent < 0 && errno != EWOULDBLOCK)
        throw std::runtime_error(strerror(errno));
}

void Socket::handle_read()
{
    if (closed)
        return;

    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received = ::recv(socket_fd, buffer, sizeof(buffer), 0);
    if (bytes_received > 0)
        read_queue.insert(read_queue.end(), buffer, buffer + bytes_received);
    else if (bytes_received == 0)
        closed = true;
    else if (bytes_received < 0 && errno != EWOULDBLOCK)
        throw std::runtime_error(strerror(errno));
}

void Socket::set_timeout(int seconds)
{
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1)
        throw std::runtime_error(strerror(errno));
}

void Socket::await_message(MessageType type, int timeout)
{
    awaited_message = type;
    timestamp = get_current_time_in_millis() + timeout * 1000;
}

bool Socket::is_timed_out() const
{
    if (!awaited_message.has_value())
        return false;

    auto current_time = get_current_time_in_millis();
    return current_time > timestamp;
}