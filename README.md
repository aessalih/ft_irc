## 🚀 Getting Started

### 🖥️ Start the Server

To start the IRC server, open a terminal and compile the source files. Then run the executable with the following arguments:

```bash
./ircserv <port> <serverPassword>
```

<port>: The port number clients will use to connect (e.g., 6667).
<serverPassword>: A custom password chosen by the server owner for authentication.

### 💬 Connect Clients

To connect a client to the server:

1. Open a **new terminal window** (without closing the server).
2. Use the following command:

```bash
nc 127.0.0.1 <port>
```

This uses netcat (nc) to create a connection to the IRC server running locally on 127.0.0.1 (localhost) at the specified <port>.

✅ Congratulations! You're now connected to the IRC server and can interact with other connected clients.

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