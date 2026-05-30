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
- [x] Main window UI — toolbar, sidebar (server/channel tree), chat view, nick list panel, topic bar, input bar
- [x] Nick list panel — embedded on the right side of the chat view in a QSplitter
- [x] Topic bar — shows topic text + channel modes, toggleable from hamburger
- [x] Hamburger menu — About, Documentation (stub), App Icon picker, Theme picker, topic/nick/emoji toggles
- [x] Persistent Preferences dialog — hamburger now opens a non-modal QDialog; stays open while browsing themes/toggles; replaces dismiss-on-click QMenu
- [x] Hamburger restored as dropdown — ☰ opens About UplinkIRC, Documentation, Preferences, Open Config, Reload Config
- [x] Open Config / Reload Config — Open Config opens config.toml in system editor; Reload Config re-applies all settings from disk without restarting
- [x] Input commands — /join, /j, /part, /nick, /me, /msg, /quote, /raw, /quit, /ping, /invite, /mode, /op, /deop, /voice, /devoice, /ban, /unban, /clear
- [x] System tray — minimize to tray on close, left-click shows window, right-click menu (Show/Quit)
- [x] Unread badge — tray icon gets red dot on unread messages; green dot for mention/PM when window not focused
- [x] Theme loader — 55 TOML themes, applies as QSS stylesheet, live switching from hamburger
- [x] App icon — single uplink.svg via Qt6::Svg; icon picker removed (one icon)
- [x] About dialog — shows dark app icon (96×96), version, and server info
- [x] Signal ordering fix — config loads after MainWindow connects signals
- [x] Version baked into binary — `version.h` generated from CMake `PROJECT_VERSION` at build time
- [x] Full documentation — configuration.md, commands.md, faq.md, ircv3.md, keyboard-shortcuts.md (beginner-friendly, real examples)
- [x] GitHub repo — public, branch-protected, invite-only contributions
- [x] GitHub Pages landing page — https://joehonkey.github.io/UplinkIRC/
- [x] README beautification — badges, icon gallery, feature tables, annotated config, commands table, download buttons
- [x] Global git hook — strips all AI attribution lines from every commit machine-wide
- [x] Nick completion — Tab key completes nicks in input bar
- [x] Slash command tab completion — Tab also completes /commands (e.g. /pi → /ping)
- [x] Input history — Up/Down arrow cycles through sent messages
- [x] Colored nicks — consistent hash color per nick in chat and nick list
- [x] In-app Documentation panel — Hamburger → Documentation opens tabbed help viewer
- [x] Save theme + icon choice to config.toml (persists across restarts)
- [x] about.png background normalized — logo visible on both light and dark backgrounds
- [x] IRC color code rendering — mIRC bold, italic, underline, strikethrough, reverse, 16 colors (fg+bg)
- [x] Per-widget font size config — Font Config dialog in hamburger; independent sizes for all UI zones; persists to config.toml
- [x] Typing indicator — IRCv3 draft/typing TAGMSG; debounced send + receive; hamburger toggle
- [x] Extended slash commands — /away, /back, /motd, /whois, /topic, /kick, /notice, /version, /ctcp, /sysinfo, /help
- [x] /sysinfo rewritten (v2) — OS/CPU/MEM/GPU/UP format; GPU via vulkaninfo (deviceName + renderer), lspci fallback; uptime from /proc/uptime
- [x] CTCP auto-replies — incoming VERSION and PING answered automatically
- [x] Channel info bar — shows #channel (modes) ServerName; modes from 324 RPL_CHANNELMODEIS
- [x] Sidebar server item clickable — opens server buffer, shows config short name
- [x] Colored nicks hamburger toggle — was config-only, now in UI
- [x] GitHub Actions CI — builds on every push across Linux, Windows, macOS
- [x] GitHub Actions release workflow — builds platform binaries on tag push, attaches to GitHub releases
- [x] v0.2.0 released
- [x] PM tabs — /msg opens sidebar buffer; incoming PMs land in sender's buffer
- [x] Nick list right-click menu — Message, Send File (stub), Whois, Give Op, Give Voice, Version
- [x] Tray icon left-click toggles window visibility
- [x] Unread dot indicator in sidebar — 🔥 for activity, 💡 red for nick mentions; clears on focus
- [x] Sidebar flat list — Halloy-style: no arrows, servers as section headers, connected icon, dock titles removed
- [x] Font Config: Network Name and Typing Indicator size controls
- [x] Info bar always visible with channel, modes, network, user count
- [x] Topic display — separate drop-down area with chat-window background, toggled by Show Topic
- [x] Panel detach/float — sidebar panel could be detached and re-docked (removed in v0.7.12; sidebar is now a fixed embedded panel)
- [x] Topic bar layout — network/user count next to channel label, not far right
- [x] /topic channel-name parsing — `/topic #channel text` correctly separates channel from topic text
- [x] Topic bar background matches input area — info bar visually joins the input row
- [x] Mode string spacing — gap between `(modes)` and `* Network` fixed
- [x] Clickable URLs in topic display — http/https links open in browser
- [x] Toolbar removed — nothing above the info bar; all panels start flush at top
- [x] Minimal dock title bar — sidebar had a slim bar with a ⧉ float button (replaced in v0.7.12 by gear toggle in the info bar)
- [x] Hamburger menu relocated — now in topic bar, left of #channel label
- [x] Status bar text shrunk — 7pt via QSS
- [x] Clickable URLs in chat messages — http/https links in PRIVMSG, actions, and notices open in browser
- [x] Reconnect with exponential backoff — auto-reconnect on unexpected disconnect; 5s→10s→20s→40s→60s; deliberate quit disables it
- [x] Sidebar right-click context menus — server items: Disconnect/Reconnect; channel items: Rejoin/Leave/Close; PM queries: Close Query
- [x] v0.3.0 released
- [x] macOS release CI fixed — MACOSX_BUNDLE property set; .app bundle produced; macdeployqt succeeds; v0.3.0 re-tagged and all three platform builds pass
- [x] Link preview — hover tooltip shows domain + page title; inline card with og:image thumbnail auto-appears below live URL messages
- [x] Link preview fixed for regular URLs — redirect following, proper UA, 32 KB buffer, multi-line title regex
- [x] Dark banner — uplink-top-banner-dark.svg used in README and About dialog
- [x] `/nick` label update — changing nick now immediately reflects in the input bar nick label
- [x] Direct image URL preview — `.png/.jpg/.jpeg/.gif/.webp` links show a thumbnail card without HTML parsing
- [x] Typing indicator layout — moved from chat-view overlay (covered text) to dedicated layout row; collapses when idle
- [x] FreeBSD download badge in README — links to build-from-source instructions; signals FreeBSD support
- [x] Cross-platform OS characters image on GitHub project page

