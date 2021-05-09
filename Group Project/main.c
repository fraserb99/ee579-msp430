#include <msp430.h> 


/**
 * main.c for Group 4
 *
 *
 * based on previous challenges and examples from TI code
 */

//Global variables and constants
//inputs
//button stuff
signed int button = -1;                         //Button variable, depends on timer used
unsigned int b_activated = 0;                   //boolean for denoting button activation
unsigned int timerCount = 0;                    //initial count for the wait time
unsigned int pressed = 0;                       //checking for pressed state, used for debouncing
unsigned int held = 0;                          //checking if the button is being pressed/held
const int debounce_b = 328;                     //count for the 50Hz, based on 32768/(2*x)= 50, want a 20ms wait for the debouncing, 1/20*10^-3 = 50
const int count_b = 820;                        //count for the count for presses, rate of 20Hz, based on 32768/(2x) = 20Hz
int activated_button[];                         //indicator for activated timer for the button

//Button 2 (called switch
signed int button2 = -1;                        //Button 2 variable
unsigned int b2_activated = 0;                  //button 2 activation
unsigned int timerCount2 = 0;                   //initial count for the wait time
unsigned int pressed2 = 0;                      //checking for pressed state, used for debouncing
unsigned int held2 = 0;                         //checking if the button is being pressed/held
const int debounce_b2 = 164;                    //count for the 100Hz, based on 32768/(2*x)= 100, want a 10ms wait for the debouncing, 1/20*10^-3 = 50
const int count_b2 = 820;                       //count for the count for presses, rate of 20Hz, based on 32768/(2x) = 20Hz
int activated_button2[];                        //indicator for activated timer for the button

//potentiometer stuff
unsigned int pot = 0;                           //Potentiometer use
unsigned int pot_activated = 0;                 //boolean for denoting potentiometer activation
unsigned int orig_val = 0;                      //Setting up value for original potentiometer vale
unsigned int value = 0;                         //value on check
signed int change = 0;                          //value to check how much the pot has changed
const int step_size = 100;                       //value to indicate step size

//temp stuff
signed int thermometer = -1;                    //thermometer use
unsigned int t_activated = 0;                   //boolean for denoting thermometer activation
signed int orig_temp = 0;                       //Setting up value for original potentiometer vale
signed int value_temp = 0;                      //value on check
unsigned int temp = 0;
signed long change_t = 0;                       //value to check how much the pot has changed
const int step_size_t = 2;                      //value to indicate step size, as error is +- 2 step size is set to 2 for more stable operation
unsigned int stable = 0;                        //boolean for determining if stable Vref
unsigned int sample_temp = 0;                   //boolean to be used to indicate a sample should be taken
const int sample_temp_time = 16384;             //used for sample time of the temperature 16384
int activated_temp[];                           //indicator for activated timer for the temperature

//Outputs
//constants for breathing and fading lights, same for all LEDs independent of user defined speeds
const int max_brightness = 32;                  //used for checking max value for brightness and counter value
const int change_period = 16;                   //period for flashing and breathing, default values
const int brightness_register_1 = 4096;         //used for brightness increment timing, 1Hz, first setting
const int brightness_register_2 = 2048;         //used for brightness increment timing, 2Hz, second setting
const int brightness_register_3 = 2048;         //used for brightness increment timing, 4Hz, third setting
//LED1 stuff
signed int led1_on = 0;                         //LED D1 on, 0 indicate off, -1 - deactivate
unsigned int led1_active = 0;                   //check if the light has been activated - can only have one setting on at a time
unsigned int led1_fo_active = 0;                //own activation check for fade out as that can turn lights of that are on, if wanted
signed int led1_blink = -1;                     //LED1 blinking light
signed int led1_fade_in = -1;                   //LED1 fade in
signed int led1_fade_out = -1;                  //LED1 fade out
signed int led1_breath = -1;                    //LED1 blinking light
unsigned int blink_rate_1 = 16384;              //rate for the blinking light of LED D1, default period 1Hz
unsigned int light_flag_1 = 0;                  //breathing and fading light flag, brighter or darker,  brighter = 0, darker = 1;
unsigned int brightness_1 = 0;                  // Varies brightness between 0 and 32
unsigned int counter_val_1 = 0;                 //counter for the LED on time
unsigned int period_1 = 2048;                   //overall period for the fading and breathing light, default is 2Hz
int activated_led1[];                           //Array for activated timers for LED D1

//LED2 stuff
signed int led2_on = 0;                         //LED D2 on
unsigned int led2_active = 0;                   //LED2 active
unsigned int led2_fo_active = 0;                //own activation check for fade out as that can turn lights of that are on, if wanted
signed int led2_blink = -1;                     //LED2 blink
signed int led2_fade_in = -1;                   //LED2 fade in
signed int led2_fade_out = -1;                  //LED2 fade out
signed int led2_breath = -1;                    //LED2 breathing light
unsigned int blink_rate_2 = 16384;              //rate for the blinking light of LED D2, default period 1Hz
unsigned int light_flag_2 = 0;                  //breathing and fading light flag, brighter or darker,  brighter = 0, darker = 1;
unsigned int brightness_2 = 0;                  //varies brightness between 0 and 32
unsigned int counter_val_2 = 0;                 //LED counter
unsigned int period_2 = 2048;                   //period for flashing and breathing, default
int activated_led2[0];                          //Array for activated timers for LED D2

//LED3 stuff
signed int led3_on = 0;                         //LED D3 on/off, colour light
unsigned int led3_active = 0;                   //LED3 active
unsigned int led3_fo_active = 0;                //own activation check for fade out as that can turn lights of that are on, if wanted
signed int led3_rot = -1;                       //rotate led3
signed int led3_dir = 0;                        //LED3 direction of rotation, -1 for backwards and 1 for forwards
signed int led3_blink = -1;                     //LE3 blink
unsigned int led3_blink_on = 0;                 //used to indicate on or off light
signed int led3_fade_in = -1;                   //LED3 fade in
signed int led3_fade_out = -1;                  //LED3 fade out
signed int led3_breath = -1;                    //LED3 blinking light
//define all the colours
#define RED (BIT1)
#define GREEN (BIT3)
#define BLUE (BIT5)
#define YELLOW (BIT1 + BIT3)
#define PURPLE (BIT1 + BIT5)
#define CYAN (BIT3 + BIT5)
#define WHITE (BIT1 + BIT3 + BIT5)
const int colours[] = {RED, GREEN, BLUE, YELLOW, PURPLE, CYAN, WHITE};    //Array for the colours
unsigned int colour = 0;                        //The colour selected
unsigned int blink_rate_3 = 16384;              //rate for the blinking light for LED D3, or rotating, default 1Hz
signed int light_flag_3 = 0;                    //breathing and fading light flag, brighter or darker,  brighter = 1, darker = -1;
signed int brightness_3 = 0;                    //used for determining brightness, initially between 0 and 4
unsigned int flash_count_3 = 0;                 //LED counter
unsigned int counter_val_3 = 1;                  //duty cycle for breathing and flashing, default of 2048Hz
unsigned int period_3 = 2048;                    //period for flashing and breathing, default of 16Hz
int activated_led3[];                           //Array for activated timers for the colour led

//buzzer stuff
unsigned int buzzer_sus = 0;                    //buzzer sustained tone
unsigned int buzzer_beep = 0;                   //buzzer beeping

//array for timer usage, 0 if not used, 1 if used 0, pos 0:TA0_0, 1:TA0_1, 2:TA0_2, 3:TA1_0, 4: TA1_1, 5:TA1_2
unsigned int timers_used[6] = {0, 0, 0, 0, 0, 0};
unsigned int timer0_activated = 0;              //Boolean used for checking if TA0 has been activated
unsigned int timer1_activated = 0;              //Boolean used for checking if TA1 has been activated
signed int activated_timers[2] = {-1, -1};      //array for activated timers for each activation

//array for used lights If 1 means active, if 0 not active, Returns array of status for each light, pos0 = D1, pos1 = D2, pos3 = D3
unsigned int lights_used[3] = {0, 0, 0};
//arrays for breathing and fading lights

//declare helper functions
int activate_timer(int timer_no, int count1, int count2);       //function for activating a specific timer
int activate_free_timer(int registers, int counts[]);           //function for activating required number of free timers
void deactivate_timer(int activated[], int len);                //deactivate unused timers
int get_timer_code(int timers[]);                               //get the specific timer code for 2 timers


/* Description of codes used for the LEDs:
 * for those with 1 timer:
 * A0_0 = 0, A0_1 = 1, A0_2 = 2, A1_0 = 3, A1_1 = 4, A1_2 = 5
 *
 * For 2 timers
 * A0_0 +           A0_1 +          A0_2 +          A1_0 +          A1_1 +
 *      A0_1 = 10,      A0_2 = 15,      A1_0 = 19,      A1_1 = 22,      A1_2 = 24,
 *      A0_2 = 11,      A1_0 = 16,      A1_1 = 20,      A1_2 = 23,
 *      A1_0 = 12,      A1_1 = 17,      A1_2 = 21,
 *      A1_1 = 13,      A1_2 = 18,
 *      A1_2 = 14,
 *
 */

