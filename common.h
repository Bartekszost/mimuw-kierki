#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <memory>

/**
 * @brief Converts a value to a string.
 * @tparam T The type of the value.
 * @param value The value to convert.
 * @return std::string The string representation of the value.
 */
template <typename T>
std::string to_string(T value);

/**
 * @brief Converts a string to a value.
 * @tparam T The type of the value.
 * @param str The string to convert.
 * @return T The value represented by the string.
 */
template <typename T>
T from_string(const std::string &str);

/**
 * @brief Represents the color of a playing card.
 */
enum class Color
{
    Hearts = 'H',
    Diamonds = 'D',
    Clubs = 'C',
    Spades = 'S'
};

/**
 * @brief Converts a color to a string.
 * @param color The color to convert.
 * @return std::string The string representation of the color.
 */
template <>
std::string to_string(Color color);

/**
 * @brief Converts a string to a color.
 * @param str The string to convert.
 * @return Color The color represented by the string.
 */
template <>
Color from_string(const std::string &str);

/**
 * @brief Enum class for different types of messages.
 */
enum class MessageType
{
    IAM,
    BUSY,
    DEAL,
    TRICK,
    WRONG,
    TAKEN,
    SCORE,
    TOTAL
};

/**
 * @brief Converts a MessageType to a string.
 * @param type The MessageType to convert.
 * @return std::string The string representation of the MessageType.
 */
template <>
std::string to_string(MessageType type);

/**
 * @brief Converts a string to a MessageType.
 * @param str The string to convert.
 * @return MessageType The MessageType represented by the string.
 */
template <>
MessageType from_string(const std::string &str);

/**
 * @brief An enumeration of possible player positions.
 */
enum class Position
{
    North = 'N',
    East = 'E',
    South = 'S',
    West = 'W'
};

/**
 * @brief Converts a Position to a string.
 * @param position The Position to convert.
 * @return std::string The string representation of the Position.
 */
template <>
std::string to_string(Position position);

/**
 * @brief Converts a string to a Position.
 * @param str The string to convert.
 * @return Position The Position represented by the string.
 */
template <>
Position from_string(const std::string &str);

/**
 * @brief Enum class for different types of deals.
 */
enum class DealType
{
    TRICK = 1,
    HEART = 2,
    QUEEN = 3,
    LORD = 4,
    KING_HEART = 5,
    SEVENTH_LAST = 6,
    BANDIT = 7,
};

/**
 * @brief Converts a DealType to a string.
 * @param type The DealType to convert.
 * @return std::string The string representation of the DealType.
 */
template <>
std::string to_string(DealType type);

/**
 * @brief Converts a string to a DealType.
 * @param str The string to convert.
 * @return DealType The DealType represented by the string.
 */
template <>
DealType from_string(const std::string &str);

/**
 * @brief Represents a playing card.
 */
class Card
{
public:
    std::string figure;
    Color color;

    /**
     * @brief Constructs a Card from a string.
     * @param str The string representation of the card.
     * @throws std::invalid_argument If the string is not a valid card representation.
     */
    Card(const std::string &str);

    /**
     * @brief Constructs a Card with the given figure and color.
     * @param figure The figure of the card.
     * @param color The color of the card.
     */
    Card(const std::string &figure, Color color);

    /**
     * @brief Compares this card to another card.
     * @param other The other card to compare to.
     * @return true if this card is considered weaker than the other card, false otherwise.
     */
    bool operator<(const Card &other) const;

    /**
     * @brief Compares this card to another card.
     * @param other The other card to compare to.
     * @return true if this card is considered stronger than the other card, false otherwise.
     */
    bool operator==(const Card &other) const;

    /**
     * @brief Compares this card to another card, with a special color that is considered stronger.
     * @param other The other card to compare to.
     * @param specialColor The color that is considered stronger (trump color).
     * @return true if this card is considered weaker than the other card, false otherwise.
     */
    bool compare(const Card &other, Color specialColor) const;

    /**
     * @brief Converts the card to a string.
     * @return std::string The string representation of the card.
     */
    std::string to_string() const;

    /**
     * @brief Parses a string to extract card data.
     *
     * This method takes a string containing card data and parses it into a vector of Card objects.
     * Each card in the string should be represented as <figure><color>.
     *
     * @param card_list The string to parse. Expected to contain a list of cards in the format <figure><color>.
     * @return A vector of Card objects extracted from the string.
     */
    static std::vector<Card> parse_cards(const std::string &card_list);

private:
    int value;

    /**
     * @brief Validates the figure and color of the card and sets the value.
     * @throws std::invalid_argument If the figure or color is invalid.
     */
    void validate_and_evaluate();
};

/**
 * @brief Class representing a message.
 */
class Message
{
public:
    MessageType type;
    std::string data;
    /**
     * @brief Construct a new Message object.
     *
     * @param type The type of the message.
     * @param data The data of the message.
     */
    Message(MessageType type, const std::string &data);

    /**
     * @brief Construct a new Message object.
     *
     * @param type The type of the message.
     */
    virtual ~Message() = default;

    /**
     * @brief Convert the message to a string.
     *
     * @return std::string The string representation of the message.
     */
    std::string to_string() const;

