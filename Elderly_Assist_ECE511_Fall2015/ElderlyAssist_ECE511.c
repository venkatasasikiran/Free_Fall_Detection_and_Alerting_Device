//******************************************************************************
//Project: Elderly Assist
//Team: Bhoopal
//      Pruthvi
//      Sasikiran
//      Ravi
//
//University: George Mason University
//Professor : Dr Kevin Lilly
//Couse: ECE_511 - Fall 2015
//MS in Computer Engineering.
//******************************************************************************

#include <msp430.h>
#include "driverlib.h"

#define TIMER_PERIOD 7
#define DUTY_CYCLE  4

// Global variables
const char call[] = "call\r\n";
const char stop[] = "stop\r\n";
unsigned char X1, X2, Y1, Y2, Z1, Z2,I2CRecvBuffer;
int x1,y1,z1;
char x1_char[] = "000";
char y1_char[] = "000";
char z1_char[] = "000";

int Emergency_flag;
int Emergency_flag_prev;

//Functions declarations
void I2CInit(void);
void transmit(const char *str);
void setup_Fall(void);
unsigned int I2CWriteByte(unsigned char SlaveAdd, unsigned char RegAdd, unsigned char Data);
unsigned int I2CReadByte(unsigned char SlaveAdd, unsigned char RegAdd);


void main(void)

{
//Stop WDT
    WDT_A_hold(WDT_A_BASE);

//Set push button on P1.1 to give interrupt on push
       P1REN |= BIT1;                            // Enable P1.1 internal resistance
       P1OUT |= BIT1;                            // Set P1.1 as pull-Up resistance
       P1IES |= BIT1;                            // P1.1 Hi/Lo edge
       P1IFG &= ~BIT1;                           // P1.1 IFG cleared
       P1IE |= BIT1;                             // P1.1 interrupt enabled

//Set push button on P2.1 to give interrupt on push
       P2REN |= BIT1;                            // Enable P2.1 internal resistance
       P2OUT |= BIT1;                            // Set P2.1 as pull-Up resistance
       P2IES |= BIT1;                            // P2.1 Hi/Lo edge
       P2IFG &= ~BIT1;                           // P2.1 IFG cleared
       P2IE |= BIT1;                             // P2.1 interrupt enabled

//P1.3 as PWM output
       GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1,GPIO_PIN3);

//debug Led setup
       P1DIR |= BIT0;                            // LED :  Set P1.0 to output direction

//P2.0 Port setup for interrupt
       P2REN |= BIT0;
       P2OUT |= BIT0;
       P2IES |= BIT0;
       P2IFG &= ~BIT0;
       P2IE |= BIT0;
       __enable_interrupt();


// Port P3.3 and P3.4 setup for UART(bluetooth)
       P3SEL |= BIT3 + BIT4;                        // P3.4,5 = USCI_A0 TXD/RXD
       UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
       UCA0CTL1 |= UCSSEL_2;                     // SMCLK
       UCA0BR0 = 6;                              // 1MHz 9600 (see User's Guide)
       UCA0BR1 = 0;                              // 1MHz 9600
       UCA0MCTL = UCBRS_0 + UCBRF_13 + UCOS16;   // Modln UCBRSx=0, UCBRFx=0,// over sampling
       UCA0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**

// I2C Port setup
       P3REN |= 0x03;
       P3OUT |= 0x03;
       P3SEL |= 0x03;

       __delay_cycles(12000);
       I2CInit();
       I2CWriteByte(0x53, 0x31, 0x03);
       __delay_cycles(1200);
       __delay_cycles(12000);

// Setup Fall
       setup_Fall(); // setup for enabling interrupt INT1 on Tap.

// Main While Loop
       while(1)
       {
           X1 =  I2CReadByte(0x53, 0x30);

           if(Emergency_flag_prev == 1)
           {
               if (X1 > 0x00)
               {
                   transmit(call);
               }
           }
           else
           {
               if (X1 > 0x00)
               {
                 transmit(stop);
               }
           }
           if(Emergency_flag ==1)
          {
               if (Emergency_flag_prev == 0)
               {
                   Emergency_flag_prev = 1;
               }
          }
           else
           {
               Emergency_flag_prev = 0;
           }

       }
        __no_operation();
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    Emergency_flag =0; // reset Emergency Flag
    Timer_A_stop(TIMER_A0_BASE);
     P1IFG &= ~BIT1; // P1.1 IFG cleared
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    Emergency_flag =1;  // Set Emergency Flag
    P1OUT ^= BIT0;                            // debug led P1.0 = toggle
    P2IFG &= ~BIT0;                          // P1.4 IFG cleared

    //P1.3 as PWM output
    Timer_A_outputPWMParam param = {0};
    param.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    param.timerPeriod = TIMER_PERIOD;
    param.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
    param.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    param.dutyCycle = DUTY_CYCLE;
    Timer_A_outputPWM(TIMER_A0_BASE, &param);

    P2IFG &= ~BIT1;// clear interrupt
}

void transmit(const char *str) {
    while (*str != 0) {
        while (!(UCTXIFG & UCA0IFG)) ;
        UCA0TXBUF = *str++;
    }
}


void I2CInit()
{
    while (UCB0CTL1 & UCTXSTP);
    P3SEL &= ~0x03;                            // Assign I2C pins to USCI_B0
    P3SEL |= 0x03;
    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 30;
    UCB0BR1 = 0;
    UCB0I2CSA = 0x53;                         // Accelorometer Slave Address is 0x53h
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
}


