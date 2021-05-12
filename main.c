#include <msp430.h>
#include <string.h>


/**
 * testing output of json building
 *
 */

// Globals
char *outMsg = "Big placeholder string because.";              // Maximum expected message length is 32 chars
unsigned int writeToDo = 1;         // Set to 1 when write is needed: normally 0
unsigned int doneFlag = 1;          // Mark when a msg section is parsed
unsigned int offset = 0;            // Changes output pointer value

// SOme stupid constant strings I gotta send
const char *inputTypeStr = "{\"InputType\":\"";
const char *inputValueStr = "\"\n\"Value:\"";
const char *endStr = "\"\n}\n";

// Test strings
char *type = "Button1";
char *value = "200";

void UARTWrite(void);

void write1(void);
void write2(void);
void write3(void);
void write4(void);
void write5(void);


int main(void){

      WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT



      if (CALBC1_1MHZ == 0xFF){  // If calibration constant erased
              while (1); // do not load, trap CPU!!
      }

      DCOCTL = 0;            // Select lowest DCOx and MODx settings
      BCSCTL1 = CALBC1_1MHZ; // Set DCO
      DCOCTL = CALDCO_1MHZ;
      P1SEL = BIT1 + BIT2;  // P1.1 = RXD, P1.2=TXD
      P1SEL2 = BIT1 + BIT2; // P1.1 = RXD, P1.2=TXD
      UCA0CTL1 |= UCSSEL_2; // SMCLK
      UCA0BR0 = 104;        // 1MHz 9600
      UCA0BR1 = 0;          // 1MHz 9600
      UCA0MCTL = UCBRS0;    // Modulation UCBRSx = 1
      UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
      //IE2 |= UCA0RXIE;      // Enable USCI_A0 RX interrupt
      IE2 |= UCA0TXIE;

      while (1){
          if (doneFlag == 1){
              doneFlag = 0;
              UARTWrite();
          }
          else {
              __bis_SR_register(LPM0_bits + GIE);   //put in low power mode LPM0 + GIE
          }
      }


}



// Write JSON message characters to TX buffer
// JSON message comes out in format: {"InputType":"<type>\n "Value":<value>}
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void){

    UCA0TXBUF = *(outMsg + offset);
    offset++;                 // Read next char of message into TX buffer

    if (*(outMsg + offset-1) == '\0'){                  // If end of message, disable TX interrupt
        IE2 &= ~UCA0TXIE;
        writeToDo++;                            // Write next part of output
        writeToDo = writeToDo%6;
        doneFlag = 1;
        offset = 0;
        // break away from low power mode
        __bic_SR_register_on_exit(LPM0_bits);
    }
}


void UARTWrite(void){

    switch (writeToDo){
        case 1:
            write1();
            break;
        case 2:
            write2();
            break;
        case 3:
            write3();
            break;
        case 4:
            write4();
            break;
        case 5:
            write5();
            break;
    }

}


void write1(void){

    outMsg = inputTypeStr;

    IE2 |= UCA0TXIE;                        // Enable TX interrupt
    UCA0TXBUF = *outMsg;      // Write first output character to buffer
    offset++;

}

void write2(void){

    outMsg = type;

    IE2 |= UCA0TXIE;                        // Enable TX interrupt
    UCA0TXBUF = *outMsg;      // Write first output character to buffer
    offset++;

}

void write3(void){

    outMsg = inputValueStr;

    IE2 |= UCA0TXIE;                        // Enable TX interrupt
    UCA0TXBUF = *outMsg;      // Write first output character to buffer
    offset++;

}

void write4(void){

    outMsg = value;

    IE2 |= UCA0TXIE;                        // Enable TX interrupt
    UCA0TXBUF = *outMsg;      // Write first output character to buffer
    offset++;

}

void write5(void){

    outMsg = endStr;

    IE2 |= UCA0TXIE;                        // Enable TX interrupt
    UCA0TXBUF = *outMsg;      // Write first output character to buffer
    offset++;

}

