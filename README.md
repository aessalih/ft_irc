## 🚀 Getting Started

### 🖥️ Start the Server

To start the IRC server, open a terminal and compile the source files. Then run the executable with the following arguments:

```bash
./ircserv <port> <serverPassword>
```

<port>: The port number clients will use to connect (e.g., 6667).
<serverPassword>: A custom password chosen by the server owner for authentication.

---

### 💬 Connect Clients

To connect a client to the server:

1. Open a **new terminal window** (without closing the server).
2. Use the following command:

```bash
nc 127.0.0.1 <port>
```

This uses netcat (nc) to create a connection to the IRC server running locally on 127.0.0.1 (localhost) at the specified <port>.

✅ Congratulations! You're now connected to the IRC server and can interact with other connected clients.

---

### 🔐 Authentication

After connecting, you'll receive a welcome message prompting you to authenticate. Follow this command sequence:

```bash
pass <serverPassword>
nick <nickname>
user <username>
```

pass: Must match the server password set when starting the server.
nick: Sets your IRC nickname.
user: Sets your username.

**Once authenticated, you're ready to join channels, send messages, and enjoy the IRC experience.**

---

### 📜 Available Commands

Below is a list of supported commands for interacting with the IRC server.  
Replace `#channel`, `user`, or other placeholders with actual names or values.

#### 🔗 JOIN

**Join a specified channel.**

- **Syntax**:
```bash
JOIN #channel
```

- **Example**:
```bash
JOIN #sabona
```

- **Description**:
Connects you to the specified channel or create it if it doesn't exist, allowing you to participate in discussions.

#### 💬 PRIVMSG

**Send and receive private messages to a channel or a specific user.**

- **Syntax**:
```bash
PRIVMSG #channel|user :message
```

- **Example**:
To a channel:
```bash
PRIVMSG #sabona :Hello, everyone!
```
To a user:
```bash
PRIVMSG alice :Hi, how are you?
```

- **Description**:
Sends a message to a channel or directly to a user.

#### 🦵 KICK

**Eject a user from a channel (requires operator privileges).**

- **Syntax**:
```bash
KICK #channel user
```

- **Example**:
```bash
KICK #sabona bob
```

- **Description**:
Removes the specified user from the channel.

#### ✉️ INVITE

**Invite a user to a channel.**

- **Syntax**:
```bash
INVITE user #channel
```

- **Example**:
```bash
INVITE alice #sabona
```

- **Description**:
Sends an invitation to the specified user to join the channel.

#### ✉️ TOPIC

**View or change the channel's topic.**

- **Syntax**:
To view:
```bash
TOPIC #channel
```
To change:
```bash
TOPIC #channel :NEW_TOPIC
```

- **Example**:
To view:
```bash
TOPIC #sabona
```
To change:
```bash
TOPIC #sabona :Welcome to our community!
```

- **Description**:
Displays the current topic or updates it (may require operator privileges).

#### ✉️ MODE

**Change a channel’s mode to configure settings (requires operator privileges).**

- **Syntax**:
```bash
MODE #channel [+-mode] [parameter]
```

- **Supported Modes**:

```bash
+i / -i: Enable/disable invite-only mode

+t / -t: Only ops can change the topic

+k / -k: Set/remove channel password

+o / -o: Grant/remove operator privileges

+l / -l: Set/remove user limit
```

- **Examples**:
Set invite-only:
```bash
MODE #sabona +i
```
Remove password:
```bash
MODE #sabona -k
```
Grant operator status:
```bash
MODE #sabona +o alice
```

- **Description**:
Modify channel settings to control access, permissions, or channel behavior.