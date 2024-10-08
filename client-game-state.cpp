#include <iostream>
#include <algorithm>

#include "client-game-state.h"

std::string card_list_string(std::vector<Card> cards)
{
    std::string result;
    for (size_t i = 0; i < cards.size(); ++i)
    {
        result += cards[i].to_string();
        if (i != cards.size() - 1)
            result += ", ";
    }
    return result;
}

ClientGameState::ClientGameState(Position position, bool verbose)
{
    order = {Position::North, Position::East, Position::South, Position::West};
    total_points = 0;
    points = 0;
    deal = 0;
    trick = 1;
    this->position = position;
    this->verbose = verbose;
    hand = std::vector<Card>();
    taken_tricks = std::vector<std::vector<Card>>();
    deal_ended = true;
    got_score = false;
    got_total = false;
    trick_ended = true;
    waiting_for_move = false;
}

void ClientGameState::new_deal(const DEALMessage &deal_message)
{
    if (!deal_ended)
        throw std::invalid_argument("Deal has not ended yet");

    deal++;
    deal_ended = false;
    got_score = false;
    got_total = false;
    trick = 1;
    trick_ended = true;
    deal_type = deal_message.type;
    starting_player = deal_message.first_player;
    hand = deal_message.cards;
    taken_tricks = std::vector<std::vector<Card>>();
    waiting_for_move = false;

    if (verbose)
        std::cout << "New deal "
                  << ::to_string<DealType>(deal_type)
                  << ": staring place "
                  << ::to_string<Position>(starting_player)
                  << ", your cards: "
                  << card_list_string(hand)
                  << ".\n";
}

void ClientGameState::new_trick(const TRICKMessage &trick_message)
{
    if (!trick_ended && trick_message.trick_number != trick)
        throw std::invalid_argument("Trick has not ended yet");

    if (deal_ended)
        throw std::invalid_argument("Deal has ended");

    if (trick_message.trick_number != trick)
        throw std::invalid_argument("Trick number is not correct");

    trick_ended = false;
    trick_cards = trick_message.cards;
    waiting_for_move = true;

    if (verbose)
        std::cout << "Trick: ("
                  << trick
                  << ") "
                  << card_list_string(trick_cards)
                  << "\nAvailable: "
                  << card_list_string(hand)
                  << "\n";
}

void ClientGameState::end_trick(const TAKENMessage &taken_message)
{

    if (deal_ended)
        throw std::invalid_argument("Deal has ended");

    if (taken_message.trick_number != trick)
        throw std::invalid_argument("Trick number is not correct");

    trick_ended = true;
    waiting_for_move = false;

    if (taken_message.taken_by == position)
        taken_tricks.push_back(taken_message.cards);

    for (const auto &card : taken_message.cards)
    {
        auto it = std::find(hand.begin(), hand.end(), card);
        if (it != hand.end())
        {
            hand.erase(it);
        }
    }

    if (verbose)
        std::cout << "A trick "
                  << trick
                  << " is taken by "
                  << ::to_string<Position>(taken_message.taken_by)
                  << ", cards "
                  << card_list_string(taken_message.cards)
                  << ".\n";

    trick = taken_message.trick_number + 1;
}

void ClientGameState::get_score(SCOREMessage score_message)
{
    if (deal_ended)
        throw std::invalid_argument("Deal has ended");

    if (got_score)
        throw std::invalid_argument("Already got score");

    got_score = true;
    deal_ended = got_score && got_total;
    waiting_for_move = false;
    points = score_message.scores[position];
    total_points += points;

    if (verbose)
    {
        std::cout << "The scores are:\n";
        for (auto const &pos: order)
            std::cout << ::to_string<Position>(pos) << " | " << score_message.scores[pos] << "\n";
    }
}

void ClientGameState::get_total(TOTALMessage total_message)
{
    if (deal_ended)
        throw std::invalid_argument("Deal has ended");

    if (got_total)
        throw std::invalid_argument("Already got total");

    got_total = true;
    deal_ended = got_score && got_total;

    waiting_for_move = false;
    total_points = total_message.totals[position];

    if (verbose)
    {
        std::cout << "The total scores are:\n";
        for (auto const &pos: order)
            std::cout << ::to_string<Position>(pos) << " | " << total_message.totals[pos] << "\n";
    }
}

std::vector<Card> ClientGameState::get_valid_moves()
{
    if (deal_ended)
        throw std::invalid_argument("Deal has ended");

    if (trick_ended)
        throw std::invalid_argument("Trick has ended");

    if (trick_cards.empty())
        return hand;

    std::vector<Card> valid_moves;
    for (const auto &card : hand)
    {
        if (card.color == trick_cards[0].color)
            valid_moves.push_back(card);
    }

    if (valid_moves.empty())
        return hand;

    return valid_moves;
}

bool ClientGameState::is_valid_move(const Card &card)
{
    std::vector<Card> valid_moves = get_valid_moves();
    return std::find(valid_moves.begin(), valid_moves.end(), card) != valid_moves.end();
}

Card ClientGameState::get_best_move()
{
    if (deal_ended)
        throw std::invalid_argument("Deal has ended");

    if (trick_ended)
        throw std::invalid_argument("Trick has ended");

    if (trick_cards.empty())
    {
        return *std::min_element(hand.begin(), hand.end());
    }

    std::vector<Card> valid_moves = get_valid_moves();

    Card first_trick_card = trick_cards[0];
    Card best_move = valid_moves[0];
    for (const auto &card : valid_moves)
        if (!card.compare(best_move, first_trick_card.color) && card.compare(first_trick_card, first_trick_card.color))
            best_move = card;

    return best_move;
}

void ClientGameState::show_cards() const
{
    if (verbose)
        std::cout << card_list_string(hand) << '\n';
}

void ClientGameState::show_tricks()
{
    if (verbose)
        for (const auto &taken_trick : taken_tricks)
            std::cout << card_list_string(taken_trick) << '\n';
}