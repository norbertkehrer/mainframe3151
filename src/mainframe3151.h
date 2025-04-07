
#include <stdbool.h>

// ***************************************************
// Definitions for the TELNET protocol.
// ***************************************************

/* Telnet commands */
#define IAC 255   /* ff: interpret as command */
#define DONT 254  /* fe: you are not to use option */
#define DO 253    /* fd: please, you use option */
#define WONT 252  /* fc: I won't use option */
#define WILL 251  /* fb: I will use option */
#define SB 250    /* fa: interpret as subnegotiation */
#define GA 249    /* f9: you may reverse the line */
#define EL 248    /* f8: erase the current line */
#define EC 247    /* f7: erase the current character */
#define AYT 246   /* f6: are you there */
#define AO 245    /* f5: abort output--but let prog finish */
#define IP 244    /* f4: interrupt process--permanently */
#define BREAK 243 /* f3: break */
#define DM 242    /* f2: data mark--for connect. cleaning */
#define NOP 241   /* f1: nop */
#define SE 240    /* f0: end sub negotiation */
#define EOR 239   /* ef: mark end of record */
#define PA1 230   /* e6: transparently send PA1 AID */
#define SYNCH 242 /* f2: for telfunc calls */

/* Telnet options */
#define TELOPT_BINARY 0 /* 8-bit data path */
#define TELQUAL_IS 0
#define TELOPT_ECHO 1      /* echo */
#define TELOPT_RCP 2       /* prepare to reconnect */
#define TELOPT_SGA 3       /* suppress go ahead */
#define TELOPT_NAMS 4      /* approximate message size */
#define TELOPT_STATUS 5    /* give status */
#define TELOPT_TM 6        /* timing mark */
#define TELOPT_RCTE 7      /* remote controlled transmission and echo */
#define TELOPT_NAOL 8      /* negotiate about output line width */
#define TELOPT_NAOP 9      /* negotiate about output page size */
#define TELOPT_NAOCRD 10   /* negotiate about CR disposition */
#define TELOPT_NAOHTS 11   /* negotiate about horizontal tabstops */
#define TELOPT_NAOHTD 12   /* negotiate about horizontal tab disposition */
#define TELOPT_NAOFFD 13   /* negotiate about formfeed disposition */
#define TELOPT_NAOVTS 14   /* negotiate about vertical tab stops */
#define TELOPT_NAOVTD 15   /* negotiate about vertical tab disposition */
#define TELOPT_NAOLFD 16   /* negotiate about output LF disposition */
#define TELOPT_XASCII 17   /* extended ascic character set */
#define TELOPT_LOGOUT 18   /* force logout */
#define TELOPT_BM 19       /* byte macro */
#define TELOPT_DET 20      /* data entry terminal */
#define TELOPT_SUPDUP 21   /* supdup protocol */
#define TELOPT_TERMTYPE 24 /* negotiate terminal type */
#define TELOPT_EOR 25      /* mark end of record */
#define TELOPT_EXOPL 255   /* extended-options-list */
#define TELOPT_OUTMRK 27   // 0x19
#define TELOPT_TTYPE 24    // 0x18
#define TELOPT_TN3270E 40  // 0x28
#define TELOPT_START_TLS 46

// ****************************************************************************
// Buffer sizes
// ****************************************************************************

#define BUFFER_SIZE 0x10000

// ****************************************************************************
// 3270 stream
// ****************************************************************************

/* Special Characters */
#define EBCDIC_NUL 0x00 /* Null Character */
#define EBCDIC_SOH 0x01 /* Start Of Header */
#define EBCDIC_STX 0x02 /* Start Of Text */
#define EBCDIC_GE 0x08  /* Graphics Escape */
#define EBCDIC_DUP 0x1c /* DUP code */
#define EBCDIC_FM 0x1e  /* Field Mark code */
#define EBCDIC_SP 0x40  /* Space */

