#include <gtest/gtest.h>

#include "client-game-state.h"
#include "client_game_state_test.h"

TEST(ClientGameStateSuite, Constructor)
{
    ClientGameState client_game_state(Position::North, true);
    ASSERT_EQ(client_game_state.total_points, 0);
    ASSERT_EQ(client_game_state.points, 0);
    ASSERT_EQ(client_game_state.deal, 0);
    ASSERT_EQ(client_game_state.trick, 1);
    ASSERT_EQ(client_game_state.position, Position::North);
    ASSERT_EQ(client_game_state.verbose, true);
    ASSERT_EQ(client_game_state.hand.size(), 0);
    ASSERT_EQ(client_game_state.taken_tricks.size(), 0);
    ASSERT_EQ(client_game_state.deal_ended, true);
    ASSERT_EQ(client_game_state.game_ended, false);
    ASSERT_EQ(client_game_state.trick_ended, true);
}

TEST(ClientGameStateSuite, NewDeal)
{
    ClientGameState client_game_state(Position::North, true);
    std::vector<Card> cards = {
        Card("AS"), Card("2S"), Card("3S"), Card("4S"), Card("5S"), Card("6S"), Card("7S"),
        Card("8S"), Card("9S"), Card("10S"), Card("JS"), Card("QS"), Card("KS")};
    DEALMessage deal_message(DealType::TRICK, Position::North, cards);

    std::string expected_output = "New deal 1: starting place N, your cards: AS, 2S, 3S, 4S, 5S, 6S, 7S, 8S, 9S, 10S, JS, QS, KS\n";
    testing::internal::CaptureStdout();
    client_game_state.new_deal(deal_message);
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, expected_output);

    ASSERT_EQ(client_game_state.deal, 1);
    ASSERT_EQ(client_game_state.deal_ended, false);
    ASSERT_EQ(client_game_state.trick, 1);
    ASSERT_EQ(client_game_state.trick_ended, true);
    ASSERT_EQ(client_game_state.deal_type, DealType::TRICK);
    ASSERT_EQ(client_game_state.starting_player, Position::North);
    ASSERT_EQ(client_game_state.hand.size(), 13);
    ASSERT_EQ(client_game_state.taken_tricks.size(), 0);
}

TEST(ClientGameStateSuite, NewTrick)
{
    ClientGameState client_game_state(Position::North, true);
    std::vector<Card> cards = {
        Card("AS"), Card("2S"), Card("3S"), Card("4S"), Card("5S"), Card("6S"), Card("7S"),
        Card("8S"), Card("9S"), Card("10S"), Card("JS"), Card("QS"), Card("KS")};
    DEALMessage deal_message(DealType::TRICK, Position::North, cards);
    client_game_state.new_deal(deal_message);

    TRICKMessage trick_message(1, std::vector<Card>{Card("AS")});

    std::string expected_output = "Trick: (1) AS\n";
    testing::internal::CaptureStdout();
    client_game_state.new_trick(trick_message);
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, expected_output);

    ASSERT_EQ(client_game_state.deal, 1);
    ASSERT_EQ(client_game_state.deal_ended, false);
    ASSERT_EQ(client_game_state.trick, 1);
    ASSERT_EQ(client_game_state.trick_ended, false);
    ASSERT_EQ(client_game_state.hand.size(), 13);
    ASSERT_EQ(client_game_state.taken_tricks.size(), 0);
}

TEST(ClientGameStateSuite, EndTrick)
{
    ClientGameState client_game_state(Position::North, true);
    std::vector<Card> cards = {
        Card("AS"), Card("2S"), Card("3S"), Card("4S"), Card("5S"), Card("6S"), Card("7S"),
        Card("8S"), Card("9S"), Card("10S"), Card("JS"), Card("QS"), Card("KS")};
    DEALMessage deal_message(DealType::TRICK, Position::North, cards);
    client_game_state.new_deal(deal_message);

    TRICKMessage trick_message(1, std::vector<Card>{Card("AS")});
    client_game_state.new_trick(trick_message);

    TAKENMessage taken_message(1, std::vector<Card>{Card("AS"), Card("AD"), Card("KD"), Card("QD")}, Position::North);

    std::string expected_output = "A trick 1 is taken by N, cards AS, AD, KD, QD.\n";
    testing::internal::CaptureStdout();
    client_game_state.end_trick(taken_message);
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, expected_output);

    ASSERT_EQ(client_game_state.deal, 1);
    ASSERT_EQ(client_game_state.deal_ended, false);
    ASSERT_EQ(client_game_state.trick, 2);
    ASSERT_EQ(client_game_state.trick_ended, true);
    ASSERT_EQ(client_game_state.hand.size(), 12);
    ASSERT_EQ(client_game_state.taken_tricks.size(), 1);
    ASSERT_EQ(client_game_state.taken_tricks[0].size(), 4);
}

