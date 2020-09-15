# Winsock2-Server-Multi_Client-Chat
This project is free to use for anyone and is written as console application for windows.

# The way it works
The way this the **server** and **client** work is very simple. When the server is online, clients connect to the server using the IP address and port of the server. Then the clients can chat with each other using the basic input **keyboard**. Remember that neither client nor the server encrypt the messages between the clients.

# How to use it
In order to use both server and client, you should run the server and type the port and IP address for the server (if you want your own network IP address for the server then just write **0.0.0.0** for the server ip address in the arguments). Now the server is online. In order for clients to chat with other clients, the **client** program should run and the port and IP address of the server should be specified as arguments. The client program asks for the name of client and then the clients can chat with each other.
Clients can be on the same computer or different.
