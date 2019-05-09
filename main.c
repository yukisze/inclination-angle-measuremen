    /*;*********************************************************************                
;*                      McMaster University                         *
;*                      2DP4 Microcontrollers                       *
;*                         					*
;*                  												*
;*********************************************************************
;*********************************************************************
;*                      Project - angle sensor                 *****
;*                                                                  *
;*                                                                  *
;**************///filename ******** Main.C ************** **************
//***********************************************************************

#include <hidef.h>      /* common defines and macros */
#include <stdio.h>
#include "derivative.h"      /* derivative-specific definitions */
#include "SCI.h"
#include "math.h"


//prototype
void OutCRLF(void);
void configureSerial(void);
void setBusSpeed(void);
void setATD4(void);
void TimeCapture(void);
void setPin(void);
void Delay1ms(unsigned int numTimes);
void mode_0(void);
void mode_1(void);

unsigned int button = 0;
int mode;
unsigned int angle;
unsigned int num1;
unsigned int num2;
unsigned short val;
unsigned int numTimes;


void OutCRLF(void)
{
SCI_OutChar(CR);
SCI_OutChar(LF);
PTJ ^= 0x20; //toggle LED D2
}

void configureSerial(void){
//configure serial communication rate
SCI_Init(38400);
SCI_OutString("Lai Suet Sze szel 400081720");
OutCRLF();
}
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#define VCOFRQ 0x00        //VCOFRQ[1:0]  32MHz <= VCOCLK <= 48MHz
#define SYNDIV 0x0F        //SYNDIV[5:0]  Syn divide is 1 + 15 = 16
#define REFFRQ 0x80        //REFFRQ[1:0]  since OSCE = 0, it has no effect
#define REFDIV 0x01        //REFDIV[3:0]  since OSCE = 0, it has no effect, fREF = fIRC1M

void setBusSpeed(void){
    CPMUPROT = 0x26;               //Protection of clock configuration is disabled
    							 // NOTE: On some Esduinos you may need to use CPMUPROT=0. Try both and see which value allows you to change the registers below in your debugger!

    CPMUCLKS = 0x80;               //PLLSEL = 1. Select Bus Clock Source:  PLL clock or Oscillator.
    CPMUOSC = 0x00;                //OSCE = 0. Select Clock Reference for OSCclk as:fOSC (1MHz).

    CPMUREFDIV = REFFRQ + REFDIV;    //Set fREF divider and selects reference clock frequency Range. fREF= 1MHz.

    CPMUSYNR = VCOFRQ + SYNDIV;      //Set Syn divide and selects VCO frequency range. fVCO = 16

    CPMUPOSTDIV = 0x01;              //Set Post Divider (0x00= 0000 0000). fPLL= 16 MHz.
                                     //Bus Clock = PLLCLK/2 = 8MHz

    while (CPMUFLG_LOCK == 0) {}  //Wait for PLL to achieve desired tolerance of target frequency. NOTE: For use when the source clock is PLL. comment out when using external oscillator as source clock

    CPMUPROT = 1;                  //Protection for clock configuration is reenabled
}

void setATD4(void)
{
ATDCTL1 = 0x2F; //10 resolution b0010 1111 ADC resolution only changes(6:5)
ATDCTL3 = 0X88;
ATDCTL4 = 0x02;//PRESCALER  = 2; ATD clock =
ATDCTL5 = 0x24;//AN4

}

//INTERRUPT 0- start/stop serial communication, reset MODE0 or MODE1
interrupt  VectorNumber_Vtimch0 void ISR_Vtimch0(void)
{
unsigned int temp;
PTJ ^=  0x01; //toggle it

if (button == 0) { button = 1;} // if the button is not pressed, the mode is sticked to 0
else {
button = 0;
}

temp = TC0;  //Renable FastFlagClear, clear the flag, allowing another TIC interrupt
}

/////////////////////////////////////////////////////////////
//INTERRUPT 1- MODE0 or MODE1
interrupt  VectorNumber_Vtimch1 void ISR_Vtimch1(void){
unsigned int temp; //DON'T EDIT THIS

if(mode == 0) {
mode = 1;
}
else mode = 0;

temp = TC1;   	//Refer back to TFFCA, we enabled FastFlagClear, thus by reading the Timer Capture input we automatically clear the flag, allowing another TIC interrupt

}


////////////////////////////////////////////////////
/////////////////////////////////////////////////////

void TimeCapture(void)//from 2dp4 lecture W7
{
//The next six assignment statements configure the Timer Input Capture
TSCR1 = 0x90;    //Timer System Control Register 1
// TSCR1[7] = TEN:  Timer Enable (0-disable, 1-enable)
// TSCR1[6] = TSWAI:  Timer runs during WAI (0-enable, 1-disable)
// TSCR1[5] = TSFRZ:  Timer runs during WAI (0-enable, 1-disable)
// TSCR1[4] = TFFCA:  Timer Fast Flag Clear All (0-normal 1-read/write clears interrupt flags)
// TSCR1[3] = PRT:  Precision Timer (0-legacy, 1-precision)
// TSCR1[2:0] not used
TSCR2 = 0x04;    //Timer System Control Register 2
// TSCR2[7] = TOI: Timer Overflow Interrupt Enable (0-inhibited, 1-hardware irq when TOF=1)
// TSCR2[6:3] not used
// TSCR2[2:0] = Timer Prescaler =4 >1/16 BUS FREQUENCY

TIOS = 0b11111100;     //channel 0 1 input capture other output capture
PERT = 0x03;     //Enable Pull-Up resistor 
TCTL3 = 0x00;    //capture disable
TCTL4 = 0x0A;    //Configured for EDG1B and EDG0B falling edge
IRQCR = 0x00; //irq pin configured for low level recognition
TIE = 0b00000011;      //ENABLE BOTH OF THE BUTTON
EnableInterrupts; 

}


