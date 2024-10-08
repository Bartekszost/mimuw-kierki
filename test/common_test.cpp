#include <gtest/gtest.h>
#include <string>

#include "common.h"
#include "common_test.h"

TEST(CommonTest, ColorToString)
{
    ASSERT_EQ(to_string<Color>(Color::Hearts), "H");
    ASSERT_EQ(to_string<Color>(Color::Spades), "S");
    ASSERT_EQ(to_string<Color>(Color::Diamonds), "D");
    ASSERT_EQ(to_string<Color>(Color::Clubs), "C");
}

TEST(CommonTest, StringToColor)
{
    ASSERT_EQ(from_string<Color>("H"), Color::Hearts);
    ASSERT_EQ(from_string<Color>("S"), Color::Spades);
    ASSERT_EQ(from_string<Color>("D"), Color::Diamonds);
    ASSERT_EQ(from_string<Color>("C"), Color::Clubs);
    ASSERT_THROW(from_string<Color>("X"), std::invalid_argument);
}

TEST(CommonTest, MessageTypeToString)
{
    ASSERT_EQ(to_string<MessageType>(MessageType::IAM), "IAM");
    ASSERT_EQ(to_string<MessageType>(MessageType::BUSY), "BUSY");
    ASSERT_EQ(to_string<MessageType>(MessageType::DEAL), "DEAL");
    ASSERT_EQ(to_string<MessageType>(MessageType::TRICK), "TRICK");
    ASSERT_EQ(to_string<MessageType>(MessageType::WRONG), "WRONG");
    ASSERT_EQ(to_string<MessageType>(MessageType::TAKEN), "TAKEN");
    ASSERT_EQ(to_string<MessageType>(MessageType::SCORE), "SCORE");
    ASSERT_EQ(to_string<MessageType>(MessageType::TOTAL), "TOTAL");
}

TEST(CommonTest, StringToMessageType)
{
    ASSERT_EQ(from_string<MessageType>("IAM"), MessageType::IAM);
    ASSERT_EQ(from_string<MessageType>("BUSY"), MessageType::BUSY);
    ASSERT_EQ(from_string<MessageType>("DEAL"), MessageType::DEAL);
    ASSERT_EQ(from_string<MessageType>("TRICK"), MessageType::TRICK);
    ASSERT_EQ(from_string<MessageType>("WRONG"), MessageType::WRONG);
    ASSERT_EQ(from_string<MessageType>("TAKEN"), MessageType::TAKEN);
    ASSERT_EQ(from_string<MessageType>("SCORE"), MessageType::SCORE);
    ASSERT_EQ(from_string<MessageType>("TOTAL"), MessageType::TOTAL);
    ASSERT_THROW(from_string<MessageType>("X"), std::invalid_argument);
}

TEST(CommonTest, PositionToString)
{
    ASSERT_EQ(to_string<Position>(Position::North), "N");
    ASSERT_EQ(to_string<Position>(Position::East), "E");
    ASSERT_EQ(to_string<Position>(Position::South), "S");
    ASSERT_EQ(to_string<Position>(Position::West), "W");
}

TEST(CommonTest, StringToPosition)
{
    ASSERT_EQ(from_string<Position>("N"), Position::North);
    ASSERT_EQ(from_string<Position>("E"), Position::East);
    ASSERT_EQ(from_string<Position>("S"), Position::South);
    ASSERT_EQ(from_string<Position>("W"), Position::West);
    ASSERT_THROW(from_string<Position>("X"), std::invalid_argument);
}

// Define a test
TEST(CardSuite, ConstructorThrows)
{
    // Assert
    ASSERT_THROW(Card("11S"), std::invalid_argument);
}

TEST(CardSuite, Constructor)
{
    // Arrange
    Card card("AS");
    // Assert
    ASSERT_EQ(card.figure, "A");
    ASSERT_EQ(card.color, Color::Spades);
}

