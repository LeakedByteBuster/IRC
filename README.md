# IRC

Our Internet Relay Chat (IRC) server works prefectly with clients such as LimeChat (User friendly graphical interface) or `nc` command.
The server features :
- private messages (1 to 1)
- channels (group chat) / operators / priviligies
- file transfert

This server supports the following commands :
- SENDFILE    
- GETFILE    
- NICK    
- PASS_USER    
- PRVMSG
- PONG
- IRCBOT
- DATE
- JOKE
- WHOAMI
- JOIN
- MODE
- KICK
- TOPIC
- INVITE
- QUIT
- PART

The main source for implementing these commands is [Modern IRC Client Protocol](https://modern.ircdocs.horse/)

# Usage

```bash
# Compile files
$> make

# run the server
$> ./ircserv <serverip> <port> <pass>

# example
$> ./ircserv localhost 9999 0000

```

```bash
# Compile file
$> make build=bot

# run the bot
$> ./a.out <serverip> <port> <pass>

# example
$> ./a.out localhost 9999 0000

```


```bash
# connect a client using nc command
$> nc -c <serverip> <port> <pass>
PASS <password>
NICK <nickname>
USER <username> 0 * <realname>

# example
$> nc -c localhost 9999 0000
PASS 0000
NICK Tommy
USER Homy 0 * lony
```

```bash
# Cleanning
$> make clean
$> make flclean
$> make fcleanbot
$> make clean

# Clean and recompile
$> make re
```

## Sources :

[what is socket programming ?](https://www.scaler.com/topics/socket-programming-in-c/)

[Command Reference](https://dd.ircdocs.horse/refs/commands/)

[Modern IRC Client Protocol](https://modern.ircdocs.horse/)

[Internet Relay Chat Protocol 1459](https://datatracker.ietf.org/doc/html/rfc1459)

[Internet Relay Chat: Client Protocol 2812](https://datatracker.ietf.org/doc/html/rfc2812)

[RFC 1459 | Message format in 'pseudo' BNF](https://www.rfcreader.com/#rfc1459)

[IRC Modes](https://modern.ircdocs.horse/#modes)

## Other Sources :

[The Pitchfork Layout](https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/spec/data/spec.bs#tld.src) a convention for laying out source, build, and resource files in a filesystem to aide in uniformity, tooling, understandability, and compartmentalization.
