#include <regex>
#include <set>

#include "common.h"

template <typename T>
std::string to_string(T value)
{
    static_assert(std::is_enum<T>::value, "to_string is available only for enums");
    return "";
}

template <typename T>
T from_string(const std::string &str)
{
    static_assert(std::is_enum<T>::value, "from_string is available only for enums");
    return T{};
}

template <>
std::string to_string(Color color)
{
    return std::string(1, static_cast<char>(color));
}

template <>
Color from_string(const std::string &str)
{
    if (str.size() != 1 || (str[0] != 'H' && str[0] != 'D' && str[0] != 'C' && str[0] != 'S'))
        throw std::invalid_argument("Invalid color string");

    return static_cast<Color>(str[0]);
}

template <>
std::string to_string(MessageType type)
{
    static std::map<MessageType, std::string> type_to_string = {
        {MessageType::IAM, "IAM"},
        {MessageType::BUSY, "BUSY"},
        {MessageType::DEAL, "DEAL"},
        {MessageType::TRICK, "TRICK"},
        {MessageType::WRONG, "WRONG"},
        {MessageType::TAKEN, "TAKEN"},
        {MessageType::SCORE, "SCORE"},
        {MessageType::TOTAL, "TOTAL"}};

    return type_to_string[type];
}

template <>
MessageType from_string(const std::string &str)
{
    static std::map<std::string, MessageType> string_to_type = {
        {"IAM", MessageType::IAM},
        {"BUSY", MessageType::BUSY},
        {"DEAL", MessageType::DEAL},
        {"TRICK", MessageType::TRICK},
        {"WRONG", MessageType::WRONG},
        {"TAKEN", MessageType::TAKEN},
        {"SCORE", MessageType::SCORE},
        {"TOTAL", MessageType::TOTAL}};

    if (string_to_type.find(str) == string_to_type.end())
        throw std::invalid_argument("Invalid message type string");

    return string_to_type[str];
}

template <>
std::string to_string<Position>(Position position)
{
    switch (position)
    {
    case Position::North:
        return "N";
    case Position::East:
        return "E";
    case Position::South:
        return "S";
    case Position::West:
        return "W";
    default:
        throw std::invalid_argument("Invalid position");
    }
}

template <>
Position from_string<Position>(const std::string &str)
{
    if (str == "N")
        return Position::North;
    else if (str == "E")
        return Position::East;
    else if (str == "S")
        return Position::South;
    else if (str == "W")
        return Position::West;
    else
        throw std::invalid_argument("Invalid position string");
}

template <>
std::string to_string<DealType>(DealType deal_type)
{
    int value = static_cast<int>(deal_type);
    return std::to_string(value);
}

template <>
DealType from_string<DealType>(const std::string &str)
{
    int value;
    try
    {
        value = std::stoi(str);
    }
    catch (const std::invalid_argument &)
    {
        throw std::invalid_argument("Invalid deal type string");
    }

    if (value < 1 || value > 7)
        throw std::invalid_argument("Invalid deal type string");

    return static_cast<DealType>(value);
}

Card::Card(const std::string &str)
{
    if (str.size() < 2)
        throw std::invalid_argument("Invalid card string");

    figure = str.substr(0, str.size() - 1);
    color = from_string<Color>(str.substr(str.size() - 1));
    validate_and_evaluate();
}

Card::Card(const std::string &figure, Color color) : figure(figure), color(color)
{
    validate_and_evaluate();
}

bool Card::operator<(const Card &other) const
{
    if (color != other.color)
    {
        return color < other.color;
    }
    return figure < other.figure;
}

bool Card::operator==(const Card &other) const
{
    return this->figure == other.figure && this->color == other.color;
}

bool Card::compare(const Card &other, Color specialColor) const
{
    if (color == other.color)
        return value < other.value;

    if (color == specialColor)
        return false;
    if (other.color == specialColor)
        return true;

    return false;
}

std::string Card::to_string() const
{
    return figure + ::to_string<Color>(color);
}

