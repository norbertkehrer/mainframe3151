
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <winsock2.h>

#include "mainframe3151.h"

int timeout_to_use = 50;

DWORD accessdirection = GENERIC_READ | GENERIC_WRITE;
DWORD dwBytesRead, dwBytesWritten;
HANDLE hSerial;

char buffer[BUFFER_SIZE];

// ************************************************************************
int open_serial_port(char *portname)
{
	hSerial = CreateFile(portname,
						 accessdirection,
						 0,
						 0,
						 OPEN_EXISTING,
						 0,
						 0);

	if (hSerial == INVALID_HANDLE_VALUE)
	{ // handle error
		// call GetLastError(); to gain more information
		printf("Error in establishing serial connection '%s' (createfile failed).\n", portname);
		return -1;
	};

	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams))
	{ // handle error
		// could not get the state of the comport
		printf("Error in establishing serial connection '%s' (getcommstate failed).\n", portname);
		return -1;
	};

	dcbSerialParams.BaudRate = TERM_BAUD_RATE;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(hSerial, &dcbSerialParams))
	{ // handle error
		printf("Error in establishing serial connection '%s' (setcommstate failed).\n", portname);
		return -1;
	};

	COMMTIMEOUTS timeouts = {0};
	timeouts.ReadIntervalTimeout = timeout_to_use;
	timeouts.ReadTotalTimeoutConstant = timeout_to_use;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = timeout_to_use;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	// use this in order to have read operations return immediately:
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;

	if (!SetCommTimeouts(hSerial, &timeouts))
	{ // handle error
		printf("Error in establishing serial connection '%s' (timeout setting failed).\n", portname);
		return -1;
	};

	return 0;
};

// ************************************************************************
void close_serial_port()
{
	CloseHandle(hSerial);
};

// ************************************************************************
DWORD readFromSerialPort(HANDLE hSerial, char *buffer, int buffersize)
{
	DWORD dwBytesRead = 0;
	int r;

	if (!ReadFile(hSerial, buffer, buffersize, &dwBytesRead, NULL))
	{ // handle error
		printf("Error reading from serial port.\n");
	};
	// r = CancelIo(hSerial);
	return dwBytesRead;
};

// ************************************************************************
DWORD writeToSerialPort(HANDLE hSerial, char *data, int length)
{
	DWORD dwBytesWritten = 0;
	int r;

	if (!WriteFile(hSerial, data, length, &dwBytesWritten, NULL))
	{ // handle error
		printf("Error writing to serial port.\n");
	};
	// r = CancelIo(hSerial);
	if (dwBytesWritten != length)
	{
		printf("Error writing to serial port (not all bytes written). Needed %i, done %i\n", length, dwBytesWritten);
	};
	return dwBytesWritten;
};

// ************************************************************************
int read_byte_from_terminal()
{
	DWORD n;

	n = readFromSerialPort(hSerial, buffer, 1); // read 1 char from serial port

	if (n == 1)
		return (buffer[0] & 0xff); // read OK --> return first character of buffer
	else
		return -1; // if not --> return -1
};

// ************************************************************************
void send_byte_to_terminal(int x)
{
	DWORD n;

	buffer[0] = (char)(x & 0xff);
	n = writeToSerialPort(hSerial, buffer, 1); // write 1 char to serial port
};

// ************************************************************************
void send_to_terminal(int num, ...)
{
	va_list valist;
	int i;
	DWORD n;

	va_start(valist, num); // initialize valist for num number of arguments
	for (i = 0; i < num; i++)
	{
		buffer[i] = va_arg(valist, int) & 0xff;
	};
	va_end(valist); // clean memory reserved for valist

	n = writeToSerialPort(hSerial, buffer, num); // write num chars to serial port
};

// ************************************************************************
void send_string_to_terminal(char *str)
{
	DWORD n;
	n = writeToSerialPort(hSerial, str, strlen(str)); // write string to serial port
};

// ************************************************************************
int await_byte_from_terminal()
{
	int r;
	r = -1;
	while (r < 0)
	{
		r = read_byte_from_terminal();
	};
	return r;
};

// ************************************************************************
int read_bytes_from_terminal(char *buffer, int buffersize)
{
	return readFromSerialPort(hSerial, buffer, buffersize); 
};
