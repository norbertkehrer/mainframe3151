// ****************************************************************************
// *   IBM Mainframe to IBM 3151 Interface
// *   Written by Norbert Kehrer in 2025.
// ****************************************************************************

#include <stdio.h>
#include <windows.h>
#include <stdbool.h>

#include "mainframe3151.h"

unsigned char host_buffer[BUFFER_SIZE];
unsigned char *host_buffer_ptr = host_buffer;
unsigned char buffer_from_terminal[BUFFER_SIZE];
unsigned char *term_buffer_ptr = buffer_from_terminal;

// ****************************************************************************
void show_usage()
{
	printf("Usage: mainframe3151 <serial port> <server> <port> [ -i ]\n\n");
	printf("  <serial port> .... COM port name (e.g. COM1, COM2, ...) for connection to the\n");
	printf("                     3151 terminal at 19,200 baud\n");
	printf("  <server> ......... Server name of mainframe for telnet connection (e.g.\n");
	printf("                     localhost, big.iron.com, ...)\n");
	printf("  <port> ........... Telnet port number (e.g. 3270, 23, ...)\n");
	printf("  -i ............... Option that can be added at the end to display everything\n");
	printf("                     at high intensity (for very dim terminals)\n\n");
	printf("  Example: mainframe3151 COM3 localhost 3270\n\n");
};

// ****************************************************************************
void send_goodbye_message_to_terminal()
{
	send_to_terminal(TERM_CMD_CLEAR_ALL);
	send_to_terminal(TERM_CMD_GRAPHIC_CHARACTER_SET);
	send_string_to_terminal("                  rrrrrrrr  rrrrrrrrrr    rrrrrr       rrrrrr\r");
	send_string_to_terminal("                  rrrrrrrr  rrrrrrrrrrrr  rrrrrrr     rrrrrrr\r");
	send_string_to_terminal("                    rrrr     rrrr   rrrr   rrrrrrr   rrrrrrr\r");
	send_string_to_terminal("                    rrrr     rrrrrrrrr     rrrrrrrr rrrrrrrr\r");
	send_string_to_terminal("                    rrrr     rrrrrrrrr     rrrr rrrrrrr rrrr\r");
	send_string_to_terminal("                    rrrr     rrrr   rrrr   rrrr  rrrrr  rrrr\r");
	send_string_to_terminal("                  rrrrrrrr  rrrrrrrrrrrr  rrrrr   rrr   rrrrr\r");
	send_string_to_terminal("                  rrrrrrrr  rrrrrrrrrr    rrrrr    r    rrrrr\r\r");
	send_to_terminal(TERM_CMD_SET_CHARACTER_ATTRIBUTE, (0x40 | TERM_HIGH_INTENSITY)); // Switch to high intensiy
	send_to_terminal(TERM_CMD_GRAPHIC_CHARACTER_SET);
	send_string_to_terminal("                    }qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqz\r");
	send_string_to_terminal("                    x"); // Vertical bar
	send_to_terminal(TERM_CMD_NORMAL_CHARACTER_SET);
	send_string_to_terminal(" IBM Mainframe to IBM 3151 Interface ");
	send_to_terminal(TERM_CMD_GRAPHIC_CHARACTER_SET);
	send_string_to_terminal("x\r"); // Vertical bar
	send_string_to_terminal("                    tqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqu\r");
	send_string_to_terminal("                    x"); // Vertical bar
	send_to_terminal(TERM_CMD_NORMAL_CHARACTER_SET);
	send_string_to_terminal("  Written by Norbert Kehrer in 2025  ");
	send_to_terminal(TERM_CMD_GRAPHIC_CHARACTER_SET);
	send_string_to_terminal("x\r");					  // Vertical bar
	send_string_to_terminal("                    x"); // Vertical bar
	send_to_terminal(TERM_CMD_NORMAL_CHARACTER_SET);
	send_string_to_terminal("   https://norbertkehrer.github.io   ");
	send_to_terminal(TERM_CMD_GRAPHIC_CHARACTER_SET);
	send_string_to_terminal("x\r"); // Vertical bar
	send_string_to_terminal("                    |qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq{\r");
	send_to_terminal(TERM_CMD_NORMAL_CHARACTER_SET);
	send_string_to_terminal("\r                          Terminal was disconnected.\r");
	send_string_to_terminal("\r                                  Good bye!\r\r                                       ");
	send_to_terminal(TERM_CMD_SET_CHARACTER_ATTRIBUTE, 0x40); // Switch back to normal intensiy again
};

