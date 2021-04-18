#include <msp430.h> 


/**
 * main.c for Group 4
 *
 *
 * based on previous challenges and examples from TI code
 */

//button stuff
unsigned int timerCount = 0;            //initial count for the wait time
unsigned int pressed = 0;               //checking for pressed state, used for debouncing
unsigned int held = 0;                  //checking if the button is being pressed/held
const int debounce_b = 328;             //count for the 50Hz, based on 32768/(2*x)= 50, want a 20ms wait for the debouncing, 1/20*10^-3 = 50
const int count_b = 655;                //count for the count for presses, rate of 20Hz, based on 32768/(2x) = 20Hz
int activated_button[];                 //indicator for activated timer for the button

//potentiometer stuff
unsigned int orig_val = 0;              //Setting up value for original potentiometer vale
unsigned int value = 0;                 //value on check
signed int change = 0;                  //value to check how much the pot has changed
const int step_size = 10;               //value to indicate step size

//temp stuff
signed int orig_temp = 0;              //Setting up value for original potentiometer vale
signed int value_temp = 0;             //value on check
unsigned int temp = 0;
signed long change_t = 0;               //value to check how much the pot has changed
const int step_size_t = 5;              //value to indicate step size
unsigned int stable = 0;                //boolean for determining if stable Vref
unsigned int sample_temp = 0;           //boolean to be used to indicate a sample should be taken
const int sample_temp_time = 16384;     //used for sample time of the temperature
int activated_temp[];                   //indicator for activated timer for the temperature

//LED1 stuff
unsigned int blink_rate_1 = 0;          //rate for the blinking light of LED D1
int activated_led1[];     //Array for activated timers for LED D1

//LED2 stuff
unsigned int blink_rate_2 = 0;          //rate for the blinking light of LED D2
int activated_led2[0];     //Array for activated timers for LED D2

//LED3 stuff
//define all the colours
#define RED (BIT1)
#define GREEN (BIT3)
#define BLUE (BIT5)
#define PURPLE (BIT1 + BIT5)
#define WHITE (BIT1 + BIT3 + BIT5)
const int colours[] = {RED, GREEN, BLUE, PURPLE, WHITE};    //Array for the colours
unsigned int colour = 0;                //The colour selected
unsigned int blink_rate_3 = 0;          //rate for the blinking light for LED D3, or rotating
int activated_led3[];     //Array for activated timers for the color led

//Booleans for the components
//inputs
signed int button = 0;                  //Button
unsigned int b_activated = 0;           //boolean for denoting button activation
unsigned int pot = 0;                   //Potentiometer
unsigned int pot_activated = 0;         //boolean for denoting potentiometer activation
signed int thermometer = 0;             //thermometer
unsigned int t_activated = 0;           //boolean for denoting thermometer activation
unsigned int switch1 = 0;               //switch 1
unsigned int sw1_activated = 0;         //boolean for denoting switch 1 activation
unsigned int switch2 = 0;               //switch 2
unsigned int sw2_activated = 0;         //boolean for denoting switch 2 activation
//outputs
unsigned int led1_on = 0;               //LED D1 on, 0 indicate off for all of these
unsigned int led1_active = 0;           //check if the light has been activated - can only have one setting on at a time
unsigned int led1_blink = 0;            //LED1 blinking light
unsigned int led1_fade_in = 0;          //LED1 fade in
unsigned int led1_fade_out = 0;         //LED1 fade out
unsigned int led1_breath = 0;           //LED1 blinking light
unsigned int led2_on = 0;               //LED D2 on
unsigned int led2_active = 0;           //LED2 active
unsigned int led2_blink = 0;            //LED2 blink
unsigned int led2_fade_in = 0;          //LED2 fade in
unsigned int led2_fade_out = 0;         //LED2 fade out
unsigned int led2_breath = 0;           //LED2 blinking light
unsigned int led3_on = 0;               //LED D3 on, colour light
unsigned int led3_active = 0;           //LED3 active
unsigned int led3_rot = 0;              //rotate led3
unsigned int led3_dir = 0;              //LED3 direction of rotation
unsigned int led3_blink = 0;            //LE3 blink
unsigned int led3_fade_in = 0;          //LED3 fade in
unsigned int led3_fade_out = 0;         //LED3 fade out
unsigned int led3_breath = 0;           //LED3 blinking light
unsigned int buzzer_sus = 0;            //buzzer sustained tone
unsigned int buzzer_beep = 0;           //buzzer beeping

