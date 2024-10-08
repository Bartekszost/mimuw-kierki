# mimuw-kierki

Computer Networks course second project on MIMUW

It is a Kierki (Black lady) card game.

## Game Rules

The game is played by four players using a standard 52-card deck. Players are seated at the table in positions N (North), E (East), S (South), and W (West). The game consists of multiple deals. In each deal, every player receives 13 cards. A player knows only their own cards. The game consists of 13 tricks. In the first trick, a chosen player leads by playing one of their cards. The remaining players, in clockwise order, play one of their cards. There is an obligation to follow suit. If a player doesn't have a card in the required suit, they can play a card in any other suit. There is no obligation to play a higher card. The player who plays the highest card in the suit led takes the trick and leads in the next trick. Standard card ranks apply (from weakest): 2, 3, 4, ..., 9, 10, Jack, Queen, King, Ace.

The objective of the game is to take as few cards as possible. Points are awarded for taking cards. The player with the fewest points at the end of the game wins. There are seven types of deals:

- Avoid taking tricks: 1 point for each trick taken.
- Avoid taking hearts: 1 point for each heart taken.
- Avoid taking queens: 5 points for each queen taken.
- Avoid taking face cards (Jacks and Kings): 2 points for each face card taken.
- Avoid taking the King of hearts: 18 points for taking this card.
- Avoid taking the seventh and last tricks: 10 points for each of these tricks taken.
- Robber: Points are awarded for all the above conditions.

A deal can be stopped before completion if all points have already been allocated.

## Server Invocation Parameters

Parameters can be given in any order. If a parameter is provided multiple times, the first or last occurrence in the list applies.

- `-p <port>`
    Specifies the port number the server should listen on. This parameter is optional. If it is not provided or is zero, the port selection should be deferred to the bind function.

- `-f <file>`
    Specifies the name of the file containing the game definition. This parameter is mandatory.

- `-t <timeout>`
    Specifies the maximum time in seconds for the server to wait. This is a positive number. If this parameter is not provided, the time is 5 seconds by default.

## Client Invocation Parameters

Parameters can be given in any order. If a parameter is provided multiple times or conflicting parameters are given, the first or last occurrence applies.

- `-h <host>`
    Specifies the IP address or hostname of the server. This parameter is mandatory.

- `-p <port>`
    Specifies the port number the server is listening on. This parameter is mandatory.

- `-4`
    Forces the use of IPv4 in communication with the server. This parameter is optional.

- `-6`
    Forces the use of IPv6 in communication with the server. This parameter is optional.

If neither the `-4` nor the `-6` parameter is provided, the IP version selection is deferred to the getaddrinfo function, with `ai_family = AF_UNSPEC`.

- `-N`, `-E`, `-S`, `-W`
    Specifies the seat the client wants to take at the table. This parameter is mandatory.

- `-a`
    This parameter is optional. If provided, the client acts as an automatic player. If not, the client acts as an intermediary between the server and the player-user.

## Communication Protocol

The server and client communicate using TCP. Messages are ASCII strings terminated by the sequence `\r\n`. Apart from this sequence, there are no other whitespace characters in the messages. Messages do not contain a terminal null character. The seat at the table is encoded as the letter `N`, `E`, `S`, or `W`. The type of deal is encoded as a digit from 1 to 7. The trick number is encoded as a number from 1 to 13 written in base 10 without leading zeros. Cards are encoded by specifying their value first:

- **2, 3, 4, …, 9, 10, J, Q, K, A**

Then the suit is specified:

- C – ♣, club
- D – ♦, diamond
- H – ♥, heart
- S – ♠, spade

## Messages Between Server and Client

- `IAM<place on the table>\r\n`
    Sent by the client to the server after establishing a connection. Informs which seat the client wants to take. If the client doesn't send this message within the timeout period, the server closes the connection with this client. This also applies to clients that send an incorrect message.

- `BUSY<list of taken places>\r\n`
    Sent by the server to the client if the chosen seat is already occupied. It also informs the client which seats are taken. After sending this message, the server closes the connection.

- `DEAL<deal type><place of the starting player><list of cards>\r\n`
    Sent by the server to clients after gathering four clients. Informs about the start of the deal. The list contains 13 cards dealt to the client.

- `TRICK<trick number><card list>\r\n`
    Sent by the server to the client requesting a card to be played. The list contains between zero and three cards currently on the table. If the client doesn't respond within the timeout, the server repeats the request.

- `WRONG<trick number>\r\n`
    Sent by the server to the client who sent an incorrect message in response to a TRICK message.

- `TAKEN<trick number><card list><place of the player that takes the trick>\r\n`
    Sent by the server to clients. Informs which client took the trick.

- `SCORE<place><point count>... \r\n`
    Sent by the server to clients after the deal ends. Informs about the scores for the deal.

- `TOTAL<place><point count>... \r\n`
    Sent by the server to clients after the deal ends. Informs about the total scores for the game.