TEST(CardSuite, Compare1)
{
    // Arrange
    Card card1("KS");
    Card card2("AS");
    // Assert
    ASSERT_TRUE(card1.compare(card2, Color::Spades));
}

TEST(CardSuite, Compare2)
{
    // Arrange
    Card card1("AH");
    Card card2("KS");
    // Assert
    ASSERT_TRUE(card1.compare(card2, Color::Spades));
}

TEST(CardSuite, ParseCards)
{
    // Arrange
    std::string card_list = "ASKH10D";
    // Act
    std::vector<Card> cards = Card::parse_cards(card_list);
    // Assert
    ASSERT_EQ(cards.size(), 3);
    ASSERT_EQ(cards[0].figure, "A");
    ASSERT_EQ(cards[0].color, Color::Spades);
    ASSERT_EQ(cards[1].figure, "K");
    ASSERT_EQ(cards[1].color, Color::Hearts);
    ASSERT_EQ(cards[2].figure, "10");
    ASSERT_EQ(cards[2].color, Color::Diamonds);
}

TEST(CardSuite, ToString)
{
    // Arrange
    Card card("AS");
    // Act
    std::string card_str = card.to_string();
    // Assert
    ASSERT_EQ(card_str, "AS");
}

TEST(CardSuite, ParseCardsEmpty)
{
    // Arrange
    std::string card_list = "";
    // Act
    std::vector<Card> cards = Card::parse_cards(card_list);
    // Assert
    ASSERT_EQ(cards.size(), 0);
}

TEST(CardSuite, ParseCardsThrows)
{
    // Arrange
    std::string card_list1 = "ASKH10D1";
    std::string card_list2 = "1ASKH10D1";
    // Assert
    ASSERT_THROW(Card::parse_cards(card_list1), std::invalid_argument);
    ASSERT_THROW(Card::parse_cards(card_list2), std::invalid_argument);
}

TEST(MessageSuite, Constructor)
{
    // Arrange
    Message message(MessageType::IAM, "N");
    // Assert
    ASSERT_EQ(message.type, MessageType::IAM);
    ASSERT_EQ(message.data, "N");
}

TEST(MessageSuite, ToString)
{
    // Arrange
    Message message(MessageType::IAM, "N");
    // Act
    std::string message_str = message.to_string();
    // Assert
    ASSERT_EQ(message_str, "IAMN\r\n");
}

TEST(MessageSuite, FromString)
{
    // Arrange
    std::string message_str1 = "IAMN\r\n";
    std::string message_str2 = "IAMN";
    // Act

    std::shared_ptr<Message> message1 = Message::from_string(message_str1);
    ASSERT_THROW(Message::from_string(message_str2), std::invalid_argument);
    // Assert
    ASSERT_EQ(message1->type, MessageType::IAM);
    ASSERT_EQ(message1->data, "N");
}

TEST(IAMMessageSuite, Constructor)
{
    // Arrange
    IAMMessage message(Position::North);
    // Assert
    ASSERT_EQ(message.position, Position::North);
}

TEST(IAMMessageSuite, ToString)
{
    // Arrange
    IAMMessage message(Position::North);
    // Act
    std::string message_str = message.to_string();
    // Assert
    ASSERT_EQ(message_str, "IAMN\r\n");
}

TEST(IAMMessageSuite, FromString)
{
    // Arrange
    std::string message_str1 = "IAMN\r\n";
    std::string message_str2 = "IAMN";
    std::string message_str3 = "IAMX\r\n";
    std::string message_str4 = "IAMNS\r\n";
    // Act
    std::shared_ptr<Message> message_ptr1 = Message::from_string(message_str1);
    ASSERT_THROW(Message::from_string(message_str2), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str3), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str4), std::invalid_argument);
    // Assert
    std::shared_ptr<IAMMessage> iam_message_ptr1 = std::dynamic_pointer_cast<IAMMessage>(message_ptr1);
    ASSERT_NE(iam_message_ptr1, nullptr);
    if (iam_message_ptr1 != nullptr)
    {
        ASSERT_EQ(iam_message_ptr1->position, Position::North);
    }
}

