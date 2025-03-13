1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

_The client knows when a command's output is fully received by looking for a special end-of-transmission character sent by the server, in this case, `RDSH_EOF_CHAR` (0x04). Since TCP streams data in chunks, the client might receive incomplete messages. To handle this, it keeps reading with `recv()` until the end marker is found. Techniques to manage partial reads include buffering the data, checking each chunk for the EOF character, and combining received fragments until the full response is assembled._

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

_A shell protocol should set clear boundaries for messages since TCP doesn’t do this automatically. One way to do this is by sending a null terminator (`\0`) at the end of each command and using `RDSH_EOF_CHAR` to mark the end of responses. If this isn't handled properly, the client might receive broken or merged messages, leading to incorrect command execution. The server might also get stuck waiting for more data, or it could execute incomplete commands, causing unexpected errors._

3. Describe the general differences between stateful and stateless protocols.

_A stateful protocol remembers past interactions, keeping track of session details between messages. Examples include TCP, which keeps a connection open for reliable communication, and HTTPS sessions that use cookies. A stateless protocol treats each request separately, without remembering previous exchanges. Examples include UDP and basic HTTP requests. Stateful protocols ensure continuity and reliability but require more resources, while stateless ones are faster and easier to scale but need extra handling if reliability is required._

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

_UDP is called "unreliable" because it doesn’t guarantee message delivery, order, or error checking. However, it’s much faster and is ideal for real-time applications where losing a few packets doesn’t matter much, like video calls, live streaming, and online games. These applications often have their own ways of handling lost data, so they don’t need the overhead that TCP’s reliability mechanisms add._

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

_The OS provides the **sockets API**, which lets applications send and receive data over networks. Sockets work like file handles but for network communication, allowing programs to connect, send, and receive messages. The API includes functions like `socket()`, `bind()`, `listen()`, `accept()`, `connect()`, `send()`, and `recv()`, making it easier to work with both TCP and UDP without dealing with low-level networking details._
