# Configuration

UplinkIRC is configured with a single TOML file. On first launch it is created automatically with the default settings — you just need to set your nickname.

---

## Config file location

| Platform | Path |
|---|---|
| Linux | `~/.config/LinuxDojo/UplinkIRC/config.toml` |
| macOS | `~/Library/Application Support/LinuxDojo/UplinkIRC/config.toml` |
| Windows | `%APPDATA%\LinuxDojo\UplinkIRC\config.toml` |
| FreeBSD | `~/.config/LinuxDojo/UplinkIRC/config.toml` |

Open the file in any text editor. Changes take effect on the next launch (live reload is planned).

---

## Full example

This is a complete config file showing every available option. Copy it, change the nick, and you are ready to go.

```toml
[ui]
theme             = "catppuccin-mocha"
show_nick_prefix  = true
show_topic        = true
show_emoji_button = false
colored_nicks     = true
typing_indicator  = true
font_family       = "IBM Plex Mono"
font_toolbar      = 10
font_sidebar      = 10
font_chat         = 10
font_nick_list    = 10
font_nick_dock    = 10
font_topic_bar    = 10
font_input_nick   = 10
font_input        = 10

[[server]]
name     = "LinuxDojo"
host     = "irc.linuxdojo.org"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "uplink"
realname = "UplinkIRC User"
# sasl_user         = "yournick"      # uncomment to enable SASL PLAIN
# sasl_password     = "yourpassword"
# nickserv_password = "yourpassword"  # alternative: NickServ IDENTIFY

[[server.channels]]
name = "#uplink"

[[server.channels]]
name = "#linux"
```

---

## The `[ui]` block

Controls the look and feel of the interface. All keys are optional — if omitted the default value is used.

