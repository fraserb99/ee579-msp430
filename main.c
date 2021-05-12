#include <msp430.h>
//#include "../jsmn.h"
#include <string.h>


/* Separate file for basic premise of UART reading
 * Handles parsing the RX line, translating JSON into commands and parameters, comparison against known commands and switch statement for activating different I/O
 * Handles creating JSON packets of input values, sending them on the TX line
 *
 * Baud rate = 115200Hz on 1MHz SMCLK, format = 8N1, modulation UCBRSx = 1
 *
 * This program is partially derived from MSP430 example code msp430g2xx3_uscia0_uart_06_9600.c, which is
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 */

// Global declarations

// RX
unsigned char unparsedMsg[];        // Stores incoming message
unsigned int msgIndex = 0;          // Increments characters in input message string
unsigned int msgComplete = 0;       // Flag when a full input message is received

// TX
char *outMsg = "Big placeholder string because.";              // Maximum expected message length is 32 chars
unsigned int writeToDo = 0;         // Set to 1 when write is needed: normally 0
unsigned int doneFlag = 1;          // Mark when a msg section is parsed
unsigned int offset = 0;            // Changes output pointer value
char *inputTypeStr = "{\"InputType\": \"";     // Some stupid constant strings I gotta send
char *inputValueStr = "\",\"Value: \"";
char *endStr = "\"}\n";

// Test strings
char *type = "Button1";
char *value = "200";

// UART functions
//void UARTRead(void);
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
      UCA0BR0 = 8;        // 1MHz 115200
      UCA0BR1 = 0;          // 1MHz 115200
      UCA0MCTL = UCBRS0;    // Modulation UCBRSx = 1
      UCA0CTL1 &= ~UCSWRST; // Initialise USCI state machine
//      IE2 |= UCA0RXIE;      // Enable USCI A0 RX interrupt
//      IE2 |= UCA0TXIE;

      // To test TX, uncomment line below:
//       writeToDo = 1;          // Informs that there is a message to send


      while (1){
          // Move this section to whichever function the output comes from
          if (doneFlag == 1){
              doneFlag = 0;
              UARTWrite();
          }
          /*if (msgComplete == 1){
              UARTRead();
          }*/
          else {
              __bis_SR_register(LPM0_bits + GIE);   //put in low power mode LPM0 with interrupt enabled
          }
      }

}

/* RX input buffer interrupt: triggered when input buffer is filled
 * Reads a single character of incoming JSON message from RX buffer into input message string

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {


    unparsedMsg[msgIndex] = UCA0RXBUF;          // Store character from RX buffer

    if (unparsedMsg[msgIndex] == "}"){          // Check for message delimiter, if end of message set flag and reset index
        msgIndex = 0;
        msgComplete = 1;
    }
    else {
        msgIndex++;
    }

}


/* Parses RX buffer from a JSON string to a struct using the JSMN library and activates
 * the relevant input and output flags

void UARTRead(void){

    // JSMN library use
    jsmn_parser parser;             // Create a parser instance
    size_t tokenCount = 5;          // Number of potential tokens (should be maximum 5, for DeviceConfig message)
    jsmntok_t tokens[tokenCount];   // Initialise tokens for parser output
    int result;                     // Number of tokens actually used

    jsmn_init(&parser);             // Initialise parser
    result = jsmn_parse(&parser, unparsedMsg, strlen(unparsedMsg), tokens, tokenCount);

    if (result < 0){
        // Failed
    }

    // Extract strings from jsmn struct
    unsigned char type[] = tokens[0];

    // Find which outputType the message is
    if (strcmp(type, "LedOutput")){
        // Check LED
        // Get state (bool)
        // Get colour (enum)
        // Activate LED
    }

    else if (strcmp(type, "LedBreathe")){
        // Check LED
        // Get period (int)
        // Get colour (enum)
        // Activate breathe
    }

    else if (strcmp(type, "LedBlink")){
        // Check LED
        // Get period (int)
        // Get colour (enum)
        // Activate blink
    }

    else if (strcmp(type, "LedFade")){
        // Check LED
        // Get state (bool)
        // Get duration (int)
        // Get colour (enum)
        // Activate fade
    }

    else if (strcmp(type, "LedCycle")){
        // Check LED
        // Get period (int)
        // Get colour (enum)
        // Activate blink
    }

    else if (strcmp(type, "BuzzerOn")){
        // Get duration (int)
        // Activate buzz
    }

    else if (strcmp(type, "BuzzerBeep")){
        // Get on duration (int)
        // Get off duration (int)
        // Activate beep
    }

    else if (strcmp(type, "DeviceConfig")){
        // Get Button1 (bool)
        // Get Button2 (bool)
        // Get Potentiometer (bool)
        // Get Temperature (bool)
    }

    msgComplete = 0;            // Mark as done

}
*/

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
        writeToDo = writeToDo%6;
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

    outMsg = type;              // Set output message to first part of JSON message
    IE2 |= UCA0TXIE;            // Enable TX interrupt
    UCA0TXBUF = *outMsg;        // Write first output character to buffer
    offset++;
}

void write3(void){

    outMsg = inputValueStr;     // Set output message to first part of JSON message
    IE2 |= UCA0TXIE;            // Enable TX interrupt
    UCA0TXBUF = *outMsg;        // Write first output character to buffer
    offset++;
}

void write4(void){

    outMsg = value;             // Set output message to first part of JSON message
    IE2 |= UCA0TXIE;            // Enable TX interrupt
    UCA0TXBUF = *outMsg;        // Write first output character to buffer
    offset++;
}

void write5(void){

    outMsg = endStr;            // Set output message to first part of JSON message
    IE2 |= UCA0TXIE;            // Enable TX interrupt
    UCA0TXBUF = *outMsg;        // Write first output character to buffer
    offset++;
}