    /**
     * @brief Create a Message object from a string.
     *
     * @param str The string to create the Message object from.
     * @return Message The created Message object.
     */
    static std::shared_ptr<Message> from_string(const std::string &str);
};

/**
 * @brief A class for IAM messages, derived from the Message class.
 */
class IAMMessage : public Message
{
public:
    Position position;

    /**
     * @brief Construct a new IAMMessage object.
     *
     * @param position The position for the IAMMessage.
     */
    IAMMessage(Position position);

    /**
     * @brief Create an IAMMessage object from a string.
     *
     * @param str The string to create the IAMMessage object from.
     * @return IAMMessage The created IAMMessage object.
     */
    static std::shared_ptr<IAMMessage> from_string(const std::string &str);
};

/**
 * @class BUSYMessage
 * @brief A class for BUSY messages, derived from the Message class.
 */
class BUSYMessage : public Message
{
public:
    std::vector<Position> positions;
    /**
     * @brief Construct a new BUSYMessage object.
     *
     * @param positions A vector of positions for the BUSYMessage.
     */
    BUSYMessage(std::vector<Position> positions);

    /**
     * @brief Create a BUSYMessage object from a string.
     *
     * @param str The string to create the BUSYMessage object from.
     * @return BUSYMessage The created BUSYMessage object.
     */
    static std::shared_ptr<BUSYMessage> from_string(const std::string &str);
};

/**
 * @brief A class for DEAL messages, derived from the Message class.
 */
class DEALMessage : public Message
{
public:
    DealType type;
    Position first_player;
    std::vector<Card> cards;

    /**
     * @brief Construct a new DEALMessage object.
     *
     * @param deal_type The type of the deal.
     * @param first_player The starting player.
     * @param cards The cards dealt.
     */
    DEALMessage(DealType type, Position first_player, const std::vector<Card> &cards);

    /**
     * @brief Create a DEALMessage object from a string.
     *
     * @param str The string to create the DEALMessage object from.
     * @return DEALMessage The created DEALMessage object.
     */
    static std::shared_ptr<DEALMessage> from_string(const std::string &str);
};

/**
 * @brief A class for TRICK messages, derived from the Message class.
 */
class TRICKMessage : public Message
{
public:
    int trick_number;
    std::vector<Card> cards;

    /**
     * @brief Construct a new TRICKMessage object.
     *
     * @param trick_number The number of the trick.
     * @param cards The cards played in the trick.
     */
    TRICKMessage(int trick_number, const std::vector<Card> &cards);

    /**
     * @brief Create a TRICKMessage object from a string.
     *
     * @param str The string to create the TRICKMessage object from.
     * @return TRICKMessage The created TRICKMessage object.
     */
    static std::shared_ptr<TRICKMessage> from_string(const std::string &str);
};

/**
 * @brief A class for WRONG messages, derived from the Message class.
 */
class WRONGMessage : public Message
{
public:
    int trick_number;

    /**
     * @brief Construct a new WRONGMessage object.
     *
     * @param trick_number The number of the trick.
     */
    WRONGMessage(int trick_number);

    /**
     * @brief Create a WRONGMessage object from a string.
     *
     * @param str The string to create the WRONGMessage object from.
     * @return WRONGMessage The created WRONGMessage object.
     */
    static std::shared_ptr<WRONGMessage> from_string(const std::string &str);
};

/**
 * @brief A class for TAKEN messages, derived from the Message class.
 */

class TAKENMessage : public Message
{
public:
    int trick_number;
    std::vector<Card> cards;
    Position taken_by;

    /**
     * @brief Construct a new TAKENMessage object.
     *
     * @param trick_number The number of the trick.
     * @param cards The cards taken in the trick.
     * @param taken_by The player who took the trick.
     */
    TAKENMessage(int trick_number, const std::vector<Card> &cards, Position taken_by);

    /**
     * @brief Create a TAKENMessage object from a string.
     *
     * @param str The string to create the TAKENMessage object from.
     * @return TAKENMessage The created TAKENMessage object.
     */
    static std::shared_ptr<TAKENMessage> from_string(const std::string &str);
};

/**
 * @brief A class for SCORE messages, derived from the Message class.
 */
class SCOREMessage : public Message
{
public:
    std::map<Position, int> scores;

    /**
     * @brief Construct a new SCOREMessage object.
     *
     * @param scores A map of positions to scores.
     */
    SCOREMessage(const std::map<Position, int> &scores);

    /**
     * @brief Create a SCOREMessage object from a string.
     *
     * @param str The string to create the SCOREMessage object from.
     * @return SCOREMessage The created SCOREMessage object.
     */
    static std::shared_ptr<SCOREMessage> from_string(const std::string &str);
};

/**
 * @brief A class for TOTAL messages, derived from the Message class.
 */
class TOTALMessage : public Message
{
public:
    std::map<Position, int> totals;

    /**
     * @brief Construct a new TOTALMessage object.
     *
     * @param totals A map of positions to totals.
     */
    TOTALMessage(const std::map<Position, int> &totals);

    /**
     * @brief Create a TOTALMessage object from a string.
     *
     * @param str The string to create the TOTALMessage object from.
     * @return TOTALMessage The created TOTALMessage object.
     */
    static std::shared_ptr<TOTALMessage> from_string(const std::string &str);
};

#endif // COMMON_H