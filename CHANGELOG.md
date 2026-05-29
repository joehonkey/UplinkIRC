# Changelog

---

<!--
Session summary — 2026-05-28 sprint #2:

What was built:
  - NickServ IDENTIFY auto: nickserv_password in [[server]] config;
    PRIVMSG NickServ :IDENTIFY sent immediately after 001 RPL_WELCOME.
  - Toolbar unified with sidebar: toolbar, status bar, hamburger button
    all use sidebarBg; no border lines; QToolBar::handle zeroed; toolbar
    layout margins zeroed; no right-click context menu.
  - "Uplink" label removed from toolbar.
  - Hamburger size doubled (font * 2); flush left with no padding.
  - Panel size persistence: QMainWindow saveGeometry/saveState saved to
    QSettings on quit (connected via aboutToQuit signal); restored in
    constructor. Max-width caps on sidebar and nick list removed so panels
    can be freely dragged.
  - Topic bar fixed: channel+modes combined on left (m_topicLabel),
    topic text in middle with stretch (m_modesLabel), server name right.
    MODE #channel sent after 366 RPL_ENDOFNAMES so modes always arrive.
  - /sysinfo full rewrite: format OS: ... CPU: ... MEM: ... GPU: ... UP: ...
    GPU from vulkaninfo --summary (deviceName + last-parens of driverInfo
    e.g. RADV STRIX1), lspci fallback. Uptime from /proc/uptime parsed to
    days/hours/minutes/seconds. MEM is total GB rounded. CPU is model name
    only (no thread count). OS is "Linux (Name version) (kernel)".
    FreeBSD uptime from kern.boottime sysctl.

Bugs fixed:
  - Topic bar showed empty modes because MODE #channel was never requested;
    fixed by sending MODE after 366 end-of-names.
  - Topic text stored in ch->topic was never displayed; now shown in bar.
  - /sysinfo CPU showed "x86_64" (arch fallback) because /proc/cpuinfo
    parsing stopped before finding model name line. Fixed in rewrite.
  - /sysinfo RAM showed "Unknown" for same reason — MEM field now reads
    MemTotal only (no MemAvailable), clean and reliable.

Known issues left open:
  - Release workflow end-to-end not yet confirmed (needs a tag push)
  - SASL EXTERNAL not implemented
  - No reconnect on disconnect
  - Emoji picker not yet built

Next priorities:
  - Reconnect with backoff
  - Connection status indicator per server
  - AppImage packaging
-->

<!--
Session summary — 2026-05-28 sprint:

What was built:
  - /help command: lists all available slash commands in the active chat buffer
    as Server-type messages. One appendMessage call per line for clean display.
  - /sysinfo rewritten: replaces the old Qt-only QSysInfo one-liner with real
    system data. On Linux reads /etc/os-release (PRETTY_NAME), /proc/cpuinfo
    (model name + thread count), /proc/meminfo (MemTotal/MemAvailable), and
    uname -r for kernel. On FreeBSD uses sysctl hw.model/hw.ncpu/hw.physmem/
    vm.stats.vm.v_free_count/hw.pagesize. macOS uses sysctl hw.model/hw.ncpu
    for CPU. Output format: OS: <name> | Kernel: <ver> | CPU: <model> (N threads) | RAM: <used>/<total>GB
  - SASL PLAIN: full IRCv3 SASL flow. Config keys sasl_user/sasl_password in
    [[server]] block. If set: sasl cap added to CAP REQ, CAP END held until
    SASL completes, AUTHENTICATE PLAIN sent on CAP ACK, base64(\0user\0pass)
    payload sent on AUTHENTICATE +. 903 success → CAP END. 904/905/906
    failure → CAP END + server message. Numerics 900/903/904/905/906 handled.

Bugs fixed:
  None — all three changes are new features with clean builds.

Known issues left open:
  - Release workflow end-to-end not yet confirmed (needs a tag push)
  - SASL EXTERNAL not implemented
  - No reconnect on disconnect
  - Emoji picker not yet built

Next priorities:
  - NickServ IDENTIFY auto
  - Reconnect with backoff
  - Connection status indicator per server
  - AppImage packaging
-->

<!--
Session summary — initial build sprint:

