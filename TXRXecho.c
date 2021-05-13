#include <msp430.h>
#include <string.h>


char inMsg[128];                    // Stores incoming message
unsigned int inOffset = 0;          // Increments characters in input message string
unsigned int msgComplete = 0;       // Flag when a full input message is received

// TX
char outMsg[128];                   // Maximum expected message length is 32 chars
unsigned int offset = 0;            // Changes output pointer value

void UARTWrite(void);

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  // UART setup
      DCOCTL = 0;                               // Select lowest DCOx and MODx settings
      BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
      DCOCTL = CALDCO_1MHZ;
      P1SEL = BIT1 + BIT2;                      // P1.1 = RXD, P1.2 = TXD
      P1SEL2 = BIT1 + BIT2;
      UCA0CTL1 |= UCSSEL_2;                     // SMCLK
      UCA0BR0 = 104;                              // 1MHz 9600
      UCA0BR1 = 0;                              // 1MHz 9600
      UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
      UCA0CTL1 &= ~UCSWRST;                     // Initialise USCI state machine
      IE2 |= UCA0RXIE;                        // Enable USCI_A0 RX interrupt

      while (1){
               // Move this section to whichever function the output comes from
               if (msgComplete == 1){
                   msgComplete = 0;
                   UARTWrite();
               }
               else {
                   __bis_SR_register(LPM0_bits + GIE);   //put in low power mode LPM0 with interrupt enabled
               }
           }

}

/* RX input buffer interrupt: triggered when input buffer is filled
 * Reads a single character of incoming JSON message from RX buffer into input message string
 */
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {


    inMsg[inOffset] = UCA0RXBUF;
    inOffset++;

    if (inMsg[inOffset-1] == '}'){          // Check for message delimiter, if end of message set flag and reset index
        inOffset = 0;
        msgComplete = 1;
        IE2 &= ~UCA0RXIE;               // TEST ONLY

    }


    __bic_SR_register_on_exit(LPM0_bits);   // Break away from low power mode


}

/* TX output buffer interrupt: triggered when output buffer is cleared
 * Writes JSON message characters from UARTWrite function to TX buffer
 */
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void){

    UCA0TXBUF = outMsg[offset];             // Read next char of message into TX buffer
    offset++;

    if (outMsg[offset-1] == '}'){          // If end of message, disable TX interrupt
        IE2 &= ~UCA0TXIE;
        offset = 0;                             // Reset output string pointer offset

        __bic_SR_register_on_exit(LPM0_bits);   // Break away from low power mode
    }
}

void UARTWrite(void){

    strcpy(outMsg, inMsg);              // Set output message to first part of JSON message
    IE2 |= UCA0TXIE;            // Enable TX interrupt
    UCA0TXBUF = outMsg[offset];        // Write first output character to buffer

}

