## ft_irc
An IRC server written from scratch in C++98.

The server handles multiple client connections and implements a subset of the IRC 2812 protocol, including user authentication, channels, private messaging, and basic command execution.

Good for learning sockets, poll/epoll, and protocol handling.

Not meant to be a full production IRC server.

## Usage
First do `make` and create the server.

Usage of server is as follows: `./ircserv <port> <password>`.

Once the server is up and running you can connect to it through IRC client or `nc` command.

Example (using `irssi`):  
`irssi -c <server_ip> -p <server_port> -w <server_pass>` optional `-n <desired_nickname>`.
