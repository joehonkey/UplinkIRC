<p align="center">
  <img src="assets/banner.svg" alt="UplinkIRC" width="800" />
</p>

A fast, secure, IRCv3-featured IRC client built with Qt6 and C++.  
Default network: **irc.linuxdojo.org:6697** — channel **#uplink**

---

## Features

- TLS/SSL connections via `QSslSocket` — plaintext IRC is not supported
- SASL PLAIN authentication — set `sasl_user` and `sasl_password` in config to authenticate before appearing on the network
- NickServ auto-identify — set `nickserv_password` to send `IDENTIFY` on connect
- **Bouncer support** — first-class ZNC and soju integration with `chathistory` replay, `znc.in/playback`, `soju.im/bouncer-networks`, `soju.im/read`, and self-message echo; configure with `bouncer = "znc"` or `bouncer = "soju"` in the server block
- IRCv3 CAP LS 302 negotiation: `multi-prefix`, `away-notify`, `server-time`, `message-tags`, `batch`, `chathistory`, `labeled-response`, `draft/typing`, `sasl`, and all ZNC/soju bouncer caps
- **Chat history replay** — on channel join, requests the last 100 messages via `CHATHISTORY LATEST`; history messages are displayed dimmed with their original timestamps so you can tell them apart from live messages
- Full IRC numerics and commands: JOIN, PART, QUIT, NICK, KICK, MODE, TOPIC, PRIVMSG, NOTICE, CTCP
- Slash commands: `/help`, `/join`, `/part`, `/nick`, `/me`, `/msg`, `/away`, `/back`, `/motd`, `/whois`, `/topic`, `/kick`, `/notice`, `/version`, `/ctcp`, `/sysinfo`, `/raw`, `/quote`, `/quit`
- Nick list sorted by prefix rank (`~&@%+`) with live updates
- Info bar — always shows `#channel (modes) * Network — N users`; topic drops below when enabled
- PM tabs — `/msg <nick>` opens a private message buffer in the sidebar
- Nick list right-click menu — Message, Whois, Give Op, Give Voice, Version
- Typing indicator — IRCv3 `draft/typing`; sends `active` immediately on first keypress, shows "nick is typing..." in real time
- Per-widget font sizes — independent control for every UI zone via **Hamburger → Font Config...**
- **Manage Servers** dialog — add, edit, and remove server connections without editing the config file; changes take effect immediately
- Auto-reconnect with exponential backoff (5s → 10s → 20s → 40s → 60s); deliberate `/quit` disables it
- Sidebar right-click menus — Disconnect/Reconnect on servers; Leave/Rejoin on channels
- 55 built-in themes, switchable live from **Hamburger → Theme**
- **App icon picker** — choose between Dark, Light (default), Light, and Avatar icon variants from the hamburger menu
- Connection status bar — persistent label showing `Connecting to…` / `Connected to…` / `Disconnected from…`; can be hidden from the hamburger menu
- Clickable URLs in chat and topic bar
- Link preview cards — URLs auto-fetch `og:title` and `og:image`; a card with title, domain, and thumbnail appears inline below the message
- System tray: left-click toggles window; minimizes to tray on close
- Unread dot indicator in sidebar (`● #channel`) — clears when channel is focused
- Message buffer cap (2,000 per channel) for stable long sessions
- Panel size persistence — sidebar and nick list sizes remembered across restarts
- Flat sidebar — servers as section headers, no expand arrows
- **Emoji picker** — click 😊 to open a searchable grid of ~400 emoji; type `:shortcode:` for inline autocomplete; full `:name:` patterns auto-substitute on send
- **Bot nick indicators** — nicks with `+B` mode show 🤖 or 👾 in the nick list
- Tab nick completion and input history (Up/Down)
- mIRC color code rendering — bold, italic, underline, colors in chat
- CTCP auto-replies for VERSION and PING
- **Windows-native** — uses the `windows11` Qt style by default on Windows; custom themes available via Hamburger → Theme

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

See [docs/configuration.md](docs/configuration.md) for the full reference, including bouncer setup.

---

## Documentation

- [Configuration](docs/configuration.md) — all config options with examples, including ZNC and soju bouncer setup
- [Commands](docs/commands.md) — available slash commands
- [IRCv3 support](docs/ircv3.md) — capability status
- [Keyboard shortcuts](docs/keyboard-shortcuts.md)
- [FAQ & Troubleshooting](docs/faq.md)

---

## Brand assets

The `assets/` directory contains all UplinkIRC brand files for free use:

| File | Description |
|---|---|
| `banner.svg` | Wide banner (2200×900) — used in README and About dialog |
| `logo.svg` | Full logo with wordmark (1200×500) |
| `wordmark.svg` | Wordmark only (650×220) |
| `icon-dark.svg` | App icon, dark variant (512×512) |
| `icon-light.svg` | App icon, light variant (512×512) |
| `icon-light-default.svg` | App icon, light default variant (512×512) |
| `icon-avatar.svg` | GitHub avatar / circular icon (512×512) |
| `icon-mark.svg` | Minimal mark (512×512) |
| `icon-tray.svg` | System tray icon (512×512) |

---

## License

MIT
