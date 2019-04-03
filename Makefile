server:
	gcc server.c open_listenfd.c queue.c -lpthread -o server
executeServer:
	./server 12345 words.txt
client:
	gcc client.c -o client
executeClient:
	./client 12345
clean:
	rm server & rm log.txt