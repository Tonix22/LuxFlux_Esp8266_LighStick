
#generate .o 

#gcc -I Common -c Server/client_simulator.c -o Server/client_simulator.o
#gcc -c Common/FileWriter.c -o Common/FileWriter.o
#link .o
#gcc Server/client_simulator.o Common/FileWriter.o -o myApp

ServerTest:
	cd Server;make
	cd Common;make
	gcc Server/client_simulator.o Common/FileWriter.o -o ServerTest
#
ClientTest:
	cd Client;make
	cd Common;make
	gcc Client/server_simulator.o Common/FileReader.o -o ClientTest