TEST(BUSYMessageSuite, Constructor)
{
    // Arrange
    std::vector<Position> positions{Position::North, Position::South};
    BUSYMessage message(positions);
    // Assert
    ASSERT_EQ(message.positions, positions);
}

TEST(BUSYMessageSuite, ToString)
{
    // Arrange
    std::vector<Position> positions{Position::North, Position::South};
    BUSYMessage message(positions);
    // Act
    std::string message_str = message.to_string();
    // Assert
    ASSERT_EQ(message_str, "BUSYNS\r\n");
}

TEST(BUSYMessageSuite, FromString)
{
    // Arrange
    std::string message_str1 = "BUSYNS\r\n";
    std::string message_str2 = "BUSYNS";
    std::string message_str3 = "BUSYXN\r\n";
    std::string message_str4 = "BUSYNSW\r\n";
    // Act
    std::shared_ptr<Message> message_ptr1 = Message::from_string(message_str1);
    ASSERT_THROW(Message::from_string(message_str2), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str3), std::invalid_argument);
    std::shared_ptr<Message> message_ptr4 = Message::from_string(message_str4);
    // Assert
    std::shared_ptr<BUSYMessage> busy_message_ptr1 = std::dynamic_pointer_cast<BUSYMessage>(message_ptr1);
    ASSERT_NE(busy_message_ptr1, nullptr);
    if (busy_message_ptr1 != nullptr)
    {
        ASSERT_EQ(busy_message_ptr1->positions.size(), 2);
        ASSERT_EQ(busy_message_ptr1->positions[0], Position::North);
        ASSERT_EQ(busy_message_ptr1->positions[1], Position::South);
    }

    std::shared_ptr<BUSYMessage> busy_message_ptr4 = std::dynamic_pointer_cast<BUSYMessage>(message_ptr4);
    ASSERT_NE(busy_message_ptr4, nullptr);
    if (busy_message_ptr4 != nullptr)
    {
        ASSERT_EQ(busy_message_ptr4->positions.size(), 3);
        ASSERT_EQ(busy_message_ptr4->positions[0], Position::North);
        ASSERT_EQ(busy_message_ptr4->positions[1], Position::South);
        ASSERT_EQ(busy_message_ptr4->positions[2], Position::West);
    }
}

TEST(DEALMessageSuite, Constructor)
{
    // Arrange
    DealType deal_type = DealType::TRICK;
    Position position = Position::North;
    std::vector<Card> cards = {
        Card("AS"), Card("2S"), Card("3S"), Card("4S"), Card("5S"), Card("6S"), Card("7S"),
        Card("8S"), Card("9S"), Card("10S"), Card("JS"), Card("QS"), Card("KS")};

    DEALMessage message(deal_type, position, cards);

    // Assert
    ASSERT_EQ(message.type, deal_type);
    ASSERT_EQ(message.first_player, position);
    ASSERT_EQ(message.cards.size(), 13);
    for (int i = 0; i < 13; ++i)
    {
        ASSERT_EQ(message.cards[i].figure, cards[i].figure);
        ASSERT_EQ(message.cards[i].color, Color::Spades);
    }
}

TEST(DEALMessageSuite, ToString)
{
    // Arrange
    DealType deal_type = DealType::TRICK;
    Position position = Position::North;
    std::vector<Card> cards = {
        Card("AS"), Card("2S"), Card("3S"), Card("4S"), Card("5S"), Card("6S"), Card("7S"),
        Card("8S"), Card("9S"), Card("10S"), Card("JS"), Card("QS"), Card("KS")};

    DEALMessage message(deal_type, position, cards);

    // Act
    std::string message_str = message.to_string();

    // Assert
    ASSERT_EQ(message_str, "DEAL1NAS2S3S4S5S6S7S8S9S10SJSQSKS\r\n");
}