//array for timer usage, 0 if not used, 1 if used 0, pos 0:TA0_0, 1:TA0_1, 2:TA0_2, 3:TA1_0, 4: TA1_1, 5:TA1_2
unsigned int timers_used[6] = {0, 0, 0, 0, 0, 0};
unsigned int timer0_activated = 0;      //Boolean used for checking if TA0 has been activated
unsigned int timer1_activated = 0;      //Boolean used for checking if TA1 has been activated
signed int activated_timers[3] = {-1, -1, -1};   //array for activated timers for each activation

//array for used lights If 1 means active, if 0 not active, Returns array of status for each light, pos0 = D1, pos1 = D2, pos3 = D3
unsigned int lights_used[3] = {0, 0, 0};

//declare helper functions
int activate_timer(int timer_no, int count1, int count2);       //function for activating a specific timer
int activate_free_timer(int registers, int counts[]);           //function for activating required number of free timers
void deactivate_timer(int activated[], int len);                 //deactivate unused timers

//main function
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	//stuff for testing
	P1DIR |= BIT0 + BIT6;      // P1.0, P1.6 output
	//P2DIR |= BIT1;             //utilise D3
	//button = 1;
	//pot = 1;
	//thermometer = 1;

	__bis_SR_register(GIE);                     //interrupt enabled

    while(1){
        //read messages here


        //input
        //check if button was requested
        if (button != -1){
            if (!b_activated){ //if not already activated
                //button set up, from slides and previous challenge
                P1OUT |= BIT3;                            //pull up resistor on P1.3
                P1REN |= BIT3;                            //Enable it
                P1IES |= BIT3;                            //high to low edge
                P1IFG &= ~BIT3;                           // Clear flag
                P1IE |= BIT3;                             //Interrupt enable

                int counts[1] = {debounce_b};
                b_activated = activate_free_timer(1, counts);
                //record which ones were activated
                activated_button[0] = activated_timers[0];
                button = activated_button[0];                //assign to check properly for timer interrupts
                if (b_activated != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                }
            }
        } else {
            b_activated = 0;                        //if button is no longer requested
            deactivate_timer(activated_button, 2);   //turn off timer
            P1IE &= ~BIT3;                          //and button interrupt
        }

        //need checks to see if potentiometer was requested
        if (pot){
            if(!pot_activated){ //if not already activated
                ADC10CTL0 = ADC10ON + CONSEQ_0;            // ADC10ON, single channel single sample
                ADC10CTL1 = INCH_1 + ADC10SSEL_1;          // input A1, clock = ACLK
                ADC10AE0 |= BIT1;                          // PA.1 ADC option select

                //get original value
                ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
                orig_val = ADC10MEM;
                pot_activated = 1;
            }

            ADC10CTL1 = INCH_1;                     //Ensure right channel
            ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
            //ADC10MEM = conversion result
            //wait for result completion, triggers interrrupt when done, ADC10Iflag
            while ((ADC10CTL1 & ADC10BUSY) == 1) {//check if busy
                //Wait for sample to take place
            }
            value = ADC10MEM;           //Just to make sure the value doesn't change during checking
            //reset ENC
            ADC10CTL0 &= ~ENC;

            if(value != orig_val){ //if value has changed
                //check by how much
                change = abs(orig_val - value);

                if (change >= step_size){
                    //send message
                    //testing light
                    P2OUT ^= BIT1;
                }

                orig_val = value; //change to new orig value

            }
            //Code here for the potentiometer, so basically send stuff at interval checks
        } else {
            pot_activated = 0;
        }

        //check if thermometer was requested
        if(thermometer){
            //based on example adc10_temp.c from URL: http://dev.ti.com/tirex/explore/node?node=AEldOKIXgnT979MckUlLRw__IOGqZri__LATEST
            if(!t_activated){
                ADC10CTL0 = ADC10ON + SREF_1 + REFON;      // ADC10ON, vr+ = Vref+f, Vr- = Vss, reference on,
                ADC10CTL1 = INCH_10 + ADC10SSEL_1;         // temp sensor, clock = ACLK
                ADC10AE0 |= BIT1;                          // PA.1 ADC option select

                //set up a timer for stabilisation, use A1
                //need a 30microsecond delay calculation time was 0.49, putting 10 to test, need only 1 timer
                int counts[1] = {10};
                t_activated = activate_free_timer(1, counts);
                activated_temp[0] = activated_timers[0];      //record which ones were activated
                thermometer = activated_temp[0];              //Set for proper check in interrupts
                if (t_activated != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                }
            }

            if (sample_temp){
                ADC10CTL1 = INCH_10;                //Ensure right channel
                ADC10CTL0 |= ENC + ADC10SC;         // Sampling and conversion start
                //based on code for pot above
                //ADC10MEM = conversion result
                while ((ADC10CTL1 & ADC10BUSY) == 1) {//check if busy
                    //Wait for sample to take place
                }
                temp = ADC10MEM;           //Just to make sure the value doesn't change during checking
                //reset ENC
                ADC10CTL0 &= ~ENC;
                value_temp = ((temp - 673) * 423) / 1024;

                if(value_temp != orig_temp){ //if value has changed
                    //check by how much
                    change_t = abs(orig_temp - value_temp);

                    if (change_t >= step_size_t){
                        //send message
                        //testing light
                        //P1OUT ^= BIT6;
                    }
                }
                orig_temp = value_temp; //change to new orig value
                //set sample to false
                sample_temp = 0;
            }


        } else {
            t_activated = 0;
            deactivate_timer(activated_temp, 1);                      //turn off timer TA0
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

        //Outputs
        //LED1
        //on/off
        if(led1_on){
            if (!led1_active) {
              P1DIR |= BIT0;        // P1.0 output
              //toggle LED D1
              P1OUT ^= BIT0;
            }

        } else {
            //P1OUT &= ~BIT0;         //turn light off
            led1_active = 0;        //not active anymore
        }

        //blinking
        if(led1_blink){
            if(!led1_active){
                P1DIR |= BIT0;
                //need a timer for the blinking, only 1
                int counts[1] = {blink_rate_1};
                led1_active = activate_free_timer(1, counts);
                activated_led1[0] = activated_timers[0]; //record which ones were activated
                if (led1_active != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                }
                //toggle LED D1
                P1OUT ^= BIT0;
            }

        } else {
            led1_active = 0;
            P1DIR &= ~BIT0;
            deactivate_timer(activated_led1, 1);
        }

        //LED2 on/off
        if(led2_on){
            if (!led2_active) {
              P1DIR |= BIT6;        // P1.6 output
              //toggle LED D2
              P1OUT ^= BIT6;
            }

        } else {
            P1OUT &= ~BIT6;         //turn light off
            led2_active = 0;        //not active anymore
        }

        //blinking
        if(led2_blink){
            if(!led2_active){
                P1DIR |= BIT6;
                //need a timer for the blinking, only 1
                int counts[1] = {blink_rate_1};
                led2_active = activate_free_timer(1, counts);
                activated_led2[0] = activated_timers[0]; //record which ones were activated
                if (led2_active != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                }
                //toggle LED D2
                P1OUT ^= BIT6;
            }

        } else {
            led2_active = 0;
            P1OUT &= ~ BIT6;
            deactivate_timer(activated_led2, 1);
        }

        //LED3 on/off
        if(led3_on){
            if (!led3_active) {
              P2DIR |= BIT1 + BIT3 + BIT5;        // P1.0 output
              //LED D3, on set colour
              P2OUT = colours[colour];
            }

        } else {
            P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
            led3_active = 0;                    //not active anymore
        }

        //circling
        if(led3_rot){
            if(!led3_active){
                P2DIR |= BIT1 + BIT3 + BIT5;
                //need a timer for the rotation speed, only 1
                int counts[1] = {blink_rate_3};
                led3_active = activate_free_timer(1, counts);
                activated_led3[0] = activated_timers[0]; //record which ones were activated
                if (led3_active != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                }
                //toggle LED D1
                P2OUT = colours[colour];
            }

        } else {
            led3_active = 0;
            P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
            deactivate_timer(activated_led3, 1);

        }
    }

}

