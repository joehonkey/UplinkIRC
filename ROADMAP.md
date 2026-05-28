# UplinkIRC Roadmap

A fast, secure, IRCv3-featured IRC client built with Qt6 and C++.  
Default network: **irc.linuxdojo.org:6697** — channel **#uplink**

---

## Completed

- [x] Project scaffold — Qt6/C++17, CMake, directory structure
- [x] Config loader — TOML format via toml++, auto-creates `~/.config/LinuxDojo/UplinkIRC/config.toml` on first run
- [x] First-run nick dialog — prompts for nick if config has placeholder `yournick`
- [x] IRC connection — SSL/TLS via QSslSocket, CAP LS 302 negotiation
- [x] IRCv3 message tag parser — full prefix + tag parsing
- [x] CAP negotiation — requests multi-prefix, away-notify, server-time, message-tags, batch, labeled-response
- [x] Full IRC numerics — 001 welcome, 332/353/366 topic+names, MOTD, ISUPPORT, nick-in-use fallback
- [x] IRC commands — JOIN, PART, QUIT, NICK, KICK, MODE, TOPIC, PRIVMSG, NOTICE, CTCP ACTION
- [x] Data model — Message, Channel, ServerSession, SessionModel
- [x] Message buffer cap — 2000 messages per channel (RAM-flat on long sessions)
- [x] Nick list — sorted by prefix rank (~&@%+), live updates on JOIN/PART/QUIT/NICK
- [x] Main window UI — toolbar, sidebar (server/channel tree), chat view, nick list dock, topic bar, input bar
- [x] QDockWidget nick list — movable left or right
- [x] Topic bar — shows topic text + channel modes, toggleable from hamburger
- [x] Hamburger menu — About, Documentation (stub), App Icon picker, Theme picker, topic/nick/emoji toggles
- [x] Input commands — /join, /part, /nick, /me, /msg, /quote, /raw, /quit
- [x] System tray — minimize to tray on close, left-click shows window, right-click menu (Show/Quit)
- [x] Unread badge — tray icon gets red dot on unread messages, balloon notification when hidden
- [x] Theme loader — 55 TOML themes, applies as QSS stylesheet, live switching from hamburger
- [x] App icons — Maindefault (flat satellite) and Mainalt (3D satellite), user picks in hamburger, persists via QSettings
- [x] About dialog — horizontal Uplink IRC Client brand image, version, server info
- [x] Signal ordering fix — config loads after MainWindow connects signals
- [x] Version baked into binary — `version.h` generated from CMake `PROJECT_VERSION` at build time
- [x] Full documentation — configuration.md, commands.md, faq.md, ircv3.md, keyboard-shortcuts.md (beginner-friendly, real examples)
- [x] GitHub repo — public, branch-protected, invite-only contributions
- [x] GitHub Pages landing page — https://joehonkey.github.io/UplinkIRC/
- [x] Global git hook — strips all AI attribution lines from every commit machine-wide
- [x] Nick completion — Tab key completes nicks in input bar
- [x] Input history — Up/Down arrow cycles through sent messages
- [x] Colored nicks — consistent hash color per nick in chat and nick list
- [x] In-app Documentation panel — Hamburger → Documentation opens tabbed help viewer
- [x] Save theme + icon choice to config.toml (persists across restarts)
- [x] about.png background normalized — logo visible on both light and dark backgrounds

---

## In Progress

*(nothing currently in progress)*

---

## Planned — Near Term

- [ ] /help command — lists available commands in chat view
- [ ] IRC color code rendering — parse mIRC color codes in messages
- [ ] Topic click to edit — click topic bar to set a new topic
- [ ] SASL authentication — PLAIN and EXTERNAL mechanisms
- [ ] NickServ IDENTIFY auto — password field in config triggers identify on connect
- [ ] Multiple servers — add/remove servers from UI, not just config
- [ ] Reconnect logic — auto-reconnect with backoff on disconnect
- [ ] Connection status indicator — visual connected/disconnected state per server

---

## Planned — Medium Term

- [ ] Bouncer support — ZNC/soju via password field, PASS before registration
- [ ] Server-time IRCv3 — display timestamps from bouncer history
- [ ] Chathistory IRCv3 — replay from bouncer on join
- [ ] Message search — search within current channel buffer
- [ ] Logging — per-channel log files at `~/.config/LinuxDojo/UplinkIRC/logs/`
- [ ] URL detection + click to open — detect URLs in messages, open in browser
- [ ] Desktop notifications — notify on mention/PM when window not focused
- [ ] Emoji picker — emoji button in input bar opens picker panel
- [ ] DCC file transfer — send/receive files
- [ ] Split view — view two channels side by side

---

## Planned — Polish + Distribution

- [ ] Virtual scrolling — render only visible messages (performance on busy channels)
- [ ] Font size controls — per-zone font size adjustment
- [ ] Window state persistence — remember size, position, dock layout
- [ ] Config editor UI — edit servers/channels from within the app
- [ ] FreeBSD port skeleton
- [ ] Flatpak / AppImage packaging
- [ ] macOS .app bundle
- [ ] Windows installer
- [ ] Auto-update check

---

## Security Backlog

- [ ] Config file permissions — enforce 0600 on config.toml
- [ ] Password field encryption — don't store plaintext passwords
- [ ] Self-signed cert option — per-server accept/reject UI
- [ ] SOCKS5 proxy support

---

## Known Issues

- No reconnect on disconnect — must restart app
- Emoji button toggle wired but picker not yet implemented
