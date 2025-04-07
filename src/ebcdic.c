
#include "mainframe3151.h"

unsigned char e2a[256] = {
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // 0
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // 1
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // 2
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // 3
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '.', '<', '(', '+', '|',  // 4
    '&', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '!', '$', '*', ')', ';', '^',  // 5
    '-', '/', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|', ',', '%', '_', '>', '?',  // 6
    ' ', '^', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '`', ':', '#', '@', '\'', '=', '"', // 7
    ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', ' ', ' ', ' ', ' ', ' ', ' ',  // 8
    ' ', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', ' ', ' ', ' ', ' ', ' ', ' ',  // 9
    ' ', '~', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', ' ', ' ', ' ', '[', ' ', ' ',  // a
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ']', ' ', ' ',  // b
    '{', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', ' ', ' ', ' ', ' ', ' ', ' ',  // c
    '}', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', ' ', ' ', ' ', ' ', ' ', ' ',  // d
    '\\', ' ', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ', ' ', ' ', ' ', ' ', ' ', // e
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ' ', ' ', ' ', ' ', ' ', ' '   // f
};
// Note: 0x4a is a "cent" symbol on the mainframe, 0x5f is a "not" symbol on the mainframe - the 3151 does not have them :-(
//       So, I used "^" for the "not" and "|" for the "cent" like many 3270 emulators do.

unsigned char e2aSpecial[256] = {
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',               // 0
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',               // 1
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',               // 2
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',               // 3
    ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', ' ', ' ', ' ', ' ', ' ', ' ',               // 4
    ' ', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', ' ', ' ', ' ', ' ', ' ', ' ',               // 5
    ' ', ' ', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ', ' ', ' ', ' ', ' ', ' ',               // 6
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, ' ',          // 7
    ' ', ' ', ' ', ' ', ' ', 0x78, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',              // 8
    ' ', 0x6f, 0x70, 0x73, 0x72, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',           // 9
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',               // a
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',               // b
    ' ', ' ', ' ', 0x7e, 0x6d, 0x6c, 0x74, 0x76, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',          // c
    ' ', ' ', 0x71, 0x6e, 0x6a, 0x6b, 0x75, 0x77, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',         // d
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, ' ', ' ', 0x3c, 0x3d, 0x3e, 0x3f, // e
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, ' ', ' ', ' ', ' ', ' ', ' '      // f
};
// Note: The letters above are underlined letters from the APL character set, but the 3151 does neither have them nor other APL characters
//       For the underlined letters I took normal uppercase letters.
//       The hex codes are the special characters available on the 3151 (like characters for drawing lines). I placed them as good as possible
//       to correspond to their EBCDIC equivalents.

unsigned char a2e[256];

unsigned char ebcdic2ascii(unsigned char c) { return e2a[c]; };

unsigned char ebcdic2asciiSpecial(unsigned char c) { return e2aSpecial[c]; };

unsigned char ascii2ebcdic(unsigned char c) { return a2e[c]; };

void init_ascii_ebcdic_conversion()
{
    int i;
    for (i = 0; i < 256; i++)
    {
        a2e[e2a[i]] = i;
    };
    a2e[(int)' '] = EBCDIC_SP;
};