void Card::validate_and_evaluate()
{
    switch (figure[0])
    {
    case 'J':
        value = 11;
        break;
    case 'Q':
        value = 12;
        break;
    case 'K':
        value = 13;
        break;
    case 'A':
        value = 14;
        break;
    default:
        try
        {
            size_t pos;
            value = std::stoi(figure, &pos);
            if (pos != figure.size() || value < 2 || value > 10)
                throw std::invalid_argument("Invalid figure");
        }
        catch (const std::invalid_argument &)
        {
            throw std::invalid_argument("Invalid figure");
        }
    }

    if (color != Color::Hearts && color != Color::Diamonds && color != Color::Clubs && color != Color::Spades)
        throw std::invalid_argument("Invalid color");
}

std::vector<Card> Card::parse_cards(const std::string &card_list)
{
    std::vector<Card> cards;
    std::set<Card> unique_cards;
    std::regex card_pattern(R"((10|[2-9JQKA])[SHDC])");
    std::sregex_iterator iter(card_list.begin(), card_list.end(), card_pattern);
    std::sregex_iterator end;

    int total_match_length = 0;

    while (iter != end)
    {
        try
        {
            Card card(iter->str());
            cards.push_back(card);
            unique_cards.insert(card);
            total_match_length += iter->str().length();
            ++iter;
        }
        catch (const std::invalid_argument &e)
        {
            throw std::invalid_argument("Invalid card list");
        }
    }

    if (total_match_length != (int)card_list.length())
    {
        throw std::invalid_argument("Invalid card list");
    }

    if (cards.size() != unique_cards.size())
        throw std::invalid_argument("Duplicate cards are not allowed");

    return cards;
}

Message::Message(MessageType type, const std::string &data)
{
    this->type = type;
    this->data = data;
}

std::string Message::to_string() const
{
    std::string header = ::to_string<MessageType>(type);

    return header + data + "\r\n";
}

std::shared_ptr<Message> Message::from_string(const std::string &str)
{
    if (str.size() < 5)
        throw std::invalid_argument("Invalid message string");

    if (str.substr(str.size() - 2) != "\r\n")
        throw std::invalid_argument("Message string does not end with \\r\\n");

    std::regex header_pattern("(IAM|BUSY|DEAL|TRICK|WRONG|TAKEN|SCORE|TOTAL)");

    if (!std::regex_search(str.begin(), str.begin() + 5, header_pattern))
        throw std::invalid_argument("Invalid message header");

    std::string new_str = str.substr(0, str.size() - 2);

    if (new_str.substr(0, 3) == "IAM")
        return IAMMessage::from_string(new_str);
    if (new_str.substr(0, 4) == "BUSY")
        return BUSYMessage::from_string(new_str);
    if (new_str.substr(0, 4) == "DEAL")
        return DEALMessage::from_string(new_str);
    if (new_str.substr(0, 5) == "TRICK")
        return TRICKMessage::from_string(new_str);
    if (new_str.substr(0, 5) == "WRONG")
        return WRONGMessage::from_string(new_str);
    if (new_str.substr(0, 5) == "TAKEN")
        return TAKENMessage::from_string(new_str);
    if (new_str.substr(0, 5) == "SCORE")
        return SCOREMessage::from_string(new_str);
    if (new_str.substr(0, 5) == "TOTAL")
        return TOTALMessage::from_string(new_str);

    throw std::invalid_argument("Invalid message header");
}

IAMMessage::IAMMessage(Position position)
    : Message(MessageType::IAM, std::string(1, static_cast<char>(position))), position(position)
{
}

std::shared_ptr<IAMMessage> IAMMessage::from_string(const std::string &str)
{
    if (str.size() != 4)
        throw std::invalid_argument("Invalid IAM message string");

    Position position = ::from_string<Position>(str.substr(3, 1));

    return std::make_shared<IAMMessage>(position);
}