////////////////////////////////////
//////////////////////////////////////////
void setPin(void){
PER1AD = 0b1111111;//disabled
DDR1AD = 0x0F;//AN4 = 0 input, others are output 1110 1111
DDRP = 0x3E;//from configure output pp1-pp5
PERP = 0x00;//all port p as output
DDRJ = 0x01;
PTJ = 0x01;

}
////////////////////////////////////////////////
//////////////////////////////////////////////////

void Delay1ms(unsigned int multiple){

unsigned int i; //loop control variable

TSCR1 = 0x90;   //enable timer and fast timer flag clear

TSCR2 = 0x00;   //Disable timer interrupt, set prescaler=1

TIOS |= 0x04;   //Enable OC2 (not necessary)

TC2 = TCNT + 8000;

for(i = 0; i < multiple; i++) {
while (!(TFLG1_C2F));
TC2 += 8000;
}

TIOS &= ~0x04; 

TSCR2 = 0x01;

}


//////////////////////////////
//////////////////////////////

void mode_0(void)
{
PTJ ^= 0x01;
num1 = angle/10;
num2 = angle%10;
//	PT1AD = 0x00 + num1;
if (num1==0){PT1AD=0x00;}
else if(num1==1){PT1AD=0b00000001;Delay1ms(100);}
else if(num1==2){PT1AD=0x02;Delay1ms(100);}
else if(num1==3){PT1AD=0x03;Delay1ms(100);}
else if(num1==4){PT1AD=0x04;Delay1ms(100;}
else if(num1==5){PT1AD=0x05;Delay1ms(100);}
else if(num1==6){PT1AD=0x06;Delay1ms(100);}
else if(num1==7){PT1AD=0x07;Delay1ms(100);}
else if(num1==8){PT1AD=0x08;Delay1ms(100);}
else if(num1==9){PT1AD=0x09;Delay1ms(100);}

// num2
if(num2==0){PTP=0b00000000;Delay1ms(100) ;}
else if(num2==1){PTP=0b00000010;Delay1ms(100);}
else if(num2==2){PTP=0b00000100;Delay1ms(100);}
else if(num2==3){PTP=0b00000110;Delay1ms(100);}
else if(num2==4){PTP=0b00001000;Delay1ms(100);}
else if(num2==5){PTP=0b00001010;Delay1ms(100);}
else if(num2==6){PTP=0b00001100;Delay1ms(100);}
else if(num2==7){PTP=0b00001110;Delay1ms(100);}
else if(num2==8){PTP=0b00010000;Delay1ms(100);}
else if(num2==9){PTP=0b00010010;Delay1ms(100) ;}
}

//////////////////////////////
//////////////////////////////

void mode_1(void){
if (angle<5){
PT1AD = 0x00;
PTP=0x00;
Lab4Delay1ms(150);
}
else if (angle<15){
PT1AD = 0x01;
PTP=0x00;
Lab4Delay1ms(150);
}
else if (angle<25){
PT1AD = 0x03;
PTP=0x00;
Lab4Delay1ms(150);
}
else if (angle<35){
PT1AD = 0x07;
PTP=0x00;
Lab4Delay1ms(150);
}
else if (angle<45){
PT1AD = 0x0F;
PTP=0x00;//00000000
Lab4Delay1ms(150);
}
else if (angle<55){
PT1AD = 0x0F;
PTP=0x02;//00000010
Lab4Delay1ms(150);
}
else if (angle<65){
PT1AD = 0x0F;
PTP=0x06; //0000 0110
Lab4Delay1ms(150);
}
else if (angle<75){
PT1AD = 0x0F;
PTP=0x0E;  //0000 1110
Lab4Delay1ms(150);
}
else if (angle<85)
{
PT1AD = 0x0F;
PTP=0x1E;  //0001 1110
Lab4Delay1ms(150);
}
else if(angle<95)
{

PT1AD = 0x0F;
PTP=0x3E;  //0011 1110
Lab4Delay1ms(150);
}
else if (angle>95)
{
PT1AD=0x00;
PTP=0x00;
}

}

void main(void){
    TimeCapture();
    setPin();
    setATD4();
    configureSerial();
    setClk();


    mode = 0;
    button = 0;

    for(;;){
        val = ATDDR0;
        Lab4Delay1ms(10);

        // calculate angle
        if (button == 1){
            if (val < 657)   //the angle at 0
                val = 0;

            else  if (val > 697)  //697 the max angle
                val = 0;

            else{
                val = val - 657;
                if (val<= 24)
                    angle = val/0.6;
                else
                angle = 40 + (val-24)/0.2;
            }
			//SCI_OutString("angle = ");real term
            SCI_OutChar(angle);
        }

        if (mode == 0) mode_0();    //mode0 - BCD
        else mode_1();  //mode 1 - bar
    }
}
