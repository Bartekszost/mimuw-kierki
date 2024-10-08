#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

#include "server-game-state.h"

ServerGameState::ServerGameState(const std::string &filename, int timeout)
{
    game_ended = false;
    this->timeout = timeout;
    order = {Position::North, Position::East, Position::South, Position::West};

    std::ifstream file(filename);
    if (!file)
        throw std::runtime_error("Could not open file");

    hands = {{}, {}, {}, {}};

    std::string line;
    while (std::getline(file, line))
    {
        deal_types.push_back(::from_string<DealType>(line.substr(0, 1)));
        starting_players.push_back(::from_string<Position>(line.substr(1, 1)));

        for (int i = 0; i < 4; i++)
        {
            std::getline(file, line);
            std::vector<Card> hand = Card::parse_cards(line);
            hands[i].push_back(hand);
        }
    }

    file.close();

    player_sockets[Position::North] = nullptr;
    player_sockets[Position::East] = nullptr;
    player_sockets[Position::South] = nullptr;
    player_sockets[Position::West] = nullptr;

    current_deal = 0;
    current_trick = 0;
    current_move = 0;

    deal_started = false;

    for (auto &position : order)
    {
        deal_scores[position] = 0;
        total_scores[position] = 0;
    }
}

std::optional<BUSYMessage> ServerGameState::new_player(Position position, std::shared_ptr<Socket> socket)
{
    if (player_sockets[position] != nullptr)
    {
        std::vector<Position> busy_positions;
        for (auto &position : order)
            if (player_sockets[position] != nullptr || player_sockets[position] == socket)
                busy_positions.push_back(position);

        return BUSYMessage(busy_positions);
    }

    player_sockets[position] = socket;

    if (socket->awaited_message == MessageType::IAM)
        socket->awaited_message = std::nullopt;

    if (deal_started)
        rejoin_client(position);

    return std::nullopt;
}

void ServerGameState::disconnect_client(const std::shared_ptr<Socket> &socket)
{
    for (auto &[position, player_socket] : player_sockets)
    {
        if (player_socket == socket)
            player_sockets[position] = nullptr;
    }
}

bool ServerGameState::are_players_ready()
{
    for (auto &[position, player_socket] : player_sockets)
        if (!player_socket)
            return false;

    return true;
}

void ServerGameState::start_deal()
{
    current_deal++;
    deal_started = true;

    deal_type = deal_types[current_deal - 1];
    starting_player = starting_players[current_deal - 1];

    starting_hands = {{}, {}, {}, {}};

    for (int i = 0; i < 4; i++)
    {
        std::vector<Card> hand = hands[i][current_deal - 1];
        Position position = order[i];

        starting_hands[i] = hand;

        send_deal_message(position);
        deal_scores[position] = 0;
    }

    current_hands = starting_hands;

    current_trick = 0;
    trick_started = false;
    current_move = 0;
    trick_cards = {};
    taken_messages = {};
    first_move = position_order(starting_player);
}

void ServerGameState::continue_game()
{
    if (!are_players_ready())
        return;

    if (!deal_started) {
        if (current_deal == static_cast<int>(deal_types.size())) {
            end_game();
            return;
        }
        start_deal();
    }

    continue_deal();
}

void ServerGameState::continue_deal()
{
    if (current_trick <= 13)
    {
        continue_trick();
        return;
    }

    for (auto pos : order)
    {
        total_scores[pos] += deal_scores[pos];
    }

    send_score_messages();
    deal_started = false;
}

void ServerGameState::continue_trick()
{
    if (!trick_started)
    {
        current_trick++;
        trick_started = true;
        current_move = 0;
        trick_cards = {};
        awaited_player = std::nullopt;
    }

    if (current_move == 4)
    {
        trick_started = false;
        calculate_points();
        send_taken_messages();

        if (current_trick == 13)
            current_trick++;

        return;
    }

    if (!awaited_player.has_value())
    {
        int player_idx = (current_move + first_move) % 4;
        awaited_player = order[player_idx];

        send_trick_message(awaited_player.value());
    }
}

std::optional<WRONGMessage> ServerGameState::handle_trick_message(std::shared_ptr<Socket> socket, const TRICKMessage &trick_message)
{
    auto position = find_position(socket);

    WRONGMessage wrong_message = WRONGMessage(current_trick);

    if (!position.has_value())
        return wrong_message;

    if (position != awaited_player)
        return wrong_message;

    if (trick_message.trick_number != current_trick)
        return wrong_message;

    auto cards = trick_message.cards;

    if (cards.empty())
        return wrong_message;

    Card last_card = cards[cards.size() - 1];
    int player_idx = position_order(position.value());

    if (!is_valid_move(last_card, position.value()))
        return wrong_message;

    trick_cards.push_back(last_card);
    current_hands[player_idx].erase(std::find(current_hands[player_idx].begin(), current_hands[player_idx].end(), last_card));
    current_move++;
    awaited_player = std::nullopt;
    socket->awaited_message = std::nullopt;

    return std::nullopt;
}