/* Command Codes (3272 & 3274-1B/D version) */
#define CMD_EAU 0x0f   /* Erase All Unprotected */
#define CMD_EW 0x05    /* Erase/Write */
#define CMD_EWA 0x0d   /* Erase/Write Alternate */
#define CMD_RB 0x02    /* Read Buffer */
#define CMD_RM 0x06    /* Read Modified */
#define CMD_WRITE 0x01 /* Write */
#define CMD_NOOP 0x03  /* No Operation */
#define CMD_SEL 0x0b   /* Select */
#define CMD_SENSE 0x04 /* Sense */
#define CMD_SID 0xe4   /* Sense ID */

#define CMD_SNA_EAU 0x6f   /* SNA Erase All Unprotected */
#define CMD_SNA_EW 0xf5    /* SNA Erase/Write */
#define CMD_SNA_EWA 0x7e   /* SNA Erase/Write Alternate */
#define CMD_SNA_RB 0xf2    /* SNA Read Buffer */
#define CMD_SNA_RM 0xf6    /* SNA Read Modified */
#define CMD_SNA_RMA 0x6e   /* SNA Read Modified All */
#define CMD_SNA_WRITE 0xf1 /* SNA Write */

#define CMD_WSF 0xf3   /* Write Structured Field */
#define CMD_MAGIC 0x11 /* handled in MacTn3270 like CMD_WSF, couldnt find this in manual (hints?) */

/* WCC bits*/
#define WCC_RESET 0x40         /* Reset Bit */
#define WCC_FORMAT 0x30        /* Printout Format */
#define WCC_START_PRINTER 0x08 /* Start Printer */
#define WCC_SOUND_ALARM 0x04   /* Sound Alarm */
#define WCC_RESTORE_KBD 0x02   /* Restore Keyboard */
#define WCC_RESET_MDT 0x01     /* Reset Modified Data Tags */

/* Orders in a Write stream */
#define ORDER_SF 0x1d  /* Start Field */
#define ORDER_SBA 0x11 /* Set Buffer Address */
#define ORDER_IC 0x13  /* Insert Cursor */
#define ORDER_PT 0x05  /* Program Tab */
#define ORDER_RA 0x3c  /* Repeat to Address */
#define ORDER_EUA 0x12 /* Erase Unprotected to Address */
#define ORDER_SA 0x28  /* Set Attribute */
#define ORDER_GE 0x08  /* Graphic Escape */
#define ORDER_SFE 0x29 /* Start Field Extended */
#define ORDER_MF 0x2c  /* Modify Field */

/* IDs for Outbound Structured Fields */
#define OSF_ACTIVATE_PARTITION 0x0e
#define OSF_BEGIN_END_FILE 0x0f85
#define OSF_CREATE_PARTITION 0x0c
#define OSF_DESTROY_PARTITION 0x0d
#define OSF_ERASE_RESET 0x03
#define OSF_LOAD_COLOR_TABLE 0x0f05
#define OSF_LOAD_FORMAT_STORAGE 0x0f24
#define OSF_LOAD_LINE_TYPE 0x0f07
#define OSF_LOAD_PROG_SYMBOL 0x06
#define OSF_MOIFIY_PARTITION 0x0f0a
#define OSF_OUTBOUND_TEXT_HEADER 0x0f71
#define OSF_OUTBOUND_TYPE_1_TEXT 0x0fc1
#define OSF_OUTBOUND_3270DS 0x40
#define OSF_PRESENT_ABS_FMT 0x4b
#define OSF_PRESENT_REL_FMT 0x4c
#define OSF_READ_PARTITION 0x01
#define OSF_RP_TYPE_QUERY 0x02 /* type fields for read partition sf */
#define OSF_RP_TYPE_QUERY_LIST 0x03
#define OSF_RP_TYPE_RMA 0x6e
#define OSF_RP_TYPE_RB 0xf2
#define OSF_RP_TYPE_RM 0xf6

#define OSF_READ_PARTITION_Q_LST 0x0103
#define OSF_READ_PARTITION_RMA 0x016e
#define OSF_READ_PARTITION_RB 0x01f2
#define OSF_READ_PARTITION_RM 0x01f6