BUSYMessage::BUSYMessage(std::vector<Position> positions)
    : Message(MessageType::BUSY, "")
{
    for (Position position : positions)
        this->data += ::to_string<Position>(position);
    this->positions = positions;
}

std::shared_ptr<BUSYMessage> BUSYMessage::from_string(const std::string &str)
{
    if (str.size() < 4 || str.size() > 8)
        throw std::invalid_argument("Invalid BUSY message string");

    std::vector<Position> positions;
    std::set<Position> unique_positions;
    for (size_t i = 4; i < str.size(); i++)
    {
        Position position = ::from_string<Position>(str.substr(i, 1));
        positions.push_back(position);
        unique_positions.insert(position);
    }

    if (positions.size() != unique_positions.size())
        throw std::invalid_argument("Duplicate positions are not allowed");

    return std::make_shared<BUSYMessage>(positions);
}

DEALMessage::DEALMessage(DealType deal_type, Position first_player, const std::vector<Card> &cards)
    : Message(MessageType::DEAL, "")
{
    this->data += ::to_string<DealType>(deal_type);
    this->data += ::to_string<Position>(first_player);
    if (cards.size() != 13)
        throw std::invalid_argument("Invalid number of cards in the deal");
    for (const Card &card : cards)
        this->data += card.to_string();
    this->type = deal_type;
    this->first_player = first_player;
    this->cards = cards;
}

std::shared_ptr<DEALMessage> DEALMessage::from_string(const std::string &str)
{
    if (str.size() < 8)
        throw std::invalid_argument("Invalid DEAL message string");

    DealType deal_type = ::from_string<DealType>(str.substr(4, 1));
    Position starting_player = ::from_string<Position>(str.substr(5, 1));
    std::vector<Card> cards = Card::parse_cards(str.substr(6));

    return std::make_shared<DEALMessage>(deal_type, starting_player, cards);
}

TRICKMessage::TRICKMessage(int trick_number, const std::vector<Card> &cards)
    : Message(MessageType::TRICK, "")
{
    if (trick_number < 1 || trick_number > 13)
        throw std::invalid_argument("Invalid trick number");

    if (cards.size() > 3)
        throw std::invalid_argument("Too many cards in the trick");

    this->data += std::to_string(trick_number);
    for (const Card &card : cards)
        this->data += card.to_string();

    this->trick_number = trick_number;
    this->cards = cards;
}

std::shared_ptr<TRICKMessage> TRICKMessage::from_string(const std::string &str)
{
    if (str.size() < 6)
        throw std::invalid_argument("Invalid TRICK message string");

    int trick_number;
    std::vector<Card> cards;

    try
    {
        trick_number = std::stoi(str.substr(5, 1));
        cards = Card::parse_cards(str.substr(6));
    }
    catch (const std::invalid_argument &)
    {
        try
        {
            trick_number = std::stoi(str.substr(5, 2));
            cards = Card::parse_cards(str.substr(7));
        }
        catch (const std::invalid_argument &)
        {
            throw std::invalid_argument("Invalid TRICK message string");
        }
    }

    return std::make_shared<TRICKMessage>(trick_number, cards);
}

WRONGMessage::WRONGMessage(int trick_number)
    : Message(MessageType::WRONG, std::to_string(trick_number)), trick_number(trick_number)
{
    if (trick_number < 1 || trick_number > 13)
        throw std::invalid_argument("Invalid trick number");
}

std::shared_ptr<WRONGMessage> WRONGMessage::from_string(const std::string &str)
{
    if (str.size() < 6 || str.size() > 7)
        throw std::invalid_argument("Invalid WRONG message string");

    int trick_number;
    try
    {
        size_t pos;
        trick_number = std::stoi(str.substr(5), &pos);
        if (pos != str.substr(5).size())
            throw std::invalid_argument("Invalid WRONG message string");
    }
    catch (const std::invalid_argument &)
    {
        throw std::invalid_argument("Invalid WRONG message string");
    }

    return std::make_shared<WRONGMessage>(trick_number);
}

