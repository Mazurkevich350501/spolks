all: server

server: main.o CommandParser.o SocketCommands.o MyClient.o MyServer.o ClientCommandsExecuting.o ServerCommandsExecuting.o
	g++ main.o CommandParser.o SocketCommands.o MyClient.o MyServer.o ClientCommandsExecuting.o ServerCommandsExecuting.o -o server

main.o: main.cpp
	g++ -c main.cpp

CommandParser.o: CommandParser.cpp
	g++ -c CommandParser.cpp

SocketCommands.o: SocketCommands.cpp
	g++ -c SocketCommands.cpp

MyClient.o: MyClient.cpp
	g++ -c MyClient.cpp

MyServer.o: MyServer.cpp
	g++ -c MyServer.cpp

ClientCommandsExecuting.o: ClientCommandsExecuting.cpp
	g++ -c ClientCommandsExecuting.cpp

ServerCommandsExecuting.o: ServerCommandsExecuting.cpp
	g++ -c ServerCommandsExecuting.cpp

clean:
	rm -rf *.o server
