# IRCv3 Support

UplinkIRC aims to be a first-class IRCv3 client. Below is the full status of every negotiated capability, including bouncer-specific extensions.

---

## How capability negotiation works

UplinkIRC uses **CAP LS 302** — it requests the server's full capability list before deciding what to ask for, and only requests capabilities the server actually advertises. This avoids failures from batching unsupported caps together. After the server acknowledges the requested caps (CAP ACK), UplinkIRC completes registration or starts SASL if needed.

---

## Active capabilities

These capabilities are fully negotiated and actively used.

### `multi-prefix`

Returns all mode prefixes for a nick in NAMES replies — for example, `@+alice` instead of just `@alice`. The nick list correctly displays users who hold more than one status simultaneously (e.g. both op and voice).

### `away-notify`

When a user in a shared channel sets or clears away status, the server sends a real-time AWAY message. UplinkIRC receives and tracks these notifications.

### `server-time`

When the server attaches a `time` tag to a message, UplinkIRC uses that timestamp instead of the local clock. This is essential for bouncer history replay — messages show the time they were originally sent, not the time the client received them. Works across all message types (PRIVMSG, NOTICE, JOIN, etc.).

### `message-tags`

The full IRCv3 message tag extension is negotiated. Tags are parsed as a key-value map with proper IRCv3 escape unescaping. This is the transport layer for `server-time`, `typing`, `batch`, and all other tag-based features.

### `batch`

`BATCH` commands are fully handled. When a batch starts (`BATCH +ref type param`), UplinkIRC buffers all messages tagged with `batch=ref` instead of processing them immediately. When the batch ends (`BATCH -ref`), the buffered messages are delivered as a unit with appropriate handling based on the batch type:

- `chathistory` — messages are delivered as history (dimmed, original timestamps, no unread count)
- `znc.in/batch/playback` — same treatment as chathistory

Safety limits apply: at most **8 batches** may be open simultaneously, and each batch may contain at most **1 000 messages**. A misbehaving or malicious server that opens a batch and never closes it cannot grow client RAM without bound.

### `chathistory`

When `chathistory` is negotiated, UplinkIRC sends `CHATHISTORY LATEST <channel> * 100` after joining each channel, requesting up to 100 recent messages. The server (or bouncer) responds with a `BATCH` of type `chathistory`.

History messages are visually distinct from live messages:
- Displayed at reduced opacity
- Timestamped with their original send time — previous-day messages show `MM/dd hh:mm` so you always know when they were sent
- Not counted as unread, so they do not badge the channel in the sidebar

This works on any server or bouncer that supports `chathistory`, including soju and modern ZNC.

### `sasl` (PLAIN and EXTERNAL)

Authenticates your account during the CAP handshake — before you appear on the network. UplinkIRC holds `CAP END` until the server confirms success (`903`) or failure (`904`/`906`). On failure the connection continues without authentication.

**PLAIN** — Add `sasl_user` and `sasl_password` to a server block. UplinkIRC sends `AUTHENTICATE PLAIN` followed by the base64-encoded `\0user\0password` payload.

**EXTERNAL** — Add `sasl_external = true`, `client_cert`, and `client_key` to a server block. UplinkIRC loads the PEM certificate and key, presents the certificate during the TLS handshake, then sends `AUTHENTICATE EXTERNAL` followed by an empty `+` response. The server derives your identity from the certificate's fingerprint — no password is transmitted. RSA and EC (ECDSA) PEM keys are both supported.

```
Client → AUTHENTICATE EXTERNAL
Server → AUTHENTICATE +
Client → AUTHENTICATE +
Server → 903 :SASL authentication successful
```

### `draft/typing`

When you start typing in the input box, UplinkIRC immediately sends `TAGMSG` with `+typing=active`. It restarts a 5-second inactivity timer on each keypress — if you stop typing for 5 seconds, it sends `+typing=paused`. When you send or clear the input, it sends `+typing=done`.

Incoming typing notifications from other users appear as "nick is typing…" above the input bar and time out automatically. The feature can be toggled from the **Preferences** dialog (☰).

### `labeled-response`

CAP is negotiated. Label matching (tying server responses to specific outgoing commands) is tracked for future use with `echo-message` and request/response flows.

---

## Bouncer capabilities

These capabilities are negotiated only when `bouncer = "znc"` or `bouncer = "soju"` is set in the server block. They are not requested against regular IRC servers.

### `znc.in/playback` — ZNC only

When available, UplinkIRC sends `PRIVMSG *playback :PLAY * 0` immediately after the welcome message. This tells ZNC's playback module to replay all buffered messages since last seen. Replayed messages arrive with `server-time` tags and are rendered with their original timestamps.

### `znc.in/self-message` — ZNC only

When you send a message from another client connected to the same ZNC, this capability causes ZNC to echo that message to UplinkIRC. Echoed messages are displayed correctly as your own outgoing messages rather than appearing as incoming messages from yourself.

