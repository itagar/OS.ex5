CXX= g++
CXXFLAGS= -c -Wall -std=c++11 -DNDEBUG
CODEFILES= ex5.tar whatsappServer.cpp whatsappClient.cpp WhatsApp.h Makefile README


# Default
default: whatsappServer whatsappClient


# Executables
whatsappServer: whatsappServer.o
	$(CXX) whatsappServer.o -o whatsappServer
	-rm -f *.o

whatsappClient: whatsappClient.o
	$(CXX) whatsappClient.o -o whatsappClient
	-rm -f *.o


# Object Files
whatsappServer.o: WhatsApp.h whatsappServer.cpp
	$(CXX) $(CXXFLAGS) whatsappServer.cpp -o whatsappServer.o

whatsappClient.o: WhatsApp.h whatsappClient.cpp
	$(CXX) $(CXXFLAGS) whatsappClient.cpp -o whatsappClient.o


# tar
tar:
	tar -cvf $(CODEFILES)


# Other Targets
clean:
	-rm -vf *.o *.tar whatsappServer whatsappClient


# Valgrind
Valgrind: whatsappServer.cpp WhatsApp.h
	$(CXX) -g -Wall -std=c++11  -o Valgrind
	valgrind --leak-check=full --show-possibly-lost=yes --show-reachable=yes --undef-value-errors=yes ./Valgrind
	rm -rvf *.o Valgrind
