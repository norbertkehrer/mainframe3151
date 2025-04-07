
#include <stdio.h>
#include <stdbool.h>

#include "mainframe3151.h"

// ************************************************************************
// Macros needed in this file:
#define TURN_ON(var, mask) var |= mask
#define TURN_OFF(var, mask) var &= (~mask)

#define INC_ADDRESS(cursor_addr)         \
    cursor_addr++;                       \
    if (cursor_addr >= TERM_BUFFER_SIZE) \
    cursor_addr = 0

// ************************************************************************
// Global variables:
int cursor_addr = 0;                            // current cursor position (as buffer address)
int final_cursor_row = 0, final_cursor_col = 0; // final cursor position (as buffer address) to be set after all orders are done
bool always_intensified = true;                 // For Norbert's terminal, which is very dim in normal mode

// ************************************************************************
int set_always_intensified(bool value)
{
    always_intensified = value;
};

// ************************************************************************
int calc_buffer_address(int ba1, int ba2)
{
    int addr;
    if ((ba1 & 0xc0) == FOURTEEN_BIT)
    {
        addr = ((ba1 & 0x3f) << 8) | ba2;
    }
    else
    {
        addr = ((ba1 & 0x3f) << 6) | (ba2 & 0x3f);
    };
    return addr % TERM_BUFFER_SIZE; // just to be on the save side!
};