### `znc.in/batch` — ZNC only

Enables ZNC's batch extension for grouping related messages. Used in conjunction with `znc.in/playback` to deliver replayed history as a batch.

### `soju.im/bouncer-networks` — soju only

After CAP negotiation, UplinkIRC sends `BOUNCER LISTNETWORKS`. soju responds with a `BOUNCER NETWORK` line for each attached network, showing the network ID, name, and connection state. Each network entry is listed in the server buffer so you can see which networks your soju instance is managing.

### `soju.im/bouncer-networks-notify` — soju only

Negotiated to receive real-time notifications when a network's connection state changes (e.g. a network goes offline or reconnects). Used to keep the network list current without polling.

### `soju.im/read` — soju only

Synchronizes your read position across all clients connected to the same soju instance. When you read messages in UplinkIRC, it sends `MARKREAD <target> timestamp=<iso8601>` to record your position. When another client advances the read marker, soju forwards the updated marker to UplinkIRC.

### `soju.im/no-implicit-names` — soju only

Tells soju not to send a NAMES list automatically on JOIN. This prevents duplicate nick list entries when soju is already managing channel state.

---

## Planned capabilities

### `echo-message`

The server echoes your sent messages back to you as delivery confirmation with an accurate timestamp. Requires `labeled-response` to correctly match echoes to outgoing messages.

### `msgid`

Assigns a unique ID to every message. Required for deduplication when combining bouncer history with live messages, and as a foundation for reactions and message redaction.

### `extended-join`

Includes the joining user's account name in JOIN messages, making it possible to display account names in join notifications without a separate WHOIS.

### `account-notify`

Notifies the client in real time when a user logs in or out of services. Useful for showing account status in the nick list.

### `chghost`

Allows a user's hostname to change without a full disconnect/reconnect cycle. Shown as a status message in shared channels.

### `cap-notify`

Notifies the client if the server gains or loses capabilities after the initial handshake. Lets the client adapt to capability changes dynamically.

### `sts` (Strict Transport Security)

If a server advertises STS, UplinkIRC would remember to always require TLS for that host and refuse plaintext fallback — similar to HSTS in web browsers.

### `draft/reply`

A client-only tag that marks a message as a reply to a specific `msgid`. Enables threaded reply display. Requires `msgid`.

### `draft/message-redaction`

A command that removes a message from history and signals other clients to hide it. Requires `msgid` to identify the target message.

### `draft/multiline`

Allows composing and sending messages that span multiple lines, delivered to clients as a `batch` of type `draft/multiline`. Useful for pastes and structured content.

### `draft/react`

A client tag for sending emoji reactions to a specific message. Reactions are attached to a `msgid`. Requires `msgid`.

### `account-tag`

Attaches the sender's NickServ account name as a `account` tag on every incoming message. Complements `account-notify` by providing per-message account context without a separate WHOIS.

### `Monitor`

A standardized command for watching when specific nicks come online or go offline. Replaces polling via ISON. The server notifies you in real time when a monitored nick connects or disconnects.

### `invite-notify`

When you are an operator or have channel privileges, the server sends you a notification when someone is invited to your channel, rather than only the inviter seeing confirmation.

### `setname`

Allows users to update their realname (GECOS) field after connecting via a `SETNAME` command. Other clients in shared channels receive a notification of the change.

### `WHOX`

An extended version of the `WHO` command. Lets clients request specific fields (account name, idle time, real name, etc.) in a single query using a custom field mask. More efficient than repeated WHOIS lookups.

### `userhost-in-names`

Modifies `NAMES` replies to include `user@host` for each nick — e.g. `@alice!alice@example.com` instead of just `@alice`. Useful for ban matching and ignore lists.

### `netsplit` / `netjoin` batch types

When a netsplit occurs, the server wraps the resulting flood of QUITs in a `BATCH` of type `netsplit`, and the subsequent reconnect JOINs in a `netjoin` batch. Clients can collapse these into a single folded entry instead of showing hundreds of individual lines.

### `standard-replies`

A structured format for servers to send notes, warnings, and errors to clients using `NOTE`, `WARN`, and `FAIL` commands with machine-readable codes. More informative than plain NOTICE messages.

### `UTF8ONLY`

An ISUPPORT token that signals the server only accepts UTF-8 encoded traffic. Allows the client to enforce UTF-8 encoding and warn the user if their input contains non-UTF-8 bytes.

### WebSocket transport

Conventions for carrying IRC lines over a WebSocket connection (`wss://`). Useful for connecting to servers hosted behind web infrastructure, or in environments where raw TCP is blocked.

### User metadata (`metadata-2`)

A framework for associating key-value metadata with users — display names, avatars, pronouns, homepage, color, status. Values are stored server-side and synced to clients. Negotiated via the `metadata-2` capability.

---

## Reference

Full IRCv3 specification and capability registry: [https://ircv3.net/](https://ircv3.net/)
