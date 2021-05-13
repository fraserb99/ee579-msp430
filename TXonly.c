#include <msp430.h>
#include <string.h>


/**
 * Testing output transmission of UART
 *
 */

// TX globals
char *outMsg = "Big placeholder string because.";              // Maximum expected message length is 32 chars
unsigned int writeToDo = 0;         // Set to 1 when write is needed: normally 0
unsigned int doneFlag = 1;          // Mark when a msg section is parsed
unsigned int offset = 0;            // Changes output pointer value
char *inputTypeStr = "{\"InputType\": \"";     // Some stupid constant strings I gotta send
char *inputValueStr = "\",\"Value\": \"";
char *endStr = "}\0";

// Test strings
char *type = "Button1";
char *value = "2000";


void UARTWrite(void);

void write1(void);
void write2(void);
void write3(void);
void write4(void);
void write5(void);


int main(void){

    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT


    // Safety first!
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
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0;    // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST; // Initialise USCI state machine


      writeToDo = 1;          // Informs that there is a message to send

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



/* TX output buffer interrupt: triggered when output buffer is cleared
 * Writes JSON message characters from UARTWrite function to TX buffer
 */
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void){

    UCA0TXBUF = *(outMsg + offset);             // Read next char of message into TX buffer
    offset++;

    if (*(outMsg + offset-1) == '\0'){          // If end of message, disable TX interrupt
        IE2 &= ~UCA0TXIE;
        writeToDo++;                            // Write next part of output
        writeToDo = writeToDo%6;                // Resets to indicate nothing more to write
        doneFlag = 1;                           // Indicate buffer empty
        offset = 0;                             // Reset output string pointer offset

        __bic_SR_register_on_exit(LPM0_bits);   // Break away from low power mode
    }
}

/* Brief function to trigger correct TX write for a full JSON message in 5 parts
 * JSON message comes out in format: {"InputType": "<type>,"Value": <value>}
 */
void UARTWrite(void){

    switch (writeToDo){
        case 0:
            break;
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


// 5 functions to write each part of the JSON output messages
void write1(void){

    outMsg = inputTypeStr;      // Set output message to first part of JSON message
    IE2 |= UCA0TXIE;            // Enable TX interrupt
    UCA0TXBUF = *outMsg;        // Write first output character to buffer
    offset++;
}

void write2(void){

    outMsg = type;              // Set output message to first parameter of JSON message
    IE2 |= UCA0TXIE;            // Enable TX interrupt
    UCA0TXBUF = *outMsg;        // Write first output character to buffer
    offset++;
}

void write3(void){

    outMsg = inputValueStr;     // Set output message to second part of JSON message
    IE2 |= UCA0TXIE;            // Enable TX interrupt
    UCA0TXBUF = *outMsg;        // Write first output character to buffer
    offset++;
}

void write4(void){

    outMsg = value;             // Set output message to second parameter of JSON message
    IE2 |= UCA0TXIE;            // Enable TX interrupt
    UCA0TXBUF = *outMsg;        // Write first output character to buffer
    offset++;
}

void write5(void){

    outMsg = endStr;            // Set output message to closing part of JSON message
    IE2 |= UCA0TXIE;            // Enable TX interrupt
    UCA0TXBUF = *outMsg;        // Write first output character to buffer
    offset++;
}