---

## In Progress

- [ ] Link preview for title-only pages — pages without og:title (plain `<title>` only) may not preview; needs verification and potential fix

---

## Planned — Near Term

- [x] /help command — lists available commands in chat view
- [x] SASL authentication — PLAIN mechanism (CAP negotiation, AUTHENTICATE, 903/904/906)
- [ ] SASL EXTERNAL — certificate-based auth (not yet implemented)
- [x] NickServ IDENTIFY auto — `nickserv_password` in config; sent to NickServ on RPL_WELCOME
- [x] Server error routing — 482 and other server errors shown in active channel buffer, not just (server)
- [x] Multiple servers — Manage Servers dialog: add, edit, remove with live connect/disconnect
- [x] Reconnect logic — auto-reconnect with backoff on disconnect
- [x] Connection status indicator — signal bars widget in topic bar; latency-based bar count (4=<50ms … 1=>300ms); blue flash=connecting, red flash=disconnected
- [x] Mention notifications — 💡 red sidebar indicator when nick is mentioned in inactive channel; 🔥 for general activity; self-nick highlighted red bold in chat
- [x] Desktop notifications — green dot on tray icon on mention/PM when window not focused; clears on window focus; toggle in Preferences

---

## Planned — Medium Term

- [ ] Message search — search within current channel buffer
- [ ] Logging — per-channel log files at `~/.config/LinuxDojo/UplinkIRC/logs/`
- [x] URL detection + click to open — http/https links in chat open in browser (v0.3.0)
- [x] Link preview persistence — cards survive channel switches (stored in Channel struct)
- [ ] DCC file transfer — send/receive files
- [ ] Split view — view two channels side by side

---

## Planned — Polish + Distribution

- [ ] Virtual scrolling — render only visible messages (performance on busy channels)
- [x] Window state persistence — sidebar width and nick panel width saved via QSettings on quit, restored on launch; sidebar is drag-resizable
- [x] Nick panel width persistence — QSplitter position saved via QSettings on quit, restored on launch
- [x] Config editor UI — Manage Servers dialog covers server-level editing
- [x] Emoji picker — searchable popup grid with :shortcode: autocomplete and auto-substitution
- [x] Bot nick indicators — 🤖/👾 shown for +B mode nicks; auto-detected on join via WHO reply; randomly assigned per nick each session, cached for stability
- [x] Configurable nick brackets — `nick_brackets` in `[ui]`; `"<>"` `"[]"` `"::::"` `""` supported; also configurable live from Hamburger → Nick Brackets
- [x] Autojoin regression fix — editing a server in the GUI no longer wipes auto-join channels; Auto-join field added to Add/Edit Server dialog
- [x] Autojoin Windows fix — config loading now uses Qt file I/O instead of toml::parse_file; paths with non-ASCII characters load correctly on Windows
- [x] Simplified channel config — `channels = "#uplink, #linux"` replaces `[[server.channels]]` array-of-tables; Reload Config syncs changes live via SessionModel::syncServers()
- [x] Close/Close Query in sidebar — right-clicking a channel shows Close (PARTs + removes buffer); right-clicking a PM query shows Close Query (removes buffer)
- [x] Channel focus on join — joining a channel now always switches focus to it
- [x] Nick panel redesign — replaced detachable QDockWidget with embedded panel in QSplitter; gear button (⚙) in header animates a full spin before toggling the user list; gear and user count remain visible when collapsed; panel background matches chat buffer color across all themes
- [x] Sidebar gear toggle — ⚙ button in the info bar (left of hamburger) collapses the server/channel list to 0 px (full-width chat) and restores it; sidebar is a fixed-width embedded panel, not drag-resizable
- [x] Native Windows style — windows11 Qt style by default; no alien dark theme on fresh installs
- [ ] FreeBSD port skeleton
- [ ] AppImage packaging for Linux
- [ ] Auto-update check

---

## Security Backlog

- [ ] Config file permissions — enforce 0600 on config.toml
- [ ] Password field encryption — don't store plaintext passwords
- [ ] Self-signed cert option — per-server accept/reject UI
- [ ] SOCKS5 proxy support

---

## Known Issues — UI

## Known Issues

- Link preview for title-only pages (no og:title) — may not preview; needs verification
- DCC Send File in nick menu is disabled — not yet implemented
