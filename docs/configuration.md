# Configuration

UplinkIRC is configured with a single TOML file. On first launch it is created automatically with default settings — you just need to fill in your nickname.

---

## Config file location

| Platform | Path |
|---|---|
| Linux / FreeBSD | `~/.config/LinuxDojo/UplinkIRC/config.toml` |
| macOS | `~/Library/Application Support/LinuxDojo/UplinkIRC/config.toml` |
| Windows | `%APPDATA%\LinuxDojo\UplinkIRC\config.toml` |

Open the file in any text editor, or use **Hamburger → Manage Servers...** to add, edit, and remove server connections from within the app. Changes made through the dialog take effect immediately and are saved automatically.

---

## Full example

This is a complete config file showing every available option.

```toml
[ui]
theme             = "catppuccin-mocha"
show_nick_prefix  = true
show_topic        = true
show_emoji_button = false
colored_nicks     = true
typing_indicator  = true
show_conn_status  = true
app_icon          = "dark"
font_family       = "IBM Plex Mono"
font_toolbar      = 10
font_sidebar      = 10
font_chat         = 10
font_nick_list    = 10
font_nick_dock    = 10
font_topic_bar    = 10
font_input_nick   = 10
font_input        = 10
font_typing       = 9

[[server]]
name     = "LinuxDojo"
host     = "irc.linuxdojo.org"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "uplink"
realname = "UplinkIRC User"
# sasl_user         = "yournick"       # uncomment to enable SASL PLAIN
# sasl_password     = "yourpassword"
# nickserv_password = "yourpassword"   # alternative: NickServ IDENTIFY on connect
# bouncer           = "soju"           # "znc" or "soju" — enables bouncer-specific caps
# bouncer_network   = "libera"         # soju only: which network to attach to

[[server.channels]]
name = "#uplink"

[[server.channels]]
name = "#linux"
```

---

## The `[ui]` block

Controls the look and feel of the interface. All keys are optional — missing keys use the default.

| Key | Type | Default | Description |
|---|---|---|---|
| `theme` | string | `"default"` | Theme name — must match a `.toml` file in `themes/` without the extension |
| `show_nick_prefix` | bool | `true` | Show your nickname label next to the message input box |
| `show_topic` | bool | `true` | Show the channel topic bar below the info bar |
| `show_emoji_button` | bool | `false` | Show the emoji button next to the input box |
| `colored_nicks` | bool | `true` | Give each nickname a unique color in chat and the nick list |
| `typing_indicator` | bool | `true` | Show "nick is typing…" notifications (IRCv3 `draft/typing`) and send your own |
| `show_conn_status` | bool | `true` | Show the connection status label in the bottom-left status bar |
| `app_icon` | string | `"dark"` | Which app icon variant to use. Choices: `"dark"`, `"light"`, `"light-default"`, `"avatar"` |
| `font_family` | string | `"IBM Plex Mono"` | Font family applied to all UI zones |
| `font_toolbar` | integer | `10` | Font size (pt) for the hamburger button |
| `font_sidebar` | integer | `10` | Font size (pt) for the server/channel tree |
| `font_chat` | integer | `10` | Font size (pt) for the message area |
| `font_nick_list` | integer | `10` | Font size (pt) for the user list |
| `font_nick_dock` | integer | `10` | Font size (pt) for the "Users (N)" dock title |
| `font_topic_bar` | integer | `10` | Font size (pt) for the channel info bar |
| `font_input_nick` | integer | `10` | Font size (pt) for your nick label next to the input |
| `font_input` | integer | `10` | Font size (pt) for the message input box |
| `font_typing` | integer | `9` | Font size (pt) for the "nick is typing…" indicator |

All font sizes and the theme can be changed live from **Hamburger → Font Config...** and **Hamburger → Theme** without editing the file.

### Example

```toml
[ui]
theme             = "nord"
colored_nicks     = true
typing_indicator  = true
show_conn_status  = true
app_icon          = "dark"
font_family       = "IBM Plex Mono"
font_chat         = 12
font_sidebar      = 10
font_input        = 11
```

---

## The `[[server]]` block

Each server gets its own `[[server]]` block. The double brackets (`[[...]]`) define an array entry, so you can have as many as you like.