// ****************************************************************************
int main(int argc, char *argv[])
{
	int result, i, bytes_received_from_host, nr_bytes, bytes_received_from_terminal;
	unsigned char char_in, char_out, c;

	// Hello message
	printf("\n");
	printf("+-------------------------------------+\n");
	printf("| IBM Mainframe to IBM 3151 Interface |\n");
	printf("+-------------------------------------+\n");
	printf("|  Written by Norbert Kehrer in 2025  |\n");
	printf("|   https://norbertkehrer.github.io   |\n");
	printf("+-------------------------------------+\n\n");

	// If not at least 3 arguments, print error message and show usage of the program
	if (argc < 4)
	{
		printf("Error: Missing parameters.\n\n");
		show_usage();
		return 1;
	};

	// Connect to 3151 terminal
	result = open_serial_port(argv[1]);
	if (result != 0)
	{
		printf("Error: Could not open serial port to 3151 terminal\n\n");
		return 1;
	};
	printf("Connected to terminal at serial port %s\n\n", argv[1]);

	// Connect to mainframe
	result = connect_to_host(argv[2], argv[3]);
	if (result != 0)
	{
		printf("Error: Could not connect to mainframe at address %s:%s\n\n", argv[2], argv[3]);
		return 1;
	};
	printf("Connected to host at address %s:%s\n\n", argv[2], argv[3]);
	printf("To disconnect press Ctrl + Shift + F12 on the terminal.\n");
	printf("But before that, please log off on the mainframe!\n\n");

	// Initialize EBCDIC conversion table and the terminal
	init_ascii_ebcdic_conversion();
	setup_terminal();

	// Check, if option for "always intensified" was specified in the command line
	set_always_intensified(false); // used for very dim terminals (like Norbert's), default is "off"
	if (argc >= 5 && argv[4][0] == '-' &&
		(argv[4][1] == 'i' || argv[4][1] == 'I')) // if option "-i" or "-I" is specified as last parameter, set it on
	{
		set_always_intensified(true);
	};

	// Wait until the host has sent something
	while (!host_data_is_available())
	{
	};

	// Wait for the first data block from the host and process it
	bytes_received_from_host = receive_from_host(host_buffer, BUFFER_SIZE);
	if (bytes_received_from_host > 0)
	{
		// Skip everything until $f5 (SNA_EW), which is the first byte of stream from Hercules --- This is a HACK - correct it ! TODO
		for (i = 0; i < bytes_received_from_host; i++)
		{
			if ((host_buffer[i] & 0xff) == CMD_SNA_EW)
				break;
		};
		process_host_data(host_buffer + i, bytes_received_from_host - i);
	};
	// Now process all the further data coming from the host and from the terminal
	while (true)
	{
		// Wait a little in order to not use so much CPU time
		Sleep(50);
		// Check for output from host and process it, if there is any
		if (host_data_is_available())
		{
			send_to_terminal(TERM_CMD_KEYBOARD_LOCK);
			bytes_received_from_host = receive_from_host(host_buffer, BUFFER_SIZE);
			if (bytes_received_from_host > 0)
			{
				// Continue reading until the IAC+EOR is received
				while ((host_buffer[bytes_received_from_host - 2] != IAC || host_buffer[bytes_received_from_host - 1] != EOR) && bytes_received_from_host < BUFFER_SIZE)
				{
					host_buffer_ptr = host_buffer + bytes_received_from_host; // set pointer for host reads
					nr_bytes = receive_from_host(host_buffer_ptr, BUFFER_SIZE - bytes_received_from_host);
					if (nr_bytes > 0)
					{
						bytes_received_from_host += nr_bytes;
					};
				};
				process_host_data(host_buffer, bytes_received_from_host);
			};
			send_to_terminal(TERM_CMD_KEYBOARD_UNLOCK);
		};

		// Check for input from terminal and process it, if any
		bytes_received_from_terminal = read_bytes_from_terminal(buffer_from_terminal, BUFFER_SIZE);
		if (bytes_received_from_terminal > 0)
		{
			// Continue reading until the LTA is received
			while (buffer_from_terminal[bytes_received_from_terminal - 1] != TERM_LTA && bytes_received_from_terminal < BUFFER_SIZE)
			{
				term_buffer_ptr = buffer_from_terminal + bytes_received_from_terminal; // set pointer for terminal reads
				nr_bytes = read_bytes_from_terminal(term_buffer_ptr, BUFFER_SIZE - bytes_received_from_terminal);
				if (nr_bytes > 0)
				{
					bytes_received_from_terminal += nr_bytes;
				};
			};
			bytes_received_from_terminal--; // remove the LTA
			DEBUG_PRINT("Received from 3151: %i bytes\n", bytes_received_from_terminal);
			for (i = 0; i < bytes_received_from_terminal; i++)
			{
				c = (buffer_from_terminal[i] & 0xff) < 0x20 ? '.' : buffer_from_terminal[i] & 0xff;
				DEBUG_PRINT("%02x%c ", buffer_from_terminal[i] & 0xff, c);
			};
			DEBUG_PRINT("\n");
			result = process_terminal_input(buffer_from_terminal, bytes_received_from_terminal);
			if (result != 0)
			{
				break;
			};
		};
	};

	// Send good-bye message to terminal
	send_goodbye_message_to_terminal();

	// Disconnect from host and terminal
	close_serial_port();
	disconnect_from_host();
	printf("Disconnected from host and from terminal.\n\n");

	return 0;
};
