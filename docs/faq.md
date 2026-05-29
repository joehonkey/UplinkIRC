# FAQ & Troubleshooting

Common questions and fixes for UplinkIRC.

---

## Installation & startup

### Where do I get UplinkIRC?

Build from source — see [Building](building.md) for step-by-step instructions. Packages for Linux, FreeBSD, macOS, and Windows are planned.

### A nick dialog appeared on first launch — what do I do?

UplinkIRC detected that your config still has the placeholder nick `yournick`. Type your desired nickname in the dialog and click OK. It will be saved to your config and used for all servers on the next connect.

### themes/ folder missing — no themes load

The `themes/` folder must be in the same directory as the `UplinkIRC` binary (or at `~/.config/LinuxDojo/UplinkIRC/themes/`). If you moved the binary, copy the folder back:

```bash
cp -r themes/ /path/to/your/binary/
```

If you built with CMake, themes are copied to the build directory automatically via a post-build step.

---

## Configuration

### Where is my config file?

| Platform | Path |
|---|---|
| Linux / FreeBSD | `~/.config/LinuxDojo/UplinkIRC/config.toml` |
| macOS | `~/Library/Application Support/LinuxDojo/UplinkIRC/config.toml` |
| Windows | `%APPDATA%\LinuxDojo\UplinkIRC\config.toml` |

Open it in any text editor and restart UplinkIRC to apply changes.

### My config change isn't taking effect

UplinkIRC reads the config once at startup. After editing `config.toml`, close and reopen the app. Live reload is planned for a future release.

### TOML parse error on startup

All string values in TOML must be wrapped in double quotes. The `#` character starts a comment outside of strings, which trips up channel names.

```toml
# Wrong — causes a parse error
name = LinuxDojo
channels = [#uplink]

# Correct
name     = "LinuxDojo"
```

For channels, each one needs its own `[[server.channels]]` block:

```toml
[[server.channels]]
name = "#uplink"

[[server.channels]]
name = "#linux"
```

See [Configuration](configuration.md) for the full format reference.

### Where do I set the channel to auto-join?

Add a `[[server.channels]]` block inside the server entry:

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
```

---

## Connecting & IRC

### Can I connect to multiple servers at once?

Yes. Add multiple `[[server]]` blocks in your config — each gets its own entry in the sidebar and connects independently.

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
```

### UplinkIRC disconnected — will it reconnect?

Yes. UplinkIRC reconnects automatically after an unexpected disconnect using exponential backoff: it waits 5 seconds, then 10, 20, 40, and caps at 60 seconds per attempt. A countdown message appears in the server buffer each time. Once reconnected, it re-joins all configured channels automatically.

If you disconnect deliberately with `/quit` or the **Disconnect** option in the sidebar right-click menu, no reconnect is attempted.

You can also right-click a server in the sidebar and choose **Reconnect** to connect immediately without waiting.

### How do I send a raw IRC command?

Use `/raw` followed by the full IRC line:

```
/raw PRIVMSG #uplink :hello
/raw MODE #uplink +m
/raw WHOIS alice
```

`/quote` works as an alias for `/raw`.

### My nick was already in use — what happened?

If your preferred nick is taken, UplinkIRC appends `_` and tries again (e.g. `yournick_`). You will see the new nick reflected next to the input box. Use `/nick yournick` once the original nick becomes available.

### How do I join a channel while connected?

Type `/join #channelname` in the input box and press Enter.

### How do I send a private message?

Use `/msg <nick> <text>`. A PM buffer for that user opens in the sidebar where you can continue the conversation. Incoming PMs from others open their own buffers automatically.

You can also right-click a nick in the user list and choose **Message** to open the PM buffer without typing a command.

### How do I use a bouncer (ZNC or soju)?

Set the `password` field for bouncer authentication, and set `bouncer = "znc"` or `bouncer = "soju"` to enable bouncer-specific IRCv3 capabilities such as chat history replay, self-message echo, and network enumeration.

**ZNC** — password format is `username/network:password`:

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
```

With `bouncer = "znc"`, UplinkIRC negotiates `znc.in/playback` and replays missed messages on connect, and echoes messages you sent from other clients via `znc.in/self-message`.

**soju** — password format is `username:password`. Use `bouncer_network` if your soju instance manages more than one IRC network:

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

With `bouncer = "soju"`, UplinkIRC negotiates `soju.im/bouncer-networks` (lists your networks in the server buffer), `soju.im/read` (syncs your read position across clients), and `soju.im/no-implicit-names`.

Both bouncer types also negotiate the standard `chathistory` capability, which automatically requests the last 100 messages for each channel on join. See [IRCv3 support](ircv3.md) for full details.

### What do the dimmed messages at the top of a channel mean?

Those are history messages replayed by your bouncer or IRC server via the `chathistory` capability. They represent messages that were sent while you were offline or disconnected.

History messages are intentionally displayed at reduced opacity and with their original timestamps so they are easy to distinguish from live messages. Previous-day messages show the date as well as the time (`MM/dd hh:mm`). They do not count as unread, so they will not badge the channel in the sidebar.

---

## Interface

### Where is the hamburger menu?

The `☰` button is in the **bottom-right corner of the user list panel** (right side of the window, below the nick list). Click it to access themes, font config, toggles, and more.

### How do I change the theme?

Open **Hamburger menu → Theme** and pick from the list. The theme applies immediately. To make it the default, set `theme` in `config.toml`:

```toml
[ui]
theme = "nord"
```

### How do I move the nick list?

The nick list panel is a floating dock — drag its title bar to the left or right to reposition it. You can also drag the edge of the panel to resize it. Both position and size are saved automatically when you quit and restored on the next launch.

### How do I show the channel topic?

The info bar at the top of the chat area always shows `#channel (modes) * NetworkName — N users`. To see the actual channel topic text, open **Hamburger menu** and toggle **Show Topic Bar** — a topic line will drop down below the info bar. You can also set it in config:

