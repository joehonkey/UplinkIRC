# IRCv3 Support

UplinkIRC aims to be a first-class IRCv3 client. Below is the current status of every negotiated capability.

---

## Negotiated capabilities

UplinkIRC uses **CAP LS 302** — it waits for the server's full capability list before deciding what to request, and only requests capabilities the server actually advertises. This avoids rejection failures from batching caps the server doesn't support.

### `multi-prefix`

**Status: Active**

Returns all mode prefixes for a nick in NAMES replies (e.g. `@+alice` instead of just `@alice`). Lets the nick list correctly display users who hold more than one status (e.g. both op and voice).

### `away-notify`

**Status: Active**

When a user in a shared channel sets or clears their away status, the server sends an AWAY message in real time. UplinkIRC receives and tracks these notifications. Visual display in the nick list is planned.

### `server-time`

**Status: Active**

When the server attaches a `time` tag to messages, UplinkIRC uses that timestamp instead of the local clock. This is important for accurate timestamps when replaying history through a bouncer — messages show the time they were originally sent, not the time your client received them.

### `message-tags`

**Status: Active**

The full IRCv3 message tag extension is negotiated and parsed. Tags appear on inbound messages and are stored with each message object. This is the foundation that `typing`, `msgid`, `server-time`, and other tag-based features build on.

### `sasl`

**Status: Active (PLAIN)**

Authenticates your account during the CAP handshake — before you appear on the network. Add `sasl_user` and `sasl_password` to any `[[server]]` block in your config. UplinkIRC sends `AUTHENTICATE PLAIN` and holds `CAP END` until the server confirms success (903) or failure (904/906). EXTERNAL (certificate-based) auth is planned.

### `draft/typing`

**Status: Active**

When you start typing in the input box, UplinkIRC sends a `TAGMSG` with `+typing=active` to the channel. When you stop or send, it sends `done`. Incoming typing notifications from other users appear as "nick is typing…" above the input bar. Can be toggled from the hamburger menu.

### `batch`

**Status: Negotiated (processing planned)**

CAP is negotiated and the `BATCH` command is parsed. Batch grouping (handling the contained messages as a unit) is not yet applied — batched messages are processed individually. Required for `chathistory` replay.

### `labeled-response`

**Status: Negotiated (matching planned)**

CAP is negotiated. Label matching (tying server responses back to specific outgoing commands) is not yet implemented. Needed for reliable `echo-message` and request/response flows.

---

## Planned capabilities

### `chathistory`

Replays message history from a bouncer on channel join. Depends on `batch` processing being complete. Planned medium-term.

### `echo-message`

The server echoes your sent messages back to you as confirmation of delivery. Enables accurate sent-message timestamps. Requires `labeled-response` to be useful.

### `msgid`

Assigns a unique ID to every message. Required for deduplication, threading, reactions, and message redaction.

### `extended-join`

Includes the joining user's account name in JOIN messages. Makes it possible to show account names in join notifications without needing a separate WHOIS.

### `account-notify`

Notifies the client when a user logs in or out of services. Useful for showing account status in the nick list.

### `chghost`

Allows a user's hostname to change without disconnecting and reconnecting. Shown as a status message in shared channels.

### `cap-notify`

Notifies the client if the server gains or loses capabilities after the initial handshake. Lets the client react to capability changes dynamically.

### `sts` (Strict Transport Security)

If a server advertises STS, UplinkIRC would remember to always use TLS for that server and refuse plaintext connections. Prevents downgrade attacks (similar to HSTS in web browsers).

---

## Reference

Full IRCv3 specification and capability registry: [https://ircv3.net/](https://ircv3.net/)
