# Changelog

---

<!--
Session summary — 2026-05-30 (planning — IRCv3 roadmap audit)

No code written this session. Analysis and planning only.

What happened:
  - Full project size audit: ~8,200 lines C++, 43 source files across src/irc/, src/ui/, src/model/, src/config/
  - Compared UplinkIRC against Halloy (Rust/iced) in depth — ~78,000 lines Rust, 205 .rs files
  - Audited entire IRCv3 spec against what UplinkIRC already implements vs has planned
  - Verified from source: ZNC + Soju bouncer support, SASL PLAIN+EXTERNAL, CHATHISTORY, batch,
    server-time, message-tags, labeled-response, away-notify, multi-prefix, draft/typing, bot mode
    display (+B), all confirmed fully implemented
  - Added 21 new IRCv3 roadmap items to ROADMAP.md (msgid, echo-message, draft/reply,
    draft/message-redaction, account-notify/tag/extended-join, Monitor, chghost, STS,
    invite-notify, setname, WHOX, userhost-in-names, netsplit/netjoin batches, Standard Replies,
    UTF8ONLY, message reactions, multiline messages, WebSocket transport, user metadata)
  - Updated docs/ircv3.md to list all new planned capabilities
  - Confirmed split view and SOCKS5 proxy were already on ROADMAP

Known issues remaining (unchanged):
  - DCC over internet (NAT/firewall blocks direct TCP)
  - No in-app update check UI
  - Message search not implemented
  - Per-channel logging not implemented
  - Split view not implemented
  - Plaintext passwords in config.toml

Next priorities (updated — see project_overview.md):
  - msgid (prerequisite for reactions, redaction, reply threading)
  - Message search (Ctrl+F)
  - Per-channel log files
  - echo-message
-->

<!--
Session summary — 2026-05-30 (v0.9.3 — Server buffer unread indicator)

What was built / fixed:
  - Server buffer unread indicator: the server name in the sidebar now turns
    purple (#cba6f7) when unread notices arrive (CTCP replies, NickServ, etc.),
    and resets to dim gray when you click into the server window. Previously the
    server item was silently excluded from the unread system entirely.
  - Server item is now selectable (Qt::ItemIsEnabled | Qt::ItemIsSelectable) so
    clicking the server name in the sidebar actually navigates to the server
    window. Before this fix, clicking it did nothing.
  - CTCP replies (VERSION and other unhandled CTCP NOTICEs) now route through
    noticeReceived instead of serverMessage, so they arrive as MessageType::Notice
    and are counted as unread. Previously they were posted as MessageType::Server
    which was explicitly excluded from unread counting.

Known issues remaining:
  - DCC over internet (NAT/firewall blocks direct TCP)
  - No in-app update check UI
  - Message search not implemented
  - Per-channel logging not implemented
  - Split view not implemented
  - Plaintext passwords in config.toml

Next priorities:
  - Message search (Ctrl+F in channel buffer)
  - Per-channel log files
  - DCC NAT traversal / passive DCC
  - In-app update check button
-->

## v0.9.3 — 2026-05-30

### Fixed
- **Server buffer unread indicator** — the server name in the sidebar now turns purple when unread notices arrive (CTCP replies, NickServ messages, server auth notices, etc.) and resets to dim gray when you switch into the server window. Previously the server item was completely excluded from the unread system, so `VERSION` replies and other server-directed notices landed silently with no visual indicator.
- **Server item now clickable** — clicking the server name in the sidebar navigates to the server window. Previously the item was non-selectable (`Qt::ItemIsEnabled` only) so clicking it did nothing.
- **CTCP replies counted as unread** — `VERSION` replies and other unhandled CTCP NOTICEs now route through `noticeReceived` and are counted toward the server buffer's unread total. Previously they were posted as `MessageType::Server` which was explicitly excluded from unread counting.

---

<!--
Session summary — 2026-05-30 (v0.9.2 — Preferences rework + link preview indent)

What was built / fixed:
  - Preferences dialog reworked:
      - Manage Servers and Documentation buttons moved to top (first thing visible)
      - Theme: collapsible list — button shows current theme, click to expand/collapse,
        stays open after selection so user can keep browsing with arrow keys + Enter
      - App Icon: QListWidget replaced with QRadioButton group (consistent with Interface checkboxes)
      - About button removed from Preferences (already in hamburger menu)
      - Docs button renamed to Documentation
      - Enter-applies-Font-Config bug fixed: setAutoDefault(false) on all buttons
      - Nick Brackets combo: smaller font (11px) and tighter padding
  - Link preview card indent: card margin-left now computed from font metrics to match
    hanging indent position instead of hardcoded 20px. Cards align with wrapped text.
  - How-To guide logo added and repositioned (64px margin-top)
  - Version bumped 0.9.0 → 0.9.2

Known issues remaining:
  - DCC over internet (NAT/firewall blocks direct TCP)
  - No in-app update check UI
  - Message search not implemented
  - Per-channel logging not implemented
  - Split view not implemented
  - Plaintext passwords in config.toml

Next priorities:
  - Message search (Ctrl+F in channel buffer)
  - Per-channel log files
  - DCC NAT traversal / passive DCC
  - In-app update check button
-->

## v0.9.2 — 2026-05-30

### Changed
- **Preferences dialog reworked** — Manage Servers and Documentation buttons are now at the top of the dialog, visible immediately on open. Theme selection is now a compact collapsible list: click the theme button to expand, use arrow keys to browse, press Enter or click to apply. The list stays open after selection so you can keep trying themes. App icon selection replaced with radio buttons. About button removed (it lives in the hamburger menu). Docs button renamed to Documentation.

### Fixed
- **Link preview card indent** — preview cards now align with the hanging indent position when hanging indent is on. The card's left margin is computed from font metrics to match where wrapped message text starts, instead of a hardcoded 20 px.
- **Enter key in theme list no longer triggers Font Config** — all buttons in the dialog have `autoDefault` disabled so Enter is never accidentally captured.
- **Nick Brackets dropdown** — reduced font size and padding so the combo box reads at the same visual weight as the rest of the dialog.

---

<!--
Session summary — 2026-05-30 (hanging indent + How-To sync)

What was built / fixed:
  - Hanging indent: wrapped message lines now align past the timestamp+nick column
    instead of wrapping back to column 0. Uses QTextBlockFormat (setLeftMargin +
    setTextIndent) on the paragraph block — NOT HTML tables, which caused messages
    to render in the centre of the chat view. Table approach was attempted and
    reverted; QTextCursor + QTextBlockFormat is the correct Qt approach.
    Applies to Privmsg, Action, Notice, and link preview cards via a shared
    insertHtmlBlock() static helper. Toggleable: Preferences → Hanging Indent
    (wrap under message), config key hanging_indent (default true).
  - insertHtmlBlock() helper added to mainwindow.cpp — replaces all m_chatView->append()
    calls for messages and cards. characterCount() > 1 guards against double blank
    line on first message after clear().
  - How-To guide (docs/howto.html) brought in sync with v0.9.0:
      - AppImage: download table row added, new section with run + zsync update steps
      - SASL EXTERNAL: full section (cert generation, NickServ fingerprint, config, GUI)
      - DCC file transfer: send/receive steps, timeout table, LAN-only warning
      - Hanging indent: new Tweaks section
      - Fixed stale troubleshooting note: "preview cards don't survive channel switch"
        → now correctly says fixed in v0.8.1
  - configuration.md: hanging_indent added to default template and options table
  - Close-session memory updated: howto.html explicitly listed as required surface

Known issues remaining:
  - DCC over internet (NAT/firewall blocks direct TCP connection)
  - No in-app update check UI
  - Message search not implemented
  - Per-channel logging not implemented
  - Split view not implemented
  - Plaintext passwords in config.toml

Next priorities:
  - Message search (Ctrl+F in channel buffer)
  - Per-channel log files
  - DCC NAT traversal / passive DCC
  - In-app update check button
-->

## v0.9.1 — 2026-05-30

### Added
- **Hanging indent** — wrapped message lines now align past the timestamp+nick column. When a long message wraps, continuation lines start under the message text, not at the left edge under the timestamp. Toggle at **☰ → Preferences → Hanging Indent (wrap under message)** or in config: `hanging_indent = true` (default on). Applies to messages, actions, notices, and link preview cards.

### Fixed
- **How-To guide sync** — `docs/howto.html` brought up to date with v0.9.0 features: AppImage download + zsync update, SASL EXTERNAL walkthrough, DCC file transfer section, hanging indent Tweaks entry. Stale troubleshooting note claiming link preview cards don't survive channel switches corrected (fixed in v0.8.1).

---

<!--
Session summary — 2026-05-30 (CHANGELOG structure fix)

What was built / fixed:
  - No code changes. Doc/infra fix session.
  - Found v0.9.0 CHANGELOG entry was buried after ~600 lines of HTML comment blocks
    and therefore not visible when GitHub renders CHANGELOG.md — v0.8.1 appeared
    as the first entry instead.
  - Root cause: close-session tooling appended new entries after the last comment
    block rather than before the first ## heading. New entries must be inserted
    immediately above the first visible ## heading, not appended to the bottom of
    the comment region.
  - Fixed by moving the v0.9.0 ## heading to line 224 (first visible position),
    removing the duplicate buried copy. Committed and pushed.
  - Also pushed the previous session's catch-up commit (MSVC fix docs) which had
    been committed locally but never pushed.

Known issues remaining (same as v0.9.0):
  - DCC over internet: local IP advertised; NAT blocks WAN connections
  - No in-app update check UI
  - Message search not implemented
  - Per-channel logging not implemented
  - Split view not implemented
  - Plaintext passwords in config.toml

Next priorities:
  - Message search (Ctrl+F in channel buffer)
  - Per-channel log files
  - DCC NAT traversal / passive DCC
  - In-app update check button
-->

<!--
Session summary — 2026-05-30 (post-v0.9.0 doc catch-up)

What was built / fixed:
  - No new features. Investigation session: confirmed v0.9.0 tag and branch are
    both on remote (origin/main is up to date). The v0.9.0 tag points to the MSVC
    hotfix commit (820af24) rather than the release commit (ee3b9ff) — cosmetic only,
    does not affect CI or release artifacts.
  - Caught that the MSVC std::min hotfix (820af24) landed after the close-session
    commit and was not documented. Added it to the v0.9.0 CHANGELOG Fixed section
    and the v0.9.0 session summary.

Known issues remaining (same as v0.9.0):
  - DCC over internet: local IP advertised; NAT blocks WAN connections
  - No in-app update check UI
  - Message search not implemented
  - Per-channel logging not implemented
  - Split view not implemented
  - Plaintext passwords in config.toml

Next priorities:
  - Message search (Ctrl+F in channel buffer)
  - Per-channel log files
  - DCC NAT traversal / passive DCC
  - In-app update check button
-->

<!--
Session summary — 2026-05-29 (exploration only — no release)

What was explored:
  - Investigated converting PreferencesDialog and DocsDialog from QDialog (separate
    window) to QFrame in-window overlays, matching the existing AboutDialog style.
  - Prototype was built and compiled cleanly: both dialogs converted to QFrame,
    shadow effect added, showCentered() wired up in mainwindow, close → hide().
  - User decided Preferences and Documentation need a proper OS window frame and
    reverted both back to QDialog. AboutDialog remains as the only in-window overlay.

Known issues remaining (unchanged):
  - Link preview cards don't survive channel switching
  - DCC Send File not implemented
  - AppImage packaging not done
-->

<!--
Session summary — 2026-05-30  post-v0.7.13 (hamburger UI polish + sidebar fix + docs overhaul)

What was built / fixed:
  - Hamburger (☰) button constrained to 22×22 px (was unconstrained width, expanding
    horizontally with the font size). Changed setFixedHeight(22) → setFixedSize(22, 22)
    and added setAutoRaise(true) to match the gear buttons.
  - Hamburger font changed from fs.toolbar*2 point-size (variable, could reach 20pt+)
    to a fixed 15px pixel-size font, matching the gear icon's visual size (gear renders
    at sz-1=15px inside a 16px pixmap). Removed hamburger from applyFontSizes() entirely
    so it no longer changes with user font preferences — consistent with gear behavior.
  - Hamburger and gear buttons no longer disappear when the sidebar is collapsed.
    Previously, the sidebar toggle set m_topicLeft->setFixedWidth(0), collapsing the
    entire left zone including the buttons. Added kBtnZoneMinW=48 constant (22+22+4px
    margin) as a floor — collapsed state now sets width to 48px, keeping both buttons
    pinned. Splitter-drag path also clamped with qMax(kBtnZoneMinW, w).
  - Full How-To guide created: docs/howto.html — left-side tree navigation, 8 sections
    (Getting Started, Basic Setup, Authentication, Multiple Servers, Interface, Chatting,
    Tweaks, Troubleshooting), platform tabs for dependencies, step badges, callout boxes,
    interactive tab selection, scroll-spy nav highlighting.
  - README download badges updated from v0.7.11 to v0.7.14.
  - README Documentation table updated: How-To Guide added as first/highlighted entry.
  - docs/index.html (GitHub Pages): nav version bump to v0.7.14, How-To added to nav bar
    and docs grid (highlighted card), footer links updated.
  - Releases: v0.7.13 (hamburger size fix), v0.7.14 (sidebar button visibility fix).

