// 'C' source line config statements

#include <xc.h>
#define _XTAL_FREQ 4000000

#include <stdio.h>

// 4-bit mode
// PORTC bits 0-3 are connected to the high 4 bits of LCD bus
// PORTC bit 4 = RS = LCD data/instruction select 0=commands, 1=data
// PORTC bit 5 = Enable signal
// note read/write = always low, we will not read
#define rs LATCbits.LATC4
#define enable LATCbits.LATC5
//#define data LATC
#define data1(val) LATC=(LATC&0xf0)|(val&0xf)
void initLCD(void);
void clearLCD(void);
void homeLCD(void);
void putcharLCD(void);
void setaddrLCD(unsigned char);
void setline0LCD(void);
void setline1LCD(void);
void sendcmd(unsigned char);
void sendchar( unsigned char);

unsigned char LCDaddr=0;    // global address variable for line control

void initLCD(void) {
    __delay_ms(1500);    // allow power up time for LCD

    // send this sequence to init to 4 bit mode:  0010 in upper bits which is PORTC low nibble

    rs = 0;     // 0=command
    NOP();
    enable = 1;
//    data = 0b0011;
    data1(0b0011);
    NOP();
    enable = 0;
    __delay_ms(10);

    rs = 0;
    NOP();
    enable = 1;
//    data = 0b0011;
    data1(0b0011);
    NOP();
    enable = 0;
    __delay_ms(10);

    rs = 0;
    NOP();
    enable = 1;
//    data = 0b0011;
    data1(0b0011);
    NOP();
    enable = 0;
    __delay_ms(10);

    rs = 0;
    NOP();
    enable = 1;
//    data = 0b0010;
    data1(0b0010);
    NOP();
    enable = 0;
    __delay_ms(10);

    sendcmd(0b00101000);    // 001 function set DL=0(data length=4) N=1(2 lines) F=0(5x8 font)
    sendcmd(0b00000001);
    sendcmd(0b00001111);    // 00001 display on-off Display=1(on) Cursor=1(on) Blink=1(on)
    sendcmd(0b00000110);    // 000001 entry mode set i/d=1(increment) S=0 (shift off)

    setline0LCD();
    printf("LCD initialized");
    __delay_ms(1000);
    clearLCD();
    setline1LCD();
    printf("LCD initialized");
    __delay_ms(1000);
    clearLCD();
    setline0LCD();
}

void sendcmd(unsigned char cmd) {
    // send the cmd in two nibbles high then low
    rs = 0;
    NOP();
    enable = 1;
    NOP();
//    data = cmd >>4;
    data1(cmd >> 4);
    NOP();
    enable = 0;
    NOP();
    __delay_ms(5);

    rs = 0;
    NOP();
    enable = 1;
    NOP();
//    data = cmd;
    data1(cmd);
    NOP();
    enable = 0;
    __delay_ms(5);
}

void clearLCD(void) {
    sendcmd(0b00000001);     // 00000001 clear display
}

void homeLCD(void) {
    sendcmd(0b00000010);    // 0000001x return home
    __delay_ms(2);          // needs more than 1.5 ms
}

void setline0LCD(void) {
    setaddrLCD(0);
    LCDaddr=0;
}
void setline1LCD(void) {
    setaddrLCD(0x40);
    LCDaddr=0x40;
}
void setaddrLCD(unsigned char address) {
    unsigned char addr;
    addr = address | 0x80;
    sendcmd(addr);
}
 void putch(char character) {
    setaddrLCD(LCDaddr);
    rs = 1;
    NOP();
    enable = 1;
    NOP();
//    data = character >> 4;
    data1(character >> 4);
    NOP();
    enable = 0;
    NOP();
    __delay_us(200);

    rs = 1;
    NOP();
    enable = 1;
    NOP();
//    data = character;
    data1(character);
    NOP();
    enable = 0;
    __delay_us(200);

    LCDaddr++;
    if (LCDaddr == 0x10) {
        LCDaddr=0x40;
    }
    if (LCDaddr == 0x50) {
        LCDaddr=0;
    }

}