std::optional<Position> ServerGameState::find_position(std::shared_ptr<Socket> socket)
{
    for (auto [pos, sock] : player_sockets)
        if (socket == sock)
            return pos;
    return std::nullopt;
}

void ServerGameState::send_trick_message(Position position)
{
    TRICKMessage trick_message = TRICKMessage(current_trick, trick_cards);

    player_sockets[position]->send(trick_message.to_string());
    player_sockets[position]->await_message(MessageType::TRICK, timeout);
}

bool ServerGameState::can_end_server() {
    if (!game_ended)
        return false;

    for (auto [pos, sock]: player_sockets)
        if (sock != nullptr)
            return false;

    return true;
}

/*
 * Private functions
 */

void ServerGameState::rejoin_client(Position position)
{
    send_deal_message(position);

    for (const auto& taken_message: taken_messages) {
        player_sockets[position]->send(taken_message.to_string());
    }

    if (trick_started && awaited_player == position)
        send_trick_message(position);
}

void ServerGameState::send_deal_message(Position position)
{
    int idx = position_order(position);
    auto hand = starting_hands[idx];

    DEALMessage deal_message = DEALMessage(deal_type, starting_player, hand);
    player_sockets[position]->send(deal_message.to_string());
}

void ServerGameState::send_score_messages()
{
    SCOREMessage score_message = SCOREMessage(deal_scores);
    TOTALMessage total_message = TOTALMessage(total_scores);

    for (auto pos : order)
    {
        player_sockets[pos]->send(score_message.to_string());
        player_sockets[pos]->send(total_message.to_string());
    }
}

void ServerGameState::send_taken_messages()
{
    TAKENMessage taken_message = TAKENMessage(current_trick, trick_cards, order[first_move]);

    taken_messages.push_back(taken_message);

    for (auto pos: order)
        player_sockets[pos]->send(taken_message.to_string());
}

void ServerGameState::calculate_points()
{
    int winner = first_move;
    Card lead_card = trick_cards[0];
    Color lead_color = lead_card.color;

    for (int i = 1; i < 4; i++)
        if (lead_card.compare(trick_cards[i], lead_color))
        {
            lead_card = trick_cards[i];
            winner = (first_move + i) % 4;
        }

    first_move = winner;
    int score = 0;

    if (deal_type == DealType::TRICK || deal_type == DealType::BANDIT)
        score++;
    if (deal_type == DealType::HEART || deal_type == DealType::BANDIT)
    {
        for (auto const &card : trick_cards)
            if (card.color == Color::Hearts)
                score++;
    }
    if (deal_type == DealType::QUEEN || deal_type == DealType::BANDIT) {
        for (auto const &card: trick_cards)
            if (card.figure == "Q")
                score += 5;
    }
    if (deal_type == DealType::LORD || deal_type == DealType::BANDIT) {
        for (auto const &card: trick_cards)
            if (card.figure == "J" || card.figure == "K")
                score += 2;
    }
    if (deal_type == DealType::KING_HEART || deal_type == DealType::BANDIT) {
        for (auto const &card: trick_cards)
            if (card.figure == "K" && card.color == Color::Hearts)
                score += 18;
    }
    if (deal_type == DealType::SEVENTH_LAST || deal_type == DealType::BANDIT) {
        if (current_trick == 7 || current_trick == 13)
            score += 10;
    }

    Position winner_pos = order[winner];
    deal_scores[winner_pos] += score;
}

void ServerGameState::end_game() {
    game_ended = true;

    for (auto [pos, socket]: player_sockets) {
        socket->closed = true;
        socket->all_messages_received = true;
    }
}

bool ServerGameState::is_valid_move(const Card &card, Position position)
{
    int player_idx = position_order(position);
    auto player_hand = current_hands[player_idx];

    std::vector<Card> valid_moves;
    if (!trick_cards.empty())
    {
        auto lead_color = trick_cards[0].color;
        for (auto player_card : player_hand)
            if (player_card.color == lead_color)
                valid_moves.push_back(player_card);
    }

    if (valid_moves.empty())
        valid_moves = player_hand;

    if (std::find(valid_moves.begin(), valid_moves.end(), card) == valid_moves.end())
        return false;

    return true;
}

int ServerGameState::position_order(Position position)
{
    switch (position)
    {
    case Position::North:
        return 0;
    case Position::East:
        return 1;
    case Position::South:
        return 2;
    case Position::West:
        return 3;
    default:
        return -1;
    }
}
