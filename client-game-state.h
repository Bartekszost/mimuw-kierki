#ifndef CLIENT_GAME_STATE_H
#define CLIENT_GAME_STATE_H

#include "common.h"

/**
 * @brief Represents the state of the game for a client.
 */
class ClientGameState
{
public:
    int total_points;
    int points;
    int deal;
    int trick;
    bool got_score;
    bool got_total;
    bool deal_ended;
    bool trick_ended;
    bool verbose;
    bool waiting_for_move;
    DealType deal_type;
    Position position;
    Position starting_player;
    std::vector<Position> order;
    std::vector<Card> hand;
    std::vector<Card> trick_cards;
    std::vector<std::vector<Card>> taken_tricks;

    /**
     * @brief Construct a new Client Game State object
     * @param position The position of the client.
     * @param verbose Whether to print messages for the user.
     */
    ClientGameState(Position position, bool verbose = false);

    /**
     * @brief Start a new deal.
     * @param deal_message The deal message.
     */
    void new_deal(const DEALMessage &deal_message);

    /**
     * @brief Start a new trick.
     * @param trick_message The trick message.
     */
    void new_trick(const TRICKMessage &trick_message);

    /**
     * @brief End the current trick.
     * @param taken_message The taken message.
     */
    void end_trick(const TAKENMessage &taken_message);

    /**
     * @brief Get score message.
     * @param points_message The points message.
     */
    void get_score(SCOREMessage score_message);

    /**
     * @brief Get total score.
     * @param total_message The total message.
     */
    void get_total(TOTALMessage total_message);

    /**
     * @brief Get the valid moves for the current state.
     * @return The valid moves.
     */
    std::vector<Card> get_valid_moves();

    /**
     * @brief Check if a move is valid.
     * @param card The card to play.
     * @return Whether the move is valid.
     */
    bool is_valid_move(const Card &card);

    /**
     * @brief Get the best move.
     * @return The best move.
     */
    Card get_best_move();

    /**
     * @brief Show the cards in the hand.
     */
    void show_cards() const;

    /**
     * @brief Show the tricks taken.
     */
    void show_tricks();
};

#endif // CLIENT_GAME_STATE_H