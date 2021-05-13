#include <msp430.h>
#include "./jsmn.h"
#include <string.h>

char unparsedMsg[];        // Stores incoming message
unsigned int msgIndex = 0;          // Increments characters in input message string
unsigned int msgComplete = 0;       // Flag when a full input message is received

char *outMsg = "Big placeholder string because.";              // Maximum expected message length is 32 chars
unsigned int doneFlag = 1;          // Mark when a msg section is parsed
unsigned int offset = 0;            // Changes output pointer value

void UARTRead(void);
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
          if (msgComplete == 1){
              UARTRead();
          }
          else {
              __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
          }
      }

}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {


    unparsedMsg[msgIndex] = UCA0RXBUF;          // Store character from RX buffer

    if (unparsedMsg[msgIndex] == '}'){          // Check for message delimiter, if end of message set flag and reset index
        msgIndex = 0;
        msgComplete = 1;
    }
    else {
        msgIndex++;
    }

    __bic_SR_register_on_exit(LPM0_bits);   // Break away from low power mode

}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void){

    UCA0TXBUF = *(outMsg + offset);             // Read next char of message into TX buffer
    offset++;

    if (*(outMsg + offset-1) == '\0'){          // If end of message, disable TX interrupt
        IE2 &= ~UCA0TXIE;
        doneFlag = 1;                           // Indicate buffer empty
        offset = 0;                             // Reset output string pointer offset

        msgComplete = 0;        //for test purposes!!
        __bic_SR_register_on_exit(LPM0_bits);   // Break away from low power mode
    }
}



void UARTRead(void){

    // JSMN library use
    jsmn_parser parser;             // Create a parser instance
    size_t tokenCount = 12;          // Number of potential tokens (object + strings + primitives, max 11 for DeviceConfig message)
    jsmntok_t tokens[tokenCount];   // Initialise tokens for parser output
    int result;                     // Number of tokens actually used

    jsmn_init(&parser);             // Initialise parser
    result = jsmn_parse(&parser, unparsedMsg, strlen(unparsedMsg), tokens, tokenCount);

    if (result < 0){
        // Not a JSON string, ignore and drop
        return;
    }


    // Check for first string token
    if (!tokens[1].type == 3){       // Get type of token[1]; expects string (token[0] is full object)
        return;     // Return if not string token, parse fail
    }

    else {
        if (doneFlag == 1){
              doneFlag = 0;
              outMsg = "yup";      // Set output message to first part of JSON message
              IE2 |= UCA0TXIE;            // Enable TX interrupt
              UCA0TXBUF = *outMsg;        // Write first output character to buffer
              offset++;
        }

        /*
        // Isolating the input string
            int length = unparsedMsg[tokens[2].end] - unparsedMsg[tokens[2].start];
            char inputType[length+1];
            memcpy(inputType, &unparsedMsg[tokens[2].start],length);
            inputType[length]='\0';
            strcpy(outMsg, inputType);
        */
    }

}