TEST(DEALMessageSuite, FromString)
{
    // Arrange
    std::string valid_message_str = "DEAL1NAS2S3S4S5S6S7S8S9S10SJSQSKS\r\n";
    std::string invalid_message_str1 = "DEAL1NAS2S3S4S5S6S7S8S9S10SJSQSKSAS\r\n";
    std::string invalid_message_str2 = "DEAL1NAS2S3S4S5S6S7S8S9S10SJSQS\r\n";

    // Act
    std::shared_ptr<Message> valid_message_ptr = Message::from_string(valid_message_str);
    ASSERT_THROW(Message::from_string(invalid_message_str1), std::invalid_argument);
    ASSERT_THROW(Message::from_string(invalid_message_str2), std::invalid_argument);

    // Assert
    std::shared_ptr<DEALMessage> valid_deal_message_ptr = std::dynamic_pointer_cast<DEALMessage>(valid_message_ptr);
    ASSERT_NE(valid_deal_message_ptr, nullptr);
    if (valid_deal_message_ptr != nullptr)
    {
        ASSERT_EQ(valid_deal_message_ptr->type, DealType::TRICK);
        ASSERT_EQ(valid_deal_message_ptr->first_player, Position::North);
        ASSERT_EQ(valid_deal_message_ptr->cards.size(), 13);
        std::vector<std::string> figures = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
        for (int i = 0; i < 13; ++i)
        {
            ASSERT_EQ(valid_deal_message_ptr->cards[i].figure, figures[i]);
            ASSERT_EQ(valid_deal_message_ptr->cards[i].color, Color::Spades);
        }
    }
}

TEST(TRICKMessageSuite, Constructor)
{
    // Arrange
    int trick_number1 = 1;
    std::vector<Card> cards{Card("AS"), Card("KH"), Card("10D")};

    // Act & Assert
    TRICKMessage message1(trick_number1, cards);
    ASSERT_EQ(message1.trick_number, trick_number1);
    ASSERT_EQ(message1.cards.size(), cards.size());
    for (size_t i = 0; i < cards.size(); ++i)
    {
        ASSERT_EQ(message1.cards[i].figure, cards[i].figure);
        ASSERT_EQ(message1.cards[i].color, cards[i].color);
    }

    // Arrange
    std::vector<Card> cards_too_long{Card("AS"), Card("KH"), Card("10D"), Card("7C")};

    // Act & Assert
    ASSERT_THROW(TRICKMessage message2(trick_number1, cards_too_long), std::invalid_argument);
}

TEST(TRICKMessageSuite, ToString)
{
    // Arrange
    int trick_number = 1;
    std::vector<Card> cards{Card("AS"), Card("KH"), Card("10D")};
    TRICKMessage message(trick_number, cards);
    // Act
    std::string message_str = message.to_string();
    // Assert
    ASSERT_EQ(message_str, "TRICK1ASKH10D\r\n");
}