//interrupts
//timer A0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
{
    if(button == 0){ //check that it is the button debouncing
       if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
            timerCount = 0;                     //Reset the timer count
            pressed = 0;                        //reset pressed
            held = 1;                           //Button is being held
            P1OUT ^= BIT6;

       } else {
           pressed = 0;                         //Button is not being pressed
       }
       //offset TA0CCR0 by the count number/period
       TA0CCR0 += count_b;
    }
    //check for how long button was pressed
    if ((button == 0) && (held == 1)) {
        if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
            timerCount += 1;

        } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
            //send timer count signal here
            //testing with light first
            P1OUT ^= BIT0;
            held = 0;                               //button has now been released
        }
        //offset TA0CCR0 by the count number/period
        TA0CCR0 += count_b;
    }

    //thermometer stuff
    if(thermometer == 0){
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

        TA0CCR0 += sample_temp_time;
    }
}

//interrupt for timer A0
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer0_A1(void){
    switch(TA0IV)
    {
    case  2:
        if(button == 1){ //check that it is the button debouncing
           if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                timerCount = 0;                     //Reset the timer count
                pressed = 0;                        //reset pressed
                held = 1;                           //Button is being held
                P1OUT ^= BIT6;

           } else {
               pressed = 0;                         //Button is not being pressed
           }
           //offset TA0CCR0 by the count number/period
           TA0CCR1 += count_b;
        }
        //check for how long button was pressed
        if ((button == 1) && (held == 1)) {
            if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
                timerCount += 1;

            } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
                //send timer count signal here
                //testing with light first
                P1OUT ^= BIT0;
                held = 0;                               //button has now been released
            }
            //offset TA0CCR0 by the count number/period
            TA0CCR1 += count_b;
        }

            //thermometer stuff
        if(thermometer == 1){
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

            TA0CCR1 += sample_temp_time;
        }
        break;
    case 4:
        if(button == 2){ //check that it is the button debouncing
           if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                timerCount = 0;                     //Reset the timer count
                pressed = 0;                        //reset pressed
                held = 1;                           //Button is being held
                P1OUT ^= BIT6;

           } else {
               pressed = 0;                         //Button is not being pressed
           }
           //offset TA0CCR0 by the count number/period
           TA0CCR2 += count_b;
        }
        //check for how long button was pressed
        if ((button == 2) && (held == 1)) {
            if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
                timerCount += 1;

            } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
                //send timer count signal here
                //testing with light first
                P1OUT ^= BIT0;
                held = 0;                               //button has now been released
            }
            //offset TA0CCR0 by the count number/period
            TA0CCR2 += count_b;
        }

        //thermometer stuff
        if(thermometer == 2){
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

            TA0CCR2 += sample_temp_time;
        }
        break;

    case 10: break;                             //Timer_A3 overflow, don't need to do anything

    }

}

