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
| `/topic <text>` | Set the current channel topic |
| `/topic #channel <text>` | Set the topic on a specific channel |
| `/kick <nick> [reason]` | Kick a user from the current channel (requires op) |
| `/invite <nick>` | Invite a user to the current channel |
| `/invite <nick> #channel` | Invite a user to a specific channel |
| `/mode <target> <flags> [args]` | Set channel or user modes |
| `/op <nick>` | Give op (`+o`) in the current channel |
| `/deop <nick>` | Remove op (`-o`) in the current channel |
| `/voice <nick>` | Give voice (`+v`) in the current channel |
| `/devoice <nick>` | Remove voice (`-v`) in the current channel |
| `/ban <mask>` | Ban a mask (`+b`) in the current channel |
| `/unban <mask>` | Remove a ban (`-b`) in the current channel |
| `/clear` | Clear the chat buffer |

### Examples

```
/join #linux
/j #uplink
/part
/part #linux see you later
/topic new topic here
/topic #uplink Welcome to the server
/kick baduser spamming
/invite alice
/invite alice #linux
/mode #uplink +m
/op alice
/deop bob
/voice alice
/ban *!*@spammer.host
/unban *!*@spammer.host
/clear
```

---

## Messaging

| Command | Description |
|---|---|
| `/me <text>` | Send an action message. Shown as `* yournick text` in chat |
| `/msg <nick> <text>` | Send a private message — opens a PM buffer in the sidebar for the conversation |
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
| `/ping <nick>` | Send a CTCP PING to a user — reply shows round-trip time in the active channel |
| `/time <nick>` | Ask a user for their local time — reply appears in the active channel |
| `/version [nick]` | Request server version, or CTCP VERSION from a nick |
| `/ctcp <nick> <command> [args]` | Send a raw CTCP request to a user |
| `/sysinfo` | Post OS, CPU, RAM, GPU, and uptime info to the current channel |

Incoming CTCP VERSION and PING requests are answered automatically.

> **Note:** `/time` and `/ping` only work if the target client supports those CTCP commands. Most IRC clients do, but bots often do not respond. If you see no reply, the other side simply does not support it.

### Examples

```
/ping alice
/time alice
/version
/version alice
/ctcp alice CLIENTINFO
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

---

## Emoji

UplinkIRC has two ways to insert emoji:

### Emoji picker button
Click ☰ to open **Preferences** and check **Show Emoji Button** (or set `show_emoji_button = true` in config). Click the 😊 button next to the input box to open a searchable grid of ~400 emoji.

### :shortcode: typing
Type a colon followed by a keyword and a completion list appears:

| You type | Result |
|---|---|
| `:smi` | autocomplete list: 😄 smile, 😃 smiley, … |
| `:trident:` | auto-replaces to 🔱 as soon as you type the closing `:` |
| `:fire:` + Enter | sends 🔥 in the message |

- Use **Up/Down** arrows to navigate the list, **Enter** or **Tab** to confirm, **Escape** to dismiss.
- Any `:shortcode:` patterns still in the text when you press Enter are resolved before the message is sent.

---

## Notes

- Commands are case-insensitive: `/JOIN` works the same as `/join`.
- Any input that does not start with `/` is sent as a regular message to the current channel or user.
- If you want to send a message that starts with `/` literally, prefix it with a space.