TEST(TRICKMessageSuite, FromString)
{
    // Arrange
    std::string message_str1 = "TRICK1ASKH10D\r\n";
    std::string message_str2 = "TRICK10ASKH10D\r\n";
    std::string message_str3 = "TRICK1ASKH10D";
    std::string message_str4 = "TRICKXASKH10D\r\n";
    std::string message_str5 = "TRICK1ASKH10DS\r\n";
    std::string message_str6 = "TRICK11ASKH10DS\r\n";
    std::string message_str7 = "TRICK1ASKH10DS2\r\n";
    std::string message_str8 = "TRICK14ASKH10DS2\r\n";
    std::string message_str9 = "TRICK110ASKH10DS2\r\n";
    std::string message_str10 = "TRICK0ASKH10DS2\r\n";

    // Act
    std::shared_ptr<Message> message_ptr1 = Message::from_string(message_str1);
    std::shared_ptr<Message> message_ptr2 = Message::from_string(message_str2);
    ASSERT_THROW(Message::from_string(message_str3), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str4), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str5), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str6), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str7), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str8), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str9), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str10), std::invalid_argument);

    // Assert
    std::shared_ptr<TRICKMessage> trick_message_ptr1 = std::dynamic_pointer_cast<TRICKMessage>(message_ptr1);
    ASSERT_NE(trick_message_ptr1, nullptr);
    if (trick_message_ptr1 != nullptr)
    {
        ASSERT_EQ(trick_message_ptr1->trick_number, 1);
        ASSERT_EQ(trick_message_ptr1->cards.size(), 3);
        ASSERT_EQ(trick_message_ptr1->cards[0].figure, "A");
        ASSERT_EQ(trick_message_ptr1->cards[0].color, Color::Spades);
        ASSERT_EQ(trick_message_ptr1->cards[1].figure, "K");
        ASSERT_EQ(trick_message_ptr1->cards[1].color, Color::Hearts);
        ASSERT_EQ(trick_message_ptr1->cards[2].figure, "10");
        ASSERT_EQ(trick_message_ptr1->cards[2].color, Color::Diamonds);
    }

    std::shared_ptr<TRICKMessage> trick_message_ptr2 = std::dynamic_pointer_cast<TRICKMessage>(message_ptr2);
    ASSERT_NE(trick_message_ptr2, nullptr);
    if (trick_message_ptr2 != nullptr)
    {
        ASSERT_EQ(trick_message_ptr2->trick_number, 10);
        ASSERT_EQ(trick_message_ptr2->cards.size(), 3);
        ASSERT_EQ(trick_message_ptr2->cards[0].figure, "A");
        ASSERT_EQ(trick_message_ptr2->cards[0].color, Color::Spades);
        ASSERT_EQ(trick_message_ptr2->cards[1].figure, "K");
        ASSERT_EQ(trick_message_ptr2->cards[1].color, Color::Hearts);
        ASSERT_EQ(trick_message_ptr2->cards[2].figure, "10");
        ASSERT_EQ(trick_message_ptr2->cards[2].color, Color::Diamonds);
    }
}

TEST(WRONGMessageSuite, Constructor)
{
    // Arrange
    int trick_number1 = 1;
    int trick_number2 = 14;
    int trick_number3 = 0;
    WRONGMessage message(trick_number1);
    ASSERT_THROW(WRONGMessage message2(trick_number2), std::invalid_argument);
    ASSERT_THROW(WRONGMessage message3(trick_number3), std::invalid_argument);

    // Assert
    ASSERT_EQ(message.trick_number, trick_number1);
}

TEST(WRONGMessageSuite, ToString)
{
    // Arrange
    int trick_number = 1;
    WRONGMessage message(trick_number);
    // Act
    std::string message_str = message.to_string();
    // Assert
    ASSERT_EQ(message_str, "WRONG1\r\n");
}

TEST(WRONGMessageSuite, FromString)
{
    // Arrange
    std::string message_str1 = "WRONG1\r\n";
    std::string message_str2 = "WRONG10\r\n";
    std::string message_str3 = "WRONG1";
    std::string message_str4 = "WRONGX1\r\n";
    std::string message_str5 = "WRONG1S\r\n";
    std::string message_str6 = "WRONG11S\r\n";
    std::string message_str7 = "WRONG1S2\r\n";
    std::string message_str8 = "WRONG14\r\n";
    std::string message_str9 = "WRONG110\r\n";
    std::string message_str10 = "WRONG0\r\n";

    // Act
    std::shared_ptr<Message> message_ptr1 = Message::from_string(message_str1);
    std::shared_ptr<Message> message_ptr2 = Message::from_string(message_str2);
    ASSERT_THROW(Message::from_string(message_str3), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str4), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str5), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str6), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str7), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str8), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str9), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message_str10), std::invalid_argument);

    // Assert
    std::shared_ptr<WRONGMessage> wrong_message_ptr1 = std::dynamic_pointer_cast<WRONGMessage>(message_ptr1);
    ASSERT_NE(wrong_message_ptr1, nullptr);
    if (wrong_message_ptr1 != nullptr)
    {
        ASSERT_EQ(wrong_message_ptr1->trick_number, 1);
    }

    std::shared_ptr<WRONGMessage> wrong_message_ptr2 = std::dynamic_pointer_cast<WRONGMessage>(message_ptr2);
    ASSERT_NE(wrong_message_ptr2, nullptr);
    if (wrong_message_ptr2 != nullptr)
    {
        ASSERT_EQ(wrong_message_ptr2->trick_number, 10);
    }
}

