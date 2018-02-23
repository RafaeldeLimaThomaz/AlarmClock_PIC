#include <xc.h>                 // allows code in this source file to access compiler - or device-specific -  features
#include <stdio.h>
#include <string.h>


// PIC16F1829 Configuration Bit Settings

// 'C' source line config statements

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = OFF         // Low-Voltage Programming Enable (Low-voltage programming enabled)
#define _XTAL_FREQ 4000000       // Define Crystal oscillation as 4 MHz
#define OFF  0
#define ON  1

// prototypes of the functions defined in header file (AlarmClock_LCD.h)

void initLCD(void);
void clearLCD(void);
void homeLCD(void);
void putcharLCD(void);
void setaddrLCD(unsigned char);
void setline0LCD(void);
void setline1LCD(void);
void sendcmd(unsigned char);
void sendchar( unsigned char);

// Variables and auxiliar characters

int mode = 0;                           // variable used to store the configuration and screen mode
int Hours = 00;                         // variable used to store the hour set by user
int Minutes = 00;                       // variable used to store the minutes set by user
int Seconds = 00;                       // variable used to store the seconds set by user
int alarm_hours = 00;                   // variable used to store the alarm hour set by user
int alarm_minutes = 00;                 // variable used to store the alarm minutes set by user
int alarm_seconds = 00;                 // variable used to store the alarm seconds set by user
int set_time = 0;                       // confirmation status (confirm changes in time?)
int set_alarm = 0;                      // confirmation status (confirm changes in alarm time?)
unsigned long H = 0;                    // auxiliar variable used when setting hours
unsigned char M = 0;                    // auxiliar variable used when setting minutes
unsigned char S = 0;                    // auxiliar variable used when setting seconds
float temp = 0;                         // variable used to store Temperature readings
char celsius[2] = "C";                  // auxiliar character for displaying Temperature in degree Celsius
unsigned long mscount=0;                // variable used for milisseconds counting
unsigned char intcount=0;               // auxiliar variable used to count 1 ms (4 * 250 us)
char alarm_state[4] = "Off";            // auxiliar character for alarm state (ON/OFF)
char set_hour[4] = "No";                // auxiliar character for confirmation status (set_hour? = YES/NO)