What was built:
  Full Qt6/C++ IRC client from scratch. Project replaces DojoIRC (Go/Wails)
  with a native Qt Widgets app targeting smaller RAM footprint and long-session
  stability.

  Completed this sprint:
  - CMakeLists.txt with Qt6, tomlplusplus, POST_BUILD theme copy
  - TOML config loader (config.h/config.cpp) with full struct hierarchy:
    UiConfig, ServerConfig, ChannelConfig, Config
  - IRCv3 message parser (ircparser.h/cpp): tags, prefix, command, params
  - IrcClient (ircclient.h/cpp): QSslSocket TLS, CAP LS 302, all IRC
    numerics and commands, CTCP ACTION
  - Data model: Message, Channel (NickEntry + prefixRank), ServerSession,
    SessionModel (central hub with signal/slot wiring)
  - MainWindow: toolbar, hamburger menu, sidebar tree, nick dock,
    topic bar, chat view, input bar
  - TrayIcon: left-click shows window, right-click Show/Quit,
    unread red-dot badge, balloon notifications
  - AppIcons: maindefault.png / mainalt.png / about.png, user picks
    in hamburger, persists via QSettings
  - AboutDialog: horizontal logo (400x100), version, server info
  - ThemeLoader: 55 TOML themes, named {{key}} substitution (avoids
    Qt arg() %10+ mangling bug), live switching
  - First-run nick dialog if config has placeholder "yournick"
  - 55 theme files ported from DojoIRC

Bugs found and fixed:
  - Signal ordering: loadConfig() emitted signals before MainWindow
    connected them → sidebar/chat empty on launch.
    Fix: moved model->loadConfig() to after window.show() in main.cpp.
  - Qt arg() mangling: %10+ placeholders corrupted with >9 args.
    Fix: replaced all .arg() chaining with named {{key}} substitution
    via fill() helper and QHash.
  - QSS placeholder selector: used QLineEdit::placeholder (invalid),
    should be QLineEdit::placeholder-text. Fixed in themeloader.cpp.
  - Missing QListWidget forward declaration in mainwindow.h.
  - Missing QApplication include in mainwindow.cpp (qApp undefined).
  - qAsConst() deprecated in Qt6: replaced with std::as_const().
  - Default channel was #uplinkirc — corrected to #uplink throughout.

Known issues open:
  - Theme and icon choice persist via QSettings only, not config.toml.
  - No auto-reconnect on disconnect.
  - Emoji button toggle visible but picker not yet built.
  - Documentation panel in hamburger opens nothing (not yet built).
  - Window position/dock layout not persisted.
-->

<!--
Session summary — docs, repo, landing page sprint:

What was built:
  - Full documentation rewrite — all 5 docs/ files rewritten from scratch,
    all DojoIRC references removed. configuration.md, commands.md, faq.md,
    ircv3.md, keyboard-shortcuts.md now reflect actual v0.1.0 feature set
    and real config format ([[server.channels]] structure, [ui] block).
  - config.toml.example updated to match actual parser format.
  - README.md rewritten with real build instructions, per-platform dep
    install commands, feature list, config example, docs index.
  - version.h.in added — CMake configure_file bakes PROJECT_VERSION into
    version.h at build time. About dialog now shows version from header
    (UPLINKIRC_VERSION macro) instead of hardcoded string. To release:
    bump VERSION in CMakeLists.txt, rebuild, tag.
  - Global git commit-msg hook at ~/.config/git/hooks/commit-msg strips
    all AI co-author/attribution lines from every commit on this machine.
    git config --global core.hooksPath set to ~/.config/git/hooks.
  - GitHub repo created: https://github.com/joehonkey/UplinkIRC (public).
  - Branch protection on main — force push and deletion blocked.
  - GitHub Pages enabled from docs/ folder.
    Live at: https://joehonkey.github.io/UplinkIRC/
  - Landing page (docs/index.html) — dark Catppuccin design, hero with
    About.png logo, cross-platform section (Linux/FreeBSD/macOS/Windows/ARM64),
    feature cards, IRCv3 capability status grid, docs links, footer.
  - about.png resized from 2172px/894KB to 760px/143KB for fast page load.
    Optimized copy also written back to resources/icons/about.png.
  - CLAUDE.md (project + global) updated with hard rule: zero AI
    attribution anywhere ever — no Co-Authored-By, no Claude tags.

Bugs found and fixed:
  - about.png was 894KB (2172×724px) — invisible on dark hero background
    and slow to load. Fixed: resized to 760px wide, 143KB. White rounded
    card container added so logo is visible against dark background.

