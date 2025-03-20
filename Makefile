# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -lrt

# Executable names
SOCKET1_EXEC = socket1
SOCKET2_EXEC = socket2

# Source files
SRC_SOCKET1 = socket1.c route_dump.c rsvp_db.c rsvp_msg.c timer_event.c avl_tree.c
SRC_SOCKET2 = socket2.c route_dump.c rsvp_db.c rsvp_msg.c timer_event.c avl_tree.c

# Object files (if you want to create them)
OBJ_SOCKET1 = $(SRC_SOCKET1:.c=.o)
OBJ_SOCKET2 = $(SRC_SOCKET2:.c=.o)

# Header files
HEADERS = rsvp_db.h rsvp_msg.h socket.h timer_event.h

# Default target
all: $(SOCKET1_EXEC) $(SOCKET2_EXEC)

# Rule to build socket1
$(SOCKET1_EXEC): $(SRC_SOCKET1) $(HEADERS)
	$(CC) $(CFLAGS) -o $(SOCKET1_EXEC) $(SRC_SOCKET1)

# Rule to build socket2
$(SOCKET2_EXEC): $(SRC_SOCKET2) $(HEADERS)
	$(CC) $(CFLAGS) -o $(SOCKET2_EXEC) $(SRC_SOCKET2)

# Optional: Rule to create object files (useful for larger projects)
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(SOCKET1_EXEC) $(SOCKET2_EXEC) $(OBJ_SOCKET1) $(OBJ_SOCKET2)