Known issues remaining:
  - Link preview cards don't survive channel switching (not stored in history)
  - DCC Send File not implemented
  - AppImage packaging not done
-->

<!--
Session summary — 2026-05-29  post-v0.7.12 (UI overhaul + signal bars)

What was built / fixed:
  - Signal bars connection/latency indicator added to topic bar — 4 stair-step bars,
    green solid when connected (bar count = ping latency: 4=<50ms, 3=<150ms,
    2=<300ms, 1=>300ms), blue flashing when connecting/reconnecting, red flashing
    when disconnected. Driven by periodic PING/PONG RTT measurement in IrcClient.
  - Desktop notification implemented as green dot on the system tray icon — fires on
    mention or PM when window is not focused. Toggle in Preferences → "Desktop
    Notifications". Clears on window activation (changeEvent). No popup balloon.
  - Full-width topic bar spanning the entire window width — left zone (sidebar width)
    holds hamburger and gear, right zone (chat width) holds signal bars + channel
    info. Left and right zones track the splitter so they stay pixel-aligned with
    the server list and chat area below. Both zones have explicit inputBg backgrounds
    so the bar renders continuously across the full window.
  - Hamburger and gear relocated from topic bar (right-side only) to the left zone
    of the new full-width bar — they now appear above the server list area.
  - Status bar (connection status label at bottom) removed entirely; statusBar()
    hidden. Input bar bottom padding bumped to compensate.
  - Removed show_conn_status config key and Preferences toggle — replaced by signal bars.
  - Added notifications = true config key for desktop notification toggle.
  - IrcClient now sends periodic PING :uplink_rtt every 30s when connected (first
    fires 2s after connect); handles PONG token match; emits pingRtt(host, ms).
    Also emits reconnecting(host) signal when doReconnect() fires.
  - Red dot on tray for general unread messages now also works (setUnread fixed
    to call updateIcon() properly).

Known issues remaining:
  - Link preview for title-only pages (no og:title) needs verification
  - AppImage packaging not done
  - DCC Send File not implemented
-->

<!--
Session summary — 2026-05-29  post-v0.7.14 (About dialog centering + docs search)

What was built / fixed:
  - About dialog now appears centered on the app window on all platforms including
    Wayland. Previous approach used QDialog::move() which Wayland compositors ignore.
    Refactored to a QFrame child widget (overlay) parented to the main window;
    positioned in parent-local coordinates via showCentered(); stays inside the
    app window, compositor has no involvement.
  - Documentation dialog gained a live search field in the tab bar corner (right of
    the Shortcuts tab). Uses QTabWidget::setCornerWidget; QLineEdit with leading
    search icon and a clear button. Typing jumps the active tab to the first match
    via QTextBrowser::find(); switching tabs re-applies the current query.

Known issues remaining:
  - Link preview cards don't survive channel switching (not stored in history)
  - DCC Send File not implemented
  - AppImage packaging not done
-->

<!--
Session summary — 2026-05-29  v0.8.0 — security hardening and stability pass

Context: a lead security/stability code review of the full codebase was performed.
Every finding was triaged, confirmed against source, and fixed. This is the first
dedicated security release for UplinkIRC.

What was fixed / built:

SECURITY
  - onSslErrors was calling ignoreSslErrors() unconditionally — MITM trivially
    possible on any TLS connection. Fixed: emit error + disconnectFromHost().
  - sendRaw emitted the raw line BEFORE writing it, so PASS :password,
    AUTHENTICATE <base64>, and NickServ IDENTIFY appeared verbatim in the raw
    log panel. Fixed: redactRawForLog() strips all three before emit.
  - Config save: QFile write with no permissions (world-readable on Linux),
    no escaping of special chars in TOML strings, no atomic write. Fixed:
    QSaveFile (atomic), QFile::setPermissions(ReadOwner|WriteOwner), tomlQuote()
    helper escapes \, ", \n, \r, \t.
  - Link previews auto-fetched every URL including private/LAN/loopback addresses.
    Fixed: isPrivateUrl() blocks loopback, 10/8, 172.16/12, 192.168/16,
    169.254/16, ::1, fc00::/7, localhost, and *.local before any fetch.
  - CTCP PING and VERSION replied unconditionally to any sender — reflection/
    amplification risk. Fixed: QHash<QString,qint64> m_ctcpTimestamps rate-limits
    both to once per nick per 5 s; PING echo payload capped at 32 bytes.

STABILITY / RAM
  - onReadyRead had no buffer cap — malicious server could send without \n and
    grow RAM indefinitely. Fixed: 64 KB cap on pending buffer, 8 KB cap per line.
  - Batch message storage had no cap — server could open BATCH and never close it,
    streaming unlimited messages. Fixed: 8 open batches max, 1 000 messages/batch.
  - QTextBrowser appended forever while model capped at 2 000 — long-session RAM
    growth in busy channels. Fixed: setMaximumBlockCount(kMessageBufferCap + 300).
  - onErrorOccurred emitted disconnected() and called scheduleReconnect() directly;
    onDisconnected() does the same on socket close — double signal + double timer.
    Fixed: onErrorOccurred emits only socketError(); onDisconnected() owns state.
  - trimmed() was stripping trailing IRC parameter spaces. Fixed: chop('\r') only.
  - Link preview image decode: loadFromData() after 200 KB cap still vulnerable to
    small compressed image that expands into huge pixel buffer. Fixed: QImageReader
    checks dimensions first; rejects > 4096×4096; scales during decode.
  - Channel::previews hash unbounded. Fixed: Channel::addPreview() evicts oldest
    entry when count >= 100.

PERFORMANCE
  - Nick comparisons used toLower()==toLower() throughout channel.h and
    sessionmodel.cpp — two allocs per call. Fixed: Qt::CaseInsensitive everywhere.
  - ServerSession::get() called toLower() + contains() + operator[] (two hash
    lookups). Fixed: one key + one find().
  - ircToHtml() allocated output string without reservation. Fixed: reserve(raw*2).
  - /sysinfo blocked UI thread on every call (vulkaninfo, lspci, powershell).
    Fixed: static cache for OS/CPU/MEM/GPU; only uptime re-queried.
  - loadConfig() and addServer() had identical server-session creation block.
    Fixed: shared spawnSession() helper.

Known issues remaining:
  - Link preview cards don't survive channel switches
  - Link preview for title-only pages (no og:title) — unverified
  - DCC Send File not implemented
  - AppImage packaging not done
  - SASL EXTERNAL (cert-based) not implemented

Next priorities:
  - Link preview card persistence across channel switches
  - AppImage packaging for Linux
  - DCC Send File
-->

<!--
Session summary — 2026-05-29  post-v0.8.0 (link preview persistence fix)

What was fixed:
  - Link preview cards now survive channel switches. The infrastructure (ch->previews,
    addPreview, refreshChatView re-injection) was already in place from a prior session
    but had a subtle URL key normalization mismatch: appendMessage stored raw regex-
    captured strings in m_previewChannels, while cardReady looked them up via
    pageUrl.toString() (QUrl-normalized). For URLs containing IRC control characters
    or percent-encoding, the keys diverged and addPreview() was never called.
    Fixed: both appendMessage and refreshChatView now normalize via
    QUrl(captured).toString() before using the string as a key.
  - ROADMAP "In Progress" stale item removed.
  - docs/faq.md updated: removed two "known limitation" notes about preview persistence.

Known issues remaining:
  - Link preview for title-only pages (no og:title) — unverified
  - DCC Send File not implemented
  - AppImage packaging not done
  - SASL EXTERNAL (cert-based) not implemented
-->

## v0.9.0 — 2026-05-30

### Added

- **DCC Send File** — right-click any nick in the user list and choose **Send File** to open a file picker and initiate a direct transfer. The recipient sees an accept/reject dialog showing the filename and size; accepted transfers save to a user-chosen path. Both sides get a live progress dialog with a cancel button. Outgoing transfers listen on a local TCP port (standard 4-byte big-endian ACK protocol); incoming transfers connect directly to the sender. A 60-second timeout fires if no connection arrives; a 30-second timeout fires on the receiver side if the sender never responds.
- **SASL EXTERNAL (certificate authentication)** — add `sasl_external = true`, `client_cert`, and `client_key` to a server block to authenticate via a TLS client certificate instead of a password. UplinkIRC loads the PEM certificate and key before the TLS handshake, negotiates `AUTHENTICATE EXTERNAL` during CAP, and sends an empty response — the server derives your identity from the certificate. Both RSA and EC (ECDSA) keys are supported. The Server dialog (☰ → Preferences → Manage Servers → Edit) gains a checkbox and browse buttons for cert/key paths.
- **AppImage packaging** — `packaging/build-appimage.sh` produces a self-contained `UplinkIRC-VERSION-ARCH.AppImage`. The AppImage embeds zsync metadata so users can update in-place with `appimageupdatetool` without a full re-download.
- **Automated release builds** — pushing a `v*` tag triggers CI on Linux (AppImage + tar.gz), Windows (zip), and macOS (DMG). All artifacts upload to the GitHub release automatically.

### Fixed

- **Link preview entity decoding** — page titles containing HTML entities (`&amp;`, `&#39;`, `&lt;`, etc.) are now decoded via `QTextDocument` before being displayed. Both `og:title` and the plain `<title>` fallback path are affected.
- **MSVC `std::min` conflict** — replaced `std::min` calls in `mainwindow.cpp` with ternary expressions to avoid the `min`/`max` macro collision on MSVC (Windows builds).

---

## v0.8.1 — 2026-05-29

### Fix

- **Link preview cards survive channel switches** — cards are stored per-channel (`Channel::previews`) and reinjected into the chat view when switching back. A URL key normalization mismatch (raw regex capture vs. `QUrl::toString()`) prevented storage for URLs containing IRC control characters or percent-encoded sequences; both extraction sites now normalize consistently.

## v0.8.0 — 2026-05-29

### Security

