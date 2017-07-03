CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
USERID=EDIT_MAKE_FILE

# Add all .cpp files that need to be compiled for your server
SERVER_FILES=simple-tcp-server.cpp

# Add all .cpp files that need to be compiled for your client
CLIENT_FILES=simple-tcp-client.cpp

# Add all .cpp files that need to be compiled
CLIENTSR_FILES=client-SR.cpp
SERVERSR_FILES=server-SR.cpp
SERVERRENO_FILES=server-Reno.cpp
SERVERMULTI_FILES=server-multithread.cpp

all: simple-tcp-server simple-tcp-client client-SR server-SR server-Reno server-multithread

*.o: *.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

simple-tcp-server: $(SERVER_FILES:.cpp=.o)
	$(CXX) -o $@ $(CXXFLAGS) $(SERVER_FILES:.cpp=.o)

simple-tcp-client: $(CLIENT_FILES:.cpp=.o)
	$(CXX) -o $@ $(CXXFLAGS) $(CLIENT_FILES:.cpp=.o)

client-SR: $(CLIENTSR_FILES:.cpp=.o)
	$(CXX) -o $@ $(CXXFLAGS) $(CLIENTSR_FILES:.cpp=.o)
server-SR: $(SERVERSR_FILES:.cpp=.o)
	$(CXX) -o $@ $(CXXFLAGS) $(SERVERSR_FILES:.cpp=.o)
server-Reno: $(SERVERRENO_FILES:.cpp=.o)
	$(CXX) -o $@ $(CXXFLAGS) $(SERVERRENO_FILES:.cpp=.o)
server-multithread: $(SERVERMULTI_FILES:.cpp=.o)
	$(CXX) -o $@ $(CXXFLAGS) $(SERVERMULTI_FILES:.cpp=.o)


clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM simple-tcp-server simple-tcp-client client-SR server-SR server-Reno server-multithread *.tar.gz

tarball: clean
	tar -czvf $(904590764-404776769-404592789).tar.gz *