//Interrupt for time A1
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void)
{
    //button stuff
    if(button == 3){ //check that it is the button debouncing
       if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
            timerCount = 0;                     //Reset the timer count
            pressed = 0;                        //reset pressed
            held = 1;                           //Button is being held
            P1OUT ^= BIT6;

       } else {
           pressed = 0;                         //Button is not being pressed
       }
       //offset TA0CCR0 by the count number/period
       TA1CCR0 += count_b;
    }
    //check for how long button was pressed
    if ((button == 3) && (held == 1)) {
        if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
            timerCount += 1;

        } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
            //send timer count signal here
            //testing with light first
            P1OUT ^= BIT0;
            held = 0;                               //button has now been released
        }
        //offset TA0CCR0 by the count number/period
        TA1CCR0 += count_b;
    }

    //thermometer stuff
    if(thermometer == 3){
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

//interrupt for timerA1
#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer1_A1(void){
    switch(TA1IV)
    {
    case  2:
        if(button == 4){ //check that it is the button debouncing
               if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                    timerCount = 0;                     //Reset the timer count
                    pressed = 0;                        //reset pressed
                    held = 1;                           //Button is being held
                    P1OUT ^= BIT6;

               } else {
                   pressed = 0;                         //Button is not being pressed
               }
               //offset TA0CCR0 by the count number/period
               TA1CCR1 += count_b;
            }
            //check for how long button was pressed
            if ((button == 4) && (held == 1)) {
                if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
                    timerCount += 1;

                } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
                    //send timer count signal here
                    //testing with light first
                    P1OUT ^= BIT0;
                    held = 0;                               //button has now been released
                }
                //offset TA0CCR0 by the count number/period
                TA1CCR1 += count_b;
            }
        break;
    case 4:
        if(button == 5){ //check that it is the button debouncing
           if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                timerCount = 0;                     //Reset the timer count
                pressed = 0;                        //reset pressed
                held = 1;                           //Button is being held
                P1OUT ^= BIT6;

           } else {
               pressed = 0;                         //Button is not being pressed
           }
           //offset TA0CCR0 by the count number/period
           TA1CCR2 += count_b;
        }
        //check for how long button was pressed
        if ((button == 5) && (held == 1)) {
            if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
                timerCount += 1;

            } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
                //send timer count signal here
                //testing with light first
                P1OUT ^= BIT0;
                held = 0;                               //button has now been released
            }
            //offset TA0CCR0 by the count number/period
            TA1CCR2 += count_b;
        }

        //thermometer stuff
        if(thermometer == 5){
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

            TA1CCR2 += sample_temp_time;
        }
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
        //debouncer timer, based on current count, check which one was used
        switch(button){
        case 0:
            TA0CCR0 = TAR + debounce_b;         //Current count plus the wanted debounce part
            break;
        case 1:
            TA0CCR1 = TAR + debounce_b;
            break;
        case 2:
            TA0CCR2 = TAR + debounce_b;
            break;
        case 3:
            TA1CCR0 = TAR + debounce_b;
            break;
        case 4:
            TA1CCR1 = TAR + debounce_b;
            break;
        case 5:
            TA1CCR2 = TAR + debounce_b;
            break;
        }

        P1IFG &= ~BIT3;                             //clear flag
    }


}