| Key | Type | Required | Description |
|---|---|---|---|
| `name` | string | yes | Display name shown in the sidebar |
| `host` | string | yes | IRC server hostname or IP address |
| `port` | integer | yes | Server port. The standard TLS port is `6697` |
| `ssl` | bool | yes | Use TLS encryption. Always set `true` — plaintext IRC is not supported |
| `nick` | string | yes | Your preferred nickname |
| `user` | string | no | Username in your hostmask (defaults to `"uplink"`) |
| `realname` | string | no | Shown in WHOIS (defaults to `"UplinkIRC User"`) |
| `password` | string | no | Sent as `PASS` during connection. Required for most bouncers; also used for password-protected servers |
| `sasl_user` | string | no | SASL username for SASL PLAIN authentication. Set together with `sasl_password` |
| `sasl_password` | string | no | SASL password for SASL PLAIN authentication. Set together with `sasl_user` |
| `nickserv_password` | string | no | Sends `PRIVMSG NickServ :IDENTIFY <password>` after connecting. Use on servers without SASL support |
| `bouncer` | string | no | Bouncer type. Set to `"znc"` or `"soju"` to enable bouncer-specific IRCv3 capabilities. Omit or set `"none"` for a direct server connection |
| `bouncer_network` | string | no | **soju only.** The network name to attach to (e.g. `"libera"`). Sent to soju's `BOUNCER LISTNETWORKS` subsystem. Leave empty if connecting to a single-network soju instance |

### Minimal server block

```toml
[[server]]
name     = "LinuxDojo"
host     = "irc.linuxdojo.org"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "uplink"
realname = "UplinkIRC User"
```

---

## Authentication

### NickServ auto-identify

If the server uses NickServ and does not support SASL, add `nickserv_password`. UplinkIRC sends `PRIVMSG NickServ :IDENTIFY <password>` immediately after receiving the welcome numeric.

```toml
[[server]]
name              = "LinuxDojo"
host              = "irc.linuxdojo.org"
port              = 6697
ssl               = true
nick              = "yournick"
user              = "uplink"
realname          = "UplinkIRC User"
nickserv_password = "yourpassword"
```

The server buffer will show `Sent NickServ IDENTIFY` when this fires. For servers that support SASL, prefer `sasl_user`/`sasl_password` — SASL identifies you before you appear on the network.

### SASL PLAIN

If the server supports SASL (Libera.Chat, OFTC, and others), use `sasl_user` and `sasl_password`. UplinkIRC negotiates the `sasl` CAP and authenticates during the handshake, before registration completes.

```toml
[[server]]
name          = "Libera"
host          = "irc.libera.chat"
port          = 6697
ssl           = true
nick          = "yournick"
user          = "uplink"
realname      = "UplinkIRC User"
sasl_user     = "yournick"
sasl_password = "yourpassword"

[[server.channels]]
name = "#linux"
```

The server buffer shows `SASL authentication successful` on connect. Authentication failure does not disconnect — the connection continues without services authentication.

---

## Bouncer support (ZNC and soju)

UplinkIRC has first-class bouncer support. Setting `bouncer = "znc"` or `bouncer = "soju"` in the server block activates bouncer-specific IRCv3 capabilities, enabling features like chat history replay, read markers, and network enumeration.

The `password` field is used for bouncer authentication and is sent as `PASS` before `NICK`/`USER`, which is what bouncers require.

### Connecting to ZNC

ZNC expects the password in the format `username/network:password`. Set `bouncer = "znc"` to activate ZNC-specific caps.

When `znc.in/playback` is available, UplinkIRC sends `PRIVMSG *playback :PLAY * 0` after the welcome message to replay all missed messages. Self-messages sent from other clients are echoed correctly via `znc.in/self-message`.

```toml
[[server]]
name     = "ZNC — Libera"
host     = "znc.example.com"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "uplink"
realname = "UplinkIRC User"
password = "joe/libera:mysecretpassword"
bouncer  = "znc"

[[server.channels]]
name = "#linux"

[[server.channels]]
name = "#archlinux"
```

If your ZNC instance carries multiple networks, add one `[[server]]` block per network, each with its own `password` entry using the correct network name:

```toml
[[server]]
name     = "ZNC — Libera"
host     = "znc.example.com"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "uplink"
realname = "UplinkIRC User"
password = "joe/libera:mysecretpassword"
bouncer  = "znc"

[[server.channels]]
name = "#linux"

[[server]]
name     = "ZNC — OFTC"
host     = "znc.example.com"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "uplink"
realname = "UplinkIRC User"
password = "joe/oftc:mysecretpassword"
bouncer  = "znc"

[[server.channels]]
name = "#debian"
```

### Connecting to soju

soju expects the password in the format `username:password`. Set `bouncer = "soju"` to activate soju-specific caps.

When `soju.im/bouncer-networks` is available, UplinkIRC sends `BOUNCER LISTNETWORKS` after CAP negotiation and lists all attached networks in the server buffer. Use `bouncer_network` to specify which network to attach to when your soju instance carries more than one.

`soju.im/read` is negotiated automatically, keeping your read position in sync across all clients connected to the same soju instance.

```toml
[[server]]
name            = "soju — Libera"
host            = "soju.example.com"
port            = 6697
ssl             = true
nick            = "yournick"
user            = "uplink"
realname        = "UplinkIRC User"
password        = "joe:mysecretpassword"
bouncer         = "soju"
bouncer_network = "libera"

[[server.channels]]
name = "#linux"
```