TEST(TAKENMessageSuite, Constructor)
{
    // Arrange
    int trick_number1 = 1;
    int trick_number2 = 14;
    int trick_number3 = 0;

    std::vector<Card> cards1{Card("AS"), Card("KH"), Card("10D"), Card("7C")};
    std::vector<Card> cards2{Card("AS"), Card("KH"), Card("10D")};

    Position position = Position::North;

    TAKENMessage message(trick_number1, cards1, position);
    ASSERT_THROW(TAKENMessage message2(trick_number2, cards1, position), std::invalid_argument);
    ASSERT_THROW(TAKENMessage message3(trick_number3, cards1, position), std::invalid_argument);
    ASSERT_THROW(TAKENMessage message4(trick_number1, cards2, position), std::invalid_argument);

    // Assert
    ASSERT_EQ(message.trick_number, trick_number1);
    ASSERT_EQ(message.cards.size(), cards1.size());
    for (size_t i = 0; i < cards1.size(); ++i)
    {
        ASSERT_EQ(message.cards[i].figure, cards1[i].figure);
        ASSERT_EQ(message.cards[i].color, cards1[i].color);
    }
    ASSERT_EQ(message.taken_by, position);
}

TEST(TAKENMessageSuite, ToString)
{
    // Arrange
    int trick_number = 1;
    std::vector<Card> cards{Card("AS"), Card("KH"), Card("10D"), Card("7C")};
    Position position = Position::North;
    TAKENMessage message(trick_number, cards, position);
    // Act
    std::string message_str = message.to_string();
    // Assert
    ASSERT_EQ(message_str, "TAKEN1ASKH10D7CN\r\n");
}

TEST(TAKENMessageSuite, FromString)
{
    // Arrange
    std::string message1 = "TAKEN1ASKH10D7CN\r\n";
    std::string message2 = "TAKEN13ASKH10D7CN\r\n";
    std::string message3 = "TAKEN1ASKS\r\n";
    std::string message4 = "TAKEN0ASKH10D7CN\r\n";
    std::string message5 = "TAKEN1ASKH10D7XCN\r\n";

    // Act
    std::shared_ptr<Message> message_ptr1 = Message::from_string(message1);
    std::shared_ptr<Message> message_ptr2 = Message::from_string(message2);
    ASSERT_THROW(Message::from_string(message3), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message4), std::invalid_argument);
    ASSERT_THROW(Message::from_string(message5), std::invalid_argument);

    // Assert
    std::shared_ptr<TAKENMessage> taken_message_ptr1 = std::dynamic_pointer_cast<TAKENMessage>(message_ptr1);
    ASSERT_NE(taken_message_ptr1, nullptr);
    if (taken_message_ptr1 != nullptr)
    {
        ASSERT_EQ(taken_message_ptr1->trick_number, 1);
        ASSERT_EQ(taken_message_ptr1->cards.size(), 4);
        ASSERT_EQ(taken_message_ptr1->cards[0].figure, "A");
        ASSERT_EQ(taken_message_ptr1->cards[0].color, Color::Spades);
        ASSERT_EQ(taken_message_ptr1->cards[1].figure, "K");
        ASSERT_EQ(taken_message_ptr1->cards[1].color, Color::Hearts);
        ASSERT_EQ(taken_message_ptr1->cards[2].figure, "10");
        ASSERT_EQ(taken_message_ptr1->cards[2].color, Color::Diamonds);
        ASSERT_EQ(taken_message_ptr1->cards[3].figure, "7");
        ASSERT_EQ(taken_message_ptr1->cards[3].color, Color::Clubs);
        ASSERT_EQ(taken_message_ptr1->taken_by, Position::North);
    }

    std::shared_ptr<TAKENMessage> taken_message_ptr2 = std::dynamic_pointer_cast<TAKENMessage>(message_ptr2);
    ASSERT_NE(taken_message_ptr2, nullptr);
    if (taken_message_ptr2 != nullptr)
    {
        ASSERT_EQ(taken_message_ptr2->trick_number, 13);
        ASSERT_EQ(taken_message_ptr2->cards.size(), 4);
        ASSERT_EQ(taken_message_ptr2->cards[0].figure, "A");
        ASSERT_EQ(taken_message_ptr2->cards[0].color, Color::Spades);
        ASSERT_EQ(taken_message_ptr2->cards[1].figure, "K");
        ASSERT_EQ(taken_message_ptr2->cards[1].color, Color::Hearts);
        ASSERT_EQ(taken_message_ptr2->cards[2].figure, "10");
        ASSERT_EQ(taken_message_ptr2->cards[2].color, Color::Diamonds);
        ASSERT_EQ(taken_message_ptr2->cards[3].figure, "7");
        ASSERT_EQ(taken_message_ptr2->cards[3].color, Color::Clubs);
        ASSERT_EQ(taken_message_ptr2->taken_by, Position::North);
    }
}