TEST(ClientGameStateSuite, EndDeal)
{
    ClientGameState client_game_state(Position::North, true);
    std::vector<Card> cards = {
        Card("AS"), Card("2S"), Card("3S"), Card("4S"), Card("5S"), Card("6S"), Card("7S"),
        Card("8S"), Card("9S"), Card("10S"), Card("JS"), Card("QS"), Card("KS")};
    DEALMessage deal_message(DealType::TRICK, Position::North, cards);
    client_game_state.new_deal(deal_message);

    TRICKMessage trick_message(1, std::vector<Card>{Card("AS")});
    client_game_state.new_trick(trick_message);

    TAKENMessage taken_message(1, std::vector<Card>{Card("AS"), Card("AD"), Card("KD"), Card("QD")}, Position::North);
    client_game_state.end_trick(taken_message);

    SCOREMessage score_message(std::map<Position, int>{{Position::North, 10}, {Position::East, 5}, {Position::South, 3}, {Position::West, 10}});
    std::string expected_output = "The scores are:\nE | 5\nN | 10\nS | 3\nW | 10\n";
    testing::internal::CaptureStdout();
    client_game_state.end_deal(score_message);
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, expected_output);

    ASSERT_EQ(client_game_state.deal, 1);
    ASSERT_EQ(client_game_state.deal_ended, true);
    ASSERT_EQ(client_game_state.trick, 2);
    ASSERT_EQ(client_game_state.trick_ended, true);
    ASSERT_EQ(client_game_state.total_points, 10);
    ASSERT_EQ(client_game_state.points, 10);
}

TEST(ClientGameStateSuite, EndGame)
{
    ClientGameState client_game_state(Position::North, true);
    std::vector<Card> cards = {
        Card("AS"), Card("2S"), Card("3S"), Card("4S"), Card("5S"), Card("6S"), Card("7S"),
        Card("8S"), Card("9S"), Card("10S"), Card("JS"), Card("QS"), Card("KS")};
    DEALMessage deal_message(DealType::TRICK, Position::North, cards);
    client_game_state.new_deal(deal_message);

    TRICKMessage trick_message(1, std::vector<Card>{Card("AS")});
    client_game_state.new_trick(trick_message);

    TAKENMessage taken_message(1, std::vector<Card>{Card("AS"), Card("AD"), Card("KD"), Card("QD")}, Position::North);
    client_game_state.end_trick(taken_message);

    SCOREMessage score_message(std::map<Position, int>{{Position::North, 10}, {Position::East, 5}, {Position::South, 3}, {Position::West, 10}});
    client_game_state.end_deal(score_message);

    TOTALMessage total_message(std::map<Position, int>{{Position::North, 20}, {Position::East, 10}, {Position::South, 6}, {Position::West, 20}});
    std::string expected_output = "The total scores are:\nE | 10\nN | 20\nS | 6\nW | 20\n";
    testing::internal::CaptureStdout();
    client_game_state.end_game(total_message);
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, expected_output);

    ASSERT_EQ(client_game_state.deal, 1);
    ASSERT_EQ(client_game_state.deal_ended, true);
    ASSERT_EQ(client_game_state.trick, 2);
    ASSERT_EQ(client_game_state.trick_ended, true);
    ASSERT_EQ(client_game_state.total_points, 20);
    ASSERT_EQ(client_game_state.points, 10);
    ASSERT_EQ(client_game_state.game_ended, true);
}

TEST(ClientGameStateSuite, GetValidMoves)
{
    ClientGameState client_game_state(Position::North, true);
    std::vector<Card> cards = {
        Card("AS"), Card("2S"), Card("3S"), Card("4S"), Card("5S"), Card("6S"), Card("7S"),
        Card("8D"), Card("9D"), Card("10D"), Card("2D"), Card("3D"), Card("4D")};
    DEALMessage deal_message(DealType::TRICK, Position::North, cards);
    client_game_state.new_deal(deal_message);

    TRICKMessage trick_message(1, std::vector<Card>{Card("AS")});
    client_game_state.new_trick(trick_message);

    std::vector<Card> valid_moves = client_game_state.get_valid_moves();
    ASSERT_EQ(valid_moves.size(), 7);

    TAKENMessage taken_message(1, std::vector<Card>{Card("AS"), Card("AD"), Card("KD"), Card("QD")}, Position::North);

    client_game_state.hand = std::vector<Card>{Card("AD"), Card("2D"), Card("3D"), Card("4C"), Card("5C"), Card("6C"), Card("7C")};
    valid_moves = client_game_state.get_valid_moves();
    ASSERT_EQ(valid_moves.size(), 7);

    client_game_state.end_trick(taken_message);

    ASSERT_THROW(client_game_state.get_valid_moves(), std::invalid_argument);
}