If your soju instance only carries one network, omit `bouncer_network`:

```toml
[[server]]
name     = "soju"
host     = "soju.example.com"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "uplink"
realname = "UplinkIRC User"
password = "joe:mysecretpassword"
bouncer  = "soju"

[[server.channels]]
name = "#uplink"
```

### Chat history replay

When `chathistory` is negotiated (supported by soju, modern ZNC, and some IRC servers), UplinkIRC automatically requests the last 100 messages for each channel after joining. History messages are:

- Displayed at reduced opacity so they are visually distinct from live messages
- Shown with their original timestamp — the date is prepended (`MM/dd hh:mm`) when the message is from a previous day
- Not counted as unread, so they do not badge the channel in the sidebar

No configuration is required — history replay happens automatically whenever the server supports it.

---

## The `[[server.channels]]` block

Channels to auto-join on connect are listed inside the server block as `[[server.channels]]` entries.

| Key | Type | Required | Description |
|---|---|---|---|
| `name` | string | yes | Channel name including the `#` |
| `password` | string | no | Channel key, for password-protected channels |

### Example — multiple channels with a key

```toml
[[server]]
name     = "LinuxDojo"
host     = "irc.linuxdojo.org"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "uplink"
realname = "UplinkIRC User"

[[server.channels]]
name = "#uplink"

[[server.channels]]
name = "#linux"

[[server.channels]]
name = "#secret"
password = "channelkey"
```

---

## Multiple servers

Add as many `[[server]]` blocks as you need. Each server appears in the sidebar independently and connects on launch.

```toml
[ui]
theme = "catppuccin-mocha"

[[server]]
name     = "LinuxDojo"
host     = "irc.linuxdojo.org"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "uplink"
realname = "UplinkIRC User"

[[server.channels]]
name = "#uplink"

[[server]]
name     = "Libera"
host     = "irc.libera.chat"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "uplink"
realname = "UplinkIRC User"
sasl_user     = "yournick"
sasl_password = "yourpassword"

[[server.channels]]
name = "#linux"

[[server.channels]]
name = "#archlinux"
```

---

## Themes

Set `theme` in `[ui]` to any theme name from the list below. The name must match the `.toml` filename in the `themes/` folder without the extension.

UplinkIRC ships with 55 built-in themes:

| Theme name | Description |
|---|---|
| `catppuccin-mocha` | Dark, muted purple tones |
| `catppuccin-latte` | Light, warm pastel variant |
| `nord` | Cool blue Arctic palette |
| `dracula` | Dark purple and pink |
| `gruvbox-dark` | Warm retro dark theme |
| `solarized-dark` | Classic Solarized dark |
| `solarized-light` | Classic Solarized light |
| `tokyo-night` | Dark Tokyo Night |
| `one-dark` | Atom One Dark |
| `default` | Built-in fallback theme |

Themes can be switched live from **Hamburger → Theme** without restarting.

### Theme search path

1. `~/.config/LinuxDojo/UplinkIRC/themes/<name>.toml` — personal themes
2. `<exe directory>/themes/<name>.toml` — shipped themes next to the binary
3. `themes/<name>.toml` — relative to the current working directory

To add a custom theme, drop a `.toml` file into `~/.config/LinuxDojo/UplinkIRC/themes/`. It appears in the hamburger menu on the next launch.

---

## Common mistakes

### Forgetting quotes around strings

All string values in TOML must be in double quotes. The `#` character starts a comment if it appears outside a string, which trips up channel names written bare.

```toml
# Wrong — parse error
name = LinuxDojo

# Correct
name = "LinuxDojo"
```

### Using single brackets for servers

`[server]` defines a single table. `[[server]]` defines an array entry and is what UplinkIRC expects. Always use double brackets.

```toml
# Wrong
[server]
name = "LinuxDojo"

# Correct
[[server]]
name = "LinuxDojo"
```

### Channel block placed after the wrong server

`[[server.channels]]` blocks attach to the most recently opened `[[server]]` block. Place each channel block immediately after its server.

```toml
# Wrong — #linux attaches to "Libera", not "LinuxDojo"
[[server]]
name = "LinuxDojo"
...

[[server]]
name = "Libera"
...

[[server.channels]]
name = "#linux"

# Correct
[[server]]
name = "LinuxDojo"
...

[[server.channels]]
name = "#linux"

[[server]]
name = "Libera"
...
```

### Nick contains spaces or invalid characters

IRC nicks cannot contain spaces. Allowed characters: letters, digits, `_`, `-`, `[`, `]`, `{`, `}`, `|`, `\`, `^`, and `` ` ``. Most servers enforce a max length of 30 characters.