```toml
[ui]
show_topic = true
```

### How do I hide my nick next to the input box?

Open **Hamburger menu** and toggle the **Nick** option. Or set it in config:

```toml
[ui]
show_nick_prefix = false
```

### How do I authenticate with NickServ automatically?

Add `nickserv_password` to your server block in `config.toml`. UplinkIRC will send `PRIVMSG NickServ :IDENTIFY <password>` immediately after connecting:

```toml
[[server]]
name              = "LinuxDojo"
host              = "irc.linuxdojo.org"
port              = 6697
ssl               = true
nick              = "yournick"
nickserv_password = "yourpassword"
```

For servers that support it, SASL PLAIN is a better choice — it identifies you before you appear on the network. See [SASL setup](configuration.md#sasl-plain-authentication) in the config docs.

### How do I use SASL to log in?

Add `sasl_user` and `sasl_password` to your server block:

```toml
[[server]]
name          = "Libera"
host          = "irc.libera.chat"
port          = 6697
ssl           = true
nick          = "yournick"
sasl_user     = "yournick"
sasl_password = "yourpassword"
```

UplinkIRC will negotiate the `sasl` CAP and authenticate during the connection handshake. The server buffer shows `SASL authentication successful` when it works. Use this instead of `nickserv_password` on networks that support it (Libera.Chat, OFTC, etc.).

### How do link previews work?

When a live message arrives containing an `http://` or `https://` URL, UplinkIRC fetches the link in the background and appends a small card below the message.

**Web pages** — UplinkIRC fetches up to 32 KB of HTML, extracts `og:title` and `og:image` metadata (or falls back to the `<title>` tag), and builds a card showing the page title, domain, and thumbnail (if an image was found).

**Direct image links** — URLs ending in `.png`, `.jpg`, `.jpeg`, `.gif`, or `.webp` are detected automatically and shown as a thumbnail card directly, without any HTML parsing. The filename is used as the card label.

Hovering over any URL in chat shows the domain immediately in the status bar and tooltip, updating to the full page title once the fetch completes.

Preview fetches are lightweight and automatic — HTML is capped at 32 KB, images at 200 KB, and results are cached in memory for the session.

**Note:** Preview cards appear only for messages received while that channel is active. Switching away and back clears the cards — this is a known limitation tracked for a future fix.

### The emoji button doesn't show

The emoji button is hidden by default. Enable it in **Hamburger → Show Emoji Button**, or set it in config:

```toml
[ui]
show_emoji_button = true
```

Once visible, clicking `😊` opens a searchable grid of ~400 emoji. You can also type `:shortcode:` directly in the input box — a completion list appears as you type, and pressing Enter, Tab, or clicking an entry inserts the emoji. Typing the full `:trident:` with the closing colon substitutes it instantly without the completion list.

### How do I minimize to the system tray?

Close the window normally — it minimizes to the system tray instead of quitting. Left-click the tray icon to bring the window back. Right-click for a menu with **Show** and **Quit**.

---

## Building from source

### What do I need to build UplinkIRC?

- CMake 3.16 or newer
- Qt 6.2 or newer (Widgets + Network + Ssl modules)
- A C++17 compiler (GCC, Clang, or MSVC)
- tomlplusplus (header-only, available via most package managers)

See [Building](building.md) for full platform-specific instructions.

### Build fails: "tomlplusplus not found"

Install the package for your platform:

```bash
# Arch Linux
sudo pacman -S tomlplusplus

# Ubuntu / Debian
sudo apt install libtomlplusplus-dev

# Fedora
sudo dnf install tomlplusplus-devel

# FreeBSD
sudo pkg install tomlplusplus

# macOS (Homebrew)
brew install tomlplusplus
```

### Build fails: Qt6 not found

Make sure Qt6 is installed and CMake can find it. If Qt is in a non-standard location, pass the prefix:

```bash
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt6 ..
```

---

## Getting help

- Join **#uplink** on `irc.linuxdojo.org` — the UplinkIRC development channel
- File bugs and feature requests on the GitHub Issues page
- Browse the full documentation index at [docs/index.md](index.md)

---

## Link previews

### Link previews don't appear for some URLs

UplinkIRC fetches the page title and thumbnail for URLs posted in chat. If a preview isn't appearing:

- **Direct image link** — `.png`, `.jpg`, `.jpeg`, `.gif`, `.webp` URLs are handled automatically as of v0.7.1 and show a thumbnail card.
- **The site redirects** (e.g. `http://` → `https://`, or bare domain → `www.`) — redirects are followed automatically since v0.7.0.
- **The site has a large `<head>` section** — the HTML buffer was raised to 32 KB in v0.7.0; pages with unusually large headers before the `<title>` tag may still miss.
- **The site blocks bots** — some sites check the User-Agent and return empty or minimal content. UplinkIRC uses a standard Chrome/Linux UA.
- **SSL certificate errors** — self-signed or expired certs block the fetch silently. There is no per-site cert bypass yet.
- **The page has no `<title>` or `og:title`** — no preview will appear; there is nothing to display.

### Previews disappear when I switch channels

Known limitation — preview cards are appended to the chat view but not stored in the message history, so they don't survive a channel switch. This is tracked for a future fix.