//main function
int main(void)
 {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	//stuff for testing
	P1DIR |= BIT0 + BIT6;      // P1.0, P1.6 output
	//P2DIR |= BIT1;             //utilise D3
	//button = 1;
	button2 = 1;
	//pot = 1;
	//thermometer = 1;
	//led1_blink = 1;
	//blink_rate_3 = 32768;
	//led3_on = 1;
	//led1_blink = 1;
	//led3_rot = 1;
	//led3_dir = 1;
	//led1_breath = 1;
    //led1_fade_in = 1;
	//led3_fade_out = 1;



	__bis_SR_register(GIE);                     //interrupt enabled

    while(1){
        //read messages here


        //input
        //check if button was requested
        if (button > -1){
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
        } else if(button == -2){
            b_activated = 0;                        //if button is no longer requested
            deactivate_timer(activated_button, 2);   //turn off timer
            P1IE &= ~BIT3;                          //and button interrupt
            P1REN |= BIT3;                          // and dissable the pullup/pulldown
            button = -1;
        }


        //check button 2, same code as button 1 pretty much, just different pins
        //Copied from button 1
        if (button2 > -1){
            if (!b2_activated){ //if not already activated
                P1OUT |= BIT5;                            //pull up resistor on P1.5
                P1REN |= BIT5;                            //Enable it
                P1IES |= BIT5;                            //high to low edge
                P1IFG &= ~BIT5;                           // Clear flag
                P1IE |= BIT5;                             //Interrupt enable

                int counts[1] = {debounce_b2};
                b2_activated = activate_free_timer(1, counts);
                //record which ones were activated
                activated_button2[0] = activated_timers[0];
                button2 = activated_button2[0];                //assign to check properly for timer interrupts
                if (b2_activated != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                }
            }
        } else if(button2 == -2){
            b2_activated = 0;                        //if button is no longer requested
            deactivate_timer(activated_button2, 2);   //turn off timer
            P1IE &= ~BIT5;                          //and button interrupt
            P1REN |= BIT5;                          // and dissable the pullup/pulldown
            button2 = -1;
        }

        //need checks to see if potentiometer was requested
        if (pot){
            if(!pot_activated){ //if not already activated
                ADC10CTL0 = ADC10ON + CONSEQ_0;            // ADC10ON, single channel single sample
                ADC10CTL1 = INCH_4 + ADC10SSEL_1;          // input A4, clock = ACLK
                ADC10AE0 |= BIT4;                          // PA.4 ADC option select

                //get original value
                ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
                orig_val = ADC10MEM;
                pot_activated = 1;
            }

            ADC10CTL0 |= SREF_0;                    //set ref to 0
            ADC10CTL1 |= INCH_4;                     //Ensure right channel
            ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
            ADC10AE0 |= BIT4;                          // PA.4 ADC option select
            //ADC10MEM = conversion result
            //wait for result completion, triggers interrupt when done, ADC10Iflag
            /*while ((ADC10CTL1 & ADC10BUSY) == 1) {//check if busy
                //Wait for sample to take place
            }*/
            value = ADC10MEM;           //Just to make sure the value doesn't change during checking
            //reset ENC
            ADC10CTL0 &= ~ENC;

            if(value != orig_val){ //if value has changed
                //check by how much
                change = abs(orig_val - value);

                if (change >= step_size){
                    //send message

                    //change to new orig value
                    orig_val = value;
                    //testing light
                    P1OUT ^= BIT6;
                }

            }
            //Code here for the potentiometer, so basically send stuff at interval checks
        } else {
            pot_activated = 0;
        }

        //check if thermometer was requested
        if(thermometer > -1){
            //based on example adc10_temp.c from URL: http://dev.ti.com/tirex/explore/node?node=AEldOKIXgnT979MckUlLRw__IOGqZri__LATEST
            if(!t_activated){
                ADC10CTL0 = ADC10ON + SREF_1 + REFON;      // ADC10ON, vr+ = Vref+f, Vr- = Vss, reference on,
                ADC10CTL1 = INCH_10 + ADC10SSEL_1;         // temp sensor, clock = ACLK
                //ADC10AE0 |= BIT1;                          // PA.1 ADC option select

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
                ADC10CTL0 |= SREF_1;                //Set ref to 1
                ADC10CTL1 |= INCH_10;                //Ensure right channel
                ADC10CTL0 |= ENC + ADC10SC;         // Sampling and conversion start
                ADC10AE0 |= BIT1;                          // PA.1 ADC option select
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
                        //change to new orig value
                        orig_temp = value_temp;
                        //testing light
                        P1OUT ^= BIT0;
                    }
                }

                //set sample to false
                sample_temp = 0;
            }


        } else if(thermometer == -2){
            t_activated = 0;
            deactivate_timer(activated_temp, 1);
            thermometer = -1;
        }


        //Outputs
        //LED1
        //on/off
        if(led1_on == 1){
            if (!led1_active) {
              P1DIR |= BIT0;        // P1.0 output
            }

            //turn on LED D1
            P1OUT |= BIT0;
            //set the value to 0
            led1_on = 0;

        } else if(led1_on == -1) {
            P1OUT &= ~BIT0;         //turn light off
            led1_active = 0;        //not active anymore
            led1_on = 0;
        }

        //blinking
        if(led1_blink > -1){
            if(!led1_active){
                P1DIR |= BIT0;
                //need a timer for the blinking, only 1
                int counts[1] = {blink_rate_1};
                led1_active = activate_free_timer(1, counts);
                activated_led1[0] = activated_timers[0]; //record which ones were activated
                led1_blink = activated_led1[0];
                if (led1_active != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                }
                //toggle LED D1
                P1OUT ^= BIT0;
            }

        } else if (led1_blink == -2){
            led1_active = 0;
            P1OUT &= ~BIT0;
            deactivate_timer(activated_led1, 1);
            led1_blink = -1;
        }

        //fade in
        if(led1_fade_in > -1){
            if(!led1_active){
                P1DIR |= BIT0;
                P1OUT &= ~BIT0; //don't want it on
                //need 2 timers for the fading in
                int counts[2] = {change_period, period_1};
                led1_active = activate_free_timer(2, counts);
                activated_led1[0] = activated_timers[0]; //record which ones were activated, first
                activated_led1[1] = activated_timers[1]; //record second one
                led1_fade_in = get_timer_code(activated_led1);
                if (led1_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                }

            }

        } else if (led1_fade_in == -2){
            //led1_active = 0; - light is still active
            deactivate_timer(activated_led1, 2);
            led1_fade_in = -1;
        }

        //fade out
        if(led1_fade_out > -1){
            if(!led1_fo_active){ //if light hasn't been activated, or if on
                P1DIR |= BIT0;
                //need 2 timers for the fading in
                int counts[2] = {change_period, period_1};
                led1_fo_active = activate_free_timer(2, counts);
                activated_led1[0] = activated_timers[0]; //record which ones were activated, first
                activated_led1[1] = activated_timers[1]; //record second one
                led1_fade_out = get_timer_code(activated_led1);
                if (led1_fo_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                } else {
                    led1_active = 1; //led is being used
                }
                //for testing
                //P1OUT ^= BIT6;
                //set the brightness to max
                brightness_1 = max_brightness;
                light_flag_1 = 1;
                //need to turn on a light first, if not already on
                P1OUT |= BIT0;
            }

        } else if (led1_fade_out == -2){
            led1_fo_active = 0;
            led1_active = 0;
            P1OUT &= ~BIT0;
            deactivate_timer(activated_led1, 2);
            led1_fade_out = -1;
        }

        //breathing
        if(led1_breath > -1){
            if(!led1_active){
                P1DIR |= BIT0;
                //need 2 timers for the breathing light
                int counts[2] = {change_period, period_1};
                led1_active = activate_free_timer(2, counts);
                activated_led1[0] = activated_timers[0]; //record which ones were activated, first
                activated_led1[1] = activated_timers[1]; //record second one
                led1_breath = get_timer_code(activated_led1);
                if (led1_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                }
            }

        } else if (led1_breath == -2){
            led1_active = 0;
            P1OUT &= ~BIT0;
            deactivate_timer(activated_led1, 2);
            led1_breath = -1;
        }

        //LED2 on/off
        if(led2_on == 1){
            if (!led2_active) {
              P1DIR |= BIT6;        // P1.6 output
            }
            //turn on LED D2
            P1OUT |= BIT6;
            led2_on = 0;

        } else if(led2_on == -1){
            P1OUT &= ~BIT6;         //turn light off
            led2_active = 0;        //not active anymore
            led2_on = 0;
        }

        //blinking
        if(led2_blink > -1){
            if(!led2_active){
                P1DIR |= BIT6;
                //need a timer for the blinking, only 1
                int counts[1] = {blink_rate_1};
                led2_active = activate_free_timer(1, counts);
                activated_led2[0] = activated_timers[0]; //record which ones were activated
                led2_blink = activated_led2[0];
                if (led2_active != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                }
                //toggle LED D2
                P1OUT ^= BIT6;
            }

        } else if (led2_blink == -2){
            led2_active = 0;
            P1OUT &= ~ BIT6;
            deactivate_timer(activated_led2, 1);
            led2_blink = -1;
        }

        //fade in
        if(led2_fade_in > -1){
            if(!led2_active){
                P1DIR |= BIT6;
                P1OUT &= ~BIT6; //don't want it on
                //need 2 timers for the fading in
                int counts[2] = {change_period, period_2};
                led2_active = activate_free_timer(2, counts);
                activated_led2[0] = activated_timers[0]; //record which ones were activated, first
                activated_led2[1] = activated_timers[1]; //record second one
                led2_fade_in = get_timer_code(activated_led2);
                if (led2_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                }
            }

        } else if (led2_fade_in == -2){
            //led2_active = 0; - light is still active
            //P1OUT &= ~BIT6;
            deactivate_timer(activated_led2, 2);
            led2_fade_in = -1;
        }

        //fade out
        if(led2_fade_out > -1){
            if(!led2_fo_active){ //if light hasn't been activated, or if on
                P1DIR |= BIT6;
                //need 2 timers for the fading in
                int counts[2] = {change_period, period_2};
                led2_fo_active = activate_free_timer(2, counts);
                activated_led2[0] = activated_timers[0]; //record which ones were activated, first
                activated_led2[1] = activated_timers[1]; //record second one
                led2_fade_out = get_timer_code(activated_led2);
                if (led2_fo_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                } else {
                    led2_active = 1; //led is being used
                }
                //for testing
                //P1OUT ^= BIT6;
                //set the brightness to max
                brightness_2 = max_brightness;
                light_flag_2 = 1;
                //need to turn on a light first, if not already on
                P1OUT |= BIT6;
            }

        } else if (led2_fade_out == -2){
            led2_fo_active = 0;
            led2_active = 0;
            P1OUT &= ~BIT6;
            deactivate_timer(activated_led2, 2);
            led2_fade_out = -1;
        }

        //breathing
        if(led2_breath > -1){
            if(!led2_active){
                P1DIR |= BIT6;
                //need 2 timers for the breathing light
                int counts[2] = {change_period, period_2};
                led2_active = activate_free_timer(2, counts);
                activated_led2[0] = activated_timers[0]; //record which ones were activated, first
                activated_led2[1] = activated_timers[1]; //record second one
                led2_breath = get_timer_code(activated_led2);
                if (led2_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                }
            }

        } else if (led2_breath == -2){
            led2_active = 0;
            P1OUT &= ~BIT6;
            deactivate_timer(activated_led2, 2);
            led2_breath = -1;
        }

        //LED3 on/off
        if(led3_on == 1){
            if (!led3_active) {
              P2DIR |= BIT1 + BIT3 + BIT5;        // P1.0 output
            }

            //LED D3, on set colour
            P2OUT = colours[colour];
            led3_on = 0;

        } else if(led3_on == -1){
            P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
            led3_active = 0;                    //not active anymore
            led3_on = 0;
        }

        //circling
        if(led3_rot > -1){
            if(!led3_active){
                P2DIR |= BIT1 + BIT3 + BIT5;
                //need a timer for the rotation speed, only 1
                int counts[1] = {blink_rate_3};
                led3_active = activate_free_timer(1, counts);
                activated_led3[0] = activated_timers[0]; //record which ones were activated
                led3_rot = activated_led3[0];
                if (led3_active != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                }
                //toggle LED D3, wanted colour
                P2OUT = colours[colour];
            }

        } else if(led3_rot == -2){
            led3_active = 0;
            P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
            deactivate_timer(activated_led3, 1);
            led3_rot = -1;

        }

        //blinking
        if(led3_blink > -1){
            if(!led3_active){
                P2DIR |= BIT1 + BIT3 + BIT5;
                //need a timer for the rotation speed, only 1
                int counts[1] = {blink_rate_3};
                led3_active = activate_free_timer(1, counts);
                activated_led3[0] = activated_timers[0]; //record which ones were activated
                led3_blink = activated_led3[0];
                if (led3_active != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //for testing
                    P1OUT ^= BIT6;
                }
                //toggle LED D3, with start colour
                P2OUT = colours[colour];
                //indicate on
                led3_blink_on = 1;
            }

        } else if(led3_blink == -2){
            led3_active = 0;
            P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
            deactivate_timer(activated_led3, 1);
            led3_blink = -1;
        }

        //fade in
        if(led3_fade_in > -1){
            if(!led3_active){
                P2DIR |= BIT1 + BIT3 + BIT5;
                P2OUT &= ~(BIT1 + BIT3 + BIT5); //don't want it on
                //need 2 timers for the fading in
                int counts[2] = {change_period, period_3};
                led3_active = activate_free_timer(2, counts);
                activated_led3[0] = activated_timers[0]; //record which ones were activated, first
                activated_led3[1] = activated_timers[1]; //record second one
                led3_fade_in = get_timer_code(activated_led3);
                if (led3_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                }
            }

        } else if (led3_fade_in == -2){
            //led3_active = 0;
            //P2OUT &= ~(BIT1 + BIT3 + BIT5);
            deactivate_timer(activated_led3, 2);
            led3_fade_in = -1;
        }

        //fade out
        if(led3_fade_out > -1){
            if(!led3_fo_active){ //if light hasn't been activated, or if on
                P2DIR |= BIT1 + BIT3 + BIT5;
                //need 2 timers for the fading in
                int counts[2] = {change_period, period_3};
                led3_fo_active = activate_free_timer(2, counts);
                activated_led3[0] = activated_timers[0]; //record which ones were activated, first
                activated_led3[1] = activated_timers[1]; //record second one
                led3_fade_out = get_timer_code(activated_led3);
                if (led3_fo_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                } else {
                    led3_active = 1; //led is being used
                }
                //for testing
                //P1OUT ^= BIT6;
                //set the brightness to max
                brightness_3 = max_brightness;
                light_flag_3 = 1;
                //need to turn on a light first, if not already on, on the specified colour
                P2OUT |= colours[colour];
            }

        } else if (led3_fade_out == -2){
            led3_fo_active = 0;
            led3_active = 0;
            P2OUT &= ~(BIT1 + BIT3 + BIT5);
            deactivate_timer(activated_led3, 2);
            led3_fade_out = -1;
        }

        //breathing
        if(led3_breath > -1){
            if(!led3_active){
                P2DIR |= BIT1 + BIT3 + BIT5;
                //need 2 timers for the breathing light
                int counts[2] = {change_period, period_3};
                led3_active = activate_free_timer(2, counts);
                activated_led3[0] = activated_timers[0]; //record which ones were activated, first
                activated_led3[1] = activated_timers[1]; //record second one
                led3_breath = get_timer_code(activated_led3);
                if (led3_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                }
            }

        } else if (led3_breath == -2){
            led3_active = 0;
            P2DIR &= ~(BIT1 + BIT3 + BIT5);
            deactivate_timer(activated_led3, 2);
            led3_breath = -1;
        }
    }


}