#define OSF_REQUEST_RECOV_DATA 0x1030
#define OSF_RESET_PARTITION 0x00
#define OSF_RESTART 0x1033
#define OSF_SCS_DATA 0x41
#define OSF_SELECT_COLOR_TABLE 0x0f04
#define OSF_SELECT_FORMAT_GROUP 0x4a
#define OSF_SET_CHKPT_INTVL 0x1032
#define OSF_SET_MSR_CONTROL 0x0f01
#define OSF_SET_PARTITION_CHR 0x0f08
#define OSF_SET_PRINTER_CHR 0x0f84
#define OSF_SET_REPLY_MODE 0x09
#define OSF_SET_WINDOW_ORIGIN 0x0b

/* Types for Extended Field Attribute */
#define EFA_RESET 0x00
#define EFA_FIELD 0xc0
#define EFA_EXT_HIGH 0x41
#define EFA_COLOR 0x42
#define EFA_P_SYM 0x43
#define EFA_BG_TRANSP 0x46

/* Buffer address modes */
#define FOURTEEN_BIT 0x00 /* 14-bit buffer address mode */
#define TWELVE_BIT 0x40   /* 12-bit buffer address mode */
#define ADDR_PART 0x3f    /* address part of the byte */

/* Attention Identificators */
#define AID_NONE 0x60   /* No AID generated (display) */
#define AID_ENTER 0x7d  /* ENTER key */
#define AID_PF1 0xf1    /* PF1 key */
#define AID_PF2 0xf2    /* PF2 key */
#define AID_PF3 0xf3    /* PF3 key */
#define AID_PF4 0xf4    /* PF4 key */
#define AID_PF5 0xf5    /* PF5 key */
#define AID_PF6 0xf6    /* PF6 key */
#define AID_PF7 0xf7    /* PF7 key */
#define AID_PF8 0xf8    /* PF8 key */
#define AID_PF9 0xf9    /* PF9 key */
#define AID_PF10 0x7a   /* PF10 key */
#define AID_PF11 0x7b   /* PF11 key */
#define AID_PF12 0x7c   /* PF12 key */
#define AID_PF13 0xc1   /* PF13 key */
#define AID_PF14 0xc2   /* PF14 key */
#define AID_PF15 0xc3   /* PF15 key */
#define AID_PF16 0xc4   /* PF16 key */
#define AID_PF17 0xc5   /* PF17 key */
#define AID_PF18 0xc6   /* PF18 key */
#define AID_PF19 0xc7   /* PF19 key */
#define AID_PF20 0xc8   /* PF20 key */
#define AID_PF21 0xc9   /* PF21 key */
#define AID_PF22 0x4a   /* PF22 key */
#define AID_PF23 0x4b   /* PF23 key */
#define AID_PF24 0x4c   /* PF24 key */
#define AID_SELECT 0x7e /* Selector light pen */
#define AID_PA1 0x6c    /* PA1 key */
#define AID_PA2 0x6e    /* PA2 (CNCL) key */
#define AID_PA3 0x6b    /* PA3 key */
#define AID_CLEAR 0x6d  /* CLEAR key */
#define AID_SYSREQ 0xf0 /* SYS REQ key */

#define AID_ISF 0x88 /* Inbound SF reply */

/* IDs for Inbound Structured Fields, Byte 2 */
#define ISF_INBOUND_SF 0x80
#define ISF_QUERY_REPLY 0x81