// ************************************************************************
int process_host_data(unsigned char *buffer, int len)
{
    unsigned char char_in, char_out, command, wcc, order, attr, b;
    int pos, ba1, ba2, addr, row, col, pa1, pa2, chr, i, count, type, value, nr_of_pairs;
    int current_char_attr;
    int extended_highlighting_value;
    bool formatted, end_of_order_reached, in_command_mode;
    bool sound_alarm;
    bool graphic_character_set;
    bool cursor_was_inserted;
    bool has_extended_highlighting;
    bool always_intensified_highlighted;

    unsigned char wsf_answer[] = {
        AID_ISF,
        0, 13,                                              // length
        ISF_QUERY_REPLY, ISF_QR_HIGHLIGHT,                  // *** Highlighting: extended is supported
        4,                                                  // number of pairs
        0x00, 0xf0,                                         // default = normal highlight
        0xf1, 0xf1,                                         // normal
        0xf2, 0xf2,                                         // blink
        0xf4, 0xf4,                                         // underscore
        0, 46,                                              // length
        ISF_QUERY_REPLY, ISF_QR_UAREA,                      // *** usable area: 80 x 24
        0x03, 0x00, 0x00, TERM_COLS, 0x00, TERM_ROWS, 0x01, // 2 flags[4,5], w(2b)[6,7], h(2b)[8,9], units[10]
        0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x03,     // xr(4b)[11-14], yr(4b)[15-18] as fractions (1/3, 1/3)
        0x09, 0x10,                                         // aw[19], ah[20]
        (TERM_BUFFER_SIZE >> 8), (TERM_BUFFER_SIZE & 0xff), // buffer size
        0x00, 0x00, 0x00,
        0x00, 0x13, 0x02, 0x00, 0x01, 0x00, TERM_COLS, 0x00,
        TERM_ROWS, 0x01, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01,
        0x00, 0x03, 0x09, 0x10,
        IAC, EOR};

    current_char_attr = 0;
    sound_alarm = false;
    graphic_character_set = false;
    always_intensified_highlighted = false;
    in_command_mode = true;
    pos = 0;
    while (pos < len)
    {
        // *** Commands
        if (in_command_mode)
        {
            formatted = false;
            command = buffer[pos++] & 0xff;
            switch (command)
            {
            case CMD_SNA_EW: // *********** Erase write
            case CMD_SNA_EWA:
                DEBUG_PRINT("SNA_EW\n");
                send_to_terminal(TERM_CMD_CLEAR_ALL);
                cursor_addr = 0;
                final_cursor_row = final_cursor_col = 0;
                cursor_was_inserted = false;
            // Attention: fall through to write
            case CMD_WRITE: // *********** Write
            case CMD_SNA_WRITE:
                DEBUG_PRINT("SNA_WRITE\n");
                wcc = buffer[pos++] & 0xff;
                if (wcc & WCC_ALARM)
                    sound_alarm = true;
                in_command_mode = false; // switch off command mode and switch to to order mode
                current_char_attr = 0;
                break;
            case CMD_RM: // *********** Read modified
            case CMD_SNA_RM:
                DEBUG_PRINT("RM\n"); // TODO!!!!
                send_char_array_to_host((char[]){AID_ENTER, 0x40, 0x40, IAC, EOR}, 5);
                in_command_mode = true; // stay in command mode
                break;
            case CMD_RB: // *********** Read buffer
            case CMD_SNA_RB:
                DEBUG_PRINT("RB\n");
                read_all_and_send_to_host();
                in_command_mode = true; // stay in command mode
                break;
            case CMD_WSF:                                                // *********** Write structured field
                DEBUG_PRINT("WSF\n");                                    // TODO !!!!
                send_char_array_to_host(wsf_answer, sizeof(wsf_answer)); // send (always the same) answer and
                pos = len;                                               // skip the arguments of the WSF command and stop stream interpretation - Hack - TODO!!!
                in_command_mode = true;                                  // stay in command mode
                break;
            case IAC: // *********** IAC
                char_in = buffer[pos++] & 0xff;
                DEBUG_PRINT("IAC %02x\n", char_in);
            default: //*********** Unknown command
                DEBUG_PRINT("Unknown command %02x\n", command);
                break;
            };
        }
        // *** Orders
        else
        {
            // Process the orders
            end_of_order_reached = false;
            while (pos < len && !end_of_order_reached)
            {
                char_in = (buffer[pos++] & 0xff);
                switch (char_in)
                {
                case ORDER_SBA: // *********** Set buffer address
                    ba1 = buffer[pos++] & 0xff;
                    ba2 = buffer[pos++] & 0xff;
                    if (ba2 == IAC)
                        ba2 = buffer[pos++] & 0xff; // if the second byte is IAC, read the next byte (it is usually the combi ff ff, being only one ff)
                    addr = calc_buffer_address(ba1, ba2);
                    row = addr / TERM_COLS;
                    col = addr % TERM_COLS;
                    send_to_terminal(TERM_CMD_SET_BUFFER_ADDRESS, row + 0x20, col + 0x20);
                    cursor_addr = addr;
                    DEBUG_PRINT("SBA %i/%i ... ", row, col);
                    break;
                case ORDER_SF: // *********** Start field
                    attr = buffer[pos++] & 0xff;
                    has_extended_highlighting = false;
                    DEBUG_PRINT("SF %02x ... ", attr);
                    // Attention: fall through to ORDER_SFE!
                case ORDER_SFE: // *********** Start field extended
                case ORDER_MF:  // *********** Modify field (to be checked, if this is really the same as SFE) --- TODO
                    // The following "if" is not redundant because of the fall through from ORDER_SF
                    if (char_in == ORDER_SFE || char_in == ORDER_MF)
                    {
                        attr = 0x60; // default field attribute used in case nothing is specified as a type/value pair later (normal protected field)
                        nr_of_pairs = buffer[pos++] & 0xff;
                        DEBUG_PRINT("SFE %02x ", nr_of_pairs);
                        has_extended_highlighting = false;
                        for (i = 0; i < nr_of_pairs; i++)
                        {
                            type = buffer[pos++] & 0xff;
                            value = buffer[pos++] & 0xff;
                            DEBUG_PRINT("%i: %02x/%02x, ", i, type, value);
                            switch (type)
                            {
                            case EFA_FIELD:
                                attr = value;
                                break;
                            case EFA_EXT_HIGH:
                                has_extended_highlighting = true;
                                extended_highlighting_value = value;
                                break;
                            };
                        };
                        DEBUG_PRINT(" ... ");
                    };
                    pa1 = 0x20; // indicates, that this is not the last parameter
                    if ((attr & ATTR_MDT) && (attr & ATTR_PROT))
                        TURN_OFF(attr, ATTR_MDT); // MDT is not valid for protected fields, so switch MDT off (field skip is not supported by 3151)
                    if ((attr & ATTR_DISPLAY) == ATTR_HP)
                        TURN_ON(current_char_attr, TERM_HIGH_INTENSITY);
                    else
                        TURN_OFF(current_char_attr, TERM_HIGH_INTENSITY);
                    // This is only for Norbert's very dim terminal:
                    if (always_intensified)
                    {
                        TURN_ON(current_char_attr, TERM_HIGH_INTENSITY); // Set everything to is intensified
                        if ((attr & ATTR_DISPLAY) == ATTR_HP)
                            always_intensified_highlighted = true; // Remember, that it is an intensified field for later
                        else
                            always_intensified_highlighted = false; // Remember, that it is a normal field for later
                    };
                    if ((attr & ATTR_DISPLAY) == ATTR_IN) // Invisibility has to be set in "start field". At the "Set Character Attribute" later is not enough.
                    {
                        TURN_ON(current_char_attr, TERM_NON_DISPLAY);
                        TURN_ON(pa1, TERM_NON_DISPLAY);
                    }
                    else
                    {
                        TURN_OFF(current_char_attr, TERM_NON_DISPLAY);
                        TURN_OFF(pa1, TERM_NON_DISPLAY);
                    };
                    pa2 = 0x40; // indicates, that this is the last parameter
                    if ((attr & ATTR_NUMERIC) && !(attr & ATTR_PROT))
                        TURN_ON(pa2, TERM_NUMERIC);
                    if (attr & ATTR_PROT)
                        TURN_ON(pa2, TERM_PROTECTED);
                    if (attr & ATTR_MDT) // modified data tag
                        TURN_ON(pa2, ATTR_MDT);
                    send_to_terminal(TERM_CMD_SET_FIELD_ATTRIBUTE, pa1, pa2); // no extended highlighting etc. here, yet. This is done in ORDER_SA.
                    INC_ADDRESS(cursor_addr);
                    formatted = true;
                    // Attention: fall through to ORDER_SA!
                case ORDER_SA: // *********** Set attribute
                    // The following "if" is not redundant because of the fall through from ORDER_SFE
                    if (char_in == ORDER_SA)
                    {
                        type = buffer[pos++] & 0xff;
                        value = buffer[pos++] & 0xff;
                        if (type == EFA_EXT_HIGH)
                        {
                            has_extended_highlighting = true;
                            extended_highlighting_value = value;
                        }
                        else
                        {
                            has_extended_highlighting = false;
                        };
                        DEBUG_PRINT("SA %02x %02x (pa1=%02x) ... ", type, value, pa1);
                    };
                    if (has_extended_highlighting)
                    {
                        switch (extended_highlighting_value)
                        {
                        case ATTR_DEFAULT:
                        case ATTR_NORMAL:
                            // TODO: check if this is correct, when SFE order is implemented !!!!!!!!!!!!!
                            TURN_OFF(current_char_attr, TERM_BLINK);
                            TURN_OFF(current_char_attr, TERM_UNDERSCORE);
                            TURN_OFF(current_char_attr, TERM_REVERSE);
                            break;
                        case ATTR_BLINK:
                            TURN_ON(current_char_attr, TERM_BLINK);
                            TURN_OFF(current_char_attr, (TERM_UNDERSCORE | TERM_REVERSE)); // It's a pity that the 3270 protocol does not support more than 1 extended highlighting option at the same time
                            break;
                        case ATTR_UNDERSCORE:
                            TURN_ON(current_char_attr, TERM_UNDERSCORE);
                            TURN_OFF(current_char_attr, (TERM_BLINK | TERM_REVERSE)); // It's a pity that the 3270 protocol does not support more than 1 extended highlighting option at the same time
                            break;
                        case ATTR_REVERSE:
                            TURN_ON(current_char_attr, TERM_REVERSE);
                            TURN_OFF(current_char_attr, (TERM_BLINK | TERM_UNDERSCORE)); // It's a pity that the 3270 protocol does not support more than 1 extended highlighting option at the same time
                            break;
                        };
                    };
                    pa1 = 0x40 | current_char_attr; // 0x40 indicates, that this is the last parameter
                    // This is only for Norbert's very dim terminal:
                    if (always_intensified && always_intensified_highlighted)
                        TURN_ON(pa1, TERM_REVERSE); // Intensified fields are shown as reverse (in addition to other extended highlightings)
                    send_to_terminal(TERM_CMD_SET_CHARACTER_ATTRIBUTE, pa1);
                    break;
                case ORDER_IC:                                  // *********** Insert cursor
                    final_cursor_row = cursor_addr / TERM_COLS; // Remember this position as the final cursor position
                    final_cursor_col = cursor_addr % TERM_COLS;
                    cursor_was_inserted = true;
                    DEBUG_PRINT("IC %i/%i ... ", final_cursor_row, final_cursor_col);
                    break;
                case ORDER_GE: // *********** Graphic Escape
                    DEBUG_PRINT("GE ... ");
                    graphic_character_set = true;
                    break;
                case ORDER_RA: // *********** Repeat to address
                    ba1 = buffer[pos++] & 0xff;
                    ba2 = buffer[pos++] & 0xff;
                    if (ba2 == IAC)
                        ba2 = buffer[pos++] & 0xff; // if the second byte is IAC, read the next byte (it is usually the combi ff ff being only one ff)
                    chr = buffer[pos++] & 0xff;
                    addr = calc_buffer_address(ba1, ba2);
                    DEBUG_PRINT("RA %04x, %02x (%i/%i) ... ", addr, chr, (int)(addr / TERM_COLS), addr % TERM_COLS);
                    do
                    {
                        send_byte_to_terminal(ebcdic2ascii(chr));
                        INC_ADDRESS(cursor_addr);
                    } while (cursor_addr != addr);
                    break;
                case ORDER_EUA: // *********** Erase unprotected to address
                                // TODO - this is not correct. I just delete all the unprotected fields until the end of the page (EOP)
                    ba1 = buffer[pos++] & 0xff;
                    ba2 = buffer[pos++] & 0xff;
                    addr = calc_buffer_address(ba1, ba2);
                    send_to_terminal(TERM_CMD_ERASE_END_OF_PAGE);
                    cursor_addr = addr; // Is his correct? TODO!!!
                    break;
                case ORDER_PT: // *********** Program tab
                    send_byte_to_terminal(TERM_HT);
                    break;
                case IAC: // *********** IAC
                    char_in = buffer[pos++] & 0xff;
                    DEBUG_PRINT("IAC %02x\n", char_in);
                    if (char_in == EOR)
                    {
                        in_command_mode = true;
                        end_of_order_reached = true;
                        break;
                    };
                    // else it is IAC + something else (e.g. 0xff) --> treat as normal character and fall through
                    // Attention: Fall through to default!
                default: // *********** Normal characters
                    if (graphic_character_set)
                    {
                        send_to_terminal(TERM_CMD_GRAPHIC_CHARACTER_SET); // for this one character, set graphic character set
                        char_out = ebcdic2asciiSpecial(char_in);          // Convert EBCDIC to ASCII with special character set
                        send_byte_to_terminal(char_out);                  // send the resulting ASCII character
                        send_to_terminal(TERM_CMD_NORMAL_CHARACTER_SET);  // switch back to normal character set again
                        graphic_character_set = false;                    // and turn off graphic character set switch again
                    }
                    else
                    {
                        char_out = ebcdic2ascii(char_in); // Convert EBCDIC to ASCII
                        send_byte_to_terminal(char_out);  // send the ASCII character
                    };
                    INC_ADDRESS(cursor_addr);
                    DEBUG_PRINT("\x1b[41m%c\x1b[0m", char_out);
                    break;
                };
            };
        };
    };

    // Set cursor to the final position (as specified in the last IC order) on the screen (if there was an IC)
    send_to_terminal(TERM_CMD_SET_CURSOR_ADDRESS, final_cursor_row + 0x20, final_cursor_col + 0x20);
    cursor_was_inserted = false;
    DEBUG_PRINT("Set the cursor to row %i, col %i\n", final_cursor_row, final_cursor_col);

    // Sound the audible alarm, if specified in the WCC bit
    if (sound_alarm)
    {
        send_byte_to_terminal(TERM_BEL); // Sound alarm command
        sound_alarm = false;
    };

    return 0;
};