
#include <stdio.h>
#include <stdbool.h>

#include "mainframe3151.h"

#define GET_BYTE_FROM_TERMINAL() (buffer[pos++] & 0xff)
#define PUT_BYTE_TO_HOST(b) buffer_to_host[host_buffer_pos++] = b

#define TURN_ON(var, mask) var |= mask
#define TURN_OFF(var, mask) var &= (~mask)

unsigned char aid_for_pfkey[] = {
    AID_PF1,
    AID_PF2,
    AID_PF3,
    AID_PF4,
    AID_PF5,
    AID_PF6,
    AID_PF7,
    AID_PF8,
    AID_PF9,
    AID_PF10,
    AID_PF11,
    AID_PF12,
    AID_PF13,
    AID_PF14,
    AID_PF15,
    AID_PF16,
    AID_PF17,
    AID_PF18,
    AID_PF19,
    AID_PF20,
    AID_PF21,
    AID_PF22,
    AID_PF23,
    AID_PF24};

unsigned char addr_map[] = {
    0x40, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
    0xd8, 0xd9, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
    0xe8, 0xe9, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f};

unsigned char last_aid = AID_ENTER;

// ************************************************************************
void print_array(char *text, unsigned char *buffer, int len)
{
    int i;
    DEBUG_PRINT("********** %s, %i bytes:\n", text, len);
    for (i = 0; i < len; i++)
    {
        DEBUG_PRINT("%02x ", buffer[i] & 0xff);
    };
    DEBUG_PRINT("\n");
};

// ************************************************************************
void setup_terminal()
{
    send_to_terminal(TERM_CMD_SET_CONTROL_4, 0x45);                   // Set: Null suppress + AID LTA
    send_to_terminal(TERM_CMD_SET_CONTROL_5, 0x2f, 0x38, 0x31, 0x5c); // Set: 19,200 bps, 8 bit, no parity, PRTS, break 500 ms, Turnaround char = ETX(0x03)
};

// ************************************************************************
void read_all_and_send_to_host()
{
    unsigned char buffer_to_host[BUFFER_SIZE];
    int host_buffer_pos;

    unsigned char answer_from_terminal[BUFFER_SIZE];
    int len_answer_from_terminal;

    unsigned char b, attr;
    int row, col, addr, ba1, ba2, pa1, pa2;

    host_buffer_pos = 0;
    // 1. Send the last AID
    PUT_BYTE_TO_HOST(last_aid);
    DEBUG_PRINT("AID %02x\n", last_aid);
    // 2. Request the cursor position and send it
    send_to_terminal(TERM_CMD_READ_CURSOR_ADDRESS);
    b = await_byte_from_terminal() & 0xff; // ESC
    b = await_byte_from_terminal() & 0xff; // Y
    b = await_byte_from_terminal() & 0xff; // row
    row = (b & 0xff) - 0x20;
    b = await_byte_from_terminal() & 0xff; // col
    col = (b & 0xff) - 0x20;
    b = await_byte_from_terminal() & 0xff; // LTA
    DEBUG_PRINT("Cursor at row %i, col %i\n", row, col);
    addr = row * TERM_COLS + col; // Build the buffer address from the row and column received as answer
    ba1 = addr_map[(addr >> 6) & 0x3f];
    ba2 = addr_map[addr & 0x3f];
    PUT_BYTE_TO_HOST(ba1);
    PUT_BYTE_TO_HOST(ba2);
    // 3. Request the complete screen conents and send it
    send_to_terminal(TERM_CMD_READ_ALL);
    DEBUG_PRINT("Response from terminal:\n");
    while (host_buffer_pos < BUFFER_SIZE)
    {
        b = await_byte_from_terminal() & 0xff;
        DEBUG_PRINT("%02x%c ", b, (b < 0x20 ? '.' : b));
        if (b == TERM_LTA) // stop reading from terminal, when LTA received
            break;
        if (b == TERM_ESC) // if ESC received, read the next byte
        {
            b = await_byte_from_terminal() & 0xff;
            switch (b)
            {
            case TERM_SFA:
                pa1 = await_byte_from_terminal() & 0xff;
                pa2 = await_byte_from_terminal() & 0xff;
                DEBUG_PRINT("(%02x/%02x) ", pa1, pa2);
                attr = 0x00;
                if (pa1 & TERM_HIGH_INTENSITY)
                    TURN_ON(attr, ATTR_HP);
                if (pa2 & TERM_PROTECTED)
                    TURN_ON(attr, ATTR_PROT);
                // The "Read All" command does not return the MDT bit, so we have to assume it is on for unprotected fields
                if (!(pa2 & TERM_PROTECTED))
                    TURN_ON(attr, ATTR_MDT);
                PUT_BYTE_TO_HOST(ORDER_SF);
                PUT_BYTE_TO_HOST(addr_map[attr & 0x3f]);
                break;
            case TERM_SBA:
                row = await_byte_from_terminal() & 0xff - 0x20;
                col = await_byte_from_terminal() & 0xff - 0x20;
                addr = row * TERM_COLS + col;
                ba1 = addr_map[(addr >> 6) & 0x3f];
                ba2 = addr_map[addr & 0x3f];
                PUT_BYTE_TO_HOST(ORDER_SBA);
                PUT_BYTE_TO_HOST(ba1);
                PUT_BYTE_TO_HOST(ba2);
                break;
            default:
                break;
            };
            continue; // Done with Escape, go on with reading
        };
        PUT_BYTE_TO_HOST(ascii2ebcdic(b)); // if normal character, just send it to host in EBCDIC
    };
    DEBUG_PRINT("\n");
    PUT_BYTE_TO_HOST(IAC);
    PUT_BYTE_TO_HOST(EOR);
    print_array("To be sent to host", buffer_to_host, host_buffer_pos);
    send_char_array_to_host(buffer_to_host, host_buffer_pos);
};