/* IDs for Inbound Structured Fields Query Reply, Byte 3 */
#define ISF_QR_SUMMARY 0x80
#define ISF_QR_UAREA 0x81 /* 129 decimal */
#define ISF_QR_ALPHA_PART 0x84
#define ISF_QR_CHARSETS 0x85 /* 133 decimal */
#define ISF_QR_COLOR 0x86    /* 134 decimal */
#define ISF_QR_HIGHLIGHT 0x87
#define ISF_QR_REPLY_MODE 0x88
#define ISF_QR_OEM_AUX_DEV 0x8f
#define ISF_QR_STORAGE_POOLS 0x96
#define ISF_QR_AUX_DEV 0x99
#define ISF_QR_RPQNAMES 0xa1      /* ??????????? */
#define ISF_QR_IMPLICIT_PART 0xa6 /* 166 decimal */
#define ISF_QR_BACKGRND_TRANSP 0xa8
#define ISF_QR_SEGMENT 0xb0
#define ISF_QR_PROCEDURE 0xb1
#define ISF_QR_LINE_TYPE 0xb2
#define ISF_QR_PORT 0xb3 /* 179 decimal */
#define ISF_QR_GRAPHIC_COLOR 0xb4
#define ISF_QR_GRAPHIC_SYM_SET 0xb6 /* 182 decimal */
#define ISF_QR_NULL 0xff

/* Fields of a Write Control Character (WCC) */
#define WCC_RESET 0x40       /* Reset Bit */
#define WCC_FORMAT 0x30      /* Printout Format */
#define WCC_132 0x00         /* 132-column lines */
#define WCC_40 0x10          /* 40-column lines */
#define WCC_64 0x20          /* 64-column lines */
#define WCC_80 0x30          /* 80-column lines */
#define WCC_START 0x08       /* Start Printer */
#define WCC_ALARM 0x04       /* Sound Alarm */
#define WCC_RESTORE_KBD 0x02 /* Restore Keyboard */
#define WCC_RMDT 0x01        /* Reset Modified Data Tags */

/* Types for Extended Field Attribute */
#define EFA_RESET 0x00
#define EFA_FIELD 0xc0
#define EFA_EXT_HIGH 0x41
#define EFA_COLOR 0x42
#define EFA_P_SYM 0x43
#define EFA_BG_TRANSP 0x46

/* Fields of an Attribute Byte */
#define ATTR_TOP 0xc0        /* top bits of the attribute */
#define ATTR_PROT 0x20       /* protected field */
#define ATTR_NUMERIC 0x10    /* numeric field */
#define ATTR_DISPLAY 0x0c    /* display attributes */
#define ATTR_LN 0x00         /* low intensity / no pen detect */
#define ATTR_LP 0x04         /* low intensity / pen detect */
#define ATTR_HP 0x08         /* high intensity / pen detect */
#define ATTR_IN 0x0c         /* invisible / no pen detect */
#define ATTR_MDT 0x01        /* Modified Data Tag */
#define ATTR_DEFAULT 0x00    /* Default */
#define ATTR_NORMAL 0xF0     /* Normal */
#define ATTR_BLINK 0xF1      /* Blink */
#define ATTR_REVERSE 0xF2    /* Reverse video */
#define ATTR_UNDERSCORE 0xF4 /* Underscore */

// ****************************************************************************
// 3151 stream
// ****************************************************************************

#define TERM_LTA 0x03      // Line Turnaround character - terminates messages from the terminal
#define TERM_BEL 0x07      // Bell
#define TERM_HT 0x09       // Horizontal tab
#define TERM_SO 0x0e       // SO
#define TERM_SI 0x0f       // SI
#define TERM_DC1 0x11      // XON
#define TERM_DC3 0x13      // XOFF
#define TERM_ESC 0x1b      // Escape
#define TERM_MOD 0x21      // Modifier for Shift + F-key
#define TERM_MOD_CTRL 0x22 // Modifier for Ctrl + Shift + F-key
#define TERM_SEND 0x38     // Send key
#define TERM_CLEAR 0x4c    // Clear
#define TERM_SBA 0x58      // Set buffer address
#define TERM_SFA 0x33      // Set field attribute
#define TERM_F1 0x61       // F1
#define TERM_F2 0x62       // F2
#define TERM_F3 0x63       // F3
#define TERM_F4 0x64       // F4
#define TERM_F5 0x65       // F5
#define TERM_F6 0x66       // F6
#define TERM_F7 0x67       // F7
#define TERM_F8 0x68       // F8
#define TERM_F9 0x69       // F9
#define TERM_F10 0x6a      // F10
#define TERM_F11 0x6b      // F11
#define TERM_F12 0x6c      // F12
#define TERM_PA1 0x6d      // PA1
#define TERM_PA2 0x6e      // PA2
#define TERM_PA3 0x6f      // PA3

