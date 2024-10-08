#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <poll.h>
#include <cstring>
#include <vector>

#include "network-common.h"
#include "server-game-state.h"

struct Args
{
    uint16_t port;
    std::string file;
    int timeout;
};

Args parse_args(int argc, char *argv[])
{
    const char *port = nullptr;
    Args args;
    args.port = 0;
    args.file = "";
    args.timeout = 5;

    int opt;
    while ((opt = getopt(argc, argv, "p:f:t:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            port = optarg;
            break;
        case 'f':
            args.file = optarg;
            break;
        case 't':
            args.timeout = std::atoi(optarg);
            break;
        default:
            std::cerr << "Usage: " << argv[0] << " -p port -f file -t timeout" << std::endl;
            std::exit(1);
        }
    }

    if (port != nullptr)
        args.port = read_port(port);

    if (args.file.empty())
    {
        std::cerr << "Usage: " << argv[0] << " -p port -f file -t timeout" << std::endl;
        std::exit(1);
    }

    return args;
}

int update_timeout(int timeout, std::shared_ptr<Socket> &client_socket)
{
    if (!client_socket->awaited_message.has_value() || client_socket->closed)
        return timeout;

    auto millis = get_current_time_in_millis();
    auto remaining_time = client_socket->timestamp - millis;

    if (remaining_time <= 0)
        return timeout;

    if (timeout == -1 || remaining_time < timeout)
        return static_cast<int>(remaining_time);

    return timeout;
}

void handle_messages(std::shared_ptr<Socket> &client_socket, ServerGameState &game_state)
{
    while (true)
    {
        std::string message_str = client_socket->extract_message();
        if (message_str.empty())
            break;

        try
        {
            std::shared_ptr<Message> message_ptr = Message::from_string(message_str);

            if (message_ptr->type == MessageType::IAM)
            {
                IAMMessage &iam_message = dynamic_cast<IAMMessage &>(*message_ptr);
                Position position = iam_message.position;

                std::optional<BUSYMessage> busy_message = game_state.new_player(position, client_socket);
                if (busy_message.has_value())
                {
                    client_socket->send(busy_message->to_string());
                    client_socket->closed = true;
                }
            }
            if (message_ptr->type == MessageType::TRICK) {
                TRICKMessage &trick_message = dynamic_cast<TRICKMessage &>(*message_ptr);

                std::optional<WRONGMessage> wrong_message = game_state.handle_trick_message(client_socket, trick_message);
                if (wrong_message.has_value())
                    client_socket->send(wrong_message->to_string());
            }
        }
        catch (std::invalid_argument &e)
        {
            std::cerr << "Invalid message: " << e.what() << '\n';
        }
    }

    if(client_socket->awaited_message == MessageType::IAM && client_socket->is_timed_out())
        client_socket->closed = true;

    if(client_socket->awaited_message == MessageType::TRICK && client_socket->is_timed_out() && game_state.find_position(client_socket).has_value())
        game_state.send_trick_message(game_state.find_position(client_socket).value());
}

void run_server(uint16_t port, const std::string &file, int timeout)
{
    Socket main_socket = Socket(port, true);
    ServerGameState game_state(file, timeout);

    std::vector<pollfd> fds;
    pollfd main_socket_pollfd;
    main_socket_pollfd.fd = main_socket.socket_fd;
    main_socket_pollfd.events = POLLIN;
    main_socket_pollfd.revents = 0;

    fds.push_back(main_socket_pollfd);

    std::vector<std::shared_ptr<Socket>> client_sockets;

    int poll_timeout = -1;

    while (!game_state.can_end_server())
    {
        int poll_result = poll(fds.data(), fds.size(), poll_timeout);

        if (poll_result == -1)
            throw std::runtime_error(strerror(errno));
        else if (poll_result == 0)
        {
            for (auto client_socket: client_sockets)
                handle_messages(client_socket, game_state);

            game_state.continue_game();
            continue;
        }
        else
        {
            if (fds[0].revents & POLLIN)
            {
                std::shared_ptr<Socket> client_socket_ptr = main_socket.accept_connection();
                client_sockets.push_back(client_socket_ptr);
                Socket &client_socket = *client_sockets.back();

                pollfd client_socket_pollfd;
                client_socket_pollfd.fd = client_socket.socket_fd;
                client_socket_pollfd.events = POLLIN | POLLOUT | POLLHUP | POLLERR;
                client_socket_pollfd.revents = 0;

                fds.push_back(client_socket_pollfd);

                client_socket.await_message(MessageType::IAM, timeout);
            }
        }

        std::vector<std::shared_ptr<Socket>> new_client_sockets;
        poll_timeout = -1;

        for (size_t i = 1; i < fds.size(); i++)
        {
            if (fds[i].revents & POLLIN)
            {
                std::shared_ptr<Socket> &client_socket_ptr = client_sockets[i - 1];
                Socket &client_socket = *client_socket_ptr;

                client_socket.handle_read();
            }

            if (fds[i].revents & POLLHUP)
            {
                std::shared_ptr<Socket> &client_socket_ptr = client_sockets[i - 1];
                Socket &client_socket = *client_socket_ptr;

                client_socket.handle_read();
                client_socket.closed = true;

                std::cerr << "Client disconnected\n";
            }

            if (fds[i].revents & POLLOUT)
            {
                std::shared_ptr<Socket> &client_socket_ptr = client_sockets[i - 1];
                Socket &client_socket = *client_socket_ptr;

                client_socket.handle_write();
            }

            if (fds[i].revents & POLLERR)
            {
                std::shared_ptr<Socket> &client_socket_ptr = client_sockets[i - 1];
                Socket &client_socket = *client_socket_ptr;

                client_socket.handle_read();
                client_socket.closed = true;
            }

            handle_messages(client_sockets[i - 1], game_state);

            if (!client_sockets[i - 1]->closed || !client_sockets[i - 1]->all_messages_received || !client_sockets[i - 1]->all_messages_sent)
            {
                poll_timeout = update_timeout(poll_timeout, client_sockets[i - 1]);

                new_client_sockets.push_back(client_sockets[i - 1]);
            }
            else
            {
                game_state.disconnect_client(client_sockets[i - 1]);
            }
        }

        client_sockets = new_client_sockets;
        std::vector<pollfd> new_fds;
        new_fds.push_back(main_socket_pollfd);

        game_state.continue_game();

        for (auto client_socket : client_sockets)
        {
            pollfd client_socket_pollfd;
            client_socket_pollfd.fd = client_socket->socket_fd;
            client_socket_pollfd.events = POLLIN | POLLOUT | POLLHUP | POLLERR;
            client_socket_pollfd.revents = 0;

            new_fds.push_back(client_socket_pollfd);
        }

        fds = new_fds;
    }
}

int main(int argc, char *argv[])
{
    try
    {
        Args args = parse_args(argc, argv);
        run_server(args.port, args.file, args.timeout);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}