TEST(SCOREMessageSuite, Constructor)
{
    // Arrange
    std::map<Position, int> scores = {
        {Position::North, 10},
        {Position::East, 20},
        {Position::South, 30},
        {Position::West, 40}};

    // Act & Assert
    SCOREMessage scoreMessage(scores);
    ASSERT_EQ(scoreMessage.scores[Position::North], scores[Position::North]);
    ASSERT_EQ(scoreMessage.scores[Position::East], scores[Position::East]);
    ASSERT_EQ(scoreMessage.scores[Position::South], scores[Position::South]);
    ASSERT_EQ(scoreMessage.scores[Position::West], scores[Position::West]);

    // Arrange
    std::map<Position, int> scores_too_small = {
        {Position::North, 10},
        {Position::East, 20},
        {Position::South, 30}};

    // Act & Assert
    ASSERT_THROW(SCOREMessage scoreMessage2(scores_too_small), std::invalid_argument);
}

TEST(SCOREMessageSuite, ToString)
{
    // Arrange
    std::map<Position, int> scores = {
        {Position::North, 10},
        {Position::East, 20},
        {Position::South, 30},
        {Position::West, 40}};

    SCOREMessage scoreMessage(scores);

    // Act
    std::string message_str = scoreMessage.to_string();

    // Assert
    ASSERT_EQ(message_str, "SCOREN10E20S30W40\r\n");
}

TEST(SCOREMessageSuite, FromString)
{
    // Arrange
    std::string message1 = "SCOREN10E20S30W40\r\n";
    std::string message2 = "SCOREW40E20N10S30\r\n";
    std::string message3 = "SCOREN10E20S30\r\n";

    // Act
    std::shared_ptr<Message> message_ptr1 = Message::from_string(message1);
    std::shared_ptr<Message> message_ptr2 = Message::from_string(message2);
    ASSERT_THROW(Message::from_string(message3), std::invalid_argument);

    // Assert
    std::shared_ptr<SCOREMessage> score_message_ptr1 = std::dynamic_pointer_cast<SCOREMessage>(message_ptr1);
    ASSERT_NE(score_message_ptr1, nullptr);
    if (score_message_ptr1 != nullptr)
    {
        ASSERT_EQ(score_message_ptr1->scores[Position::North], 10);
        ASSERT_EQ(score_message_ptr1->scores[Position::East], 20);
        ASSERT_EQ(score_message_ptr1->scores[Position::South], 30);
        ASSERT_EQ(score_message_ptr1->scores[Position::West], 40);
    }

    std::shared_ptr<SCOREMessage> score_message_ptr2 = std::dynamic_pointer_cast<SCOREMessage>(message_ptr2);
    ASSERT_NE(score_message_ptr2, nullptr);
    if (score_message_ptr2 != nullptr)
    {
        ASSERT_EQ(score_message_ptr2->scores[Position::North], 10);
        ASSERT_EQ(score_message_ptr2->scores[Position::East], 20);
        ASSERT_EQ(score_message_ptr2->scores[Position::South], 30);
        ASSERT_EQ(score_message_ptr2->scores[Position::West], 40);
    }
}