// ************************************************************************
int process_terminal_input(unsigned char *buffer, int len)
{
    int pos, i, row, col, addr, ba1, ba2, b;
    bool is_short_answer;

    unsigned char chr, aid;

    unsigned char buffer_to_host[BUFFER_SIZE];
    int host_buffer_pos;

    unsigned char answer_from_terminal[BUFFER_SIZE];
    int len_answer_from_terminal;

    host_buffer_pos = 0;
    pos = 0;
    // Read until you find the ESC character
    while (pos < len && (GET_BYTE_FROM_TERMINAL() != TERM_ESC))
    {
    };
    // If ESC not found, return
    if (pos >= len)
    {
        return 0;
    };

    is_short_answer = false;
    aid = AID_ENTER;
    chr = GET_BYTE_FROM_TERMINAL();
    switch (chr)
    {
    case TERM_SEND:
        aid = AID_ENTER;
        break;
    case TERM_CLEAR:
        aid = AID_CLEAR;
        is_short_answer = true;
        break;
    case TERM_F1: // Function keys
    case TERM_F2:
    case TERM_F3:
    case TERM_F4:
    case TERM_F5:
    case TERM_F6:
    case TERM_F7:
    case TERM_F8:
    case TERM_F9:
    case TERM_F10:
    case TERM_F11:
    case TERM_F12:
        aid = aid_for_pfkey[chr - TERM_F1];
        break;
    case TERM_MOD: // Modifier for F-keys
        chr = GET_BYTE_FROM_TERMINAL();
        switch (chr)
        {
        case TERM_F1: // Function keys
        case TERM_F2:
        case TERM_F3:
        case TERM_F4:
        case TERM_F5:
        case TERM_F6:
        case TERM_F7:
        case TERM_F8:
        case TERM_F9:
        case TERM_F10:
        case TERM_F11:
        case TERM_F12:
            aid = aid_for_pfkey[12 + chr - TERM_F1];
            break;
        case TERM_PA1:
            aid = AID_PA1;
            is_short_answer = true;
            break;
        case TERM_PA2:
            aid = AID_PA2;
            is_short_answer = true;
            break;
        case TERM_PA3:
            aid = AID_PA3;
            is_short_answer = true;
            break;
        default:
            DEBUG_PRINT("Unknown key with modifier %02x\n", chr);
            break;
        };
        break;
    case TERM_MOD_CTRL: // Modifier Ctrl + Shift for F-keys
        chr = GET_BYTE_FROM_TERMINAL();
        switch (chr)
        {
        case TERM_F1: // Function keys F1-F11 with Ctrl/Shift are mapped to PF13-PF23 (like with Shift only)
        case TERM_F2:
        case TERM_F3:
        case TERM_F4:
        case TERM_F5:
        case TERM_F6:
        case TERM_F7:
        case TERM_F8:
        case TERM_F9:
        case TERM_F10:
        case TERM_F11:
            aid = aid_for_pfkey[12 + chr - TERM_F1];
            break;
        case TERM_F12: // Ctrl/Shift F12 --> Disconnect
            return -1; // Return with error code
            break;
        };
        break;
    default:
        DEBUG_PRINT("Unknown key %02x\n", chr);
        break;
    };
    PUT_BYTE_TO_HOST(aid);
    last_aid = aid;
    if (!is_short_answer) // Skip this, if it is a short answer (CLEAR od PAx key)
    {
        // 1. Request the cursor position
        send_to_terminal(TERM_CMD_READ_CURSOR_ADDRESS);
        b = await_byte_from_terminal() & 0xff; // ESC
        b = await_byte_from_terminal() & 0xff; // Y
        b = await_byte_from_terminal() & 0xff; // row
        row = (b & 0xff) - 0x20;
        b = await_byte_from_terminal() & 0xff; // col
        col = (b & 0xff) - 0x20;
        b = await_byte_from_terminal() & 0xff; // LTA
        DEBUG_PRINT("Cursor at row %i, col %i\n", row, col);
        addr = row * TERM_COLS + col; // Build the buffer address from the row and column received as answer
        ba1 = addr_map[(addr >> 6) & 0x3f];
        ba2 = addr_map[addr & 0x3f];
        PUT_BYTE_TO_HOST(ba1);
        PUT_BYTE_TO_HOST(ba2);
        // 2. Request the modified fields
        send_to_terminal(TERM_CMD_READ_PAGE);
        DEBUG_PRINT("Fields from terminal:\n");
        while (host_buffer_pos < BUFFER_SIZE)
        {
            b = await_byte_from_terminal() & 0xff;
            if (b == TERM_LTA) // stop reading from terminal, when LTA received
                break;
            DEBUG_PRINT("%02x%c ", b, (b < 0x20 ? '.' : b));
            if (b == TERM_ESC) // if ESC received, read the next byte
            {
                b = await_byte_from_terminal() & 0xff;
                if (b == TERM_SBA) // if it is an SBA:
                {
                    row = await_byte_from_terminal() & 0xff - 0x20;
                    col = await_byte_from_terminal() & 0xff - 0x20;
                    addr = row * TERM_COLS + col;
                    ba1 = addr_map[(addr >> 6) & 0x3f];
                    ba2 = addr_map[addr & 0x3f];
                    PUT_BYTE_TO_HOST(ORDER_SBA);
                    PUT_BYTE_TO_HOST(ba1);
                    PUT_BYTE_TO_HOST(ba2);
                };
                continue; // Done with Escape, go on with reading
            };
            PUT_BYTE_TO_HOST(ascii2ebcdic(b)); // if normal character, just send it to host in EBCDIC
        };
        DEBUG_PRINT("\n");
    };
    PUT_BYTE_TO_HOST(IAC);
    PUT_BYTE_TO_HOST(EOR);
    print_array("To be sent to host", buffer_to_host, host_buffer_pos);
    send_char_array_to_host(buffer_to_host, host_buffer_pos);
    return 0; // Return without error
};