- **TLS certificate verification enforced** — `onSslErrors` previously called `ignoreSslErrors()` unconditionally, making every SSL connection silently vulnerable to MITM attacks. Now emits a TLS error and disconnects. Certificates must be valid.
- **Credentials never logged** — `PASS`, `AUTHENTICATE`, and `NickServ `IDENTIFY` commands are redacted before emission to the raw log. Passwords no longer appear in any visible panel.
- **Config file owner-only permissions** — `config.toml` is written with mode `0600`. Credentials in the config are no longer world-readable on multi-user systems.
- **Atomic config save** — replaced `QFile` write with `QSaveFile`. A crash during save cannot leave the config in a corrupt or truncated state.
- **TOML string escaping** — passwords, realnames, or any other string containing `"`, `\`, or newlines can no longer corrupt the config file.
- **Link preview blocks private/LAN addresses** — auto-previews now reject loopback, RFC 1918 (10/8, 172.16/12, 192.168/16), link-local (169.254/16), and `.local` hostnames. A malicious user cannot post a link to probe your internal network.
- **CTCP rate limiting** — `VERSION` and `PING` replies are limited to once per nick per 5 seconds. Reflected `PING` payloads are capped at 32 bytes to prevent amplification.

### Stability & RAM

- **Inbound buffer DoS protection** — `onReadyRead()` caps the pending input buffer at 64 KB; oversized lines (> 8 KB) are dropped. A server sending data without newlines can no longer grow RAM without bound.
- **IRC line parsing fix** — `trimmed()` was incorrectly stripping meaningful trailing spaces from IRC parameters; replaced with a simple `\r` chop.
- **Batch message caps** — max 8 open batches, 1 000 messages per batch. A misbehaving server cannot fill RAM via an unclosed `BATCH`.
- **QTextBrowser block count bounded** — `setMaximumBlockCount(kMessageBufferCap + 300)` keeps the chat view in sync with the message model, preventing RAM growth on long sessions in busy channels.
- **Duplicate disconnect signal fixed** — `onErrorOccurred()` no longer double-emits `disconnected()` or double-schedules reconnect.
- **Link preview image decode safety** — `QImageReader` checks dimensions before decoding; images over 4096×4096 are rejected; scaling happens during decode rather than after, preventing compressed-bomb inflation.
- **Channel previews capped at 100 entries** per channel; oldest entry evicted when the cap is reached.

### Performance

- **Nick comparisons use `Qt::CaseInsensitive`** — eliminates temporary `toLower()` string allocations on every comparison in large channels.
- **`ServerSession::get()` single lookup** — was calling `toLower()` and hashing twice per call; now one key + one `find()`.
- **`ircToHtml()` pre-allocates** — `out.reserve(raw.size() * 2)` avoids repeated reallocations on every message render.
- **`/sysinfo` caches static fields** — OS, CPU, MEM, and GPU are gathered once; only uptime is re-queried. No more blocking the UI thread on `vulkaninfo`/`lspci` after the first call.
- **Server session creation deduplicated** — `loadConfig()` and `addServer()` now share a single `spawnSession()` helper; less surface for divergence bugs.

---

## v0.7.15 — 2026-05-29

- Fix: About dialog now appears centered on the app window on Wayland and X11 — refactored from a top-level QDialog (position ignored by Wayland compositor) to an in-app QFrame overlay positioned in parent-local coordinates
- Feature: Documentation search — live search field in the tab bar (right of Shortcuts); clears with ×, jumps to first match in the active tab, re-runs on tab switch

---

## v0.7.14 — 2026-05-29

- Fix: hamburger (☰) and gear (⚙) buttons stay visible in the topic bar when the sidebar is collapsed; only the server list collapses beneath them

---

## v0.7.13 — 2026-05-29

- **Full-width topic bar** — spans above the server list and chat area; hamburger (☰) and gear (⚙) sit in the left zone above the server list; signal bars + channel info in the right zone above the chat
- **Signal bars indicator** — 4-bar stair-step signal strength widget in the topic bar; solid green when connected (bar count reflects ping latency), blue flashing when connecting/reconnecting, red flashing when disconnected
- **Latency measurement** — IrcClient sends a periodic PING every 30 s (first ping 2 s after connect); PONG round-trip time drives the signal bar count
- **Desktop notification dot** — tray icon gets a bright green dot on nick mention or PM when the window is not focused; clears automatically when the window gains focus; toggle in Preferences → Desktop Notifications
- **Status bar removed** — connection status label at the bottom of the window is gone; replaced entirely by signal bars
- Fix: tray icon unread (red) and notify (green) dot states now render correctly via unified `updateIcon()` logic
- Fix: `show_conn_status` config key and Preferences toggle removed; `notifications` key added (default `true`)
- Fix: hamburger button constrained to 22×22 px matching the gear icon; ☰ rendered at 15 px pixel-size for visual parity

---

<!--
Session summary — 2026-05-29  post-v0.7.12

What was built / fixed:
  - Sidebar is now drag-resizable. The handle between the sidebar panel and
    the chat area can be dragged left/right; the splitterMoved signal updates
    m_sidebarExpandedWidth live so the toggle button always restores to the
    last user-set width.
  - Sidebar width persists across sessions via QSettings key "sidebarWidth",
    saved on quit and restored at startup before the first layout pass.
  - Sidebar minimum width reduced from 140 px to 112 px (~4 character widths
    narrower) so users can squeeze it down more.
  - docs/faq.md updated: removed "sidebar is not drag-resizable" note; now
    describes drag-resize + persistence matching the nick panel entry.
  - ROADMAP: window state persistence item updated; "Sidebar not drag-resizable"
    known issue removed.

Known issues remaining:
  - DCC Send File not implemented
  - AppImage packaging not done
  - Link preview for title-only pages (no og:title) needs verification
-->

## v0.7.12-post — 2026-05-29

- Sidebar is now drag-resizable — drag the divider between the channel list and chat area to set the width
- Sidebar width persists across sessions (saved/restored via QSettings)
- Sidebar minimum width reduced slightly so it can be squeezed narrower

---

<!--
Session summary — 2026-05-29  v0.7.12

What was built / fixed:
  - Sidebar gear toggle relocated to the info bar (topicBar), immediately left
    of the hamburger button. The gear is now always visible regardless of
    sidebar state; no need to hunt for a button that disappears when the panel
    is hidden.
  - Sidebar converted from QDockWidget to embedded QWidget panel inside the
    new m_mainSplitter. Float/detach removed — the sidebar is no longer
    detachable. The QDockWidget, its title bar widget, and the ⧉ float button
    are gone.
  - Sidebar toggle collapses the panel to 0 px (full-width chat). Re-clicking
    the gear restores the fixed 180 px width. Handle width is 0 — users cannot
    drag to resize; only the gear controls visibility.
  - Nick panel gear header drift fixed: when the user list was hidden
    (setVisible false), Qt's VBox removed the list's stretch=1 contribution and
    floated the lone non-stretch header to vertical center. Fix: added a
    stretch spacer (stretch=1) below the nick list so there is always a stretch
    item anchoring the header at top. Nick list stretch bumped to 100 so the
    spacer is visually imperceptible when the list is visible.
  - Nick panel splitter width now persists across sessions: saved with
    QSettings key "nickSplitter" on quit, restored on launch.
  - Dark strip at bottom of sidebar eliminated: the old addStretch(1) in the
    sidebar VBox created a ~1% gap that showed the panel background color
    (different from the tree background). Removing it (sidebar tree fills
    panel with stretch=1, no spacer) fixes the artifact. A CSS rule for
    #sidebarPanel sets background to sidebarBg as a safety net.
  - CSS added for #sidebarToggleBtn (inputBg background, matches topicBar,
    accent on hover) and #sidebarPanel (sidebarBg background).
  - QTimer::singleShot sets initial mainSplitter sizes to 180/rest after first
    layout so the sidebar starts at the right width on every launch.

Design decisions:
  - Collapsed sidebar goes to 0 px (not a narrow strip) because the gear is
    now in the topicBar — there is nothing that needs to stay visible in the
    sidebar panel itself when hidden.
  - Sidebar width is fixed; mainSplitter handle is 0 px. Users resize the nick
    panel (right side) but cannot resize the sidebar. This matches the request
    "limit to pretty much the length it was."
  - addStretch approach for nick panel kept (needed to pin header to top) but
    stretch ratio is 100:1 — spacer is <1 px tall at normal window heights.

Known issues remaining:
  - DCC Send File not implemented
  - AppImage packaging not done
  - Link preview for title-only pages (no og:title) needs verification
-->

## v0.7.12 — 2026-05-29

- Sidebar gear toggle moved into the info bar, left of the hamburger — always visible, works whether the sidebar is open or closed
- Sidebar converted from floating dock to embedded panel; float/detach removed
- Sidebar collapses fully to 0 when hidden, expanding the chat to full width; fixed at ~180 px when shown; not drag-resizable
- Nick panel gear now stays pinned at the top of the panel when the user list is collapsed (was drifting to vertical center)
- Nick panel width persists across sessions
- Dark strip at bottom of sidebar eliminated

---

<!--
Session summary — 2026-05-29  v0.7.11

What was built / fixed:
  - Nick panel redesign: replaced the detachable QDockWidget with an embedded
    panel on the right side of the chat view inside a QSplitter. The panel is
    no longer floatable or detachable.
  - Animated gear toggle: a gear button (⚙) in the nick panel header spins one
    full rotation (~500ms) on click before collapsing the user list to a thin
    strip. Clicking again re-expands. The animation uses a QTimer at 16ms with
    a QPainter-rotated pixmap icon; debounce prevents double-firing mid-spin.
  - Collapsed state: only the nick list hides; the gear button and user count
    label remain visible in the header so the panel width stays unchanged.
  - Nick panel styling: background and text color now match the chat buffer
    ({{bufferBg}} / {{text}}) across all themes. Header tool button and label
    use transparent backgrounds. Splitter handle width set to 0 — no border
    between chat and nick panel.
  - Link preview persistence: added previews QHash to Channel struct so card
    HTML is retained across channel switches.

Design decisions:
  - Iterated on collapsed state: initial design hid the user count label and
    shrank panel to 24px; user preferred keeping gear + count visible with the
    list hidden and panel width unchanged. Simpler code — removed all
    splitter-resize logic from the toggle path.
  - makeGearIcon() placed before applyFontSizes() to resolve forward-decl
    link error on first build.

Known issues remaining:
  - DCC Send File not implemented
  - AppImage packaging not done
  - Link preview for title-only pages (no og:title) needs verification
  - Nick panel splitter width does not persist across restarts (no QSettings
    save for QSplitter position in central widget)

Next priorities:
  - Desktop notifications on mention/PM when window is not focused
  - Link preview for title-only pages — verify and fix if needed
  - Connection status indicator per server (currently global status bar only)
  - AppImage packaging for Linux
-->

## v0.7.11 — 2026-05-29

- Nick panel embedded in chat splitter — no longer a floating/detachable dock
- Gear button (⚙) animates a full spin before collapsing or expanding the user list; gear and user count remain visible when collapsed
- Nick panel background and text match the active theme's chat buffer colors
- Splitter handle hidden — chat and user list share a seamless background
- Link preview cards now persist across channel switches

---

<!--
Session summary — 2026-05-29  v0.7.9

What was built / fixed:
  - Server error routing: IRC numerics ≥ 400 (e.g. 482 chanoprivsneeded, 401
    nosuchnick, 404 cannotsendtochan, 421 unknowncommand, etc.) now appear in
    the active channel buffer as red "!!" messages instead of being silently
    swallowed into the (server) buffer. A new errorMessage signal in IrcClient
    routes them through SessionModel::onErrorMessage, which posts to
    m_activeChannel when on that server, falling back to (server) if no
    channel is active.
  - Multi-prefix removal fix: NickEntry now tracks all active mode prefixes in
    a QSet<QChar>. recomputePrefix() picks the highest-ranked one. Previously,
    -o on a @+ nick cleared the prefix entirely instead of demoting to +.
  - /time <nick>: sends CTCP TIME to a user; their local time appears in the
    active channel. Works the same way as /ping. If the target client does not
    support CTCP TIME, no reply appears (this is normal — many bots don't
    implement it). Added to tab completion and /help.
  - /ping and /time confirmation messages now stored in channel history: the
    "Pinged nick" and "Querying time for nick" local lines previously used
    appendMessage() which bypassed the model buffer, causing them to vanish on
    channel switch. Both now go through SessionModel::localMessage() and
    persist like any other message.
  - Mention notifications: when your nick is mentioned in an inactive channel,
    that channel shows 💡 in red in the sidebar. Regular unread activity shows
    🔥. Both clear when you open the channel.
  - Self-nick highlight in chat: any message containing your nick (word
    boundary, case-insensitive) renders your nick in red bold inline in the
    message text.