TEST(TOTALMessageSuite, Constructor)
{
    // Arrange
    std::map<Position, int> totals = {
        {Position::North, 15},
        {Position::East, 25},
        {Position::South, 35},
        {Position::West, 45}};

    // Act & Assert
    TOTALMessage totalMessage(totals);
    ASSERT_EQ(totalMessage.totals[Position::North], totals[Position::North]);
    ASSERT_EQ(totalMessage.totals[Position::East], totals[Position::East]);
    ASSERT_EQ(totalMessage.totals[Position::South], totals[Position::South]);
    ASSERT_EQ(totalMessage.totals[Position::West], totals[Position::West]);

    // Arrange
    std::map<Position, int> totals_too_small = {
        {Position::North, 15},
        {Position::East, 25},
        {Position::South, 35}};

    // Act & Assert
    ASSERT_THROW(TOTALMessage totalMessage2(totals_too_small), std::invalid_argument);
}

TEST(TOTALMessageSuite, ToString)
{
    // Arrange
    std::map<Position, int> totals = {
        {Position::North, 15},
        {Position::East, 25},
        {Position::South, 35},
        {Position::West, 45}};

    TOTALMessage totalMessage(totals);

    // Act
    std::string message_str = totalMessage.to_string();

    // Assert
    ASSERT_EQ(message_str, "TOTALN15E25S35W45\r\n");
}

TEST(TOTALMessageSuite, FromString)
{
    // Arrange
    std::string message1 = "TOTALN15E25S35W45\r\n";
    std::string message2 = "TOTALW45E25N15S35\r\n";
    std::string message3 = "TOTALN15E25S35\r\n";

    // Act
    std::shared_ptr<Message> message_ptr1 = Message::from_string(message1);
    std::shared_ptr<Message> message_ptr2 = Message::from_string(message2);
    ASSERT_THROW(Message::from_string(message3), std::invalid_argument);

    // Assert
    std::shared_ptr<TOTALMessage> total_message_ptr1 = std::dynamic_pointer_cast<TOTALMessage>(message_ptr1);
    ASSERT_NE(total_message_ptr1, nullptr);
    if (total_message_ptr1 != nullptr)
    {
        ASSERT_EQ(total_message_ptr1->totals[Position::North], 15);
        ASSERT_EQ(total_message_ptr1->totals[Position::East], 25);
        ASSERT_EQ(total_message_ptr1->totals[Position::South], 35);
        ASSERT_EQ(total_message_ptr1->totals[Position::West], 45);
    }

    std::shared_ptr<TOTALMessage> total_message_ptr2 = std::dynamic_pointer_cast<TOTALMessage>(message_ptr2);
    ASSERT_NE(total_message_ptr2, nullptr);
    if (total_message_ptr2 != nullptr)
    {
        ASSERT_EQ(total_message_ptr2->totals[Position::North], 15);
        ASSERT_EQ(total_message_ptr2->totals[Position::East], 25);
        ASSERT_EQ(total_message_ptr2->totals[Position::South], 35);
        ASSERT_EQ(total_message_ptr2->totals[Position::West], 45);
    }
}