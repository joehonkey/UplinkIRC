# Slash Commands

Type any of these commands in the message input box and press Enter.

---

## Channel commands

| Command | Description |
|---|---|
| `/join #channel` | Join a channel |
| `/j #channel` | Short alias for `/join` |
| `/part [#channel]` | Leave a channel. Defaults to the current channel if omitted |
| `/part #channel reason` | Leave with a part message |
| `/topic` | Show the current channel topic |
| `/topic <text>` | Set the channel topic |
| `/kick <nick> [reason]` | Kick a user from the current channel (requires op) |

### Examples

```
/join #linux
/j #uplink
/part
/part #linux see you later
/topic new topic here
/kick baduser spamming
```

---

## Messaging

| Command | Description |
|---|---|
| `/me <text>` | Send an action message. Shown as `* yournick text` in chat |
| `/msg <nick> <text>` | Send a private message to a user |
| `/notice <target> <text>` | Send a NOTICE to a user or channel |

### Examples

```
/me waves hello
/msg alice hey, are you around?
/notice alice heads up
```

---

## Nick and identity

| Command | Description |
|---|---|
| `/nick <name>` | Change your nickname |
| `/away [message]` | Set yourself as away with an optional message |
| `/back` | Clear your away status |
| `/whois <nick>` | Look up info about a user |

### Examples

```
/nick coolnick
/away grabbing coffee
/back
/whois alice
```

---

## Connection

| Command | Description |
|---|---|
| `/quit [message]` | Disconnect from the current server with an optional quit message |
| `/motd [server]` | Request the message of the day |

### Examples

```
/quit
/quit later everyone
/motd
```

---

## CTCP

| Command | Description |
|---|---|
| `/version [nick]` | Request server version, or CTCP VERSION from a nick |
| `/ctcp <nick> <command> [args]` | Send a CTCP request to a user |
| `/sysinfo` | Post your OS, kernel, CPU, and RAM info to the current channel |

Incoming CTCP VERSION and PING requests are answered automatically.

### Examples

```
/version
/version alice
/ctcp alice PING
/ctcp alice TIME
/sysinfo
```

---

## Raw IRC protocol

| Command | Description |
|---|---|
| `/raw <line>` | Send a raw IRC protocol line directly to the server |
| `/quote <line>` | Alias for `/raw` |

Use these when you need to send a command that UplinkIRC does not have a built-in shortcut for yet.

### Examples

```
/raw MODE #uplink +m
/raw INVITE alice #uplink
/quote JOIN #test
```

---

## Help

| Command | Description |
|---|---|
| `/help` | Print a list of all available commands in the current chat buffer |

---

## Notes

- Commands are case-insensitive: `/JOIN` works the same as `/join`.
- Any input that does not start with `/` is sent as a regular message to the current channel or user.
- If you want to send a message that starts with `/` literally, prefix it with a space.
