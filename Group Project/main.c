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
const int step_size = 50;               //value to indicate step size

//temp stuff
signed int orig_temp = 0;              //Seting up value for original potentiometer vale
signed int value_temp = 0;             //value on check
unsigned int temp = 0;
signed long change_t = 0;               //value to check how much the pot has changed
const int step_size_t = 5;              //value to indicate step size
unsigned int stable = 0;                //boolean for determining if stable Vref
unsigned int sample_temp = 0;           //boolean to be used to indicate a sample should be taken
const int sample_temp_time = 16384;     //used for sample time of the temperature
//Booleans for the components
//inputs
unsigned int button = 0;                //Button
unsigned int b_activated = 0;           //boolean for denoting button activation
unsigned int pot = 0;                   //Potentiometer
unsigned int pot_activated = 0;           //boolean for denoting potentiometer activation
unsigned int thermometer = 0;                  //thermometer
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
	P1DIR |= BIT0 + BIT6;      // P1.0, P1.6 output
	button = 1;
	thermometer = 1;

	__bis_SR_register(GIE);                     //interrupt enabled

    while(1){
        //read messages here

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

        //need checks to see if potentiometer was requested
        if (pot){
            /*if(!pot_activated){ //if not already activated
                ADC12CTL0 = ADC12ON + CONSEQ_2;            // ADC10ON, single channel repeat mode
                ADC12CTL1 = INCH_1 + ADC12SSEL_1 + CSTARTADD_0;          // input A1, clock = ACLK, MEM0
                ADC12AE0 |= BIT1;                          // PA.1 ADC option select

                //get original value
                ADC12CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
                orig_val = ADC12MEM0;
                pot_activated = 1;
            }

            ADC12CTL0 |= ENC + ADC12SC;             // Sampling and conversion start
            //ADC10MEM = conversion result
            value = ADC12MEM0;           //Just to make sure the value doesn't change during checking

            if(value != orig_val){ //if value has changed
                //check by how much
                change = abs(orig_val - value);

                if (change >= step_size){
                    //send message
                    //testing light
                    P1OUT ^= BIT6;
                }

                orig_val = value; //change to new orig value

            }
            //Code here for the potentiometer, so basically send stuff at interval checks*/
        } else {
            pot_activated = 0;

        }

        //check if thermometer was requested
        if(thermometer){
            //based on example adc10_temp.c from URL: http://dev.ti.com/tirex/explore/node?node=AEldOKIXgnT979MckUlLRw__IOGqZri__LATEST
            if(!t_activated){
                ADC10CTL0 = ADC10ON + SREF_1 + REFON;   // ADC10ON, vr+ = Vref+f, Vr- = Vss, reference on,
                ADC10CTL1 = INCH_10 + ADC10SSEL_1;           // temp sensor, clock = ACLK
                ADC10AE0 |= BIT1;                          // PA.1 ADC option select

                //set up a timer for stabilisation
                TA1CTL = TASSEL_1 + MC_2;          // ACLK, continuous mode
                TA1CCTL0 |= CCIE;
                TA1CCR0 = 10;                           //need a 30microsecond delay calculation time was 0.49, putting 10 to test

                t_activated = 1;
            }

            if (sample_temp){
                ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
                //based on code for pot above
                //ADC10MEM = conversion result
                temp = ADC10MEM;           //Just to make sure the value doesn't change during checking
                value_temp = ((temp - 673) * 423) / 1024;

                if(value_temp != orig_temp){ //if value has changed
                    //check by how much
                    change_t = abs(orig_temp - value_temp);

                    if (change_t >= step_size_t){
                        //send message
                        //testing light
                        P1OUT ^= BIT6;
                    }
                }
                orig_temp = value_temp; //change to new orig value
                //set sample to false
                sample_temp = 0;
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

//Interrupt for time A1
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void)
{
    if(thermometer){
        if (!stable){
            stable = 1;
            //get original value, if possible here
            ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
            temp = ADC10MEM;
            // oC = ((A10/1024)*1500mV)-986mV)*1/3.55mV = A10*423/1024 - 278, taken from example
            orig_temp = ((temp - 673) * 423) / 1024;

        } else {
            sample_temp = 1;        //start a sample
        }

        TA1CCR0 += sample_temp_time;
    }
}

//testing if it will stop the trap
/*
#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer1_A1(void){
    switch(TA0IV)
    {
    case  2:
            break;

    case 10: break;                             //Timer_A3 overflow, don't need to do anything

    }
}
*/