| Key | Type | Default | Description |
|---|---|---|---|
| `theme` | string | `"default"` | Theme to apply. Must match the filename in `themes/` without the `.toml` extension. See [Themes](#themes) below |
| `show_nick_prefix` | bool | `true` | Show your nickname next to the message input box |
| `show_topic` | bool | `true` | Show the topic bar at the top of the chat view |
| `show_emoji_button` | bool | `false` | Show the emoji button to the right of the message input (emoji picker is coming soon) |
| `colored_nicks` | bool | `true` | Give each nickname a unique color in chat and the nick list |
| `typing_indicator` | bool | `true` | Show "nick is typing..." notifications (IRCv3 `draft/typing`) |
| `font_family` | string | `"IBM Plex Mono"` | Font family for all UI zones |
| `font_toolbar` | integer | `10` | Font size (pt) for the hamburger button and app label |
| `font_sidebar` | integer | `10` | Font size (pt) for the server/channel tree |
| `font_chat` | integer | `10` | Font size (pt) for the message area |
| `font_nick_list` | integer | `10` | Font size (pt) for the user list |
| `font_nick_dock` | integer | `10` | Font size (pt) for the "Users (N)" dock title |
| `font_topic_bar` | integer | `10` | Font size (pt) for the channel info bar |
| `font_input_nick` | integer | `10` | Font size (pt) for your nick label next to the input |
| `font_input` | integer | `10` | Font size (pt) for the message input box |

All font sizes can be changed live from **Hamburger → Font Config...** without editing the file.

### Example

```toml
[ui]
theme             = "nord"
show_nick_prefix  = true
show_topic        = true
show_emoji_button = false
colored_nicks     = true
typing_indicator  = true
font_family       = "IBM Plex Mono"
font_chat         = 12
font_sidebar      = 10
font_input        = 11
```

---

## The `[[server]]` block

Each server you want to connect to gets its own `[[server]]` block. The double brackets (`[[...]]`) mean you can have as many as you want — just add another block.

| Key | Type | Required | Description |
|---|---|---|---|
| `name` | string | yes | Display name shown in the sidebar. Can be anything |
| `host` | string | yes | IRC server hostname or IP address |
| `port` | integer | yes | Server port. Standard TLS port is 6697 |
| `ssl` | bool | yes | Use TLS encryption. Always use `true` — plain IRC is not secure |
| `nick` | string | yes | Your preferred nickname. Cannot contain spaces |
| `user` | string | no | Username shown in your hostmask (defaults to `"uplink"`) |
| `realname` | string | no | Your "real name" shown in WHOIS (defaults to `"UplinkIRC User"`) |
| `password` | string | no | Server password, sent as `PASS` during connection. Used for bouncers (ZNC, soju) and password-protected servers |
| `sasl_user` | string | no | SASL username for SASL PLAIN authentication. Must be set together with `sasl_password` |
| `sasl_password` | string | no | SASL password for SASL PLAIN authentication. Must be set together with `sasl_user` |
| `nickserv_password` | string | no | If set, sends `PRIVMSG NickServ :IDENTIFY <password>` immediately after connecting. Use when the server does not support SASL |

### Minimal server block

This is the smallest valid server config. Just change `yournick` to your nickname.

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

### NickServ auto-identify

If the server uses NickServ for nick registration (common on older networks or those without SASL), add `nickserv_password` to the server block. UplinkIRC will send `PRIVMSG NickServ :IDENTIFY <password>` immediately after receiving `001 RPL_WELCOME`.

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

[[server.channels]]
name = "#uplink"
```

The server buffer will show `Sent NickServ IDENTIFY` when this fires. If the server supports SASL, prefer `sasl_user`/`sasl_password` instead — SASL authenticates before you appear on the network.

---

### SASL PLAIN authentication

If the server requires SASL authentication (common on Libera.Chat, OFTC, and others), add `sasl_user` and `sasl_password` to the server block. UplinkIRC will negotiate the `sasl` CAP and authenticate before completing registration.

```toml
[[server]]
name         = "Libera"
host         = "irc.libera.chat"
port         = 6697
ssl          = true
nick         = "yournick"
user         = "uplink"
realname     = "UplinkIRC User"
sasl_user     = "yournick"
sasl_password = "yourpassword"

[[server.channels]]
name = "#linux"
```

The server buffer will show `SASL authentication successful` on connect, or an error message if it fails. Authentication failure does not disconnect — the connection continues (though services may require auth for certain channels).

---

## The `[[server.channels]]` block

Channels to auto-join on connect go inside the server block using `[[server.channels]]`. Each channel gets its own block.

| Key | Type | Required | Description |
|---|---|---|---|
| `name` | string | yes | Channel name, including the `#` |
| `password` | string | no | Channel key, if the channel requires one |

### Example — multiple channels

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

Add as many `[[server]]` blocks as you need. Each server and its channels appear in the sidebar independently.

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

[[server.channels]]
name = "#linux"

[[server.channels]]
name = "#archlinux"
```

---

## Bouncer support (ZNC / soju)

Set `password` in the server block to send a `PASS` command at connection. This is the standard way to authenticate with ZNC and soju.

**ZNC** — password format is `username/network:password`:

```toml
[[server]]
name     = "ZNC"
host     = "znc.example.com"
port     = 6697
ssl      = true
nick     = "yournick"
user     = "uplink"
realname = "UplinkIRC User"
password = "joe/libera:mysecretpassword"

[[server.channels]]
name = "#linux"
```

**soju** — password format is `username:password`:

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

[[server.channels]]
name = "#linux"
```

`PASS` is sent before `NICK`/`USER` during the handshake, which is what bouncers require.

---

## Themes

Set `theme` in the `[ui]` block to any theme name from the list below. The name must match the `.toml` filename inside the `themes/` folder (without the extension).

UplinkIRC ships with 55 built-in themes. A few popular ones:

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

To use a theme:

```toml
[ui]
theme = "nord"
```

### Theme search path

UplinkIRC looks for theme files in this order:

1. `~/.config/LinuxDojo/UplinkIRC/themes/<name>.toml` — your personal themes folder
2. `<exe directory>/themes/<name>.toml` — shipped themes next to the binary
3. `themes/<name>.toml` — relative to the current working directory

To add a custom theme, create a `.toml` file in `~/.config/LinuxDojo/UplinkIRC/themes/`. It appears in the hamburger menu immediately on next launch.

---

## Common mistakes

### Forgetting quotes around strings

All string values in TOML must be in double quotes. The `#` character starts a TOML comment if it appears outside a string.

```toml
# Wrong — these will cause a parse error
name = LinuxDojo
channels = [#uplink]

# Correct
name = "LinuxDojo"
```

### Using single brackets for servers

`[server]` (single brackets) defines a single table. `[[server]]` (double brackets) defines an array entry and is what UplinkIRC expects.

```toml
# Wrong — this only works for one server and is parsed differently
[server]
name = "LinuxDojo"

# Correct — works for one server or many
[[server]]
name = "LinuxDojo"
```

### Channel block placed outside the server block

The `[[server.channels]]` blocks must come **after** the `[[server]]` block they belong to, before the next `[[server]]` block.

```toml
# Wrong — channel is after the second server block
[[server]]
name = "LinuxDojo"
...

[[server]]
name = "Libera"
...

[[server.channels]]  # This attaches to "Libera", not "LinuxDojo"!
name = "#linux"

# Correct — channel immediately follows its server
[[server]]
name = "LinuxDojo"
...

[[server.channels]]
name = "#uplink"

[[server]]
name = "Libera"
...
```

### Nick contains spaces or is too long

IRC nicks cannot contain spaces. Most servers enforce a max length (usually 30 characters). Use only letters, numbers, `_`, `-`, `[`, `]`, `{`, `}`, `|`, `\`, `^`, and `` ` ``.