Known issues open:
  - Theme and icon choice persist via QSettings only, not config.toml.
  - No auto-reconnect on disconnect.
  - Emoji picker not yet built.
  - In-app Documentation panel not yet built (resources/docs/ doesn't exist).
-->

<!--
Session summary — font and input bar polish:

What was built:
  - IBM Plex Mono set as default application font (size 10, monospace fallback hint).
    Applied via QApplication::setFont() in main.cpp before any window is created.
  - Nick prefix label background fix committed: moved QWidget#inputBar and
    QWidget#inputBar QLabel rules into the theme QSS template. Removed inline
    stylesheet from the inputBar widget. This was built last session but the code
    commit landed after the docs close — documented here for completeness.

Bugs found and fixed:
  - Nick prefix QLabel white-background artifact: confirmed fix committed as 63c89a9.

Known issues open:
  - No auto-reconnect on disconnect.
  - Emoji picker not yet built.
-->

<!--
Session summary — icon, color codes, and input bar fix sprint:

What was built:
  - Synced ROADMAP/CHANGELOG: 4 commits from prior session landed after close-session
    had already run. Brought all docs surfaces back in line.
  - UserPromptSubmit hook: .claude/hooks/close-session-check.sh + .claude/settings.json
    detect "close session" / "end session" and inject the 8-step checklist into Claude's
    context so it cannot be skipped regardless of where in the session it fires.
  - Single SVG icon: replaced maindefault.png, mainalt.png, about1.png with one
    uplink.svg. Added Qt6::Svg to CMakeLists.txt find_package and target_link_libraries.
    Removed icon picker submenu from hamburger. Removed `icon` field from UiConfig,
    config.cpp load/save, and config.toml. AppIcons simplified to two inline functions.
  - mIRC color code rendering: ircToHtml() static function in mainwindow.cpp processes
    raw IRC text before HTML insertion. Handles \x02 bold, \x1D italic, \x1F underline,
    \x1E strikethrough, \x16 reverse (fg/bg swap), \x03fg,bg color (16 mIRC colors),
    \x0F reset. Applied to Privmsg, Action, Notice message types.
  - Nick prefix label background fix: moved inputBar and nick label styling entirely into
    the theme QSS template (QWidget#inputBar, QWidget#inputBar QLabel rules). Removed
    inline stylesheet from the inputBar widget. Fixes white-box artifact caused by Qt's
    stylesheet inheritance breaking background:transparent on children of styled parents.

Bugs found and fixed:
  - Nick prefix QLabel rendered with white background on themed builds. Root cause: Qt
    does not correctly propagate background:transparent through parent widgets that have
    inline stylesheets. Fix: remove inline stylesheet from inputBar, move all input bar
    styling into the theme QSS where Qt can resolve it in one pass.

Known issues open:
  - No auto-reconnect on disconnect.
  - Emoji picker not yet built.
-->

<!--
Session summary — UX features + docs panel sprint:

What was built:
  - Nick completion: Tab key at end of a word looks up matching nicks in the
    current channel's nick list and cycles through them on repeated presses.
  - Input history: Up/Down arrow in the input bar cycles through previously
    sent messages per-channel (standard IRC client behavior).
  - Colored nicks: consistent hash-based color per nick applied in chat view
    and nick list. Toggle in hamburger menu, persists to config.toml.
  - In-app Documentation panel: Hamburger → Documentation opens a QDialog
    (DocsDialog) with a tab per doc file rendered as plain text. New files:
    src/ui/docsdialog.h, src/ui/docsdialog.cpp. Resources registered in
    resources.qrc.
  - Save theme + icon to config.toml: previously these only persisted via
    QSettings. Now written back to config file on change so they survive
    reinstall / config copy.
  - About.png logo added to README.md hero section.
  - Configuration doc updated with icon key table.
  - Keyboard shortcuts doc updated — Tab/Up/Down marked as implemented.

Bugs found and fixed:
  - about.png had a white background that rendered invisible against the dark
    GitHub Pages hero section. Fix: normalized background to transparent /
    dark-compatible. Updated both docs/about.png and resources/icons/about.png.
  - Input bar config/QSettings interaction had a minor inconsistency causing
    prefs not to round-trip correctly on first launch after a clean config.
    Fixed in config.cpp.

Known issues open:
  - No auto-reconnect on disconnect — must restart app.
  - Emoji picker not yet built (button toggle is wired).
-->

<!--
Session summary — CI green sprint:

What was fixed:
  - CI was failing on Windows only. Linux and macOS were already passing after
    the per-platform Qt install switch. Windows aqtinstall was using arch
    'win64_msvc2019_64' which doesn't exist for Qt 6.8.x — Qt 6.7+ moved to
    MSVC 2022 builds. Fixed: changed arch to 'win64_msvc2022_64'.
  - Windows runner was hitting a Python permission error on the Windows Store
    python.EXE stub. Fixed: added explicit actions/setup-python@v5 step before
    jurplel/install-qt-action so aqtinstall gets a clean Python install.
  - Both fixes applied to ci.yml and release.yml.
  - All three platforms (ubuntu-24.04, windows-latest, macos-latest) now pass.

Known issues open:
  - Release workflow not yet confirmed end-to-end (no new tag pushed this session).
  - No auto-reconnect on disconnect.
  - Emoji picker not yet built.
-->

<!--
Session summary — v0.2.0 sprint:

What was built:
  - Sidebar: server item is now the server buffer — clicking "LinuxDojo" (or
    whatever the server short name is) opens the server message buffer.
    The redundant "(server)" child item is gone. Short name pulled from config.
  - Per-widget font config: Font Config dialog in hamburger lets the user set
    independent font sizes for Toolbar, Sidebar, Chat, User List, Users Title,
    Topic Bar, Nick Label, and Input. Family picker + live preview. Persists to
    config.toml under font_* keys.
  - Topic bar replaced with channel info bar: shows #channel (modes) ServerName
    all left-aligned. Modes come from actual server data (324 RPL_CHANNELMODEIS
    now parsed). No toggle button — hamburger "Show Topic Bar" controls the bar.
    Bar uses theme sidebarBg color so it works correctly on dark themes.
  - Typing indicator: IRCv3 draft/typing via TAGMSG. Sends typing=active after
    1s debounce, typing=done on send or clear. Receives and displays "nick is
    typing..." above the input bar. Per-nick 6s auto-clear. Hamburger toggle,
    on by default.
  - CTCP: auto-replies to incoming VERSION and PING. CTCP replies shown in server
    buffer. Outgoing CTCP via /ctcp command.
  - New slash commands: /away, /back, /motd, /whois, /topic, /kick, /notice,
    /version, /ctcp, /sysinfo. Version string sourced from version.h macro.
  - Colored nicks toggle added to hamburger menu (was config-only before).
  - GitHub Actions CI: per-platform Qt install (apt on Linux, aqtinstall on
    Windows, Homebrew on macOS). Builds on every push to main.
  - GitHub Actions release workflow: builds Linux tarball, Windows zip (windeployqt),
    macOS dmg (macdeployqt) and attaches them to GitHub releases on tag push.
  - v0.2.0 tagged and pushed. Tag re-pushed to trigger release workflow.
  - CMakeLists.txt: FetchContent fallback for tomlplusplus so CI doesn't need
    the system package.

Bugs fixed:
  - 324 RPL_CHANNELMODEIS was never parsed — channel modes always empty on join.
    Fixed: ircclient.cpp now handles numeric 324 and emits modesReceived.
    modesChanged signal added to SessionModel so the UI refreshes.
  - Topic bar used palette(mid) background which rendered invisible on dark themes.
    Fixed: objectName "topicBar" + QSS rule using theme sidebarBg/border colors.
  - CI failing: jurplel/install-qt-action with Qt 6.7.3 caused aqt package-not-found
    errors on all platforms. Fixed: Linux uses apt, Windows aqtinstall 6.8.2,
    macOS Homebrew. qtsvg is included in base Qt6 install, not a separate module.

Known issues open:
  - CI release workflow untested end-to-end (tag re-push happened after workflows
    were committed; outcome unknown at session close).
  - No auto-reconnect on disconnect.
  - Emoji picker not yet built.
  - /help command not yet implemented.
-->

## [Unreleased]

**NickServ auto-identify, UI polish, panel persistence, topic bar fix, /sysinfo v2**

- NickServ IDENTIFY auto — add `nickserv_password` to any `[[server]]` block; sent to NickServ on connect
- SASL PLAIN authentication — add `sasl_user` and `sasl_password` to any `[[server]]` block; full CAP negotiation flow; 903/904/906 handled
- `/help` command — lists all available slash commands in the chat buffer
- `/sysinfo` rewritten — format: `OS: Linux (Arch Linux rolling) (7.0.10-arch1-1) CPU: ... MEM: 24 GB GPU: AMD Radeon 890M Graphics (RADV STRIX1) (Vulkan) UP: 4 days 3 hours...`; GPU from `vulkaninfo`, fallback `lspci`; uptime from `/proc/uptime`
- Panel size persistence — sidebar and nick list sizes saved on quit, restored on launch; no more max-width caps
- Toolbar unified with sidebar — toolbar, status bar, hamburger all use `sidebarBg`; borders and handle gap removed; "Uplink" label removed; hamburger doubled in size and flush left; no right-click context menu
- Topic bar — shows `#channel (modes)` left, topic text middle, server name right; `MODE #channel` requested after join so modes always populate

**Fix:** Topic text was stored but never displayed — now shown in the topic bar  
**Fix:** Channel modes never populated — `MODE #channel` now sent after 366 RPL_ENDOFNAMES  
**Fix:** `/sysinfo` CPU showed `x86_64` fallback — fixed in rewrite  
**Fix:** `/sysinfo` RAM showed `Unknown` — fixed in rewrite  

---

## [0.2.0] — 2026-05-28

**Font config, typing indicator, info bar, extended commands, CI/CD**

- Per-widget font size config — Font Config dialog (hamburger) sets independent sizes for Toolbar, Sidebar, Chat, User List, Users Title, Topic Bar, Nick Label, and Input; family picker with live preview; persists to `config.toml`
- Typing indicator — IRCv3 `draft/typing` via TAGMSG; debounced send + receive; shows "nick is typing..." above input bar; hamburger toggle, on by default
- Channel info bar — replaces topic text bar; shows `#channel (modes) ServerName` left-aligned; modes populated from `324 RPL_CHANNELMODEIS`; uses theme colors
- Sidebar server item is now clickable — shows server buffer (MOTD, numerics); redundant `(server)` child removed; displays short name from config
- Extended slash commands — `/away`, `/back`, `/motd`, `/whois`, `/topic`, `/kick`, `/notice`, `/version`, `/ctcp`, `/sysinfo`
- CTCP — auto-replies to incoming VERSION and PING; `/ctcp` for arbitrary requests; replies shown in server buffer
- Colored nicks toggle added to hamburger menu
- GitHub Actions CI — builds on every push to main across Linux, Windows, macOS
- GitHub Actions release — builds platform binaries (Linux tarball, Windows zip, macOS dmg) and attaches to GitHub releases on tag push
- `tomlplusplus` FetchContent fallback in CMakeLists — CI no longer needs the system package
- Bumped version to 0.2.0

**Fix:** `324 RPL_CHANNELMODEIS` now parsed — channel modes populate correctly on join  
**Fix:** Topic bar background now uses theme `sidebarBg` — visible on all dark themes  
**Fix:** CI Qt install — switched to per-platform install to avoid aqtinstall mirror failures

**Known:** Release CI workflow not yet confirmed green end-to-end  
**Known:** No auto-reconnect on disconnect  
**Known:** Emoji picker not yet implemented

---

## [Unreleased] — v0.1.0

**Initial release — full IRC client foundation**

- Project scaffold — Qt6/C++17, CMake, directory structure
- TOML config loader via toml++ — auto-creates config on first launch
- First-run nick dialog — prompts if config has placeholder `yournick`
- IRC connection — TLS via QSslSocket, CAP LS 302 negotiation
- IRCv3 message tag parser — full prefix + tag parsing
- CAP negotiation — `multi-prefix`, `away-notify`, `server-time`, `message-tags`, `batch`, `labeled-response`
- Full IRC numerics — 001 welcome, 332/353/366 topic+names, MOTD, ISUPPORT, nick-in-use fallback
- IRC commands — JOIN, PART, QUIT, NICK, KICK, MODE, TOPIC, PRIVMSG, NOTICE, CTCP ACTION
- Slash commands — `/join`, `/part`, `/nick`, `/me`, `/msg`, `/quote`, `/raw`, `/quit`
- Data model — Message, Channel, ServerSession, SessionModel
- Message buffer cap — 2000 messages per channel
- Nick list — sorted by prefix rank (~&@%+), live updates on JOIN/PART/QUIT/NICK
- Main window UI — toolbar, sidebar tree, chat view, nick list dock, topic bar, input bar
- QDockWidget nick list — movable left or right
- Topic bar — channel topic + modes, toggleable from hamburger
- Hamburger menu — About, Documentation (stub), App Icon picker, Theme picker, topic/nick/emoji toggles
- System tray — minimize to tray on close, left-click shows window, right-click Show/Quit
- Unread badge — red dot on tray icon, balloon notification when hidden
- Theme loader — 55 TOML themes, live switching from hamburger
- App icons — Maindefault (flat satellite) and Mainalt (3D satellite), user picks in hamburger
- About dialog — horizontal logo, version, server info

**Fix:** Signal ordering — `loadConfig()` now called after `window.show()` so sidebar/chat populate correctly  
**Fix:** Qt arg() mangling with >9 placeholders in QSS — replaced with named `{{key}}` substitution  
**Fix:** `QLineEdit::placeholder` → `QLineEdit::placeholder-text` in theme stylesheets  

**Known:** Theme and icon choice persist via QSettings only, not saved to config.toml  
**Known:** No auto-reconnect on disconnect — must restart the app  
**Known:** Emoji picker not yet implemented (button toggle is wired)  
**Known:** Documentation panel opens nothing (not yet built)  