//Helper functions
//function for activating a specific timer
int activate_timer(int timer_no, int count1, int count2){
    if(timer_no == 0){
        TA0CTL = TASSEL_1 + MC_2 + TAIE;          // ACLK, continuous mode, interrupt enabled
        TA0CCTL0 |= CCIE;                         // TA0CCR0 interrupt enabled
        if(count2 > 0){
            TA0CCTL1 |= CCIE;                         // TA0CCR1 interrupt enabled
            TA0CCR0 = count1;
            TA0CCR1 = count2;
            //set them to being used
            timers_used[0] = count1;
            timers_used[1] = count2;
        } else {
            TA0CCR0 = count1;

            timers_used[0] = count1;
        }
        timer0_activated = 1;
        return 1;
    } else if (timer_no == 1){
        TA1CTL = TASSEL_1 + MC_2 + TAIE;          // ACLK, continuous mode, interrupt enabled
        TA1CCTL0 |= CCIE;
        if(count2 > 0){
            TA1CCTL1 |= CCIE;
            TA1CCR0 = count1;
            TA1CCR1 = count2;

            timers_used[3] = count1;
            timers_used[4] = count2;
        } else {
            TA1CCR0 = count1;
            timers_used[3] = count1;
        }

        timer1_activated = 1;
        return 1;
    }

    return 0;

}

