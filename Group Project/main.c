#include <msp430.h> 


/**
 * main.c for Group 4
 *
 * Written by Amanda Hellstenius,
 *
 */

//button stuff
unsigned int timerCount = 0;            //initial count for the wait time
unsigned int pressed = 0;               //checking for pressed state, used for debouncing
unsigned int held = 0;                  //checking if the button is being pressed/held
const int debounce = 328;               //count for the 50Hz, based on 32768/(2*x)= 50, want a 20ms wait for the debouncing, 1/20*10^-3 = 50
const int count = 655;                  //count for the count, rate of 20Hz, based on 32768/(2x) = 20Hz

//potentiometer stuff
unsigned int orig_val = 0;              //Seting up value for original potentiometer vale
unsigned int value = 0;                 //value on check
signed int change = 0;                  //value to check how much the pot has changed
const int step_size = 10;            //value to indicate step size
//Booleans for the components
//inputs
unsigned int button = 0;                //Button
unsigned int b_activated = 0;           //boolean for denoting button activation
unsigned int pot = 0;                   //Potentiometer
unsigned int pot_activated = 0;           //boolean for denoting potentiometer activation
unsigned int temp = 0;                  //thermometer
unsigned int t_activated = 0;           //boolean for denoting thermometer activation
unsigned int switch1 = 0;                //switch 1
unsigned int sw1_activated = 0;           //boolean for denoting switch 1 activation
unsigned int switch2 = 0;               //switch 2
unsigned int sw2_activated = 0;           //boolean for denoting switch 2 activation
//outputs
unsigned int led1 = 0;                  //LED D1
unsigned int led2 = 0;                  //LED D2
unsigned int led3 = 0;                  //LED D3
unsigned int buzzer = 0;                //buzzer

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	//stuff for testing
	P1DIR |= BIT0;      // P1.0 output
	button = 1;


	__bis_SR_register(GIE);                     //interrupt enabled

    while(1){
        //read messages here


        //need checks to see if potentiometer actually was requested
        if (pot){
            if(!pot_activated){ //if not already activated
                ADC10CTL0 = ADC10ON + CONSEQ_2;            // ADC10ON, single channel repeat mode
                ADC10CTL1 = INCH_1 + ADC10SSEL_1;          // input A1, clock = ACLK
                ADC10AE0 |= BIT1;                          // PA.1 ADC option select

                //get original value
                ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
                orig_val = ADC10MEM;
                pot_activated = 1;
            }

            ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
            //ADC10MEM = conversion result
            value = ADC10MEM;           //Just to make sure the value doesn't change during checking

            if(value != orig_val){ //if value has changed
                //check by how much
                change = abs(orig_val - value);

                if (change >= step_size){
                    //send message
                }

                orig_val = value; //change to new orig value

            }
            //Code here for the potentiometer, so basically send stuff at interval checks
        } else {
            pot_activated = 0;

        }

        //check if button was requested
        if (button){
            if (!b_activated){ //if not already activated
                //button set up, from slides and previous challenge
                P1OUT |= BIT3;                            //pull up resistor on P1.3
                P1REN |= BIT3;                            //Enable it
                P1IES |= BIT3;                            //high to low edge
                P1IFG &= ~BIT3;                           // Clear flag
                P1IE |= BIT3;                             //Interrupt enable

                TA0CTL = TASSEL_1 + MC_2 + TAIE;          // ACLK, continuous mode, interrupt enabled
                TA0CCTL0 |= CCIE;                         // TA0CCR0 interrupt enabled
                TA0CCTL1 |= CCIE;                         // TA0CCR1 interrupt enabled
                TA0CCR0 = count;
                TA0CCR1 = debounce;

                b_activated = 1;                //has now been activated
            }
        } else {
            b_activated = 0;                    //if button is no longer requested
            TA0CTL &= ~TAIE;                     //stop timer interrupt on TA0
            P1IE &= ~BIT3;                       //and button interrupt
        }

        //check if thermometer was requested
        if(temp){
            if(!t_activated){

                t_activated = 1;
            }
        } else {
            t_activated = 0;
        }

        //check switch 1
        if(switch1){
            if(!sw1_activated){

                sw1_activated = 1;
            }
        } else {
            sw1_activated = 0;
        }

        //check switch 2
        if(switch2){
            if(!sw2_activated){

                sw2_activated = 1;
            }
        } else {
            sw2_activated = 0;
        }

        //LED1
        if(led1){
            P1DIR |= BIT0;      // P1.0 output
            //toggle LED D1
            P1OUT ^= BIT0;
        }

    }

}

//interrupts
//button:
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
{
    if((held == 1) && !(P1IN & BIT3)) {
        timerCount += 1;
    } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
        //send timer count signal here
        //testing with light first
        P1OUT ^= BIT0;
        held = 0;                               //button has now been released

    }

    //offset TA0CCR0 by the count number/period
    TA0CCR0 += count;
}

//interrupt for the debouncing timer, also based on TI example 8
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer0_A1(void){
    switch(TA0IV)
    {
    case  2:
        if(button){ //check that it is the button debouncing
           if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                timerCount = 0;                     //Reset the timer count
                pressed = 0;                        //reset pressed
                held = 1;                           //Button is being held

           } else {
               pressed = 0;                         //Button is not being pressed
           }
               //noted that it needed to run constantly to be more exact
                TA0CCR1 += debounce;
        }

        //does switches need debouncing?

            break;

    case 10: break;                             //Timer_A3 overflow, don't need to do anything

    }

}

//Interrupt for the button, based on the slides/previous task
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void){
    //if it was the button that was pressed
    if(P1IFG & BIT3){
        pressed = 1;                            //Turn on flag for debouncer/check if button was pressed properly
        //debouncer timer, based on current count
        TA0CCR1 = TAR + debounce;               //Current count plus the wanted debounce part
    }

    P1IFG &= ~BIT3;                             //clear flag

}