void main(void) {
    //unsigned char n;
    unsigned long thissec=0, lastsec=0, hours=0;
    unsigned char minutes, seconds;


    // initialize registers 

    ANSELA = 0b00010000;                            // ans4 = RA4 = analog channel 3 = potentiometer
    TRISA =  0b11011101;                            // only ra1 and ra5 are used as outputs
                                                    // ra4 is connected to potentiometer, so keep it as input
                                                    // ra2 is connected to switch
                                                    // ra3 is used by board as Vpp
    ANSELC = 0;                                     // analog select register C set as Digital I/O
    TRISC = 0;                                      // Tri state register C set as output

    ANSELB = 0;                                     // analog select register B set as Digital I/O
    TRISBbits.TRISB4 = 1;                           // pin B4 is set as input (button1) [data direction register]
    TRISBbits.TRISB5 = 1;                           // pin B5 is set as input (button2) [data direction register]
    TRISBbits.TRISB6 = 1;                           // pin B6 is set as input (button3) [data direction register]
    TRISBbits.TRISB7 = 0;                           // pin B7 is set as output (beep/buzzer) [data direction register]

    OSCCONbits.IRCF=0b1101;                         // run at 4MHz
    OPTION_REGbits.T0CS=0;                          // TMR0 clock is based on Fosc/4
    OPTION_REGbits.PSA=1;                           // do not use the prescaler for TMR0
    TMR0=(256-250+2);                               // roll over after 250 usec
    INTCONbits.GIE = 1;                             //enable general interrupts
    INTCONbits.T0IE=1;                              // Timer 0 interrupt enabled
    
    ADCON0 = 0b01110101;                            //Select Temperature Indicator
    ADCON1 = 0b10010000;                            //left justified - FOSC/8 speed - Vref is Vdd
    FVRCONbits.TSEN = 1;                            // enable internal temperature indicator
    FVRCONbits.TSRNG = 1;                           // Temperature Indicator Range Selection bit [VOUT = VDD - 2VT (Low Range)]
    
    initLCD();                                      // initialize LCD display

// main loop
    while(1) {

            __delay_us(5);                          //wait for ADC charging capacitor to settle
              GO = 1;                               //conversion is in progress
              while (GO)                            //wait for conversion to be finished
              {
                
              }

               temp = (ADRES)*0.881 - 391.28;       // transfer function(calibrated sensor)
                                                    // [ADRES contains analog reading of temperature from ADC]


            // Set mode, to configure hours, minutes, seconds, alarm hours, alarm minutes, alarm seconds, and confirm the adjusted time

             if(PORTBbits.RB6 == 0)
            {
                __delay_ms(10);

                mode++;   
            }

            if (mode > 8)
            {
                mode = 0;    
            }

            switch (mode)
            {
                case 0:
                    // counting the passing time
                    if(set_time == 1)
                    {

                       mscount = 0;
                       
                       set_time = 0;
                       strcpy(set_hour,"No");
                       set_hour[2]=' ';
                       H = Hours;
                       M = Minutes;
                       S = Seconds;

                    }
                       Hours = 0;
                       Minutes = 0;
                       Seconds = 0;
                    
                         thissec = H*3600 + M*60 + S + mscount/1000;

                         if(hours == 23 && minutes == 59 && seconds == 59)
                         {
                             mscount = 0;
                             H = 0;
                             M = 0;
                             S = 0;
                         }
                     if (thissec != lastsec)

                         {
                               hours =  (thissec)/3600 ;
                               minutes =  (thissec)/60 - (hours)*60;
                               seconds =  thissec - (hours * 3600) - (minutes*60);

                               lastsec = thissec;
                         }
                    

                    // when alarm time is reached by current time, a beep goes on
                    if(alarm_hours == hours && alarm_minutes <= minutes && minutes < alarm_minutes + 1 && set_alarm == 1)
                    {
                        LATBbits.LATB7 = 1;         // beep on
                        __delay_ms(50);             // hold 50 milisseconds
                        
                         LATBbits.LATB7 = 0;        // beep off
                         __delay_ms(50);            // hold 50 milisseconds
                        
                         LATBbits.LATB7 = 1;
                        __delay_ms(50);
                        
                         LATBbits.LATB7 = 0;
                         __delay_ms(50);
                        
                         LATBbits.LATB7 = 1;
                        __delay_ms(50);
                        
                         LATBbits.LATB7 = 0;
                         __delay_ms(50);
                        
                         LATBbits.LATB7 = 1;
                         __delay_ms(50);
                        
                         LATBbits.LATB7 = 0;
                         __delay_ms(300);
                        
                    }
                    if(set_alarm == 0)
                    {
                        LATBbits.LATB7 = 0;
                    }
                    // display on lcd (Hours, minutes, seconds, alarm state (ON/OFF), temperature [degree celsius])
                     setline0LCD();
                     printf("%2.2lu:%2.2u:%2.2u Alm:%s",hours, minutes, seconds, alarm_state);
                     setline1LCD();

                     printf("Temp: %3.1f \xDF%s ",temp,celsius);

                break;

                case 1:
                    
                   // inscreasing hours
                   if(PORTBbits.RB5 == 0)
                     {
                         __delay_ms(10);

                         if(Hours == 23)
                         {
                          Hours = 0;
                         }
                         else
                         {
                           Hours++;
                         }     
                     }
                    
                    // decreasing hours
                    if(PORTBbits.RB4 == 0)
                     {
                         __delay_ms(10);

                          if(Hours == 0)
                             {
                                Hours = 23;
                             }
                         else
                             {
                                Hours--;
                             }
                     }
                    
                    // display on lcd
                    setline0LCD();
                    printf("Set Time           ");
                    setline1LCD();
                    printf("Hours: %2.2u         ", Hours);
                    
                    break;

                case 2:
                    // inscreasing minutes
                    if(PORTBbits.RB5 == 0)
                     {
                         __delay_ms(10);

                         if(Minutes == 59)
                         {
                            Minutes = 0;
                         }
                         else
                         {
                            Minutes++;
                         }
                     }
                    // decreasing minutes
                    if(PORTBbits.RB4 == 0)
                     {
                         __delay_ms(10);

                         if(Minutes == 0)
                         {
                          Minutes = 59;
                         }
                         else
                         {
                           Minutes--;
                         }
                     }
                    // display on lcd
                    setline0LCD();
                    printf("Set Time             ");
                    setline1LCD();
                    printf("Minutes: %2.2u        ", Minutes);

                    break;

                case 3:
                     // increasing seconds
                     if(PORTBbits.RB5 == 0)
                     {
                         __delay_ms(10);

                         if(Seconds == 59)
                         {
                            Seconds = 0;
                         }
                         else
                         {
                            Seconds++;
                         }
                     }
                    // decreasing seconds
                    if(PORTBbits.RB4 == 0)
                     {
                         __delay_ms(10);

                         if(Seconds == 0)
                         {
                          Seconds = 59;
                         }
                         else
                         {
                           Seconds--;
                         }
                     }
                    // display on lcd
                    setline0LCD();
                    printf("Set Time              ");
                    setline1LCD();
                    printf("Seconds: %2.2u        ", Seconds);

                    break;

                case 4:
                    // increasing alarm hours
                    if(PORTBbits.RB5 == 0)
                     {
                         __delay_ms(10);

                         if(alarm_hours == 23)
                         {
                          alarm_hours = 0;
                         }
                         else
                         {
                           alarm_hours++;
                         }
                     }
                    // decreasing alarm hours
                    if(PORTBbits.RB4 == 0)
                     {
                         __delay_ms(10);

                          if(alarm_hours == 0)
                         {
                          alarm_hours = 23;
                         }
                         else
                         {
                           alarm_hours--;
                         }
                     }
                    // display on lcd
                    setline0LCD();
                    printf("Set Alarm             ");
                    setline1LCD();
                    printf("Hours: %2.2u         ", alarm_hours);

                    break;

                case 5:
                    // increasing alarm minutes
                    if(PORTBbits.RB5 == 0)
                     {
                         __delay_ms(10);

                         if(alarm_minutes == 59)
                         {
                            alarm_minutes = 0;
                         }
                         else
                         {
                            alarm_minutes++;
                         }
                     }
                    // decreasing alarm minutes
                    if(PORTBbits.RB4 == 0)
                     {
                         __delay_ms(10);

                         if(alarm_minutes == 0)
                         {
                          alarm_minutes = 59;
                         }
                         else
                         {
                           alarm_minutes--;
                         }
                     }
                    // display on lcd
                    setline0LCD();
                    printf("Set Alarm             ");
                    setline1LCD();
                    printf("Minutes: %2.2u", alarm_minutes);

                    break;

                case 6:
                    // increasing alarm seconds
                    if(PORTBbits.RB5 == 0)
                     {
                         __delay_ms(10);

                         if(alarm_seconds == 59)
                         {
                            alarm_seconds = 0;
                         }
                         else
                         {
                            alarm_seconds++;
                         }
                     }
                    // decreasing alarm seconds
                    if(PORTBbits.RB4 == 0)
                     {
                         __delay_ms(10);

                         if(alarm_seconds == 0)
                         {
                          alarm_seconds = 59;
                         }
                         else
                         {
                           alarm_seconds--;
                         }
                     }
                    // display on lcd
                    setline0LCD();
                    printf("Set Alarm             ");
                    setline1LCD();
                    printf("Seconds: %2.2u", alarm_seconds);

                    break;

                case 7:
                    // confirming configured time
                    if(PORTBbits.RB5 == 0)
                     {
                         __delay_ms(10);
                         strcpy(set_hour,"Yes");
                         set_time = 1;
                     }

                    if(PORTBbits.RB4 == 0)
                     {
                         __delay_ms(10);
                         strcpy(set_hour,"No");
                         set_hour[2]=' ';
                         set_time = 0;
                     }
                    // display on lcd
                    setline0LCD();
                    printf("Set Time?               ");
                    setline1LCD();
                    printf("%s            ", set_hour );

                    break;

                case 8:
                    // confirming alarm state (ON/OFF)
                    if(PORTBbits.RB5 == 0)
                     {
                         __delay_ms(10);
                         strcpy(alarm_state,"On");
                         alarm_state[2]=' ';
                         set_alarm = 1;
                     }

                    if(PORTBbits.RB4 == 0)
                     {
                         __delay_ms(10);
                         strcpy(alarm_state,"Off");
                         set_alarm = 0;
                     }
                    // display on lcd
                    setline0LCD();
                    printf("Set alarm             ");
                    setline1LCD();
                    printf("State: %s      ", alarm_state );

                    break;
            }                      
    }
}
// interruption service routine for counting Timer0 overflows (reliable time base)
void interrupt myISR(void) {
    TMR0 = (256-250+2+19);       // roll over after 250 usec
    INTCONbits.T0IF = 0;         // clear TIMER0 interruption flag
    
    // if 4 * 250 us (1 ms) has passed, increment number of milisseconds and restart counting
    if(++intcount == 4) {
        mscount++;
        intcount=0;
    }
}