//interrupts
//timer A0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
{
    //check for how long button was pressed
    if ((button == 0) && (held == 1)) {
        if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
            timerCount += 1;

        } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
            //send timer count signal here
            //led1_fade_out = 1;
            //testing with light first
            //P1OUT ^= BIT6;
            held = 0;                               //button has now been released
        }
        //offset TA0CCR0 by the count number/period
        TA0CCR0 += count_b;
    } else if (button == 0){ //check that it is the button debouncing
        if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
             timerCount = 0;                     //Reset the timer count
             pressed = 0;                        //reset pressed
             held = 1;                           //Button is being held
             //P1OUT ^= BIT6;
             //offset TA0CCR0 by the count number/period
             //change the value held in the array
             timers_used[0] = count_b;
             TA0CCR0 += count_b;

        } else {
            pressed = 0;                         //Button is not being pressed
        }

     }

    //button 2, copied code
    if ((button2 == 0) && (held2 == 1)) {
        if ((held2 == 1) && !(P1IN & BIT5)) { //check how long its being held for
            timerCount2 += 1;

        } else if ((held2 == 1) && (P1IN & BIT5)){   //if button was released
            //send timer count signal here
            //testing with light first
            //P1OUT ^= BIT0;
            held2 = 0;                               //button has now been released
        }
        //offset TA0CCR0 by the count number/period
        TA0CCR0 += count_b2;
    } else if (button2 == 0){ //check that it is the button debouncing
        if ((pressed2 == 1) && !(P1IN & BIT5)){   //button was pressed properly
             timerCount2 = 0;                     //Reset the timer count
             pressed2 = 0;                        //reset pressed
             held2 = 1;                           //Button is being held
             //P1OUT ^= BIT6;
             //offset TA0CCR0 by the count number/period
             //change the value held in the array
             timers_used[0] = count_b2;
             TA0CCR0 += count_b2;

        } else {
            pressed2 = 0;                         //Button is not being pressed
        }

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
            timers_used[0] = sample_temp_time;
            TA0CCR0 += sample_temp_time;

        } else {
            sample_temp = 1;        //start a sample
        }

        TA0CCR0 += sample_temp_time;
    }

    //outputs
    //LED1 blinking
    if(led1_blink == 0){
        P1OUT ^= BIT0;
        TA0CCR0 += blink_rate_1;
    }

    //fading in/out and breathing, part 1, can be 10, 11, 12, 13, 14
    if(((led1_fade_in >= 10) && (led1_fade_in <= 14)) || ((led1_fade_out >= 10) && (led1_fade_out <= 14)) || ((led1_breath >= 10) && (led1_breath <= 14))){
        if(counter_val_1 < brightness_1){
            P1OUT |= BIT0;                              //Turn on light
        } else {
            P1OUT &= ~BIT0;
        }
        counter_val_1 += 1;                             //Increment the value to read from the array

        if(counter_val_1 >= max_brightness){                       //reached max value, reset
            counter_val_1 = 0;
        }
        TA0CCR0 += change_period;
    }

    //LED2 blinking
    if(led2_blink == 0){
        P1OUT ^= BIT6;
        TA0CCR0 += blink_rate_2;
    }

    //fading in/out and breathing, part 1, can be 10, 11, 12, 13, 14
    if(((led2_fade_in >= 10) && (led2_fade_in <= 14)) || ((led2_fade_out >= 10) && (led2_fade_out <= 14)) || ((led2_breath >= 10) && (led2_breath <= 14))){
        if(counter_val_2 < brightness_2){
            P1OUT |= BIT6;                              //Turn on light
        } else {
            P1OUT &= ~BIT6;
        }
        counter_val_2 += 1;                             //Increment the value to read from the array

        if(counter_val_2 >= max_brightness){                       //reached max value, reset
            counter_val_2 = 0;
        }
        TA0CCR0 += change_period;
    }

    //LED3 rotating
    if(led3_rot == 0){
        //verify boundaries
        if ((colour == 0) && (led3_dir == -1)){
            colour = 6;
        } else if ((colour == 6) && (led3_dir == 1)){
            colour = 0;
        } else {
            colour = colour + led3_dir; //change colour based on direction
        }
        //swap colour
        P2OUT = colours[colour];
        TA0CCR0 += blink_rate_3;
    }
    //LED3 blink
    if(led3_blink == 0){
        if(led3_blink_on){
            P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
            led3_blink_on = 0;
            TA0CCR0 += blink_rate_3;
        } else {
            if ((colour == 0) && (led3_dir == -1)){
                colour = 6;
            } else if ((colour == 6) && (led3_dir == 1)){
                colour = 0;
            } else {
                colour = colour + led3_dir; //change colour based on direction
            }
            led3_blink_on = 1;
            //swap colour
            P2OUT = colours[colour];
            TA0CCR0 += blink_rate_3;
        }
    }

    //fading in/out and breathing, part 1, can be 10, 11, 12, 13, 14
    if(((led3_fade_in >= 10) && (led3_fade_in <= 14)) || ((led3_fade_out >= 10) && (led3_fade_out <= 14)) || ((led3_breath >= 10) && (led3_breath <= 14))){
        if(counter_val_3 < brightness_3){
            P2OUT |= colours[colour];                              //Turn on light
        } else {
            P2OUT &= ~(BIT1 + BIT3 + BIT5);
        }
        counter_val_3 += 1;                             //Increment the value to read from the array

        if(counter_val_3 >= max_brightness){                       //reached max value, reset
            counter_val_3 = 0;
        }
        TA0CCR0 += change_period;
    }
}

