#define WINVER 0x0501  /* Windows Version = Windows XP, definition needed for some reason to be able to use getaddrinfo */
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "mainframe3151.h"

#define SEND_ARRAY(a) send_char_array_to_host(a, sizeof(a))

SOCKET host_socket;

// ****************************************************************************
void send_char_array_to_host(char *arr, int len)
{
	int i;
	i = send(host_socket, arr, len, 0);
	if (i < 0)
	{
		printf("Send failed. RC = %i\n", i);
		return;
	}
	DEBUG_PRINT("Data sent.\n");
};

// ****************************************************************************
void receive_array_and_forget_it()
{
	char server_reply[BUFFER_SIZE];
	int i, recv_size;

	// Receive a reply from the server
	if ((recv_size = recv(host_socket, server_reply, BUFFER_SIZE, 0)) == SOCKET_ERROR)
	{
		printf("Error: Receiving from host failed");
	};

	DEBUG_PRINT("Received from host: %i bytes\n", recv_size);
	for (i = 0; i < recv_size; i++)
	{
		DEBUG_PRINT("%02x%c ", server_reply[i] & 0xff, ebcdic2ascii(server_reply[i] & 0xff));
	};
	DEBUG_PRINT("\n");
};

// ****************************************************************************
int receive_from_host(char *arr, int size)
{
	int i, recv_size;

	// Receive a reply from the server
	if ((recv_size = recv(host_socket, arr, size, 0)) == SOCKET_ERROR)
	{
		printf("recv failed");
		return -1;
	}

	DEBUG_PRINT("Received from host: %i bytes\n", recv_size);
	for (i = 0; i < recv_size; i++)
	{
		DEBUG_PRINT("%02x%c ", arr[i] & 0xff, ebcdic2ascii(arr[i] & 0xff));
	};
	DEBUG_PRINT("\n");

	return recv_size;
};

// ****************************************************************************
int host_data_is_available()
{
	int result;
	fd_set readfds;
	struct timeval tv;

	FD_ZERO(&readfds);
	FD_SET(host_socket, &readfds);

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	result = select(0, &readfds, NULL, NULL, &tv);
	if (result == SOCKET_ERROR)
	{
		printf("Error: select host failed");
		return 0;
	};
	if (result > 0)
		return 1;
	else
		return 0;
};

// ****************************************************************************
int connect_to_host(char *host_name, char *port)
{
	WSADATA wsa;
	struct sockaddr_in server;

	struct addrinfo *address_info = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;
	DWORD dwRetval = 0;

	int i;
	int iResult;

	char negotiate_ttype[] = {
		IAC, WILL, TELOPT_TTYPE,
		IAC, SB, TELOPT_TTYPE, TELQUAL_IS, 'I', 'B', 'M', '-', '3', '2', '7', '8', '-', '2', '-', 'E', IAC, SE // IBM-3278-2-E
	};

	char negotiate_eor[] = {
		IAC, WILL, TELOPT_EOR,
		IAC, DO, TELOPT_EOR,
		IAC, WILL, TELOPT_BINARY,
		IAC, DO, TELOPT_BINARY //
	};

	DEBUG_PRINT("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Error: Winsock initialization failed with error Code : %d", WSAGetLastError());
		return 1;
	};

	// Setup the hints address info structure which is passed to the getaddrinfo() function
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Get the address info from the server name + port number
	dwRetval = getaddrinfo(host_name, port, &hints, &address_info);
	if (dwRetval != 0)
	{
		printf("Error: getaddrinfo failed with error code: %d\n", dwRetval);
		WSACleanup();
		return 1;
	};

	// Attempt to connect to an address until one succeeds
	for (ptr = address_info; ptr != NULL; ptr = ptr->ai_next)
	{
		// Create a socket for connecting to server
		host_socket = socket(ptr->ai_family, ptr->ai_socktype,
							 ptr->ai_protocol);
		if (host_socket == INVALID_SOCKET)
		{
			printf("Error: socket creation failed with error code: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(host_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(host_socket);
			host_socket = INVALID_SOCKET;
			continue;
		}
		break;
	};
	freeaddrinfo(address_info);
	if (host_socket == INVALID_SOCKET)
	{
		printf("Error: Unable to connect to server.\n");
		WSACleanup();
		return 1;
	};
	DEBUG_PRINT("Connected.\n");

	// Negotiation of the protocol between the host and the terminal (a hack, TODO: do it in a correct way)
	SEND_ARRAY(negotiate_ttype);   // Inform the host about the terminal type (IBM 3278-E)
	receive_array_and_forget_it(); // Get and ignore the answer from the host
	SEND_ARRAY(negotiate_eor);	   // Inform the host about the EOR handling and that it is a binary stream (answer from host will be in the first real data package later)

	return 0;
};

// ****************************************************************************
void disconnect_from_host()
{
	closesocket(host_socket);
	WSACleanup();
};
