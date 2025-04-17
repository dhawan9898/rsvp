# Document Makefile
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -lrt -pthread

# Executable names
SOCKET1_EXEC = rsvp

# Source files
SRC_SOCKET1 = rsvp_main.c rsvpd.c rsvp_sh.c route_dump.c rsvp_db.c rsvp_msg.c timer_event.c log.c label_mgt.c

# Object files (if you want to create them)
OBJ_SOCKET1 = $(SRC_SOCKET1:.c=.o)

# Header files
HEADERS = rsvp_db.h rsvp_msg.h socket.h timer-event.h log.h rsvp_sh.h

# Default target
all: $(SOCKET1_EXEC)

# Rule to build socket1
$(SOCKET1_EXEC): $(SRC_SOCKET1) $(HEADERS)
	$(CC) $(CFLAGS) -o $(SOCKET1_EXEC) $(SRC_SOCKET1)

# Optional: Rule to create object files (useful for larger projects)
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(SOCKET1_EXEC) $(OBJ_SOCKET1) $(RSVP_SHELL)

