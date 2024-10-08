#ifndef SERVER_GAME_STATE_H
#define SERVER_GAME_STATE_H

#include <optional>

#include "network-common.h"
#include "common.h"

class ServerGameState
{
public:
    int timeout;

    // current trick data
    int current_trick;
    int current_move;
    int first_move;
    bool trick_started;

    std::vector<Card> trick_cards;
    std::optional<Position> awaited_player;

    // current deal data
    int current_deal;
    bool deal_started;

    DealType deal_type;
    Position starting_player;

    std::vector<std::vector<Card>> current_hands;
    std::vector<std::vector<Card>> starting_hands;
    std::vector<TAKENMessage> taken_messages;

    // Whole game data
    bool game_ended;
    std::vector<Position> order;
    std::vector<DealType> deal_types;
    std::vector<std::vector<std::vector<Card>>> hands;
    std::vector<Position> starting_players;
    std::map<Position, std::shared_ptr<Socket>> player_sockets;
    std::map<Position, int> deal_scores;
    std::map<Position, int> total_scores;

    /**
     * @brief Construct a new Server Game State object
     *
     * @param filename File name
     * @param timeout Timeout
     */
    ServerGameState(const std::string &filename, int timeout);

    /**
     * @brief New player
     *
     * @param position Position
     * @param socket Socket
     * @return std::optional<BUSYMessage>
     */
    std::optional<BUSYMessage> new_player(Position position, std::shared_ptr<Socket> socket);

    /**
     * @brief Disconnect player
     *
     * @param socket Socket
     */
    void disconnect_client(const std::shared_ptr<Socket>& socket);

    /**
     * @brief Check if all players are ready
     *
     * @return Are all players ready
     */
    bool are_players_ready();

    /**
     * @brief Start a new deal, send deal messages, etc.
     */
    void start_deal();

    /**
     * @brief Continue game, send necessary messages, etc.
     */
    void continue_game();

    /**
     * @brief Continue deal, send necessary messages, etc.
     */
    void continue_deal();

    /**
     * @brief Continue trick, send necessary messages, etc.
     */
    void continue_trick();

    /**
     * @brief Handle trick message from client
     *
     * @param socket Socket
     * @param trick_message Trick message
     * @return std::optional<WRONGMessage>
     */
    std::optional<WRONGMessage> handle_trick_message(std::shared_ptr<Socket> socket, const TRICKMessage& trick_message);

    /**
     * @brief Find the position of a client with this socket
     *
     * @param socket Socket
     * @return std::optional<Position>
     */
    std::optional<Position> find_position(std::shared_ptr<Socket> socket);

    /**
     * @brief Sends trick message to player at position.
     *
     * @param position
     */
    void send_trick_message(Position position);

    /**
     * @brief Cheks if server can be terminated.
     *
     * @return Can end server.
     */
    bool can_end_server();

private:

    /**
     * @brief Sends necessary messages to rejoin the client.
     *
     * @param position The player's position.
     */
    void rejoin_client(Position position);

    /**
     * @brief Sends deal message to player at position.
     *
     * @param position The player's position.
     */
    void send_deal_message(Position position);

    /**
     * @brief Sends scores to the players.
     */
    void send_score_messages();

    /**
     * @brief Sends taken messages to all players.
     */
    void send_taken_messages();

    /**
     * @brief Calculates points, and sends out TAKEN messages.
     */
    void calculate_points();

    /**
     * @brief Ends the game, disconnects clients, etc.
     */
    void end_game();

    /**
     * @brief Checks if this card could have been played.
     *
     * @param card The card.
     * @param position The player's position.
     */
    bool is_valid_move(const Card &card, Position position);

    /**
     * @brief Translates the position to int in order.
     *
     * @param position The position.
     */
    static int position_order(Position position);
};

#endif // SERVER_GAME_STATE_H