//interrupt for timer A0
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer0_A1(void){
    switch(TA0IV)
    {
    case  2:
        //check for how long button was pressed
        if ((button == 1) && (held == 1)) {
            if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
                timerCount += 1;

            } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
                //send timer count signal here
                led3_rot = 1;
                //testing with light first
               // P1OUT ^= BIT0;
                held = 0;                               //button has now been released
            }
            //offset TA0CCR1 by the count number/period
            TA0CCR1 += count_b;
        } else if (button == 1){ //check that it is the button debouncing
            if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                 timerCount = 0;                     //Reset the timer count
                 pressed = 0;                        //reset pressed
                 held = 1;                           //Button is being held
                 P1OUT ^= BIT6;
                 //offset TA0CCR0 by the count number/period
                 //change the value held in the array
                 timers_used[0] = count_b;
                 TA0CCR1 += count_b;

            } else {
                pressed = 0;                         //Button is not being pressed
            }

         }

        //button 2, copied code
        if ((button2 == 1) && (held2 == 1)) {
            if ((held2 == 1) && !(P1IN & BIT5)) { //check how long its being held for
                timerCount2 += 1;

            } else if ((held2 == 1) && (P1IN & BIT5)){   //if button was released
                //send timer count signal here
                //testing with light first
                P1OUT ^= BIT0;
                held2 = 0;                               //button has now been released
            }
            //offset TA0CCR1 by the count number/period
            TA0CCR1 += count_b2;
        } else if (button2 == 1){ //check that it is the button debouncing
            if ((pressed2 == 1) && !(P1IN & BIT5)){   //button was pressed properly
                 timerCount2 = 0;                     //Reset the timer count
                 pressed2 = 0;                        //reset pressed
                 held2 = 1;                           //Button is being held
                 P1OUT ^= BIT0;
                 //offset TA0CCR0 by the count number/period
                 //change the value held in the array
                 timers_used[0] = count_b2;
                 TA0CCR1 += count_b2;

            } else {
                pressed2 = 0;                         //Button is not being pressed
            }

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

        //outputs
        //LED1 blinking
        if(led1_blink == 1){
            P1OUT ^= BIT0;
            TA0CCR1 += blink_rate_1;
        }

        //fading in/out and breathing, part 1, can be 15, 16, 17, or 18
        if(((led1_fade_in >= 15) && (led1_fade_in <= 18)) || ((led1_fade_out >= 15) && (led1_fade_out <= 18)) || ((led1_breath >= 15) && (led1_breath <= 18))){
            if(counter_val_1 < brightness_1){
                P1OUT |= BIT0;                              //Turn on light
            } else {
                P1OUT &= ~BIT0;
            }
            counter_val_1 += 1;                             //Increment the value to read from the array

            if(counter_val_1 >= max_brightness){                       //reached max value, reset
                counter_val_1 = 0;
            }
            TA0CCR1 += change_period;
        }

        //fade in part 2
        if(led1_fade_in == 10) {
            if(light_flag_1 == 0){
                brightness_1 += 1;                //increase brightness
            }

            if(brightness_1 == max_brightness){               //if max brightness
                P1OUT |= BIT0;             //leave it on
                led1_fade_in = -2;          //completed task
            }

            TA0CCR1 += period_1;
        }

        //fade out part 2
        if(led1_fade_out == 10) {
            if(light_flag_1 == 1) {
                brightness_1 -= 1;                //decrease brightness
            }

             if (brightness_1 == 0) {       //reached lowest value
                P1OUT &= ~BIT0;                 //keep light off
                led1_fade_out = -2;          //completed task, deactivate it
            }

            TA0CCR1 += period_1;
        }

        //breathing part 2
        if(led1_breath == 10){
            if(light_flag_1 == 0){
                brightness_1 += 1;                //increase brightness
            } else {
                brightness_1 -= 1;                //decrease brightness
            }

            if(brightness_1 == max_brightness){               //if max brightness
                light_flag_1 = 1;                //set to decrease
            } else if (brightness_1 == 0) {       //reached lowest value
                light_flag_1 = 0;                //increase light
            }

            TA0CCR1 += period_1;
        }

        //LED2 blinking
        if(led2_blink == 1){
            P1OUT ^= BIT6;
            TA0CCR1 += blink_rate_2;
        }

        //fading in/out and breathing, part 1, can be 15, 16, 17, or 18
        if(((led2_fade_in >= 15) && (led2_fade_in <= 18)) || ((led2_fade_out >= 15) && (led2_fade_out <= 18)) || ((led2_breath >= 15) && (led2_breath <= 18))){
            if(counter_val_2 < brightness_2){
                P1OUT |= BIT6;                              //Turn on light
            } else {
                P1OUT &= ~BIT6;
            }
            counter_val_2 += 1;                             //Increment the value to read from the array

            if(counter_val_2 >= max_brightness){                       //reached max value, reset
                counter_val_2 = 0;
            }
            TA0CCR1 += change_period;
        }

        //fade in part 2
        if(led2_fade_in == 10) {
            if(light_flag_2 == 0){
                brightness_2 += 1;                //increase brightness
            }

            if(brightness_2 == max_brightness){               //if max brightness
                P1OUT |= BIT6;             //leave it on
                led2_fade_in = -1;          //completed task
            }

            TA0CCR1 += period_2;
        }

        //fade out part 2
        if(led2_fade_out == 10) {
            if(light_flag_2 == 1) {
                brightness_2 -= 1;                //decrease brightness
            }

             if (brightness_2 == 0) {       //reached lowest value
                P1OUT &= ~BIT6;                 //keep light off
                led2_fade_out = -2;          //completed task, deactivate it
            }

            TA0CCR1 += period_2;
        }

        //breathing part 2
        if(led2_breath == 10){
            if(light_flag_2 == 0){
                brightness_2 += 1;                //increase brightness
            } else {
                brightness_2 -= 1;                //decrease brightness
            }

            if(brightness_2 == max_brightness){               //if max brightness
                light_flag_2 = 1;                //set to decrease
            } else if (brightness_2 == 0) {       //reached lowest value
                light_flag_2 = 0;                //increase light
            }

            TA0CCR1 += period_2;
        }

        //LED3 rotating
        if(led3_rot == 1){
            //verify boundaries
            if ((colour == 0) && (led3_dir == -1)){
                colour = 6;
            } else if ((colour == 6) && (led3_dir == 1)){
                colour = 0;
            } else {
                colour = colour + led3_dir; //change colour based on direction
            }
            //swap colour
            P2OUT = colours[colour];
            TA0CCR1 += blink_rate_3;
        }
        //LED3 blink
        if(led3_blink == 1){
            if(led3_blink_on){
                P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
                led3_blink_on = 0;
                TA0CCR1 += blink_rate_3;
            } else {
                if ((colour == 0) && (led3_dir == -1)){
                    colour = 6;
                } else if ((colour == 6) && (led3_dir == 1)){
                    colour = 0;
                } else {
                    colour = colour + led3_dir; //change colour based on direction
                }
                led3_blink_on = 1;
                //swap colour
                P2OUT = colours[colour];
                TA0CCR1 += blink_rate_3;
            }
        }

        //fading in/out and breathing, part 1, can be 15, 16, 17, or 18
        if(((led3_fade_in >= 15) && (led3_fade_in <= 18)) || ((led3_fade_out >= 15) && (led3_fade_out <= 18)) || ((led3_breath >= 15) && (led3_breath <= 18))){
            if(counter_val_3 < brightness_3){
                P2OUT |= colours[colour];                              //Turn on light
            } else {
                P2OUT &= ~(BIT1 + BIT3 + BIT5);
            }
            counter_val_3 += 1;                             //Increment the value to read from the array

            if(counter_val_3 >= max_brightness){                       //reached max value, reset
                counter_val_3 = 0;
            }
            TA0CCR1 += change_period;
        }

        //fade in part 2
        if(led3_fade_in == 10) {
            if(light_flag_3 == 0){
                brightness_3 += 1;                //increase brightness
            }

            if(brightness_3 == max_brightness){               //if max brightness
                P2OUT |= colours[colour];             //leave it on
                led3_fade_in = -1;          //completed task
            }

            TA0CCR1 += period_3;
        }

        //fade out part 2
        if(led3_fade_out == 10) {
            if(light_flag_3 == 1) {
                brightness_3 -= 1;                //decrease brightness
            }

             if (brightness_3 == 0) {       //reached lowest value
                P2OUT &= ~(BIT1 + BIT3 + BIT5);                 //keep light off
                led3_fade_out = -2;          //completed task, deactivate it
            }

            TA0CCR1 += period_3;
        }

        //breathing part 2
        if(led3_breath == 10){
            if(light_flag_3 == 0){
                brightness_3 += 1;                //increase brightness
            } else {
                brightness_3 -= 1;                //decrease brightness
            }

            if(brightness_3 == max_brightness){               //if max brightness
                light_flag_3 = 1;                //set to decrease
            } else if (brightness_3 == 0) {       //reached lowest value
                light_flag_3 = 0;                //increase light
                //change colour
                //verify boundaries
                if ((colour == 0) && (led3_dir == -1)){
                    colour = 6;
                } else if ((colour == 6) && (led3_dir == 1)){
                    colour = 0;
                } else {
                    colour = colour + led3_dir; //change colour based on direction
                }
            }

            TA0CCR1 += period_3;
        }
        break;
    case 4:
        //check for how long button was pressed
        if ((button == 2) && (held == 1)) {
            if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
                timerCount += 1;

            } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
                //send timer count signal here
                led3_rot = 1;
                //testing with light first
               // P1OUT ^= BIT0;
                held = 0;                               //button has now been released
            }
            //offset TA0CCR0 by the count number/period
            TA0CCR2 += count_b;
        } else if (button == 2){ //check that it is the button debouncing
            if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                 timerCount = 0;                     //Reset the timer count
                 pressed = 0;                        //reset pressed
                 held = 1;                           //Button is being held
                 P1OUT ^= BIT6;
                 //offset TA0CCR0 by the count number/period
                 //change the value held in the array
                 timers_used[0] = count_b;
                 TA0CCR2 += count_b;

            } else {
                pressed = 0;                         //Button is not being pressed
            }

         }

        //button 2, copied code
        if ((button2 == 2) && (held2 == 1)) {
            if ((held2 == 1) && !(P1IN & BIT5)) { //check how long its being held for
                timerCount2 += 1;

            } else if ((held2 == 1) && (P1IN & BIT5)){   //if button was released
                //send timer count signal here

                held2 = 0;                               //button has now been released
            }
            //offset TA0CCR0 by the count number/period
            TA0CCR2 += count_b2;
        } else if (button2 == 2){ //check that it is the button debouncing
            if ((pressed2 == 1) && !(P1IN & BIT5)){   //button was pressed properly
                 timerCount2 = 0;                     //Reset the timer count
                 pressed2 = 0;                        //reset pressed
                 held2 = 1;                           //Button is being held

                 //change the value held in the array
                 timers_used[0] = count_b2;
                 TA0CCR2 += count_b2;

            } else {
                pressed2 = 0;                         //Button is not being pressed
            }

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

            //TA0CCR2 += sample_temp_time;
        }

        //outputs
        //LED1 blinking
        if(led1_blink == 2){
            P1OUT ^= BIT0;
            TA0CCR2 += blink_rate_1;
        }

        //fading in/out and breathing, part 1, can be 19, 20, or 21
        if(((led1_fade_in >= 19) && (led1_fade_in <= 21)) || ((led1_fade_out >= 19) && (led1_fade_out <= 21)) || ((led1_breath >= 19) && (led1_breath <= 21))){
            if(counter_val_1 < brightness_1){
                P1OUT |= BIT0;                              //Turn on light
            } else {
                P1OUT &= ~BIT0;
            }
            counter_val_1 += 1;                             //Increment the value to read from the array

            if(counter_val_1 >= max_brightness){                       //reached max value, reset
                counter_val_1 = 0;
            }
            TA0CCR2 += change_period;
        }

        //fade in part 2
        if((led1_fade_in == 11) || (led1_fade_in == 15)) {
            if(light_flag_1 == 0){
                brightness_1 += 1;                //increase brightness
            }

            if(brightness_1 == max_brightness){               //if max brightness
                P1OUT |= BIT0;             //leave it on
                led1_fade_in = -1;          //completed task
            }

            TA0CCR2 += period_1;
        }

        //fade out part 2
        if((led1_fade_out == 11) || (led1_fade_out == 15)) {
            if(light_flag_1 == 1) {
                brightness_1 -= 1;                //decrease brightness
            }

             if (brightness_1 == 0) {       //reached lowest value
                P1OUT &= ~BIT0;                 //keep light off
                led1_fade_out = -2;          //completed task, deactivate it
            }

            TA0CCR2 += period_1;
        }

        //breathing part 2
        if((led1_breath == 11) || (led1_breath == 15)){
            if(light_flag_1 == 0){
                brightness_1 += 1;                //increase brightness
            } else {
                brightness_1 -= 1;                //decrease brightness
            }

            if(brightness_1 == max_brightness){               //if max brightness
                light_flag_1 = 1;                //set to decrease
            } else if (brightness_1 == 0) {       //reached lowest value
                light_flag_1 = 0;                //increase light
            }

            TA0CCR2 += period_1;
        }

        //LED2 blinking
        if(led2_blink == 2){
            P1OUT ^= BIT6;
            TA0CCR2 += blink_rate_2;
        }

        //fading in/out and breathing, part 1, can be 19, 20, or 21
        if(((led2_fade_in >= 19) && (led2_fade_in <= 21)) || ((led2_fade_out >= 19) && (led2_fade_out <= 21)) || ((led2_breath >= 19) && (led2_breath <= 21))){
            if(counter_val_2 < brightness_2){
                P1OUT |= BIT6;                              //Turn on light
            } else {
                P1OUT &= ~BIT6;
            }
            counter_val_2 += 1;                             //Increment the value to read from the array

            if(counter_val_2 >= max_brightness){                       //reached max value, reset
                counter_val_2 = 0;
            }
            TA0CCR2 += change_period;
        }

        //fade in part 2
        if((led2_fade_in == 11) || (led2_fade_in == 15)) {
            if(light_flag_2 == 0){
                brightness_2 += 1;                //increase brightness
            }

            if(brightness_2 == max_brightness){               //if max brightness
                P1OUT |= BIT6;             //leave it on
                led2_fade_in = -1;          //completed task
            }

            TA0CCR2 += period_2;
        }

        //fade out part 2
        if((led2_fade_out == 11) || (led2_fade_out == 15)) {
            if(light_flag_2 == 1) {
                brightness_2 -= 1;                //decrease brightness
            }

             if(brightness_2 == 0) {       //reached lowest value
                P1OUT &= ~BIT6;                 //keep light off
                led2_fade_out = -2;          //completed task, deactivate it
            }

            TA0CCR2 += period_2;
        }

        //breathing part 2
        if((led2_breath == 11) || (led2_breath == 15)){
            if(light_flag_2 == 0){
                brightness_2 += 1;                //increase brightness
            } else {
                brightness_2 -= 1;                //decrease brightness
            }

            if(brightness_2 == max_brightness){               //if max brightness
                light_flag_2 = 1;                //set to decrease
            } else if (brightness_2 == 0) {       //reached lowest value
                light_flag_2 = 0;                //increase light
            }

            TA0CCR2 += period_2;
        }

        //LED3 rotating
        if(led3_rot == 2){
            //verify boundaries
            if ((colour == 0) && (led3_dir == -1)){
                colour = 6;
            } else if ((colour == 6) && (led3_dir == 1)){
                colour = 0;
            } else {
                colour = colour + led3_dir; //change colour based on direction
            }
            //swap colour
            P2OUT = colours[colour];
            TA0CCR2 += blink_rate_3;
        }

        //LED3 blink
        if(led3_blink == 2){
            if(led3_blink_on){
                P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
                led3_blink_on = 0;
                //TA0CCR2 += blink_rate_3;
            } else {
                if ((colour == 0) && (led3_dir == -1)){
                    colour = 6;
                } else if ((colour == 6) && (led3_dir == 1)){
                    colour = 0;
                } else {
                    colour = colour + led3_dir; //change colour based on direction
                }
                led3_blink_on = 1;
                //swap colour
                P2OUT = colours[colour];
                TA0CCR2 += blink_rate_3;
            }
        }
        //fading in/out and breathing, part 1, can be 19, 20, or 21
        if(((led3_fade_in >= 19) && (led3_fade_in <= 21)) || ((led3_fade_out >= 19) && (led3_fade_out <= 21)) || ((led3_breath >= 19) && (led3_breath <= 21))){
            if(counter_val_3 < brightness_3){
                P2OUT |= colours[colour];                              //Turn on light
            } else {
                P2OUT &= ~(BIT1 + BIT3 + BIT5);
            }
            counter_val_3 += 1;                             //Increment the value to read from the array

            if(counter_val_3 >= max_brightness){                       //reached max value, reset
                counter_val_3 = 0;
            }
            TA0CCR2 += change_period;
        }

        //fade in part 2
        if((led3_fade_in == 11) || (led3_fade_in == 15)) {
            if(light_flag_3 == 0){
                brightness_3 += 1;                //increase brightness
            }

            if(brightness_3 == max_brightness){               //if max brightness
                P2OUT |= colours[colour];             //leave it on
                led3_fade_in = -1;          //completed task
            }

            TA0CCR2 += period_3;
        }

        //fade out part 2
        if((led3_fade_out == 11) || (led3_fade_out == 15)) {
            if(light_flag_3 == 1) {
                brightness_3 -= 1;                //decrease brightness
            }

             if(brightness_3 == 0) {       //reached lowest value
                P2OUT &= ~(BIT1 + BIT3 + BIT5);    //keep light off
                led3_fade_out = -2;          //completed task, deactivate it
            }

            TA0CCR2 += period_3;
        }

        //breathing part 2
        if((led3_breath == 11) || (led3_breath == 15)){
            if(light_flag_3 == 0){
                brightness_3 += 1;                //increase brightness
            } else {
                brightness_3 -= 1;                //decrease brightness
            }

            if(brightness_3 == max_brightness){               //if max brightness
                light_flag_3 = 1;                //set to decrease
            } else if (brightness_3 == 0) {       //reached lowest value
                light_flag_3 = 0;                //increase light
                //change colour
                //verify boundaries
                if ((colour == 0) && (led3_dir == -1)){
                    colour = 6;
                } else if ((colour == 6) && (led3_dir == 1)){
                    colour = 0;
                } else {
                    colour = colour + led3_dir; //change colour based on direction
                }
            }

            TA0CCR2 += period_2;
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
    //check for how long button was pressed
    if ((button == 3) && (held == 1)) {
        if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
            timerCount += 1;

        } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
            //send timer count signal here
            led1_blink = 1;
            //testing with light first
           // P1OUT ^= BIT0;
            held = 0;                               //button has now been released
        }
        //offset TA0CCR0 by the count number/period
        TA1CCR0 += count_b;
    } else if (button == 3){ //check that it is the button debouncing
        if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
             timerCount = 0;                     //Reset the timer count
             pressed = 0;                        //reset pressed
             held = 1;                           //Button is being held
             P1OUT ^= BIT6;
             //offset TA0CCR0 by the count number/period
             //change the value held in the array
             timers_used[0] = count_b;
             TA1CCR0 += count_b;

        } else {
            pressed = 0;                         //Button is not being pressed
        }

     }

    //button 2, copied code
    if ((button2 == 3) && (held2 == 1)) {
        if ((held2 == 1) && !(P1IN & BIT5)) { //check how long its being held for
            timerCount2 += 1;

        } else if ((held2 == 1) && (P1IN & BIT5)){   //if button was released
            //send timer count signal here

            held2 = 0;                               //button has now been released
        }
        //offset TA0CCR0 by the count number/period
        TA1CCR0 += count_b2;
    } else if (button2 == 3){ //check that it is the button debouncing
        if ((pressed2 == 1) && !(P1IN & BIT5)){   //button was pressed properly
             timerCount2 = 0;                     //Reset the timer count
             pressed2 = 0;                        //reset pressed
             held2 = 1;                           //Button is being held
             //change the value held in the array
             timers_used[0] = count_b2;
             TA1CCR0 += count_b2;

        } else {
            pressed2 = 0;                         //Button is not being pressed
        }

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
    //outputs
    //LED1 blinking
    if(led1_blink == 3){
        P1OUT ^= BIT0;
        TA1CCR0 += blink_rate_1;
    }

    //fading in/out and breathing, part 1, can be 22 or 23
    if(((led1_fade_in == 22) && (led1_fade_in == 23)) || ((led1_fade_out == 22) && (led1_fade_out == 23)) || ((led1_breath == 22) && (led1_breath == 23))){
        if(counter_val_1 < brightness_1){
            P1OUT |= BIT0;                              //Turn on light
        } else {
            P1OUT &= ~BIT0;
        }
        counter_val_1 += 1;                             //Increment the value to read from the array

        if(counter_val_1 >= max_brightness){                       //reached max value, reset
            counter_val_1 = 0;
        }
        TA1CCR0 += change_period;
    }

    //fade in part 2
    if((led1_fade_in == 12) || (led1_fade_in == 16) || (led1_fade_in == 19)) {
        if(light_flag_1 == 0){
            brightness_1 += 1;                //increase brightness
        }

        if(brightness_1 == max_brightness){               //if max brightness
            P1OUT |= BIT0;             //leave it on
            led1_fade_in = -1;          //completed task
        }

        TA1CCR0 += period_1;
    }

    //fade out part 2
    if((led1_fade_out == 12) || (led1_fade_out == 16) || (led1_fade_out == 19)) {
        if(light_flag_1 == 1) {
            brightness_1 -= 1;                //decrease brightness
        }

         if (brightness_1 == 0) {       //reached lowest value
            P1OUT &= ~BIT0;                 //keep light off
            led1_fade_out = -2;          //completed task, deactivate it
        }

        TA1CCR0 += period_1;
    }

    //breathing part 2
    if((led1_breath == 12) || (led1_breath == 16) || (led1_breath == 19)){
        if(light_flag_1 == 0){
            brightness_1 += 1;                //increase brightness
        } else {
            brightness_1 -= 1;                //decrease brightness
        }

        if(brightness_1 == max_brightness){               //if max brightness
            light_flag_1 = 1;                //set to decrease
        } else if (brightness_1 == 0) {       //reached lowest value
            light_flag_1 = 0;                //increase light
        }

        TA1CCR0 += period_1;
    }

    //LED2 blinking
    if(led2_blink == 3){
        P1OUT ^= BIT6;
        TA1CCR0 += blink_rate_2;
    }

    //fading in/out and breathing, part 1, can be 22 or 23
    if(((led2_fade_in == 22) && (led2_fade_in == 23)) || ((led2_fade_out == 22) && (led2_fade_out == 23)) || ((led2_breath == 22) && (led2_breath == 23))){
        if(counter_val_2 < brightness_2){
            P1OUT |= BIT6;                              //Turn on light
        } else {
            P1OUT &= ~BIT6;
        }
        counter_val_2 += 1;                             //Increment the value to read from the array

        if(counter_val_2 >= max_brightness){                       //reached max value, reset
            counter_val_2 = 0;
        }
        TA1CCR0 += change_period;
    }

    //fade in part 2
    if((led2_fade_in == 12) || (led2_fade_in == 16) || (led2_fade_in == 19)) {
        if(light_flag_2 == 0){
            brightness_2 += 1;                //increase brightness
        }

        if(brightness_2 == max_brightness){               //if max brightness
            P1OUT |= BIT6;             //leave it on
            led2_fade_in = -1;          //completed task
        }

        TA1CCR0 += period_2;
    }

    //fade out part 2
    if((led2_fade_out == 12) || (led2_fade_out == 16) || (led2_fade_out == 19)) {
        if(light_flag_2 == 1) {
            brightness_2 -= 1;                //decrease brightness
        }

         if (brightness_2 == 0) {       //reached lowest value
            P1OUT &= ~BIT6;                 //keep light off
            led2_fade_out = -2;          //completed task, deactivate it
        }

        TA1CCR0 += period_2;
    }

    //breathing part 2
    if((led2_breath == 12) || (led2_breath == 16) || (led2_breath == 19)){
        if(light_flag_2 == 0){
            brightness_2 += 1;                //increase brightness
        } else {
            brightness_2 -= 1;                //decrease brightness
        }

        if(brightness_2 == max_brightness){               //if max brightness
            light_flag_2 = 1;                //set to decrease
        } else if (brightness_2 == 0) {       //reached lowest value
            light_flag_2 = 0;                //increase light
        }

        TA1CCR0 += period_2;
    }

    //LED3 rotating
    if(led3_rot == 3){
        //verify boundaries
        if ((colour == 0) && (led3_dir == -1)){
            colour = 6;
        } else if ((colour == 6) && (led3_dir == 1)){
            colour = 0;
        } else {
            colour = colour + led3_dir; //change colour based on direction
        }
        //swap colour
        P2OUT = colours[colour];
        TA1CCR0 += blink_rate_3;
    }
    //LED3 blink
    if(led3_blink == 3){
        if(led3_blink_on){
            P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
            led3_blink_on = 0;
            TA1CCR0 += blink_rate_3;
        } else {
            if ((colour == 0) && (led3_dir == -1)){
                colour = 6;
            } else if ((colour == 6) && (led3_dir == 1)){
                colour = 0;
            } else {
                colour = colour + led3_dir; //change colour based on direction
            }
            led3_blink_on = 1;
            //swap colour
            P2OUT = colours[colour];
            TA1CCR0 += blink_rate_3;
        }
    }

    //fading in/out and breathing, part 1, can be 22 or 23
    if(((led3_fade_in == 22) && (led3_fade_in == 23)) || ((led3_fade_out == 22) && (led3_fade_out == 23)) || ((led3_breath == 22) && (led3_breath == 23))){
        if(counter_val_3 < brightness_3){
            P2OUT |= colours[colour];                              //Turn on light
        } else {
            P2OUT &= ~(BIT1 + BIT3 + BIT5);
        }
        counter_val_3 += 1;                             //Increment the value to read from the array

        if(counter_val_3 >= max_brightness){                       //reached max value, reset
            counter_val_3 = 0;
        }
        TA1CCR0 += change_period;
    }

    //fade in part 2
    if((led3_fade_in == 12) || (led3_fade_in == 16) || (led3_fade_in == 19)) {
        if(light_flag_3 == 0){
            brightness_3 += 1;                //increase brightness
        }

        if(brightness_3 == max_brightness){               //if max brightness
            P2OUT |= colours[colour];             //leave it on
            led3_fade_in = -1;          //completed task
        }

        TA1CCR0 += period_3;
    }

    //fade out part 2
    if((led3_fade_out == 12) || (led3_fade_out == 16) || (led3_fade_out == 19)) {
        if(light_flag_3 == 1) {
            brightness_3 -= 1;                //decrease brightness
        }

         if (brightness_3 == 0) {       //reached lowest value
            P2OUT &= ~(BIT1 + BIT3 + BIT5);                 //keep light off
            led3_fade_out = -2;          //completed task, deactivate it
        }

        TA1CCR0 += period_3;
    }

    //breathing part 2
    if((led3_breath == 12) || (led3_breath == 16) || (led3_breath == 19)){
        if(light_flag_3 == 0){
            brightness_3 += 1;                //increase brightness
        } else {
            brightness_3 -= 1;                //decrease brightness
        }

        if(brightness_3 == max_brightness){               //if max brightness
            light_flag_3 = 1;                //set to decrease
        } else if (brightness_3 == 0) {       //reached lowest value
            light_flag_3 = 0;                //increase light
            //change colour
            //verify boundaries
            if ((colour == 0) && (led3_dir == -1)){
                colour = 6;
            } else if ((colour == 6) && (led3_dir == 1)){
                colour = 0;
            } else {
                colour = colour + led3_dir; //change colour based on direction
            }
        }

        TA1CCR0 += period_3;
    }
}