/* Field/Character Attribute masks */
#define TERM_NON_DISPLAY 0x10
#define TERM_HIGH_INTENSITY 0x08
#define TERM_BLINK 0x04
#define TERM_UNDERSCORE 0x02
#define TERM_REVERSE 0x01
#define TERM_NUMERIC 0x08
#define TERM_PROTECTED 0x02
#define TERM_MDT 0x01

/* Commands: the first arguments for the function "send_to_terminal" with the first argument being the number of bytes to follow */
#define TERM_CMD_SET_CONTROL_4 4, TERM_ESC, '#', '9'
#define TERM_CMD_SET_CONTROL_5 7, TERM_ESC, '$', '9'
#define TERM_CMD_READ_CURSOR_ADDRESS 2, TERM_ESC, '5'
#define TERM_CMD_READ_PAGE 2, TERM_ESC, '8'
#define TERM_CMD_READ_ALL 3, TERM_ESC, '#', '8'
#define TERM_CMD_CLEAR_ALL 3, TERM_ESC, '!', 'L'
#define TERM_CMD_SET_BUFFER_ADDRESS 4, TERM_ESC, 'X'
#define TERM_CMD_SET_CURSOR_ADDRESS 4, TERM_ESC, 'Y'
#define TERM_CMD_SET_FIELD_ATTRIBUTE 4, TERM_ESC, '3'
#define TERM_CMD_SET_CHARACTER_ATTRIBUTE 3, TERM_ESC, '4'
#define TERM_CMD_ERASE_END_OF_PAGE 2, TERM_ESC, 'J'
#define TERM_CMD_GRAPHIC_CHARACTER_SET 6, TERM_ESC, '>', 'A', TERM_ESC, '<', 'A'
#define TERM_CMD_NORMAL_CHARACTER_SET 6, TERM_ESC, '>', '@', TERM_ESC, '<', '@'
#define TERM_CMD_KEYBOARD_LOCK 2, TERM_ESC, ':'
#define TERM_CMD_KEYBOARD_UNLOCK 2, TERM_ESC, ';'

// ****************************************************************************
// IBM 3151 terminal
// ****************************************************************************

#define TERM_BAUD_RATE CBR_19200
#define TERM_ROWS 24
#define TERM_COLS 80
#define TERM_BUFFER_SIZE (TERM_ROWS * TERM_COLS)

// ****************************************************************************
// Debugging
// ****************************************************************************

#define DEBUG 0
#define DEBUG_PRINT(...)         \
    do                           \
    {                            \
        if (DEBUG)               \
            printf(__VA_ARGS__); \
    } while (0)

// ****************************************************************************
// Function prototypes
// ****************************************************************************

/* ebcdic */
unsigned char ebcdic2ascii(unsigned char c);
unsigned char ebcdic2asciiSpecial(unsigned char c);
unsigned char ascii2ebcdic(unsigned char c);
void init_ascii_ebcdic_conversion();

/* comport*/
int open_serial_port(char *portname);
void close_serial_port();
void send_byte_to_terminal(int x);
void send_to_terminal(int num, ...);
void send_string_to_terminal(char *str);
int await_byte_from_terminal();

/* telnet */
int receive_from_host(char *arr, int size);
void send_char_array_to_host(char *arr, int len);
int host_data_is_available();
int connect_to_host(char *host_name, char *port);
void disconnect_from_host();

/* terminal */
void setup_terminal();
void read_all_and_send_to_host();
int process_terminal_input(unsigned char *buffer, int len);
int read_bytes_from_terminal(char *buffer, int buffersize);

/* host */
int set_always_intensified(bool value);
int process_host_data(unsigned char *buffer, int len);
