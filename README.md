This project has been created as part of the 42 curriculum by nachabi-, aelmsafe, yel-ouam.

## Description

**ft_irc** is a lightweight, multi-client IRC (Internet Relay Chat) server implementation written in C++98. This project aims to replicate the core functionality of an IRC server, allowing multiple clients to connect, authenticate, create and join channels, and exchange messages in real-time.

The server handles:
- Client authentication and registration
- Channel creation and management
- Private and channel messaging
- Operator privileges and channel administration
- Basic IRC commands (JOIN, NICK, USER, PRIVMSG, etc.)
- Multiple client connections via socket programming
- Non-blocking I/O using `poll()` for efficient client handling

This project demonstrates:
- Network programming fundamentals
- Client-server architecture
- Socket communication and multiplexing
- Protocol parsing and implementation
- Concurrent client handling without threading

## Instructions
### Compilation

Clone this repository to your local machine:

git clone https://github.com/0obito/IRC.git
cd IRC

make

This will generate the executable `ircserv`.

### Execution

./ircserv <port> <password>

- `<port>`: The port number on which the server will listen (1024-65535)
- `<password>`: The connection password required for client authentication

### Usage Examples

1. **Start the server:**
   ./ircserv 6667 mysecretpassword

2. **Connect with an IRC client:**
   
   A client such as `nc`, HexChat, or another IRC client can be used to connect to the server.

For example, using Netcat:

nc 127.0.0.1 4343

Then register the client:

PASS 333
NICK someone
USER someone 0 * :someone's-name

Once registered, the client can use IRC commands such as:

JOIN #channel
PRIVMSG #channel :Hello everyone!
PART #channel

3. **IRC commands supported:**
### Registration

* `PASS` — provides the server password.
* `NICK` — sets or changes the client's nickname.
* `USER` — provides the client's username and real name.

### Channel Commands

* `JOIN` — joins or creates a channel.
* `PART` — leaves a channel.
* `TOPIC` — views or changes a channel topic.
* `INVITE` — invites a user to an invite-only channel.
* `KICK` — removes a user from a channel.
* `MODE` — changes channel modes.

### Messaging

* `PRIVMSG` — sends a private message to a user or a message to a channel.

### Operator Commands

Channel operators can perform privileged actions such as:

* Kicking users
* Inviting users
* Changing channel topics
* Changing supported channel modes
* Granting or removing operator privileges

## Resources

The following resources were used to understand the IRC protocol, networking, and C++ concepts required for the project:

* RFC 1459 — Internet Relay Chat Protocol
* RFC 2812 — Internet Relay Chat: Client Protocol
* C++ documentation for classes, containers, strings, and standard library features
* Linux socket programming documentation
* Documentation for `socket()`, `bind()`, `listen()`, `accept()`, `send()`, `recv()`, and `fcntl()`
* Documentation and tutorials about TCP client/server communication
* Documentation about non-blocking sockets and event-driven network programming

#### AI Usage

AI tools were used as a learning and development aid during the project.

They were mainly used for:

* Explaining IRC protocol concepts and RFC terminology.
* Clarifying networking concepts such as sockets, TCP connections, non-blocking sockets, and event-driven communication.
* Explaining system calls such as `socket`, `bind`, `listen`, `accept`, `recv`, and `send`.
* Helping understand event-handling mechanisms such as `epoll`.
* Discussing the structure and responsibilities of classes such as `Server`, `Client`, and `Channel`.
* Helping understand and debug IRC message parsing.
* Reviewing code logic and identifying possible bugs during development.
* Providing explanations and suggestions when debugging compilation or runtime errors.
* Helping design and reason about command handlers and IRC protocol behavior.

AI was used as a supplementary learning and debugging tool. The project code was implemented, tested, and adapted by the project authors.