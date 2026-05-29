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

Not automatically yet. Auto-reconnect with backoff is planned. For now, close and reopen the app to reconnect.

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

Set the `password` field in your server block. UplinkIRC sends it as `PASS` before registration, which is how bouncers authenticate.

ZNC format — `username/network:password`:

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
```

soju format — `username:password`:

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
```

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

### The emoji button doesn't do anything

The emoji picker UI is not built yet. Toggle the emoji button in the hamburger menu to show or hide the button — the picker will be wired up in a future release.

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
