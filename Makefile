CC = g++
BUILD_DIR = build
CFLAGS = -std=c++11 -Wall -Werror

ROUTER_EXECUTABLE_FILE = router.out
SYSTEM_EXECUTABLE_FILE = system.out
SIM_EXECUTABLE_FILE = sim.out

all: $(SERVER_EXECUTABLE_FILE) \
	$(CLIENT_EXECUTABLE_FILE) \
	$(SIM_EXECUTABLE_FILE)


InterfaceSensitivityList = \
	interface/Interface.cpp \
	interface/Interface.hpp 

SystemSensitivityList = \
	system/System.cpp \
	system/System.hpp 

RouterSensitivityList = \
	router/Router.cpp \
	router/Router.hpp


$(SYSTEM_EXECUTABLE_FILE): $(SystemSensitivityList)
	$(CC) $(CFLAGS) system/System.cpp -o $(SYSTEM_EXECUTABLE_FILE)

$(ROUTER_EXECUTABLE_FILE): $(RouterSensitivityList)
	$(CC) $(CFLAGS) router/Router.cpp -o $(ROUTER_EXECUTABLE_FILE)

$(SIM_EXECUTABLE_FILE): $(InterfaceSensitivityList) $(ROUTER_EXECUTABLE_FILE) $(SYSTEM_EXECUTABLE_FILE)
	$(CC) $(CFLAGS) interface/Interface.cpp -o $(SIM_EXECUTABLE_FILE)



.PHONY: clean
clean:
	rm *.out
	rm -r fifos/