//function for checking which timer is free,
int activate_free_timer(int registers, int counts[]){
    //check which registers are free, if less than the register amount, return error
    int free[6] = {};
    unsigned int index = 0;
    unsigned int i;
    for(i=0; i < 6; i++){
       if((timers_used[i] == 0) || (timers_used[i] == counts[index])){
           free[index] = i;
           index++;
       }

    }
    //reset activated timers
    for(i=0; i<3; i++){
        activated_timers[i] = -1;
    }


    if(index < registers){
        //return error
        return 0;
    } else { //right number was found, activate them
        for (i=0; i < registers + 1; i++){
            switch (free[i]){
            case 0:
                //activate register A0_0, check if A0 is already active
                if(!timer0_activated){
                    activate_timer(0, counts[i], 0);
                } else if(timers_used[0] == 0) {
                    TA0CCTL0 |= CCIE;
                    TA0CCR0 = counts[i];
                } //if timer was used continue

                //set it as active
                timers_used[0] = counts[i];
                //set it as activated
                activated_timers[i] = 0;
                break;
            case 1:
                //activate A0_1
                //if A0_0 wasn't available then A0 was already activated so don't need to check again
                if(timers_used[1] == 0){
                    TA0CCTL1 |= CCIE;
                    TA0CCR1 = counts[i];
                }
                //set it as active
                timers_used[1] = counts[i];
                //set it as activated
                activated_timers[i] = 1;
                break;
            case 2:
                //activate A0_2
                if(timers_used[2] == 0){
                    TA0CCTL2 |= CCIE;
                    TA0CCR2 = counts[i];
                }
                //set it as active
                timers_used[2] = counts[i];
                //set it as activated
                activated_timers[i] = 2;
                break;
            case 3:
                //activate A1_0check if A1 is already active
                if(!timer1_activated){
                    activate_timer(1, counts[i], 0);
                } else if(timers_used[3] == 0) {
                    TA1CCTL0 |= CCIE;
                    TA1CCR0 = counts[i];
                }
                //set it as active
                timers_used[3] = counts[i];
                //set it as activated
                activated_timers[i] = 3;
                break;
            case 4:
                //activate A1_1
                if(timers_used[4] == 0){
                   TA1CCTL1 |= CCIE;
                   TA1CCR1 = counts[i];
                }

                //set it as active
                timers_used[4] = counts[i];
                //set it as activated
                activated_timers[i] = 4;
                break;
            case 5:
                //activate A1_2
                if(timers_used[4] == 0){
                    TA1CCTL2 |= CCIE;
                    TA1CCR2 = counts[i];
                }
                //set it as active
                timers_used[5] = counts[i];
                //set it as activated
                activated_timers[i] = 5;
                break;
            default:
                return 0;
            }
        }
        return 1;
    }

}

//function to turn off the timers activated - or set the count value to 0, check if each value is now zero so the full timer should be turned off
void deactivate_timer(int activated[], int len){
    unsigned int i;
    for(i=0; i < len; i++){
        int deactivate = activated[i];

        switch (deactivate){ //deactivate specific timer count register
        case 0:
            TA0CCR0 = 0;
            timers_used[0] = 0;
            break;
        case 1:
            TA0CCR1 = 0;
            timers_used[1] = 0;
            break;
        case 2:
            TA0CCR2 = 0;
            timers_used[2] = 0;
            break;
        case 3:
            TA1CCR0 = 0;
            timers_used[3] = 0;
            break;
        case 4:
            TA1CCR1 = 0;
            timers_used[4] = 0;
            break;
        case 5:
            TA1CCR2 = 0;
            timers_used[5] = 0;
            break;
        }
    }

    //check if all timers for A0 are 0
    int all_0 = 0;
    for(i=0; i<3; i++){
        if(timers_used[i] == 0){
            all_0++;
        }
    }

    if(all_0 == 3){
        TA0CTL = MC_0;      //turn off A0
        timer0_activated = 0;
    }

    //check if all registers for A1 is 0
    int all_1 = 0;
        for(i=3; i<6; i++){
            if(timers_used[i] == 0){
                all_1++;
            }
        }

        if(all_1 == 3){
            TA1CTL = MC_0;      //turn off A0
            timer1_activated = 0;
        }
}



