# CLAUDE.md — UplinkIRC

## Project
- **Name:** UplinkIRC
- **Stack:** Qt6 / C++17
- **Purpose:** Fast, secure, IRCv3-featured IRC client for LinuxDojo.org and general use
- **Owner:** joehonkey

## Default Network
- **Server:** LinuxDojo — `irc.linuxdojo.org:6697` (SSL)
- **Default channel:** `#uplinkirc`
- **Config template:** `config.toml.example` — auto-copied to `~/.config/uplinkirc/config.toml` on first launch

## Key Directories
- `src/irc/` — IRC protocol layer (parser, client, cap negotiation)
- `src/ui/` — Qt Widgets UI (main window, panels, dialogs)
- `themes/` — TOML color themes (55 included, format carried from DojoIRC)
- `docs/` — User-facing documentation
- `resources/` — Icons and static assets

## Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
Requires: `qt6-base` (Qt6 Core, Widgets, Network)

## Run
```bash
./build/UplinkIRC
```

## Rules
- Prefer small, minimal edits.
- Read existing files before changing structure.
- Preserve the current coding style.
- Do not make unrelated changes.
- Keep functions small and readable.
- Reuse existing helpers before creating new ones.
- Do not add new dependencies unless necessary.

## Code Style
- C++17, Qt6 conventions
- Use Qt signal/slot for all async communication
- `QSslSocket` for all IRC connections (SSL default, plain optional)
- Header guards via `#pragma once`
- Class members prefixed `m_`

## Safety
- Do not edit secrets, tokens, or env files unless explicitly asked.
- Ask before destructive operations.
- Do not ever remove files without asking first.

## IRC / IRCv3
- `IrcParser` handles IRCv3 message tags — always parse through it
- `IrcClient` owns the socket and emits signals — UI never touches the socket directly
- CAP negotiation goes in `IrcClient::processLine` (CAP LS 302 already wired)