TAKENMessage::TAKENMessage(int trick_number, const std::vector<Card> &cards, Position taken_by)
    : Message(MessageType::TAKEN, "")
{
    if (trick_number < 1 || trick_number > 13)
        throw std::invalid_argument("Invalid trick number");

    if (cards.size() != 4)
        throw std::invalid_argument("Invalid number of cards in the trick");

    this->data += std::to_string(trick_number);
    for (const Card &card : cards)
        this->data += card.to_string();
    this->data += ::to_string<Position>(taken_by);

    this->trick_number = trick_number;
    this->cards = cards;
    this->taken_by = taken_by;
}

std::shared_ptr<TAKENMessage> TAKENMessage::from_string(const std::string &str)
{
    if (str.size() < 9)
        throw std::invalid_argument("Invalid TAKEN message string");

    int trick_number;
    std::vector<Card> cards;
    Position taken_by;

    try
    {
        trick_number = std::stoi(str.substr(5, 1));
        cards = Card::parse_cards(str.substr(6, str.size() - 7));
        taken_by = ::from_string<Position>(str.substr(str.size() - 1));
    }
    catch (const std::invalid_argument &)
    {
        try
        {
            trick_number = std::stoi(str.substr(5, 2));
            cards = Card::parse_cards(str.substr(7, str.size() - 8));
            taken_by = ::from_string<Position>(str.substr(str.size() - 1));
        }
        catch (const std::invalid_argument &)
        {
            throw std::invalid_argument("Invalid TAKEN message string");
        }
    }

    return std::make_shared<TAKENMessage>(trick_number, cards, taken_by);
}

template <typename T>
std::shared_ptr<T> parse_score_message(const std::string &str, const std::string &regex_str)
{
    std::regex message_regex(regex_str);
    std::smatch match;

    if (!std::regex_match(str, match, message_regex))
    {
        throw std::invalid_argument("Invalid message string");
    }

    std::map<Position, int> values;
    for (size_t i = 1; i < match.size(); i++)
    {
        if (match[i].str().empty())
            continue;
        Position position = ::from_string<Position>(std::string(1, match[i].str()[0]));
        int value = std::stoi(match[i].str().substr(1));
        values[position] = value;
    }

    return std::make_shared<T>(values);
}

SCOREMessage::SCOREMessage(const std::map<Position, int> &scores)
    : Message(MessageType::SCORE, "")
{
    if (scores.size() != 4)
    {
        throw std::invalid_argument("Invalid scores map size");
    }

    for (auto pos : {Position::North, Position::East, Position::South, Position::West})
    {
        auto it = scores.find(pos);
        if (it == scores.end())
        {
            throw std::invalid_argument("Missing score for position");
        }
        this->data += ::to_string<Position>(pos);
        this->data += std::to_string(it->second);
    }

    this->scores = scores;
}

std::shared_ptr<SCOREMessage> SCOREMessage::from_string(const std::string &str)
{
    return parse_score_message<SCOREMessage>(str, "^SCORE((N[0-9]{1,})|(E[0-9]{1,})|(S[0-9]{1,})|(W[0-9]{1,})){4}$");
}

TOTALMessage::TOTALMessage(const std::map<Position, int> &totals)
    : Message(MessageType::TOTAL, "")
{
    if (totals.size() != 4)
    {
        throw std::invalid_argument("Invalid totals map size");
    }

    for (auto pos : {Position::North, Position::East, Position::South, Position::West})
    {
        auto it = totals.find(pos);
        if (it == totals.end())
        {
            throw std::invalid_argument("Missing total for position");
        }
        this->data += ::to_string<Position>(pos);
        this->data += std::to_string(it->second);
    }

    this->totals = totals;
}

std::shared_ptr<TOTALMessage> TOTALMessage::from_string(const std::string &str)
{
    return parse_score_message<TOTALMessage>(str, "^TOTAL((N[0-9]{1,})|(E[0-9]{1,})|(S[0-9]{1,})|(W[0-9]{1,})){4}$");
}
