#include "eci.h"


int get_eci_mode(unsigned int value) {
    switch(value) {
        case 0:
        case 2: return Cp437;
        case 1:
        case 3: return ISO8859_1;
        case 4: return ISO8859_2;
        case 5: return ISO8859_3;
        case 6: return ISO8859_4;
        case 7: return ISO8859_5;
        case 8: return ISO8859_6;
        case 9: return ISO8859_7;
        case 10: return ISO8859_8;
        case 11: return ISO8859_9;
        case 12: return ISO8859_10;
        case 13: return ISO8859_11;
        case 14: return ISO8859_12;
        case 15: return ISO8859_13;
        case 16: return ISO8859_14;
        case 17: return ISO8859_15;
        case 18: return ISO8859_16;
        case 20: return SJIS;
        case 21: return Cp1250;
        case 22: return Cp1251;
        case 23: return Cp1252;
        case 24: return Cp1256;
        case 25: return UnicodeBigUnmarked;
        case 26: return UTF8;
        case 27:
        case 170: return ASCII;
        case 28: return Big5;
        case 29: return GB18030;
        case 30: return EUC_KR;
        default: return -1;
    }
}