//interrupt for timerA1
#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer1_A1(void){
    switch(TA1IV)
    {
    case  2:
        //check for how long button was pressed
        if ((button == 4) && (held == 1)) {
            if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
                timerCount += 1;

            } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
                //send timer count signal here
                led1_blink = 1;
                //testing with light first
               // P1OUT ^= BIT0;
                held = 0;                               //button has now been released
            }
            //offset TA1CCR1 by the count number/period
            TA1CCR1 += count_b;
        } else if (button == 4){ //check that it is the button debouncing
            if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                 timerCount = 0;                     //Reset the timer count
                 pressed = 0;                        //reset pressed
                 held = 1;                           //Button is being held
                 P1OUT ^= BIT6;
                 //offset TA0CCR0 by the count number/period
                 //change the value held in the array
                 timers_used[0] = count_b;
                 TA1CCR1 += count_b;

            } else {
                pressed = 0;                         //Button is not being pressed
            }

         }

        //button 2, copied code
        if ((button2 == 4) && (held2 == 1)) {
            if ((held2 == 1) && !(P1IN & BIT5)) { //check how long its being held for
                timerCount2 += 1;

            } else if ((held2 == 1) && (P1IN & BIT5)){   //if button was released
                //send timer count signal here
                held2 = 0;                               //button has now been released
            }
            //offset TA0CCR0 by the count number/period
            TA1CCR1 += count_b2;
        } else if (button2 == 4){ //check that it is the button debouncing
            if ((pressed2 == 1) && !(P1IN & BIT5)){   //button was pressed properly
                 timerCount2 = 0;                     //Reset the timer count
                 pressed2 = 0;                        //reset pressed
                 held2 = 1;                           //Button is being held
                 //change the value held in the array
                 timers_used[0] = count_b2;
                 TA1CCR1 += count_b2;

            } else {
                pressed2 = 0;                         //Button is not being pressed
            }

         }

        //outputs
        //LED1 blinking
        if(led1_blink == 4){
            P1OUT ^= BIT0;
            TACCR1 += blink_rate_1;
        }

        //fading in/out and breathing, part 1, can be 24
        if((led1_fade_in == 22) || (led1_fade_out == 24) || (led1_breath == 24)){
            if(counter_val_1 < brightness_1){
                P1OUT |= BIT0;                              //Turn on light
            } else {
                P1OUT &= ~BIT0;
            }
            counter_val_1 += 1;                             //Increment the value to read from the array

            if(counter_val_1 >= max_brightness){                       //reached max value, reset
                counter_val_1 = 0;
            }
            TA1CCR1 += change_period;
        }

        //fade in part 2
        if((led1_fade_in == 13) || (led1_fade_in == 17) || (led1_fade_in == 20) || (led1_fade_in == 22)) {
            if(light_flag_1 == 0){
                brightness_1 += 1;                //increase brightness
            }

            if(brightness_1 == max_brightness){               //if max brightness
                P1OUT |= BIT0;             //leave it on
                led1_fade_in = -1;          //completed task
            }

            TA1CCR1 += period_1;
        }

        //fade out part 2
        if((led1_fade_out == 13) || (led1_fade_out == 17) || (led1_fade_out == 20) || (led1_fade_out == 22)) {
            if(light_flag_1 == 1) {
                brightness_1 -= 1;                //decrease brightness
            }

             if (brightness_1 == 0) {       //reached lowest value
                P1OUT &= ~BIT0;                 //keep light off
                led1_fade_out = -2;          //completed task, deactivate it
            }

            TA1CCR1 += period_1;
        }

        //breathing part 2
        if((led1_breath == 13) || (led1_breath == 17) || (led1_breath == 20) || (led1_breath == 22)){
            if(light_flag_1 == 0){
                brightness_1 += 1;                //increase brightness
            } else {
                brightness_1 -= 1;                //decrease brightness
            }

            if(brightness_1 == max_brightness){               //if max brightness
                light_flag_1 = 1;                //set to decrease
            } else if (brightness_1 == 0) {       //reached lowest value
                light_flag_1 = 0;                //increase light
            }

            TA1CCR1 += period_1;
        }

        //LED2 blinking
        if(led2_blink == 4){
            P1OUT ^= BIT6;
            TA1CCR1 += blink_rate_2;
        }

        //fading in/out and breathing, part 1, can be 24
        if((led2_fade_in == 22) || (led2_fade_out == 24) || (led2_breath == 24)){
            if(counter_val_2 < brightness_2){
                P1OUT |= BIT6;                              //Turn on light
            } else {
                P1OUT &= ~BIT6;
            }
            counter_val_2 += 1;                             //Increment the value to read from the array

            if(counter_val_2 >= max_brightness){                       //reached max value, reset
                counter_val_2 = 0;
            }
            TA1CCR1 += change_period;
        }

        //fade in part 2
        if((led2_fade_in == 13) || (led2_fade_in == 17) || (led2_fade_in == 20) || (led2_fade_in == 22)) {
            if(light_flag_2 == 0){
                brightness_2 += 1;                //increase brightness
            }

            if(brightness_2 == max_brightness){               //if max brightness
                P1OUT |= BIT6;             //leave it on
                led2_fade_in = -1;          //completed task
            }

            TA1CCR1 += period_2;
        }

        //fade out part 2
        if((led2_fade_out == 13) || (led2_fade_out == 17) || (led2_fade_out == 20) || (led2_fade_out == 22)) {
            if(light_flag_2 == 1) {
                brightness_2 -= 1;                //decrease brightness
            }

             if (brightness_2 == 0) {       //reached lowest value
                P1OUT &= ~BIT6;                 //keep light off
                led2_fade_out = -2;          //completed task, deactivate it
            }

            TA1CCR1 += period_2;
        }

        //breathing part 2
        if((led2_breath == 13) || (led2_breath == 17) || (led2_breath == 20) || (led2_breath == 22)){
            if(light_flag_2 == 0){
                brightness_2 += 1;                //increase brightness
            } else {
                brightness_2 -= 1;                //decrease brightness
            }

            if(brightness_2 == max_brightness){               //if max brightness
                light_flag_2 = 1;                //set to decrease
            } else if (brightness_2 == 0) {       //reached lowest value
                light_flag_2 = 0;                //increase light
            }

            TA1CCR1 += period_2;
        }

        //LED3 rotating
        if(led3_rot == 4){
            //verify boundaries
            if ((colour == 0) && (led3_dir == -1)){
                colour = 6;
            } else if ((colour == 6) && (led3_dir == 1)){
                colour = 0;
            } else {
                colour = colour + led3_dir; //change colour based on direction
            }
            //swap colour
            P2OUT = colours[colour];
            TA1CCR1 += blink_rate_3;
        }
        //LED3 blink
        if(led3_blink == 4){
            if(led3_blink_on){
                P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
                led3_blink_on = 0;
                TA1CCR1 += blink_rate_3;
            } else {
                if ((colour == 0) && (led3_dir == -1)){
                    colour = 6;
                } else if ((colour == 6) && (led3_dir == 1)){
                    colour = 0;
                } else {
                    colour = colour + led3_dir; //change colour based on direction
                }
                led3_blink_on = 1;
                //swap colour
                P2OUT = colours[colour];
                TA1CCR1 += blink_rate_3;
            }
        }

        //fading in/out and breathing, part 1, can be 24
        if((led3_fade_in == 22) || (led3_fade_out == 24) || (led3_breath == 24)){
            if(counter_val_3 < brightness_3){
                P2OUT |= colours[colour];                              //Turn on light
            } else {
                P2OUT &= ~(BIT1 + BIT3 + BIT5);
            }
            counter_val_3 += 1;                             //Increment the value to read from the array

            if(counter_val_3 >= max_brightness){                       //reached max value, reset
                counter_val_3 = 0;
            }
            TA1CCR1 += change_period;
        }

        //fade in part 2
        if((led3_fade_in == 13) || (led3_fade_in == 17) || (led3_fade_in == 20) || (led3_fade_in == 22)) {
            if(light_flag_3 == 0){
                brightness_3 += 1;                //increase brightness
            }

            if(brightness_3 == max_brightness){               //if max brightness
                P2OUT |= colours[colour];             //leave it on
                led3_fade_in = -1;          //completed task
            }

            TA1CCR1 += period_3;
        }

        //fade out part 2
        if((led3_fade_out == 13) || (led3_fade_out == 17) || (led3_fade_out == 20) || (led3_fade_out == 22)) {
            if(light_flag_3 == 1) {
                brightness_3 -= 1;                //decrease brightness
            }

             if (brightness_3 == 0) {       //reached lowest value
                P2OUT &= ~(BIT1 + BIT3 + BIT5);   //keep light off
                led3_fade_out = -2;          //completed task, deactivate it
            }

            TA1CCR1 += period_3;
        }

        //breathing part 2
        if((led3_breath == 13) || (led3_breath == 17) || (led3_breath == 20) || (led3_breath == 22)){
            if(light_flag_3 == 0){
                brightness_3 += 1;                //increase brightness
            } else {
                brightness_3 -= 1;                //decrease brightness
            }

            if(brightness_3 == max_brightness){               //if max brightness
                light_flag_3 = 1;                //set to decrease
            } else if (brightness_3 == 0) {       //reached lowest value
                light_flag_3 = 0;                //increase light
                //change colour
                //verify boundaries
                if ((colour == 0) && (led3_dir == -1)){
                    colour = 6;
                } else if ((colour == 6) && (led3_dir == 1)){
                    colour = 0;
                } else {
                    colour = colour + led3_dir; //change colour based on direction
                }
            }

            TA1CCR1 += period_3;
        }
        break;
    case 4:
        //check for how long button was pressed
        if ((button == 5) && (held == 1)) {
            if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
                timerCount += 1;

            } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
                //send timer count signal here
                led1_blink = 1;
                //testing with light first
               // P1OUT ^= BIT0;
                held = 0;                               //button has now been released
            }
            //offset TA0CCR0 by the count number/period
            TA1CCR2 += count_b;
        } else if (button == 5){ //check that it is the button debouncing
            if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                 timerCount = 0;                     //Reset the timer count
                 pressed = 0;                        //reset pressed
                 held = 1;                           //Button is being held
                 P1OUT ^= BIT6;
                 //offset TA0CCR0 by the count number/period
                 //change the value held in the array
                 timers_used[0] = count_b;
                 TA1CCR2 += count_b;

            } else {
                pressed = 0;                         //Button is not being pressed
            }

         }

        //button 2, copied code
        if ((button2 == 5) && (held2 == 1)) {
            if ((held2 == 1) && !(P1IN & BIT5)) { //check how long its being held for
                timerCount2 += 1;

            } else if ((held2 == 1) && (P1IN & BIT5)){   //if button was released
                //send timer count signal here

                held2 = 0;                               //button has now been released
            }
            //offset TA0CCR0 by the count number/period
            TA1CCR2 += count_b2;
        } else if (button2 == 5){ //check that it is the button debouncing
            if ((pressed2 == 1) && !(P1IN & BIT5)){   //button was pressed properly
                 timerCount2 = 0;                     //Reset the timer count
                 pressed2 = 0;                        //reset pressed
                 held2 = 1;                           //Button is being held

                 //change the value held in the array
                 timers_used[0] = count_b2;
                 TACCR2 += count_b2;

            } else {
                pressed2 = 0;                         //Button is not being pressed
            }

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
        //outputs
        //LED1 blinking
        if(led1_blink == 5){
            P1OUT ^= BIT0;
            TA1CCR2 += blink_rate_1;
        }

        //fade in part 2
        if((led1_fade_in == 14) || (led1_fade_in == 18) || (led1_fade_in == 21) || (led1_fade_in == 23) || (led1_fade_in == 24)) {
            if(light_flag_1 == 0){
                brightness_1 += 1;                //increase brightness
            }

            if(brightness_1 == max_brightness){               //if max brightness
                P1OUT |= BIT0;             //leave it on
                led1_fade_in = -1;          //completed task
            }

            TA1CCR2 += period_1;
        }

        //fade out part 2
        if((led1_fade_out == 14) || (led1_fade_out == 18) || (led1_fade_out == 21) || (led1_fade_out == 23) || (led1_fade_out == 24)) {
            if(light_flag_1 == 1) {
                brightness_1 -= 1;                //decrease brightness
            }

             if (brightness_1 == 0) {       //reached lowest value
                P1OUT &= ~BIT0;                 //keep light off
                led1_fade_out = -2;          //completed task, deactivate it
            }

            TA1CCR2 += period_1;
        }

        //breathing part 2
        if((led1_breath == 14) || (led1_breath == 18) || (led1_breath == 21) || (led1_breath == 23) || (led1_breath == 24)){
            if(light_flag_1 == 0){
                brightness_1 += 1;                //increase brightness
            } else {
                brightness_1 -= 1;                //decrease brightness
            }

            if(brightness_1 == max_brightness){               //if max brightness
                light_flag_1 = 1;                //set to decrease
            } else if (brightness_1 == 0) {       //reached lowest value
                light_flag_1 = 0;                //increase light
            }

            TA1CCR2 += period_1;
        }

        //LED2 blinking
        if(led2_blink == 5){
            P1OUT ^= BIT6;
            TA1CCR2 += blink_rate_2;
        }

        //fade in part 2
        if((led2_fade_in == 14) || (led2_fade_in == 18) || (led2_fade_in == 21) || (led2_fade_in == 23) || (led2_fade_in == 24)) {
            if(light_flag_2 == 0){
                brightness_2 += 1;                //increase brightness
            }

            if(brightness_2 == max_brightness){               //if max brightness
                P1OUT |= BIT6;             //leave it on
                led2_fade_in = -1;          //completed task
            }

            TA1CCR2 += period_2;
        }

        //fade out part 2
        if((led2_fade_out == 14) || (led2_fade_out == 18) || (led2_fade_out == 21) || (led2_fade_out == 23) || (led2_fade_out == 24)) {
            if(light_flag_2 == 1) {
                brightness_2 -= 1;                //decrease brightness
            }

             if(brightness_2 == 0) {       //reached lowest value
                P1OUT &= ~BIT6;                 //keep light off
                led2_fade_out = -2;          //completed task, deactivate it
            }

            TA1CCR2 += period_2;
        }

        //breathing part 2
        if((led2_breath == 14) || (led2_breath == 18) || (led2_breath == 21) || (led2_breath == 23) || (led2_breath == 24)){
            if(light_flag_2 == 0){
                brightness_2 += 1;                //increase brightness
            } else {
                brightness_2 -= 1;                //decrease brightness
            }

            if(brightness_2 == max_brightness){               //if max brightness
                light_flag_2 = 1;                //set to decrease
            } else if (brightness_2 == 0) {       //reached lowest value
                light_flag_2 = 0;                //increase light
            }

            TA1CCR2 += period_2;
        }

        //LED3 rotating
        if(led3_rot == 5){
            //verify boundaries
            if ((colour == 0) && (led3_dir == -1)){
                colour = 6;
            } else if ((colour == 6) && (led3_dir == 1)){
                colour = 0;
            } else {
                colour = colour + led3_dir; //change colour based on direction
            }
            //swap colour
            P2OUT = colours[colour];
            TA1CCR2 += blink_rate_3;
        }
        //LED3 blink
        if(led3_blink == 5){
            if(led3_blink_on){
                P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
                led3_blink_on = 0;
                TA1CCR2 += blink_rate_3;
            } else {
                if ((colour == 0) && (led3_dir == -1)){
                    colour = 6;
                } else if ((colour == 6) && (led3_dir == 1)){
                    colour = 0;
                } else {
                    colour = colour + led3_dir; //change colour based on direction
                }
                led3_blink_on = 1;
                //swap colour
                P2OUT = colours[colour];
                TA1CCR2 += blink_rate_3;
            }
        }
        //fade in part 2
        if((led3_fade_in == 14) || (led3_fade_in == 18) || (led3_fade_in == 21) || (led3_fade_in == 23) || (led3_fade_in == 24)) {
            if(light_flag_3 == 0){
                brightness_3 += 1;                //increase brightness
            }

            if(brightness_3 == max_brightness){               //if max brightness
                P2OUT |= colours[colour];             //leave it on
                led3_fade_in = -1;          //completed task
            }

            TA1CCR2 += period_3;
        }

        //fade out part 2
        if((led3_fade_out == 14) || (led3_fade_out == 18) || (led3_fade_out == 21) || (led3_fade_out == 23) || (led3_fade_out == 24)) {
            if(light_flag_3 == 1) {
                brightness_3 -= 1;                //decrease brightness
            }

             if(brightness_3 == 0) {       //reached lowest value
                P2OUT &= ~(BIT1 + BIT3 + BIT5); //keep light off
                led3_fade_out = -2;          //completed task, deactivate it
            }

            TA1CCR2 += period_3;
        }

        //breathing part 2
        if((led3_breath == 14) || (led3_breath == 18) || (led3_breath == 21) || (led3_breath == 23) || (led3_breath == 24)){
            if(light_flag_3 == 0){
                brightness_3 += 1;                //increase brightness
            } else {
                brightness_3 -= 1;                //decrease brightness
            }

            if(brightness_3 == max_brightness){               //if max brightness
                light_flag_3 = 1;                //set to decrease
            } else if (brightness_3 == 0) {       //reached lowest value
                light_flag_3 = 0;                //increase light
                //change colour
                //verify boundaries
                if ((colour == 0) && (led3_dir == -1)){
                    colour = 6;
                } else if ((colour == 6) && (led3_dir == 1)){
                    colour = 0;
                } else {
                    colour = colour + led3_dir; //change colour based on direction
                }
            }

            TA1CCR2 += period_3;
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
    } else if(P1IFG & BIT5){ //if it was button 2
        pressed2 = 1;
        //debouncer timer, based on current count, check which one was used
        switch(button2){
        case 0:
            TA0CCR0 = TAR + debounce_b2;         //Current count plus the wanted debounce part
            break;
        case 1:
            TA0CCR1 = TAR + debounce_b2;
            break;
        case 2:
            TA0CCR2 = TAR + debounce_b2;
            break;
        case 3:
            TA1CCR0 = TAR + debounce_b2;
            break;
        case 4:
            TA1CCR1 = TAR + debounce_b2;
            break;
        case 5:
            TA1CCR2 = TAR + debounce_b2;
            break;
        }
        P1IFG &= ~BIT5;                             //clear flag
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
       if((timers_used[i] == 0)){
           free[index] = i;
           index++;
       }

    }
    //reset activated timers
    activated_timers[0] = -1;
    activated_timers[1] = -1;


    if(index < registers){
        //return error
        return 0;
    } else { //right number was found, activate them
        for (i=0; i < registers; i++){
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

//Grab the correct code for the timers
int get_timer_code(int timers[]){
    int first = timers[0];
    int second = timers[1];

    switch (first){ //check which timer is first
            case 0:
                //get second value
                switch(second){
                case 1:
                    return 10;
                case 2:
                    return 11;
                case 3:
                    return 12;
                case 4:
                    return 13;
                case 5:
                    return 14;
                }
                break;
            case 1:
                //get second value
                switch(second){
                case 2:
                    return 15;
                case 3:
                    return 16;
                case 4:
                    return 17;
                case 5:
                    return 18;
                }
                break;
            case 2:
                //get second value
                switch(second){
                case 3:
                    return 19;
                case 4:
                    return 20;
                case 5:
                    return 21;
                }
                break;
            case 3:
                //get second value
                if(second == 4){
                    return 22;
                } else if (second == 5){
                    return 23;
                }
                break;
            case 4:
                if(second == 5){
                    return 24;
                }
                break;
            }
    return 0; //error occurred.
}