Known issues remaining:
  - Link preview cards lost on channel switch (not stored in message history)
  - Link preview for title-only pages not verified
  - DCC Send File not implemented
  - AppImage packaging not done

Next priorities:
  - Desktop notifications (system notification on mention/PM)
  - Link preview persistence across channel switches
  - AppImage packaging
-->

<!--
Session summary — 2026-05-29  v0.7.7

What was built / fixed:
  - Persistent Preferences dialog: hamburger (☰) now opens a non-modal QDialog that
    stays open while the user browses themes, toggles options, etc. Replaces the old
    QMenu that dismissed after every click. All settings (theme list, app icon, font
    config, 6 UI toggles, nick brackets, Manage Servers, About, Docs) live in one place.
    PreferencesDialog emits signals; MainWindow connects them — logic unchanged, UX fixed.
  - Bot icon randomization (carried from post-0.7.6): +B nicks now get a truly random
    icon (🤖 or 👾) assigned on first appearance instead of a deterministic hash value.
    Cached per nick for session stability.

Known issues remaining:
  - Link preview cards lost on channel switch
  - Link preview for title-only pages not verified
  - Server errors (482 etc.) in (server) buffer, not active channel
  - DCC Send File not implemented
  - AppImage packaging not done
  - MODE prefix removal loses lower-ranked prefixes until next NAMES
  - Hamburger menu briefly shrinks on theme switch (now less relevant — dialog stays open)
-->

<!--
Session summary — 2026-05-29  v0.7.8

