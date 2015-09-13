# c_sockets
The client sends via socket channel a file name to the server.
The server creates the file if doesn't exist, reads the file and send the contents to the client.
When changes occur on the file, the server sends the contents again to the server, monitorizing it until socket close.
