#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <cstring>

#include "network-common.h"
#include "client-game-state.h"

struct Args
{
    const char *host;
    uint16_t port;
    IPVersion ip_version;
    Position position;
    bool automatic;
};

Args parse_args(int argc, char *argv[])
{
    const char *port = nullptr;
    bool position_set = false;
    int opt;
    Args args;
    args.host = nullptr;
    args.port = 0;
    args.ip_version = IPVersion::Unspecified;
    args.position = Position::North;
    args.automatic = false;

    while ((opt = getopt(argc, argv, "h:p:46NESWa")) != -1)
    {
        switch (opt)
        {
        case 'h':
            args.host = optarg;
            break;
        case 'p':
            port = optarg;
            break;
        case '4':
            args.ip_version = IPVersion::IPv4;
            break;
        case '6':
            args.ip_version = IPVersion::IPv6;
            break;
        case 'N':
            args.position = Position::North;
            position_set = true;
            break;
        case 'E':
            args.position = Position::East;
            position_set = true;
            break;
        case 'S':
            args.position = Position::South;
            position_set = true;
            break;
        case 'W':
            args.position = Position::West;
            position_set = true;
            break;
        case 'a':
            args.automatic = true;
            break;
        default:

            throw std::runtime_error("Usage: " + std::string(argv[0]) + " -h <host> -p <port> [-4|-6] [-N|-E|-S|-W] [-a]");
        }
    }

    if (port != nullptr)
        args.port = read_port(port);

    if (args.host == nullptr || args.port == 0 || !position_set)
        throw std::runtime_error("Usage: " + std::string(argv[0]) + " -h <host> -p <port> [-4|-6] [-N|-E|-S|-W] [-a]");

    return args;
}

void handle_messages(Args args, Socket &socket, ClientGameState &client_game_state)
{
    while (true)
    {
        std::string message_str = socket.extract_message();
        if (message_str.empty())
            return;

        try
        {
            std::shared_ptr<Message> message_ptr = Message::from_string(message_str);

            if (message_ptr->type == MessageType::DEAL)
            {
                DEALMessage deal_message = dynamic_cast<DEALMessage &>(*message_ptr);
                client_game_state.new_deal(deal_message);
            }
            else if (message_ptr->type == MessageType::BUSY)
            {
                BUSYMessage busy_message = dynamic_cast<BUSYMessage &>(*message_ptr);
                if (!args.automatic)
                {
                    std::cout << "Place busy, list of busy places received: ";

                    if (!busy_message.positions.empty())
                    {
                        auto it = busy_message.positions.begin();
                        std::cout << ::to_string<Position>(*it);
                        ++it;
                        for (; it != busy_message.positions.end(); ++it)
                        {
                            std::cout << ", " << ::to_string<Position>(*it);
                        }
                    }
                    std::cout << ".\n";
                }
                throw std::runtime_error("Place busy");
            }
            else if (message_ptr->type == MessageType::TRICK)
            {
                TRICKMessage trick_message = dynamic_cast<TRICKMessage &>(*message_ptr);
                client_game_state.new_trick(trick_message);

                if (args.automatic && client_game_state.waiting_for_move)
                {
                    Card card = client_game_state.get_best_move();
                    TRICKMessage response(client_game_state.trick, std::vector<Card>{card});
                    socket.send(response.to_string());
                    client_game_state.waiting_for_move = false;
                }
            }
            else if (message_ptr->type == MessageType::WRONG)
            {
                WRONGMessage wrong_message = dynamic_cast<WRONGMessage &>(*message_ptr);
                if (!args.automatic)
                    std::cout << "Wrong message received in trick " << wrong_message.trick_number << ".\n";
            }
            else if (message_ptr->type == MessageType::TAKEN)
            {
                TAKENMessage taken_message = dynamic_cast<TAKENMessage &>(*message_ptr);
                client_game_state.end_trick(taken_message);
            }
            else if (message_ptr->type == MessageType::SCORE)
            {
                SCOREMessage score_message = dynamic_cast<SCOREMessage &>(*message_ptr);
                client_game_state.get_score(score_message);
            }
            else if (message_ptr->type == MessageType::TOTAL)
            {
                TOTALMessage total_message = dynamic_cast<TOTALMessage &>(*message_ptr);
                client_game_state.get_total(total_message);
            }
        }
        catch (std::invalid_argument &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}

void handle_user_input(Socket &socket, ClientGameState &client_game_state)
{
    std::string input;
    std::cin >> input;

    if (input == "cards")
        client_game_state.show_cards();

    if (input == "tricks")
        client_game_state.show_tricks();

    if (input[0] == '!')
    {
        try
        {
            Card card(input.substr(1));
            if (client_game_state.waiting_for_move && client_game_state.is_valid_move(card))
            {
                TRICKMessage trick_message(client_game_state.trick, std::vector<Card>{card});
                socket.send(trick_message.to_string());
                client_game_state.waiting_for_move = false;
            }
            else
            {
                std::cout << "You cannot play a card now.\n";
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return;
        }
    }
}

void run_client(Args args)
{
    Socket socket(args.host, args.port, args.ip_version, args.automatic);
    ClientGameState client_game_state(args.position, !args.automatic);

    IAMMessage iam_message(args.position);
    socket.send(iam_message.to_string());

    struct pollfd fds[2];

    fds[0].fd = socket.socket_fd;
    fds[0].events = POLLIN | POLLOUT | POLLHUP;
    int n = 1;

    if (!args.automatic)
    {
        fds[1].fd = STDIN_FILENO;
        fds[1].events = POLLIN;
        n = 2;
    }

    handle_messages(args, socket, client_game_state);

    while (!socket.closed || !socket.all_messages_received)
    {
        int ret = poll(fds, n, -1);
        if (ret == -1)
            throw std::runtime_error(strerror(errno));

        if (fds[0].revents & POLLIN)
            socket.handle_read();

        if (fds[0].revents & POLLOUT)
            socket.handle_write();

        if (fds[0].revents & POLLHUP)
            socket.handle_read();

        if (!args.automatic && fds[1].revents & POLLIN)
            handle_user_input(socket, client_game_state);

        handle_messages(args, socket, client_game_state);
    }

    if (!client_game_state.deal_ended)
        throw std::runtime_error("The game has not ended yet");
}

int main(int argc, char *argv[])
{
    try
    {
        Args args = parse_args(argc, argv);
        run_client(args);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}