unsigned int I2CWriteByte(unsigned char SlaveAdd, unsigned char RegAdd, unsigned char Data)
{
    UCB0I2CSA = SlaveAdd;                      // write Accelorometer address
    while (UCB0CTL1 & UCTXSTP);
    UCB0CTL1 |= (UCTR + UCTXSTT);             // I2C TX, start condition

    while((UCB0IFG & UCTXIFG)==0);            // wait for Tx interuppt

    UCB0TXBUF = RegAdd;                        // write register address
    while((UCB0IFG & UCTXIFG)==0);            // wait for Tx interuppt

    UCB0TXBUF = Data;                          // write data
    while((UCB0IFG & UCTXIFG)==0);             //wait for Tx interuppt

    UCB0CTL1 |= UCTXSTP;                    // send Stop
    while((UCB0CTL1 & UCTXSTP)==1);
    return 0;
}



unsigned int I2CReadByte(unsigned char SlaveAdd, unsigned char RegAdd)
{
        UCB0I2CSA = SlaveAdd;                       // write Accelorometer address
        while (UCB0CTL1 & UCTXSTP);
        UCB0CTL1 |= (UCTR + UCTXSTT);             // I2C TX, start condition
        while((UCB0IFG & UCTXIFG)==0);
        UCB0TXBUF = RegAdd;                         // write register address
        while((UCB0IFG & UCTXIFG)==0);

        UCB0CTL1 |= (UCTXSTT);                      // I2C TX, start condition
        while((UCB0IFG & UCTXIFG)==0);

        UCB0CTL1 &= ~UCTR;                          // Enable Reciver Mode
        while(!(UCB0IFG & UCRXIFG)) ;
        I2CRecvBuffer = UCB0RXBUF;                  // Read Rx buffer
        while(!(UCB0IFG & UCRXIFG)) ;
        I2CRecvBuffer = UCB0RXBUF;                  // Read Rx buffer

        UCB0CTL1 |= UCTXNACK;                       // Transmit NACK
        UCB0CTL1 |= UCTXSTP;                        // Transmit stop
        while((UCB0CTL1 & UCTXSTP));

        return I2CRecvBuffer;
}

void setup_Fall()
{

           //Send the Interrupts to INT1 pin
           I2CWriteByte(0x53, 0x2F, 0x00);

           //Look for taps on the X, Y, Z axis only.
           I2CWriteByte(0x53, 0x2A, 0x07);
           //Set the Tap Threshold to 3g
           I2CWriteByte(0x53, 0x1D, 0x38);
           //Set the Tap Duration that must be reached
           I2CWriteByte(0x53, 0x21, 0x10);
           //100ms Latency before the second tap can occur.
           I2CWriteByte(0x53, 0x22, 0x50);

          // Fall detection configuration
           I2CWriteByte(0x53, 0x23, 0xFF);
           I2CWriteByte(0x53, 0x29, 0x10);      //Fall g value   16 * 0.0625g
           I2CWriteByte(0x53, 0x28, 0x09);      //fall duration  9 * 5ms


           I2CWriteByte(0x53, 0x31, 0x0A);
               // writeRegister(DATA_FORMAT, 0x01);

           //Enable the Fall Interrrupt.
           I2CWriteByte(0x53, 0x2E, 0x04);

           //Enable the single and double tap Interrrupt.
           //I2CWriteByte(0x53, 0x2E, 0x6)
}

// fuctions used for debugging the software while issues

//Read_XYZ_Axis()
//{
//    while (1)
//    {
//        X1 =  I2CReadByte(0x53, 0x32);
//        X2 =  I2CReadByte(0x53, 0x33);
//        Y1 =  I2CReadByte(0x53, 0x34);
//        Y2 =  I2CReadByte(0x53, 0x35);
//        Z1 =  I2CReadByte(0x53, 0x36);
//        Z2 =  I2CReadByte(0x53, 0x37);
//
//        x1 = (((int)X2) << 8) | X1;
//
//        __delay_cycles(10000);
//        getDecStr (x1_char,3, x1);
//        transmit(" x1:");
//        transmit(x1_char);
//       __delay_cycles(50000);
//
//        y1 = (((int)Y2) << 8) | Y1;
//
//        __delay_cycles(10000);
//        getDecStr (y1_char,3, y1);
//        transmit("  y1:");
//        transmit(y1_char);
//       __delay_cycles(50000);
//
//        z1 = (((int)Z2) << 8) | Z1;
//
//        __delay_cycles(10000);
//        getDecStr (z1_char,3, z1);
//        transmit("  z1:");
//        transmit(z1_char);
//
//        __delay_cycles(50000);
//         transmit(enter);
//         __delay_cycles(50000);
//    }
//}


//Transmit_interrupt_source()
//{
//X1 =  I2CReadByte(0x53, 0x30);
//
// if (X1 > 0x00)
//
// {
//   getDecStr (x1_char,3, X1);
//     transmit(" x1:");
//    transmit(x1_char);
//}
//}


//String Handler
//
//void getDecStr (char* str, char len, int val)
//{
//  char i;
//
//  for(i=1; i<=len; i++)
//  {
//    str[len-i] = (char) ((val % 10UL) + '0');
//    val/=10;
//  }
//
//  str[i-1] = '\0';
//}
