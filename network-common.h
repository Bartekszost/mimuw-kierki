#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H

#include <cstdint>
#include <memory>
#include <netdb.h>
#include <deque>
#include <cstdint>
#include <optional>
#include "common.h"

#define MAX_MESSAGE_SIZE 50
#define MAX_BUFFER_SIZE 4096

/**
 * @brief IP version
 */
enum class IPVersion
{
    Unspecified,
    IPv4,
    IPv6
};

/**
 * @brief Read port number from string
 *
 * @param port Port number as string
 * @return Port number as uint16_t
 */
uint16_t read_port(const char *port);

/**
 * @brief Get current time in milliseconds
 *
 * @return Current time in milliseconds
 */
long long get_current_time_in_millis();

/**
 * @brief Get current time as string
 *
 * @return Current time as string
 */
std::string current_time_to_string();

/**
 * @brief Socket class
 */
class Socket
{
private:
    std::deque<char> read_queue;
    std::deque<char> write_queue;
    bool verbose;
    std::string sender_ip;
    uint16_t sender_port;
    std::string receiver_ip;
    uint16_t receiver_port;

public:
    int socket_fd;
    bool closed;
    bool all_messages_received;
    bool all_messages_sent;

    std::optional<MessageType> awaited_message;
    long long timestamp;

    /**
     * @brief Construct a new Socket object
     *
     * @param host Host name
     * @param port Port number
     * @param ip_version IP version
     * @param verbose Whether to print network logs
     */
    Socket(const char *host, uint16_t port, IPVersion ip_version, bool verbose = false);

    /**
     * @brief Construct a new Socket object (Server side)
     *
     * @param port Port number
     * @param verbose Whether to print network logs
     */
    Socket(uint16_t port, bool verbose = false);

    /**
     * @brief Construct a new Socket object
     *
     * @param socket_fd Socket file descriptor
     * @param sender_ip Sender IP address
     * @param sender_port Sender port number
     * @param receiver_ip Receiver IP address
     * @param receiver_port Receiver port number
     * @param verbose Whether to print network logs
     */
    Socket(int socket_fd, const std::string &sender_ip, uint16_t sender_port, const std::string &receiver_ip, uint16_t receiver_port, bool verbose = false);

    /**
     * @brief Destroy the Socket object
     */
    ~Socket();

    /**
     * @brief Accept connection on the server side
     * @return New socket object representing the connection
     */
    std::shared_ptr<Socket> accept_connection();

    /**
     * @brief Put data into the writing queue
     * @param message Data to be put into the writing queue
     */
    void send(const std::string &message);

    /**
     * @brief Read data from the reading queue
     * @return Data read from the reading queue
     */
    std::string extract_message();

    /**
     * @brief Write as many bytes as possible from the writing queue
     */
    void handle_write();

    /**
     * @brief Read as many bytes as possible from the socket
     */
    void handle_read();

    /**
     * @brief Set timeout on the socket
     * @param seconds timeout in seconds
     */
    void set_timeout(int seconds);

    /**
     * @brief Set message to be awaited and timestamp
     * @param type message type to be awaited
     * @param timeout timeout in seconds
     */
     void await_message(MessageType type, int timeout);

    /**
    * @brief Check if the socket has timed out
     *
     * @return Is timed out
    */
     bool is_timed_out() const;
};

#endif // NETWORK_COMMON_H