What was built / fixed:
  - Hamburger menu restored as dropdown: ☰ now shows About UplinkIRC, Documentation,
    Preferences (opens dialog), Open Config (opens config.toml in system editor),
    Reload Config (re-applies all settings from disk without restarting). Order is
    intentional: About / Docs / Preferences / sep / Open Config / Reload Config.
  - About dialog: replaced wide banner.svg with the 96×96 dark app icon (app-icon-dark.svg).
  - New slash commands: /j (join alias), /ping <nick>, /invite <nick> [#ch], /mode,
    /op, /deop, /voice, /devoice, /ban, /unban, /clear. All added to /help output
    and docs/commands.md.
  - /ping round-trip time: CTCP PING reply is now intercepted in IrcClient; latency
    computed from echoed timestamp; displayed as "Ping reply from nick: Nms" in the
    active channel (not the server buffer). A new ctcpPingReply signal routes it through
    SessionModel::onCtcpPingReply which posts to m_activeChannel.
  - Tab completion for slash commands: handleTabComplete() now handles prefix.startsWith('/')
    by matching against a static sorted command list. Suffix is always a space (not ": ").
    Nick completion behavior is unchanged.

Known issues left open:
  - Server errors (482 etc.) in (server) buffer, not active channel
  - Link preview cards lost on channel switch
  - Link preview for title-only pages not verified
  - DCC Send File not implemented
  - AppImage packaging not done
  - MODE prefix removal loses lower-ranked prefixes until next NAMES

Next priorities:
  - Route server errors to active channel buffer
  - Link preview persistence across channel switches
  - Desktop notifications on mention/PM
-->

<!--
Session summary — 2026-05-29  v0.7.10

What was built / fixed:
  - Windows autojoin fix: toml::parse_file() was silently failing on Windows
    paths with non-ASCII characters (e.g. accented usernames in AppData).
    Replaced with Qt-native QFile read + toml::parse(string) so Qt handles
    the path and toml++ only parses the content string.

  - Simplified channel config format: [[server.channels]] array-of-tables
    replaced with a single comma-separated string on the [[server]] block:
      channels = "#uplink, #dojoirc"
    Load handles the new string format. Save always writes the new format.
    Legacy [[server.channels]] format removed entirely — no backward compat.

  - Reload Config now syncs channel list: SessionModel::syncServers() added;
    called from the Reload Config handler so manually edited channels in
    config.toml take effect immediately on the next reconnect without restart.

  - Close / Close Query in sidebar right-click: SessionModel::closeBuffer()
    added — for channels it sends PART then removes the buffer from the hash;
    for PM queries it removes the buffer directly. onChannelRemoved deletes
    the sidebar item. Channels show Close (after Rejoin/Leave); PM queries
    show Close Query (was previously no menu at all).

  - All docs updated: config.toml.example, docs/configuration.md (full
    example, minimal block, all bouncer/SASL/multi-server examples, channels
    section rewritten, old ordering gotcha removed), README.md (minimal +
    annotated examples, download badges bumped), CHANGELOG.md, docs/index.html
    feature card blurb updated.

Known issues remaining:
  - Link preview cards lost on channel switch (not in message history)
  - Link preview for title-only pages (no og:title) unverified
  - DCC Send File not implemented
  - AppImage packaging not done
  - Desktop notifications not implemented

Next priorities:
  - Desktop notifications on mention/PM
  - Link preview persistence across channel switches
  - AppImage packaging for Linux
  - DCC Send File
-->

<!--
Session summary — 2026-05-30 (v0.9.0)

What was built / fixed:
  - Link preview entity decoding: QTextDocument-based decodeEntities() applied
    to all three extractTitle() paths (og:title both attribute orderings and
    <title> fallback). Pages with &amp;, &#39;, &lt; etc. now show decoded titles.
  - DCC Send File: full implementation — DccSend (src/irc/dccsend.{h,cpp}) opens
    a TCP listener and streams file data with 4-byte big-endian ACK protocol;
    DccReceive (src/irc/dccreceive.{h,cpp}) connects to sender, writes file,
    sends cumulative ACKs. IrcClient parses incoming DCC SEND CTCP and emits
    dccSendReceived. SessionModel re-emits to MainWindow. Right-click nick menu
    "Send File" action wired up; incoming offers show accept/reject dialog with
    filename and size; both sides get a live QProgressDialog with cancel.
    60-second send timeout (no connection), 30-second receive timeout.
  - SASL EXTERNAL: sasl_external, client_cert, client_key added to ServerConfig,
    config.{h,cpp}, and IrcClient. IrcClient loads PEM cert+key onto QSslSocket
    before TLS handshake; negotiates AUTHENTICATE EXTERNAL + empty + response.
    RSA and EC keys both tried. ServerDialog gains checkbox + browse buttons.
  - AppImage packaging: packaging/build-appimage.sh — downloads linuxdeploy +
    Qt plugin on first run; DESTDIR install into AppDir; UPDATE_INFORMATION env
    var embeds zsync metadata for in-place updates via appimageupdatetool.
    themeloader.cpp gains ../share/uplinkirc/themes candidate for AppImage layout.
    CMakeLists.txt installs desktop file + SVG icon. release.yml Linux job builds
    AppImage after tar.gz; both .AppImage and .AppImage.zsync uploaded to release.
  - Docs: full SASL EXTERNAL walkthrough (cert generation, fingerprint registration,
    config); DCC Send section in commands.md; AppImage install + update in faq.md;
    ircv3.md SASL section expanded; README feature table + download badges updated.
  - Released v0.9.0, pushed tag v0.9.0 to trigger CI.
  - MSVC std::min conflict: replaced std::min calls in mainwindow.cpp with ternary
    expressions to avoid Windows min/max macro collision. Committed as hotfix after
    the session summary, so the close-session docs missed it.

Known issues remaining:
  - DCC over internet — local socket IP advertised; NAT/firewall on sender blocks
    direct connections. Works on LAN; WAN requires NAT traversal (future).
  - No in-app update check UI — zsync metadata is embedded and appimageupdatetool
    works externally, but there is no "Check for Updates" button inside the app.
  - Message search not yet implemented
  - Per-channel logging not yet implemented
  - Split view not yet implemented
  - Password field encryption (plaintext passwords in config.toml)
  - SOCKS5 proxy not yet supported

Next priorities:
  - Message search (Ctrl+F in channel buffer)
  - Per-channel log files
  - DCC NAT traversal or passive DCC
  - In-app update check button (trigger appimageupdatetool or show version badge)
-->

## [0.8.1] — 2026-05-30

### Fixed
- **Link preview card persistence** — preview cards now survive channel switches. URL keys are normalised before storage, and `refreshChatView` reinjects stored cards when switching back to a channel.

---

## [0.8.0] — 2026-05-29

### Security
- **TLS certificate verification enforced** — `ignoreSslErrors()` removed; `onSslErrors` now disconnects. Connections to servers with invalid or self-signed certificates fail immediately with an error in the server buffer. No silent bypass.
- **Credential redaction in raw log** — `PASS`, `AUTHENTICATE`, and `NickServ IDENTIFY` payloads are stripped from the raw log before `rawReceived` is emitted. Passwords never appear in any visible panel.
- **Config file hardening** — `config.toml` is written with `QSaveFile` (atomic rename) and `0600` permissions. All string values are escaped with `tomlQuote()` before writing.
- **Link preview LAN blocking** — `isPrivateUrl()` blocks loopback, RFC 1918 private ranges (`10.x`, `172.16–31.x`, `192.168.x`), link-local (`169.254.x`), and `.local` hostnames. A URL posted in chat cannot probe local services.
- **CTCP rate limiting** — `VERSION` and `PING` CTCP replies are limited to once per nick per 5 seconds. Reflected `PING` payloads are capped at 32 bytes.
- **DoS buffer caps** — inbound IRC buffer capped at 64 KB, batch messages at 1 000 per batch / 8 open batches, `QTextBrowser` block count bounded at `kMessageBufferCap + 300`.
- **Image decode safety** — `QImageReader` now checks reported dimensions before allocating; images larger than 4096 × 4096 are skipped.

---

## [0.7.10] — 2026-05-29

### Changed
- **Simplified channel config** — `channels = "#uplink, #dojoirc"` replaces the old `[[server.channels]]` array-of-tables blocks. All channels for a server go on one line, comma-separated. The Manage Servers dialog and Reload Config both honour the new format.

### Fixed
- **Config loading on Windows** — replaced `toml::parse_file()` with Qt-native file I/O so paths containing non-ASCII characters (e.g. usernames with accented letters) load correctly on Windows.

---

## [0.7.9] — 2026-05-29

### Added
- **`/time <nick>`** — sends a CTCP TIME request to a user; their local time appears in the active channel buffer (e.g. `* Time reply from sig: 2026-05-29T19:21:22.579Z`). If the target client does not support CTCP TIME, no reply is shown — this is normal for many bots.
- **Mention notifications** — when your nick is mentioned in a channel you are not viewing, that channel shows **💡** in red in the sidebar. Regular unread activity shows **🔥**. Both indicators clear as soon as you open the channel.
- **Self-nick highlight** — any message containing your nick is rendered with your nick in **red bold** inline in the chat. Word-boundary match, case-insensitive.

### Fixed
- **Server errors now appear in the active channel** — IRC error replies (numeric ≥ 400: 482 chanoprivsneeded, 401 nosuchnick, 404 cannotsendtochan, 421 unknowncommand, etc.) now show as red `!!` messages in your current channel instead of silently going to the `(server)` buffer.
- **Multi-prefix removal** — `-o` on an `@+` nick (op + voice) now correctly demotes to `+` voice instead of clearing the prefix entirely. `NickEntry` now tracks all active mode prefixes in a `QSet<QChar>`.
- **`/ping` and `/time` confirmation lines persist across channel switches** — "Pinged nick" and "Querying time for nick" local messages were previously lost when switching channels. They now go through the model buffer and survive re-renders.

---

## [0.7.8] — 2026-05-29

### Added
- **New slash commands** — `/j` (join alias), `/invite`, `/mode`, `/op`, `/deop`,
  `/voice`, `/devoice`, `/ban`, `/unban`, `/clear`, `/ping <nick>`
- **Tab completion for commands** — type a partial command (e.g. `/pi`) and press Tab
  to complete it. Cycles through matches the same as nick completion.
- **`/ping` round-trip time** — reply shows latency in milliseconds (e.g.
  `Ping reply from BeeMO: 4ms`) in the active channel alongside the sent confirmation.

### Changed
- **Hamburger menu restored as dropdown** — ☰ now opens a small menu with:
  **About UplinkIRC**, **Documentation**, **Preferences** (opens the settings dialog),
  **Open Config** (opens `config.toml` in your system editor), and **Reload Config**
  (re-applies all settings from disk without restarting).
- **About dialog** — now shows the app icon instead of the wide banner.

---

## [0.7.7] — 2026-05-29

### Added
- **Persistent Preferences dialog** — clicking ☰ now opens a non-modal dialog that stays
  open while you browse. Theme list, app icon picker, font config, UI toggles (topic bar,
  nick prefix, emoji button, typing indicator, connection status, colored nicks), nick
  brackets selector, and quick buttons for Manage Servers, Documentation, and About — all
  in one place, no more dismiss-on-click.

### Changed
- **Bot icon randomization** — `+B` nicks now receive a randomly assigned icon (🤖 or 👾)
  on first appearance instead of a deterministic hash value. Cached per nick for session
  stability.

---

<!--
Session summary — 2026-05-29  v0.7.6

What was built / fixed:
  - Hamburger menu relocated from bottom of nick list to left end of topic bar (left
    of #channel label). Menu popup now drops down-right from the button. Fixed height
    constraint removed so topic bar sets the size naturally.

  - Autojoin regression (root cause): ServerDialog::serverConfig() returned a
    ServerConfig with an empty channels list, so editing any server in the GUI silently
    wiped all [[server.channels]] entries. Fixed by adding an "Auto-join" field
    (comma-separated) to ServerDialog. Field round-trips correctly through edit and save.

  - Channel focus on join: onChannelAdded() only switched focus when activeChannel()
    was empty. Removed the guard; joining now always switches to the new channel.

  - Nick brackets GUI: "Nick Brackets" submenu added to hamburger menu using the same
    QWidgetAction/QListWidget pattern as App Icon.

  - CI: windows-latest pinned to windows-2025 in both ci.yml and release.yml ahead of
    the June 15, 2026 GitHub Actions cutover.

Known issues remaining:
  - Link preview cards lost on channel switch
  - Link preview for title-only pages not verified
  - Server errors (482 etc.) in (server) buffer, not active channel
  - Hamburger menu briefly shrinks on theme switch
  - DCC Send File not implemented
  - AppImage packaging not done
  - MODE prefix removal loses lower-ranked prefixes until next NAMES
-->

## v0.7.6 — 2026-05-29

### Fixed
- **Autojoin regression** — editing a server in Manage Servers was silently wiping all
  auto-join channels from the config. Root cause: `ServerDialog::serverConfig()` returned
  an empty `channels` list. Fixed by adding an **Auto-join** field (comma-separated
  channel names) to the Add/Edit Server dialog — channels now survive a dialog round-trip.
- **Channel focus on join** — manually joining a channel when another was already active
  did not switch focus. Joining now always switches to the new channel.

### Added
- **Nick Brackets in hamburger menu** — `nick_brackets` can now be changed from
  **Hamburger → Nick Brackets** without editing `config.toml`. Four choices shown with
  previews: `<nick>`, `[nick]`, `::nick::`, `nick`.
- **Hamburger moved to topic bar** — the ☰ button now sits at the left end of the
  `#channel (modes) * Network` info bar instead of the bottom of the nick list panel.

### CI
- Pinned `windows-latest` → `windows-2025` in both `ci.yml` and `release.yml` ahead of
  the June 15, 2026 GitHub Actions cutover.

---

## v0.7.5 — 2026-05-29

### Added
- **Configurable nick brackets** — new `nick_brackets` key in `[ui]` controls
  the characters that wrap nicks in chat messages. Value is split at its
  midpoint: `"<>"` → `<nick>`, `"[]"` → `[nick]`, `"::::"` → `::nick::`,
  `""` → no brackets. Defaults to `"<>"`. Existing configs without the key
  are unaffected.

---

<!--
Session summary — 2026-05-29  v0.7.5 — Configurable nick brackets

What was built:
  - nick_brackets config key in [ui] block. UiConfig gained a nickBrackets
    field defaulting to "<>". Config::load() reads nick_brackets from TOML
    with fallback to "<>". Config::save() writes it back. kDefaultConfig
    updated with the new key and a comment (careful: TOML comments with "()"
    inside R"()" raw strings close the literal — used safe comment text).

  - formatMessage() in mainwindow.cpp now reads m_config.ui.nickBrackets,
    splits at midpoint (even length) or first/last char (odd length), HTML-
    escapes both halves, and wraps the nick. The old hardcoded &lt;%3&gt;
    replaced with dynamic nickOpen + nick + nickClose. Empty string = no
    brackets at all.

  - docs/configuration.md: added "Nick bracket style" section with a table
    of all supported values, copy-paste examples, and a note about the
    midpoint-split parsing rule. Added nick_brackets to the full example
    block and the [ui] options table.

  - README.md: added nick_brackets with inline comment to the annotated
    config block.

  - v0.7.4 CI all green; Windows binary confirmed available.

Bugs fixed:
  - None (feature addition only)

Known issues remaining:
  - Link preview for title-only pages not verified
  - Link preview cards lost on channel switch
  - Hamburger menu briefly shrinks on theme switch
  - Server errors (482 etc.) in (server) buffer not active channel
  - DCC Send File not implemented
  - AppImage packaging not done
  - MODE prefix removal loses lower-ranked prefixes until next NAMES

-->

## v0.7.4 — 2026-05-29

### Fixed
- **Nick list: multi-prefix nicks** — `@+nick` was storing `+nick` as the nick
  name due to only stripping one leading prefix char; now consumes all leading
  prefix characters and keeps the highest-ranked one.
- **Nick list: MODE changes not refreshing display** — `+o`, `-o`, `+v`, `+B`
  etc. received after the initial NAMES reply now update `NickEntry.prefix` in
  place, re-sort the list, and emit `nickListChanged` so the display reflects
  immediately. Previously `modesChanged` only refreshed the topic bar.
- **Bot icons not showing on join** — client now sends `WHO #channel` after
  NAMES; `352 RPL_WHOREPLY` flags field (`B`) populates `botNicks` on every
  join/restart, matching Halloy behaviour.
- **Bot icons moved to right of nick name** — was `🤖 @nick`, now `@nick 🤖`.
- **Emoji picker: colored squares behind buttons** — global QSS
  `QToolButton { background-color: sidebarBg }` was leaking into the picker;
  buttons are now transparent with a subtle semi-transparent hover tint.
- **Typing indicator: layout shift** — label row now stays visible (empty text)
  when nobody is typing so the chat view height never shifts; hides only when
  the typing indicator feature is toggled off entirely.
- **Scrollbar width** — halved from 8 px to 4 px (border-radius 4 → 2 px).

---

## v0.7.2 — 2026-05-29

### Fixed
- **Windows: links invisible on dark themes** — `QTextBrowser` link color now
  set from the theme accent via `setDefaultStyleSheet`; previously links
  inherited the document default (black) regardless of theme.
- **Windows: nicks black when Colored Nicks is off** — replaced `palette(text)`
  (QSS-only syntax) with the theme's actual text color in inline HTML; on
  Windows `palette(text)` was falling back to black inside the HTML renderer.
- **Windows: `/sysinfo` returning Unknown for CPU/MEM/GPU/Uptime** — added
  Windows implementations: CPU from registry, MEM via `GlobalMemoryStatusEx`,
  GPU via PowerShell `Get-CimInstance`, Uptime via `GetTickCount64`.
- **Font Config dialog: fonts squashed on Windows** — `QFontComboBox` now has a
  minimum height so font names render at a readable size.
- **Font Config dialog: visual clutter** — size controls reorganized into a
  two-column grid (5 rows instead of 10); all options preserved.

---

<!--
Session summary — 2026-05-29  v0.7.4 — Windows polish, nick list correctness, WHO bot detection, emoji picker

What was built / fixed:
  - Windows theme colors: QTextBrowser link color and nick color when
    coloredNicks=off were both black on Windows. Root cause: palette(text)
    in inline HTML CSS doesn't resolve in Qt's HTML renderer on Windows
    (only works in QSS widget rules). Fixed by storing the loaded Theme struct
    as m_theme on MainWindow and using m_theme.text directly. Link color fixed
    by setting document()->setDefaultStyleSheet() with theme accent color,
    updated on theme change.

  - /sysinfo Windows: all four sysinfo functions returned "Unknown" on Windows.
    Added #if defined(Q_OS_WIN) branches: CPU from registry
    HKLM\...\CentralProcessor\0\ProcessorNameString, MEM via
    GlobalMemoryStatusEx, GPU via PowerShell Get-CimInstance
    Win32_VideoController, Uptime via GetTickCount64().

  - Font Config dialog Windows: QFontComboBox was rendering fonts in a tiny
    sliver because global QSS QToolButton padding was leaking in. Fixed with
    setMinimumHeight(30). Also reorganized 10 individual size rows into a
    2-column grid (5 rows) to reduce visual clutter while keeping all options.

  - Scrollbar width: halved from 8px to 4px (border-radius 4→2px).

  - Typing indicator layout shift: label was being setVisible(false) when
    nobody was typing, causing the chat view to resize on every typing event.
    Fixed by keeping label always visible with empty text; only truly hides
    when the typing feature is toggled off.

  - Nick list multi-prefix: setNicks() and addNick() in channel.h only stripped
    one leading prefix char. With multi-prefix negotiated, @+nick stored as
    nick="+nick". Fixed by consuming all leading prefix chars in a while loop,
    keeping highest-ranked.

  - Nick list MODE updates: onModesReceived() called parseBotModes() and emitted
    modesChanged (→ topic bar refresh only). Nick prefixes never updated live.
    Added modeToPrefix() helper mapping o→@, h→%, v→+, a→&, q→~. Now parses
    mode string, updates NickEntry.prefix in place, re-sorts, emits
    nickListChanged. Also emits nickListChanged after parseBotModes so +B takes
    effect immediately.

  - WHO-based bot detection: previously bots only showed icons if a +B MODE
    change arrived while connected. On restart/join, server doesn't replay
    existing umodes. Fixed by sending WHO #channel after 366 ENDOFNAMES.
    Added whoEntryReceived signal to IrcClient, case 352 handler extracts
    nick and flags, onWhoEntry() in SessionModel checks for 'B' in flags and
    updates ch->botNicks, emits nickListChanged.

  - Bot icons moved to right: was botIconForNick + " " + e.display(), now
    e.display() + " " + botIconForNick.

  - Emoji picker colored squares: global QSS QToolButton { background-color:
    sidebarBg; padding: 4px 10px 4px 0px } was applied to emoji picker buttons.
    Asymmetric padding shifted emoji off-center; sidebarBg made colored squares
    visible. Per-button setStyleSheet() overrides to transparent background,
    0 padding, and semi-transparent hover tint.

  - Emoji picker search grid alignment: when fewer than 9 results returned,
    QGridLayout stretched partial columns to fill scroll area width. Fixed by
    setting column minimum widths for all 9 columns and adding a stretch to
    column kCols to absorb leftover space.

  - docs/index.html: added Download section with direct binary links for all
    three platforms, updated version badge v0.2.0→v0.7.2, added Download to
    nav, changed hero primary button to "Download v0.7.2".

  - README download badges: hardcoded v0.7.1 filename in /releases/latest/
    download/ URLs; updated to v0.7.2.

Bugs fixed:
  - Windows: links black on dark themes
  - Windows: nicks black when coloredNicks=off
  - Windows: /sysinfo all Unknown
  - Windows: Font Config fonts squashed + too many rows
  - Nick list: multi-prefix nicks showing wrong nick name
  - Nick list: MODE changes not updating prefix display
  - Bot icons: not shown on join/restart (only on live MODE change)
  - Bot icons: on left instead of right
  - Emoji picker: colored squares behind buttons on all platforms
  - Emoji picker: search results spread across full width
  - Typing indicator: chat view height shifting up/down

Known issues remaining:
  - Link preview for title-only pages not fully verified
  - Link preview cards lost on channel switch
  - Hamburger menu briefly shrinks on theme switch (root cause unknown)
  - Server errors (482 etc.) appear in (server) buffer, not active channel
  - DCC Send File not implemented
  - AppImage packaging not done

-->

<!--
Session summary — 2026-05-29  v0.7.1 — Bug fixes: /nick label, image preview, typing indicator layout

What was built / fixed:
  - /nick display bug: typing /nick newname sent the NICK command correctly but
    the nick label next to the input bar never updated. Root cause: setNick()
    pre-sets m_nick to the new nick before the server echoes it back, so the
    NICK handler's check (msg.nick == m_nick) compared the old server-prefix
    nick against the already-updated value — always false. Fix: compare newNick
    (what the server confirmed) against m_nick using case-insensitive toLower()
    so server nick normalisation is also handled correctly.

  - Direct image URL preview: pasting a .png/.jpg/.jpeg/.gif/.webp URL in chat
    now shows a thumbnail card directly. Previously the URL was fetched as HTML,
    extractTitle found no <title> tag, and nothing was shown. Fix: isImageUrl()
    helper checks the URL path extension; if it matches, fetch() calls
    fetchImage() directly with the filename as the card title, bypassing HTML
    parsing entirely.

  - Typing indicator overlap: the indicator was a QLabel child of m_chatView,
    positioned as a floating overlay via move() at the bottom-left of the chat
    area. When the chat was full of messages it covered the bottom line of text.
    Fix: moved the label into the main VBox layout as a dedicated row between
    the chat view and the input bar. setVisible(false) collapses it to zero
    height, so there is no wasted space when no one is typing.
    repositionTypingLabel() reduced to a no-op; the resize-event call in the
    event filter becomes harmless.

  - README: added FreeBSD badge linking to the build-from-source dependency
    section so FreeBSD users know the platform is supported.

  - README: added cross-platform OS characters image (BSD devil / Tux / robot /
    Windows logo) under the download badges. Image was RGB with baked-in
    checkerboard pixels (not true RGBA transparency); flood-filled from corners
    with 20-unit colour tolerance and re-saved as RGBA to remove the background.

  - CI: Windows build flaky failure investigated. windows-latest runner mid-
    transition to windows-2025-vs2026. Re-run of the failed job passed. No
    workflow changes needed — failure was transient runner allocation, not code.

  - v0.7.1 tagged and released; all three platform builds (Linux, Windows,
    macOS) passed on the release workflow.

Bugs fixed:
  - /nick not updating the nick label (IrcClient NICK handler comparison)
  - No preview for direct image URLs (.png/.jpg/.gif/.webp etc.)
  - Typing indicator overlapping chat text when chat view was full

Known issues remaining:
  - Link preview for pages with only <title> (no og:title) — debug logging
    confirmed extraction should work; not yet fully verified in-app
  - Link preview cards lost on channel switch (not in message history)
  - Hamburger menu briefly shrinks on theme switch (root cause unknown)
  - Server errors (482 etc.) appear in (server) buffer, not active channel
  - DCC Send File not implemented
  - AppImage packaging not done

Next priorities:
  - Confirm / fix link preview for title-only pages (no og:title)
  - Route server errors to active channel buffer
  - Desktop notifications on mention/PM
  - AppImage packaging for Linux
  - DCC Send File
-->

<!--
Session summary — 2026-05-29  v0.6.0 — Emoji, bot icons, menu icons, Windows polish

What was built:
  - Emoji picker popup (EmojiPicker, Qt::Popup) — ~400 emoji in a 9-col scrollable grid
    with search box; opens from the 😊 button in the input bar; inserts at cursor
  - Inline :shortcode: autocomplete — type :smi and a no-focus overlay list appears above
    the input bar; Up/Down/Enter/Tab/click to select; Escape dismisses
  - Auto-substitute on closing colon — typing :trident: auto-replaces in the input field
    as soon as the closing colon is typed
  - Substitute on submit — any remaining :shortcode: patterns in the message are resolved
    to emoji before sending, so typing :trident: and pressing Enter sends 🔱
  - +B bot nick icons — nicks with +B user mode or channel user mode show 🤖 or 👾,
    chosen deterministically per nick via hash; tracked in Channel::botNicks and
    ServerSession::botNicks; parsed from both channel MODE and user MODE events
  - QPainter-drawn menu icons in hamburger — 12 distinct line icons for every menu item
    (info circle, server stack, open book, A-shape, half-circle, framed landscape,
    bar layout, person, smiley, speech bubble, signal bars, three persons);
    palette-aware (adapts to dark/light theme); replaces unicode text prefixes
  - Windows native style by default — QApplication::setStyle("windows11") set on Windows;
    ThemeLoader skips QSS when theme = "default" so fresh Windows install looks native;
    custom themes still apply when user picks one
  - Windows font default — Consolas on Windows, IBM Plex Mono on Linux/macOS
  - Win32 console window suppressed — WIN32 added to qt_add_executable; conhost.exe
    no longer spawned; closing nothing kills the GUI
  - Typing indicator overlay — moved from a VBox layout row to a child overlay of
    m_chatView; floats at bottom-left of chat area; transparent, no row or box;
    repositions on chat view resize via event filter
  - Typing indicator background removed — was using inputBg color; now transparent
  - Emoji button crop fixed — setFixedSize(30,30) + font-size:16px; padding:0
  - GitHub Actions Node.js 24 opt-in — FORCE_JAVASCRIPT_ACTIONS_TO_NODE24=true added
    to both ci.yml and release.yml before June 2nd forced-upgrade deadline

Bugs fixed:
  - Windows: console window (conhost.exe) spawned as parent of GUI process — killed GUI on close
  - Windows: monospace font falling back to random sans-serif — now defaults to Consolas
  - Windows: whole UI looked alien (custom dark QSS on native Windows) — native style now default
  - Typing indicator: gray background stripe between chat and input bar
  - Typing indicator: separate layout row created a visible container even with transparent bg
  - Emoji button: face emoji cropped because button had no height constraint and default padding
  - :shortcode: completion: required popup interaction; couldn't type full :name: directly

Known issues remaining:
  - Hamburger menu briefly shrinks when a theme is applied (QMenu sizeHint re-polish)
  - Link preview cards don't survive channel switch (not in message history)
  - DCC Send File stub in nick menu not yet implemented
  - Server errors (482 etc.) show in (server) buffer, not active channel

Next priorities:
  - Link preview persistence across channel switches
  - Server error routing to active channel buffer
  - AppImage packaging for Linux
  - Desktop notifications on mention/PM
-->

## v0.7.1 — 2026-05-29

### Fixed
- **`/nick` label not updating** — changing your nick with `/nick newname` sent the command correctly but the nick label next to the input bar kept showing the old nick. The server echoes back your nick change in a `NICK` message; the handler was comparing the wrong value and always missed the self-nick case.
- **No preview for direct image URLs** — pasting a `.png`, `.jpg`, `.jpeg`, `.gif`, or `.webp` link in chat now shows a thumbnail card. Previously these URLs were fetched as HTML, no `<title>` was found, and nothing appeared.
- **Typing indicator overlapping chat text** — the typing indicator floated as an overlay on top of the chat view and covered the last line of text when the window was full. It is now a proper layout row between the chat area and the input bar; it takes up no space when no one is typing.

### Added
- FreeBSD badge in README linking to build-from-source instructions
- Cross-platform OS characters artwork (BSD daemon / Tux / robot / Windows logo) on the GitHub project page

---

<!--
Session summary — 2026-05-29 macOS CI fix + link preview:

What was built/fixed:
  - macOS release CI was failing: MACOSX_BUNDLE not set in CMakeLists.txt,
    so CMake produced a plain binary instead of a .app bundle. macdeployqt
    requires a .app and errored out. Fixed by adding:
      if(APPLE) set_target_properties(UplinkIRC PROPERTIES MACOSX_BUNDLE TRUE) endif()
    v0.3.0 tag deleted, commit pushed, tag re-applied at new HEAD and pushed.
    All three platform builds (Linux, Windows, macOS) now pass.

  - LinkPreview class (src/ui/linkpreview.{h,cpp}) added:
    * Fetches up to 16KB of HTML per URL (abort-on-cap)
    * Extracts og:title (both attribute orders) + <title> fallback
    * Extracts og:image URL and fetches thumbnail (200KB cap, 120x90px max)
    * WhatsApp/2 User-Agent for better og:title hit rate on gated sites
    * 50-entry in-memory cache; one in-flight page fetch at a time
    * Concurrent image fetches (independent QNetworkReply per image)
    * Crash fix: abort() can fire finished() synchronously; old code called
      deleteLater() on m_reply after finished() had already nulled it.
      Fixed by disconnect(this) before abort() + stable local reply pointer
      in lambdas guarded by m_reply != reply check.

  - Hover tooltip: event filter on chat viewport + anchorAt(); shows domain
    name immediately on hover, updates to page title when fetch completes.
    Status bar also mirrors the title. Wayland-safe: uses mapToGlobal from
    mouse event position rather than QCursor::pos() in async callbacks.

  - Inline preview card: auto-triggered for live Privmsg/Action/Notice
    messages containing http/https URLs. Card appended to active channel
    view when cardReady fires. Layout: bordered table with thumbnail (if
    available) + bold title + domain. Colors drawn from Qt palette for
    theme compatibility.

Bugs fixed:
  - macOS CI crash (MACOSX_BUNDLE missing)
  - LinkPreview crash: null deref in fetch() when abort() fired finished()
    synchronously

Known issues left open:
  - Link preview cards don't survive channel switching (re-render doesn't
    re-fetch; would need to store cards in channel message history)
  - Hamburger menu still shrinks on theme switch (root cause unknown)
  - Server errors (482 etc.) still go to (server) buffer, not active channel
  - AppImage packaging not done
  - DCC Send File not implemented

Next priorities:
  - Route server errors (482 etc.) to active channel buffer
  - Connection status indicator per server in sidebar
  - AppImage packaging for Linux
  - DCC Send File
-->

<!--
Session summary — 2026-05-28 .gitignore cleanup:

What was fixed:
  - CLAUDE.md and .claude/ were listed in the public .gitignore, revealing
    internal tooling to anyone who reads the repo. Moved both entries to
    .git/info/exclude (local-only, never pushed). Public .gitignore now
    contains only standard build artifacts and common ignores.

Known issues left open: same as previous session.
Next priorities: reconnect with backoff, URL click in chat, server error routing.
-->

<!--
Session summary — 2026-05-28 hamburger menu + server management:

What was built:
  - Hamburger menu now opens above the button, anchored to the top-right
    corner using popup() with the button's top-right global position.
    Qt's screen-edge detection handles the flip automatically.
  - Theme list in hamburger is now scrollable and compact: uses
    QMenu::max-height stylesheet for native scroll arrows, compact item
    padding (2px). Individual actions used (not QWidgetAction) so the
    menu closes cleanly on selection.
  - ServerDialog: Add Server dialog with three sections — Connection
    (name, host, port, SSL checkbox), Identity (nick, user, realname),
    Authentication (server password, SASL user/password, NickServ
    password). Password fields use echo mode Password.
  - ManageServersDialog: list of all configured servers with Add, Edit
    (double-click or button), and Remove. Edit pre-fills the form.
  - SessionModel.addServer(), removeServer(), updateServer() — runtime
    server management without restart; connect/disconnect/reconnect live.
  - ServerConfig operator== added for dirty-checking on edit.
  - Hamburger menu entry changed from "Add Server..." to "Manage Servers..."
  - Explicit QMenu::item padding added to theme stylesheet to stabilize
    menu item height across theme switches.

Known issues left open:
  - Burger menu still visually shrinks when a theme is applied — root
    cause not yet identified; QMenu re-polish on setStyleSheet() appears
    to be computing a different sizeHint despite explicit item padding.
  - No reconnect on disconnect.
  - Server errors (482 etc.) appear in (server) buffer, not active channel.
  - URL click-to-open only in topic bar, not chat messages.
  - DCC Send File not implemented.

Next priorities:
  - Reconnect with exponential backoff
  - URL detection + click-to-open in chat messages
  - Route 482/channel errors to active buffer
-->

<!--
Session summary — 2026-05-28 UI layout + hamburger relocation:

What was built/fixed:
  - Topic bar background changed to match input area (inputBg) so the info
    bar blends with the input row rather than the sidebar.
  - Mode string spacing fixed: (modes) and * NetworkName now have a 6px gap
    between them instead of being flush against each other.
  - Clickable URLs in topic display: linkifyTopic() wraps http/https URLs in
    <a href> tags; QLabel configured with TextBrowserInteraction +
    setOpenExternalLinks so clicking opens the browser.
  - Toolbar hidden: nothing sits above #channel (+nt) * Network bar. Dock
    widgets and central widget all align at the top of the content area.
  - Dock title bars replaced with minimal custom bars: each dock (sidebar,
    nick list) gets a 16px title bar containing only a ⧉ float/detach button.
    Lists are flush with the topic bar; detach is still available.
  - Hamburger menu relocated from toolbar to the bottom-right of the nick
    list panel, sized to match the input bar height, so it sits in the
    natural bottom-right corner of the chat layout.
  - Status bar font shrunk to 7pt via QSS (Connected to irc.linuxdojo.org).

Known issues left open:
  - No reconnect on disconnect
  - Server errors (482 etc.) go to (server) buffer, not active channel
  - Emoji picker not built
  - DCC Send File not implemented
  - URL click-to-open only works in topic bar, not in chat messages
-->

<!--
Session summary — 2026-05-28 panel/dock + topic fixes:

What was fixed:
  - Panel detach restored: both sidebar and nick list panels can now be
    floated (detached) and re-docked using the float button in their title
    bars. Root cause: setTitleBarWidget(new QWidget) replaced the Qt title
    bar with a 0-height invisible widget, removing the float button entirely.
    Fix: removed custom title bar widgets; switched to Qt default title bar
    with empty title string and minimal QSS (0px padding, no border). Docks
    now show only a small float button with no text label.
  - "Users (N)" text removed from nick list panel: refreshNickList() was
    calling m_nickDock->setWindowTitle("Users (N)") on every nick list
    refresh, overwriting the empty title set at construction.
  - Sidebar re-dock location fixed: when the floating sidebar was closed
    (X button), it was re-appearing in the center/wrong area. Fix: the
    visibilityChanged handler now explicitly calls
    addDockWidget(Qt::LeftDockWidgetArea, m_sidebarDock) before setFloating
    to ensure it snaps back to the left.
  - Floating dock disappear fixed: closing a floating dock window now
    re-docks it to its home area instead of hiding it. Both sidebar and
    nick list docks have visibilityChanged handlers for this.
  - Sidebar dock stored as m_sidebarDock member (was anonymous local) to
    support the above.
  - Topic bar layout fixed: "* NetworkName — N users" label was positioned
    at the far right of the info bar. Moved to be immediately right of the
    "#channel (modes)" label, with the stretch spacer on the right.
  - /topic command parsing fixed: typing "/topic #channel new topic" now
    correctly sends TOPIC #channel :new topic. Previously the entire args
    string (including the channel name) was used as the topic text, causing
    the topic to be prefixed with the channel name or sending a malformed
    topic.

Known issues left open:
  - Topic set errors (482 channel-op-needed) go to the (server) buffer, not
    the active channel — user sees "nothing" when rejected. Needs error
    routing to active channel.
  - No reconnect on disconnect
  - DCC Send File not implemented
  - Emoji picker not built

Next priorities:
  - Route server errors (482, etc.) to active channel buffer
  - Reconnect with backoff
  - Connection status indicator per server
-->

<!--
Session summary — 2026-05-28 sprint #3:

What was built:
  - PM tabs: incoming PRIVMSGs open a sidebar buffer named after the sender.
    /msg <nick> opens the buffer and switches to it. Outgoing messages echo
    into the PM buffer. openPM() in SessionModel emits channelAdded for new
    conversations.
  - Nick list right-click menu: nick name (bold, disabled title), Message,
    Send File (disabled — DCC pending), Whois, Give Op, Give Voice, Version.
    Clean nick stored in Qt::UserRole on each list item.
  - Tray icon left-click now toggles window visibility (hide if visible,
    show+raise if hidden).
  - Unread indicator: dot only (● #channel) in sidebar — no color/bold change
    on the channel text. Tray badge removed entirely.
  - Sidebar flat list redesign: no expand arrows, no collapse, servers as
    muted uppercase section headers. Small grey circle icon on connected
    server. Branch lines hidden via QSS. Dock title bars ("Servers",
    "Users (N)") removed. Server header hover no longer causes visual shift.
  - Font Config additions: Network Name and Typing Indicator size controls
    (font_server_header, font_typing). Both persist to config.toml.
  - Nick list item padding tightened (1px vs 2px).
  - Topic bar redesign: info bar always visible showing #channel (modes)
    left and "* NetworkName — N users" right. User count updates live.
    Separate topic display (bufferBg background) drops below when Show Topic
    is toggled. Topic no longer appears inline in the info bar.
  - Channels indented 8px from server name (was 14px).

Bugs fixed:
  - Modes never populated in info bar because MODE #channel was never
    requested; fixed by sending MODE after 366 RPL_ENDOFNAMES.
  - Topic text was stored but never displayed; now shown in topic display.
  - CLAUDE.md was tracked in git despite being in .gitignore; untracked.
    AI-referencing language stripped from changelog comment blocks.

Known issues left open:
  - No reconnect on disconnect
  - DCC Send File not implemented
  - Emoji picker not built
  - Halloy-style sidebar/userlist aesthetic not fully matched
-->

<!--
Session summary — 2026-05-28 doc audit:

What was done:
  Full audit of all documentation against actual implemented features.
  Found and fixed stale/wrong content across three files:

  docs/index.html:
    - Version badge updated v0.1.0 → v0.2.0 (nav + footer)
    - SASL cap entry changed from "planned" (grey) to "active" (green)
    - draft/typing cap added as active (was missing entirely)
    - Two new feature cards added: SASL & NickServ, Persistent layout

  docs/faq.md:
    - "position is not persisted yet" updated to reflect panel persistence fix
    - Stale App Icon Picker FAQ removed (feature was removed from UI)
    - New entry: NickServ auto-identify setup with full config example
    - New entry: SASL PLAIN setup with full config example and network advice

  docs/ircv3.md:
    - SASL section moved from Planned to Active; updated to describe PLAIN
      implementation, config keys, and 903/904/906 handling
    - draft/typing added as new Active section with description

Known issues left open:
  - Emoji picker not built
  - No reconnect on disconnect
-->

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
  - Project rules updated: zero AI attribution anywhere, ever.

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
  - Synced ROADMAP/CHANGELOG: 4 commits from prior session landed after close had
    already run. Brought all docs surfaces back in line.
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

<!--
Session summary — 2026-05-28 UI polish sprint:

What was fixed:
  - Toolbar/hamburger right-click context menu eliminated. Qt's built-in toolbar
    visibility menu ("Main") appeared on right-click because right-click events
    propagated from the QToolButton → QToolBar → QMainWindow, where QMainWindow
    rendered the toolbar list menu despite NoContextMenu on the toolbar. Fix:
    switched both toolbar and hamburger to CustomContextMenu policy with a no-op
    lambda, which absorbs the event before it reaches QMainWindow.
  - Size grip (grey square, bottom-right corner) removed. Two-pronged fix:
    statusBar()->setSizeGripEnabled(false) in code, plus QSizeGrip { width: 0;
    height: 0; image: none } in the theme stylesheet.
  - Sidebar and nick list item spacing tightened. Items had 2px/1px vertical
    padding and Qt's default row height. Fixed with height: 18px in QSS for both
    QTreeWidget::item and QListWidget::item, plus setSpacing(0) on the nick list
    widget.

Investigated but reverted this session (not committed):
  - QMainWindow::separator styling attempts (transparent, sidebarBg, red diagnostic)
  - setFrameShape(QFrame::NoFrame) on sidebar, nick list, chat view
  - setTitleBarWidget(new QWidget()) on dock widgets (broke panel detach)
  - setCorner() calls to constrain dock separator height
  - Fusion style forced via QStyleFactory (app.setStyle)
  - Input bar bottom padding reduction
  - QSplitter refactor replacing dock widgets (broke entire layout, fully reverted)
  - windowState key renamed to windowState_v2

Known issues left open:
  - Dock separator lines visible at left/right edges of chat area, extending into
    the toolbar region — root cause confirmed as QMainWindow::separator; color
    and size changes did not fully suppress them; QSplitter refactor would fix
    this definitively but was reverted due to regressions
  - No reconnect on disconnect
  - DCC Send File not implemented
  - Emoji picker not built
-->

<!--
Session summary — 2026-05-29 v0.3.0 sprint:

What was built:
  - Clickable URLs in chat messages: swapped QTextEdit for QTextBrowser
    (drop-in, adds anchorClicked signal). Added linkifyHtml() that runs
    the same URL regex as linkifyTopic() on the already-HTML output of
    ircToHtml(). Applied to Privmsg, Action, and Notice messages.
    QDesktopServices::openUrl opens the link in the system browser.
    Topic bar already had this; chat now matches.
  - Reconnect with exponential backoff: IrcClient gained QTimer
    m_reconnectTimer, m_intentionalDisconnect flag, and m_reconnectDelay
    counter. Unexpected disconnect or socket error triggers scheduleReconnect()
    which emits "Reconnecting in Ns..." to the server buffer and fires the
    timer. Each failed attempt doubles the delay: 5s → 10s → 20s → 40s → 60s
    (capped). Successful connect resets to 5s. User-initiated quit() sets
    m_intentionalDisconnect to skip scheduling. SessionModel::onConnected
    already auto-joins configured channels after reconnect.
  - Sidebar right-click context menus: server items show Disconnect (if
    connected) or Reconnect (if disconnected). Channel items (#, &) show
    Rejoin (PART + 500ms delayed JOIN) and Leave. Menu built in
    MainWindow::onSidebarContextMenu using itemAt(pos) and UserRole data.

Bugs fixed: none — all new features.

Known issues left open:
  - Hamburger menu briefly shrinks on theme switch (root cause unknown)
  - Server errors (482 etc.) still go to (server) buffer, not active channel
  - Emoji picker not built
  - DCC Send File not implemented
  - Dock separator lines at chat edges

Next priorities:
  - Route server errors (482 etc.) to active channel buffer
  - Connection status indicator per server in sidebar
  - AppImage packaging for Linux
-->

<!--
Session summary — 2026-05-29  README redesign / repo beautification

What was built:
  - README.md completely redesigned from a plain bullet list to a rich,
    visually structured GitHub page:
    * 5 badges: latest release, CI status, MIT license, Qt6/C++17, IRCv3
    * 3 platform download buttons (Linux, Windows, macOS) linking directly
      to v0.6.0 release assets
    * App icon gallery — all 5 icon variants (Dark, Light Default, Light,
      Avatar, Mark) displayed side-by-side with labels
    * Features reorganized into 6 category tables: Security & Auth,
      IRC Protocol & IRCv3, Interface & Themes, Chat Features, Nick List
      & Sidebar, Connectivity & Servers
    * Quick Start up top with collapsible <details> blocks per OS for
      dependency install (Arch, Ubuntu, Fedora, FreeBSD, macOS)
    * Full annotated TOML config example with inline comments on every key,
      including multi-server, SASL, NickServ, bouncer
    * Slash commands table (all commands in one place)
    * Emoji shortcut examples inline
    * Keyboard shortcuts table
    * Documentation links table
    * Brand assets section with logo SVG displayed + file table

Bugs fixed: none (docs-only session)
Regressions: none
Known issues unchanged from v0.6.0

Next priorities:
  - Route server errors (482 etc.) to active channel buffer
  - Link preview persistence across channel switches
  - Desktop notifications on mention/PM
  - AppImage packaging for Linux
-->

<!--
Session summary — 2026-05-29  v0.7.0 — new dark banner + link preview fix

What was built / fixed:
  - New dark banner (uplink-top-banner-dark.svg) — replaces assets/banner.svg
    (GitHub README) and resources/icons/banner.svg (About dialog via QRC).
    Both locations updated in the same commit; QRC picks it up automatically.

  - Link preview fixed for regular URLs (e.g. linuxdojo.org):
    Root cause 1: Qt does not follow redirects by default.
      Fix: QNetworkRequest::RedirectPolicyAttribute →
           NoLessSafeRedirectPolicy on both page and image fetches.
           This was the primary blocker for bare-domain and http→https URLs.
    Root cause 2: WhatsApp/2 User-Agent blocked or returns minimal HTML
      on many sites.
      Fix: Switched to a standard Chrome/Linux UA with Accept + Accept-Language
           headers so servers return full HTML.
    Root cause 3: 16 KB HTML cap too small; some sites have large <head>
      sections before <title>.
      Fix: Raised to 32 KB; also updated title regex from [^<]{1,200} to
           [\s\S]{1,300}? so multi-line <title> tags are matched, then
           .simplified() collapses whitespace.
    Timeout also raised from 4 s to 6 s.

Bugs fixed:
  - Link preview silently failing for sites with http→https redirects
  - Link preview silently failing for sites that reject WhatsApp/2 UA
  - Link preview missing for sites with late or multi-line <title> tags

Regressions: none — og:title / og:image still extracted as before;
  thumbnail path unchanged.

Known issues remaining:
  - Link preview cards lost when switching channels (not in message history)
  - Server errors (482 etc.) still route to (server) buffer, not active channel
  - DCC Send File stub not yet implemented
  - Hamburger shrink on theme switch (root cause unknown)

Next priorities:
  - Route server errors to active channel buffer
  - Link preview persistence across channel switches
  - Desktop notifications on mention/PM
  - AppImage packaging for Linux
-->

## [0.7.0] — 2026-05-29

**New dark banner; link preview works for regular URLs**

- New dark banner (`uplink-top-banner-dark.svg`) used in both the GitHub README and the About dialog
- Fix: link preview now works for plain websites like `linuxdojo.org`
  — Qt redirect policy set to `NoLessSafeRedirectPolicy` (was silently not following http→https or bare-domain redirects)
  — User-Agent switched from `WhatsApp/2` to a standard Chrome UA with `Accept` headers (many sites rejected the old UA)
  — HTML fetch buffer raised from 16 KB to 32 KB so late `<title>` tags are reached
  — Title regex updated to match multi-line `<title>` content
  — Timeout raised from 4 s to 6 s

---

## [Unreleased] — 2026-05-29

**Repository beautification**

- README redesigned — badges (release, CI, license, Qt6/C++17, IRCv3), platform download buttons, app icon gallery (all 5 variants), features organized into 6 category tables, collapsible OS dependency install sections, full annotated TOML config example, slash commands table, keyboard shortcuts table, documentation links table, brand assets section with logo

---

## [0.6.0] — 2026-05-29

**Emoji picker, :shortcode: autocomplete, bot icons, native Windows style, menu icons**

- Emoji picker — click 😊 in the input bar to open a searchable popup grid of ~400 emoji; click any to insert at cursor
- Inline `:shortcode:` autocomplete — type `:smi` to get a live match list above the input; navigate with Up/Down, confirm with Enter/Tab/click, dismiss with Escape
- Auto-substitute on closing colon — typing `:trident:` replaces instantly in the input field when the closing `:` is typed
- Substitute on submit — any `:shortcode:` patterns remaining in a message are resolved to emoji before sending; `:trident:` + Enter sends 🔱
- Bot nick icons — nicks with `+B` mode display 🤖 or 👾 (chosen per-nick by hash, stable); tracked for both channel user mode and global user mode `+B`
- QPainter-drawn menu icons — each hamburger menu item has a distinct monochrome line icon drawn via QPainter; palette-aware so icons adapt to dark/light themes
- Windows: native style by default — `windows11` Qt style set on Windows; no QSS applied for the default theme; custom themes still apply when selected
- Windows: Consolas default font — `IBM Plex Mono` on Linux/macOS, `Consolas` on Windows
- Fix: Windows console window (`conhost.exe`) suppressed — `WIN32` subsystem flag set in CMakeLists.txt; closing nothing kills the GUI
- Fix: typing indicator now overlays the chat background — removed from layout row, now a child overlay of chat view, transparent, no box or stripe
- Fix: emoji button crop — `setFixedSize(30,30)` + `font-size:16px; padding:0`
- CI: opt in to Node.js 24 before GitHub's June 2nd forced upgrade deadline

---

## [Unreleased] — 2026-05-29

**macOS CI fix, link preview card with thumbnail**

- macOS release build fixed — `MACOSX_BUNDLE` property now set in CMakeLists.txt so CMake produces a `.app` bundle; `macdeployqt` can now find and package it. v0.3.0 re-tagged.
- Link preview on hover — hovering any `http/https` URL in chat shows the domain name in the status bar and a tooltip; the page title replaces it when the fetch completes
- Inline preview card — URLs in live messages auto-fetch `og:title` + `og:image` and render a small card below the message: bold title, domain, and thumbnail image (capped at 120×90px, 200KB)
- Fix: crash in `LinkPreview::fetch()` when `abort()` fired `finished()` synchronously, causing a null-pointer `deleteLater()` call

---

## [0.3.0] — 2026-05-29

**Reconnect backoff, clickable URLs in chat, sidebar context menus**

- Clickable URLs in chat messages — `http://` and `https://` links in PRIVMSG, actions, and notices are now live links; click to open in the system browser
- Auto-reconnect with exponential backoff — unexpected disconnects trigger automatic reconnect: 5s → 10s → 20s → 40s → 60s (capped); server buffer shows countdown; deliberate `/quit` disables it
- Sidebar right-click menus — right-click a server to **Disconnect** or **Reconnect**; right-click a `#channel` to **Rejoin** or **Leave**

---

## [Unreleased] — 2026-05-28

**Server management UI + hamburger menu improvements**

- Hamburger menu now opens above the button, anchored top-right — no more downward popup
- Theme list is scrollable with compact item padding — all 55 themes reachable via scroll arrows
- Theme application no longer causes visual glitch on menu close
- **Manage Servers...** dialog — view, add, edit, and remove servers from within the app; no config file editing required
- Add Server dialog — three-section form: Connection (name, host, port, SSL), Identity (nick, username, real name), Authentication (server password, SASL user/password, NickServ password)
- Edit server pre-fills the form with existing values; changes reconnect immediately
- Remove server disconnects immediately and removes from config
- All server changes persisted to `config.toml` on OK
- Fix: explicit `QMenu::item` padding in theme stylesheet stabilizes menu height across theme switches
- Known: menu still briefly shrinks on theme switch (investigation ongoing)
- Fix: `CLAUDE.md` and `.claude/` removed from public `.gitignore`; moved to `.git/info/exclude`

---

## [Unreleased]

**UI layout — topic bar, dock titles, hamburger relocation**

- Topic bar background now matches the input area (`inputBg`) — info bar blends with the input row
- Mode string spacing fixed — `(+nt)` and `* Network` now have a proper gap between them
- Clickable URLs in topic display — `http://` and `https://` URLs are live links; click to open in browser
- Toolbar removed — nothing sits above the `#channel` info bar; all panels start flush at the top
- Dock title bars replaced with minimal custom bars — each panel (server list, user list) has a 16px strip with only a `⧉` float/detach button; content is flush with the info bar
- Hamburger menu relocated to bottom-right of the nick list panel — sits level with the input bar; status bar text shrunk to 7pt

---

**UI polish — right-click menu, size grip, item spacing**

- Toolbar and hamburger right-click context menu suppressed — Qt's "Main" toolbar visibility popup no longer appears on right-click
- Status bar size grip (grey square, bottom-right corner) removed
- Sidebar channel list and nick list items tighter — reduced vertical padding and row height

---

**Panel detach, topic bar layout, /topic fix**

- Panel detach restored — sidebar (server/channel list) and nick list panels can now be floated and re-docked using the float button; closing a floating panel re-docks it instead of hiding it
- Topic bar layout — `* NetworkName — N users` now sits right next to `#channel (modes)`, not at the far right
- `/topic` command — now accepts `/topic #channel new topic` (channel name optional; correctly separates it from the topic text)

**Fix:** Nick list panel was showing "Users (N)" label above the list — stopped refreshNickList from overwriting the dock title  
**Fix:** Sidebar panel re-docked to wrong area (behind chat) — now explicitly re-adds to left dock area  
**Fix:** `/topic #uplink some text` was sending the channel name as part of the topic string  

---

**PM tabs, nick menu, sidebar redesign, topic bar, font config additions**

- PM tabs — `/msg <nick>` opens a sidebar buffer; incoming PMs land in sender's buffer; both sides echoed
- Nick list right-click menu — Message, Send File (disabled), Whois, Give Op, Give Voice, Version
- Tray icon left-click toggles window — hides if visible, shows if hidden
- Unread indicator — dot only (`● #uplink`) in sidebar; no tray badge
- Sidebar flat list — no expand arrows, servers as muted uppercase section headers, grey circle icon on connected server, dock titles removed, hover on server headers fixed
- Font Config — added "Network Name" and "Typing Indicator" size controls; persist as `font_server_header` / `font_typing`
- Info bar always visible — `#uplink (+nt)` left, `* LinuxDojo — 42 users` right; user count live
- Topic display — separate area below info bar with chat-window background, shown/hidden by "Show Topic" toggle
- Channels indented 8px from server name; nick list items tighter

**Fix:** Modes never populated — `MODE #channel` now sent after `366 RPL_ENDOFNAMES`  
**Fix:** Topic text was stored but never shown — now displayed in topic display area  

---

**Doc audit — fixed stale content across index.html, faq.md, ircv3.md**

- `docs/index.html` — version updated to v0.2.0; SASL cap marked active; `draft/typing` cap added; SASL/NickServ and panel persistence feature cards added
- `docs/faq.md` — panel persistence FAQ updated; NickServ IDENTIFY and SASL PLAIN FAQ entries added with full config examples
- `docs/ircv3.md` — SASL moved from Planned to Active with full description; `draft/typing` added as Active

---

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
