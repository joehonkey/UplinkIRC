<p align="center">
  <img src="docs/about.png" alt="UplinkIRC" width="480" />
</p>

A fast, secure, IRCv3-featured IRC client built with Qt6 and C++.  
Default network: **irc.linuxdojo.org:6697** — channel **#uplink**

---

## Features

- TLS/SSL connections via QSslSocket — plaintext IRC not supported
- IRCv3 CAP LS 302 negotiation: `multi-prefix`, `away-notify`, `server-time`, `message-tags`, `batch`, `labeled-response`
- Full IRC numerics and commands: JOIN, PART, QUIT, NICK, KICK, MODE, TOPIC, PRIVMSG, NOTICE, CTCP ACTION
- Slash commands: `/join`, `/part`, `/nick`, `/me`, `/msg`, `/raw`, `/quote`, `/quit`
- Nick list sorted by prefix rank (~&@%+) with live updates
- Topic bar showing channel topic and modes, toggleable
- 55 built-in themes, switchable from the hamburger menu
- System tray: minimizes to tray on close, unread badge, balloon notifications
- Message buffer cap (2000 per channel) for stable long sessions
- Movable nick list dock (left or right)
- Per-session nick dialog if config has placeholder `yournick`
- mIRC color code rendering — bold, italic, underline, colors in chat

---

## Building

### Dependencies

- CMake 3.16+
- Qt 6.2+ (Core, Widgets, Network, Svg)
- C++17 compiler (GCC, Clang, MSVC)
- [tomlplusplus](https://github.com/marzer/tomlplusplus)

### Install dependencies

**Arch Linux**
```bash
sudo pacman -S qt6-base qt6-svg cmake tomlplusplus
```

**Ubuntu / Debian**
```bash
sudo apt install cmake qt6-base-dev libqt6svg6-dev libtomlplusplus-dev
```

**Fedora**
```bash
sudo dnf install cmake qt6-qtbase-devel qt6-qtsvg-devel tomlplusplus-devel
```

**FreeBSD**
```bash
sudo pkg install cmake qt6-base qt6-svg tomlplusplus
```

**macOS (Homebrew)**
```bash
brew install cmake qt tomlplusplus
```

### Build

```bash
git clone https://github.com/joehonkey/UplinkIRC.git
cd UplinkIRC
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/UplinkIRC
```

The `themes/` folder is copied to the build directory automatically by CMake.

---

## Configuration

Config file is created automatically on first launch at:

| Platform | Path |
|---|---|
| Linux / FreeBSD | `~/.config/LinuxDojo/UplinkIRC/config.toml` |
| macOS | `~/Library/Application Support/LinuxDojo/UplinkIRC/config.toml` |
| Windows | `%APPDATA%\LinuxDojo\UplinkIRC\config.toml` |

Minimal example:

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
```

See [docs/configuration.md](docs/configuration.md) for the full reference.

---

## Documentation

- [Configuration](docs/configuration.md) — all config options with examples
- [Commands](docs/commands.md) — available slash commands
- [IRCv3 support](docs/ircv3.md) — capability status
- [Keyboard shortcuts](docs/keyboard-shortcuts.md)
- [FAQ & Troubleshooting](docs/faq.md)

---

## License

MIT
