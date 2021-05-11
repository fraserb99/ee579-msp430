#include <msp430.h> 
#include "../jsmn.h"
#include <stdio.h>
#include <string.h>

/* Separate file for basic premise of UART reading
 * Handles parsing the RX line, translating JSON into commands and parameters, comparison against known commands and switch statement for activating different I/O
 * Handles creating JSON packets of input values, sending them on the TX line
 *
 */

// Global declarations
unsigned char unparsedMsg[];        // Stores incoming message
unsigned int msgIndex = 0;          // Increments characters in message string
unsigned int msgComplete = 0;       // Flag for when a full input message is received
unsigned char outMsg[];             // Holds JSON structured message to write to TX

void UARTWrite(unsigned int writeMessage);
void UARTRead(void);

int main(void) {

    // Safety checks first
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    if (CALBC1_1MHZ==0xFF)                    // If calibration constant erased
    {
        while(1);                             // Do not load, trap CPU!!
    }

    // UART setup
    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
    DCOCTL = CALDCO_1MHZ;
    P1SEL = BIT1 + BIT2;                      // P1.1 = RXD, P1.2 = TXD
    P1SEL2 = BIT1 + BIT2;
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 8;                              // 1MHz 115200
    UCA0BR1 = 0;                              // 1MHz 115200
    UCA0MCTL = UCBRS2 + UCBRS0;               // Modulation UCBRSx = 5
    UCA0CTL1 &= ~UCSWRST;                     // Initialise USCI state machine
    IE2 |= UCA0RXIE;                          // Enable USCI A0 RX interrupt

    while(1){

        if (msgComplete == 1){
            UARTRead();

        }
    }

}

// Read characters from RX buffer, save as string, parse for JSON object
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

// Write JSON message characters to TX buffer
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void){

    UCA0TXBUF = outMsg[i++];                 // Read next char of message into TX buffer

    if (i == sizeof outMsg-1){               // If end of message, disable TX interrupt
        IE2 &= ~UCA0TXIE;
    }
}



void UARTWrite(unsigned int writeMessage){

    // Convert to JSON message, ? as delimiter
    outMsg = "{\"array[0][writeMessage]\":\"" + array[1][writeMessage] + "\"}";

    IE2 |= UCA0TXIE;                        // Enable TX interrupt

}

/* Parses RX buffer from a JSON string to a struct using the JSMN library and activates
 * the relevant input and output flags
 *
 */
void UARTRead(void){

    // JSMN library use
    jsmn_parser parser;             // Create a parser instance
    size_t tokenCount = 8;          // Number of potential tokens (should be maximum 5, for DeviceConfig message)
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
        // Get ButtonPushed (bool)
        // Get Switch (bool)
        // Get Potentiometer (bool)
        // Get Temperature (bool)
    }

    msgComplete = 0;            // Mark as done

}

