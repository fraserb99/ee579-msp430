#include <msp430.h> 


/**
 * main.c for Group 4
 *
 * Main file for the EE579 Advanced Microcontroller Applications
 * Created by Group 4.
 *
 * Code in here is based on previous challenges and examples from TI code
 *
 * Includes all functionality for Inputs and Outputs specified in the design
 * document (2 buttons, 1 potentiometer, 1 thermometer, 3 lights, and 1 buzzer
 *
 * Timers are allocated dynamically based on activation of function (input/output)
 * For any initialisation in the same occurrence of a while loop the functionalities
 * will be allocated timers depending on order of first encounter ie :
 * Button 1, Button 2, Potentiometer, Thermometer, LED 1, LED 2, LED 3, Buzzer and
 * their related configurations.
 * The exact order of these can be found in the while loop in the main function.
 */

/**
 * Setup of global variables and constants, specified based on functionality
 */

/**
* sending messages
* first row is flag, second is value to send, codes for index:
* 0 = error, 1 = button, 2 = button2, 3 = potentiometer, 4 = temperature
*
* for error if value is 1 - generic, if value is 2 - no more timers, 3 - clashing rules
*/
int send[2][5] = {{0, 0, 0, 0, 0},
                  {0, 0, 0, 0, 0}};
unsigned int send_message = 0;                  //flag used to indicate that at least one message needs to be sent

/**
 * Description of codes used for the timers, A0_0 is reserved for UART
 * for those with 1 timer:
 * A0_1 = 1, A0_2 = 2, A1_0 = 3, A1_1 = 4, A1_2 = 5
 *
 * For 2 timers:
 * A0_1 +          A1_0 +          A1_1 +
 *    A0_2 = 10,      A1_1 = 11,      A1_2 = 13,
 *                    A1_2 = 12,
 *
 */

/**
 * inputs
 * The codes for activation are based on the timers, if value is 0 that means not active, if -1 means disable/deactivate
 */

//Button 1
signed int button = 0;                          //Button variable, depends on timer used
unsigned int b_activated = 0;                   //boolean for denoting button activation
unsigned int timer_count = 0;                   //initial count for the wait time
unsigned int pressed = 0;                       //checking for pressed state, used for debouncing
unsigned int held = 0;                          //checking if the button is being pressed/held
const int debounce_b = 328;                     //count for the 50Hz, based on 32768/(2*x)= 50, want a 20ms wait for the debouncing, 1/20*10^-3 = 50
const int count_b = 820;                        //count for the count for presses, rate of 20Hz, based on 32768/(2x) = 20Hz
int activated_button[];                         //indicator for activated timer for the button

//Button 2
signed int button2 = 0;                         //Button 2
unsigned int b2_activated = 0;                  //button 2 activation
unsigned int timer_count2 = 0;                  //initial count for the wait time
unsigned int pressed2 = 0;                      //checking for pressed state, used for debouncing
unsigned int held2 = 0;                         //checking if the button is being pressed/held
const int debounce_b2 = 164;                    //count for the 100Hz, based on 32768/(2*x)= 100, want a 10ms wait for the debouncing, 1/20*10^-3 = 50
const int count_b2 = 820;                       //count for the count for presses, rate of 20Hz, based on 32768/(2x) = 20Hz
int activated_button2[];                        //indicator for activated timer for the button

//Potentiometer
signed int pot = 0;                             //Potentiometer use
unsigned int pot_activated = 0;                 //boolean for denoting potentiometer activation
unsigned int orig_val = 0;                      //Setting up value for original potentiometer vale
unsigned int value = 0;                         //value on check/compare to original value
signed int change = 0;                          //value to check how much the potentiometer value has changed
const int step_size = 100;                      //value to indicate step size, set as a constant
unsigned int stable_pot = 0;                    //used to check first value after stabilisation timers
unsigned int sample_pot = 0;                    //used to denote if pot should be sampled
const int sample_time = 328;                    //sample every 20ms, based on 32768/2x = 50Hz
int activated_pot[];                            //array used for timer activation and deactivation

//Thermometer
signed int thermometer = 0;                     //thermometer use
unsigned int t_activated = 0;                   //boolean for denoting thermometer activation
signed int orig_temp = 0;                       //Setting up value for original potentiometer vale
signed int value_temp = 0;                      //value on check
unsigned int temp = 0;                          //temporary held value before calculation
signed long change_t = 0;                       //value to check how much the temp has changed
const int step_size_t = 2;                      //value to indicate step size, as error is +- 2 step size is set to 2 for more stable operation
unsigned int stable = 0;                        //boolean for determining if stable Vref
unsigned int sample_temp = 0;                   //boolean to be used to indicate a sample should be taken
const int sample_temp_time = 16384;             //used for sample time of the temperature, every second (1Hz)
int activated_temp[];                           //indicator for activated timer for the temperature

/**
 * Outputs
 * variables and constants for breathing and fading lights, same for all LEDs independent of user defined speeds
 */
signed int max_brightness = 32;                 //used for checking max value for brightness and counter value, based on single light
signed int change_period = 16;                  //period for flashing and breathing, default values, based on single light
const int max_brightness1 = 32;                 //Values used for this, used for 1 blinking light
const int change_period1 = 16;                  //values for the period
const int max_brightness2 = 16;                 //values for more than 1 breathing light
const int change_period2 = 8;                   // >1 breathing light
const int brightness_register_1 = 4096;         //used for brightness increment timing, 1Hz, first setting
const int brightness_register_2 = 2048;         //used for brightness increment timing, 2Hz, second setting
const int brightness_register_3 = 1024;         //used for brightness increment timing, 4Hz, third setting

//LED1
signed int led1_on = 0;                         //LED D1 on, 0 indicate off, -1 - deactivate
unsigned int led1_active = 0;                   //check if the light has been activated - can only have one setting on at a time
unsigned int led1_fo_active = 0;                //own activation check for fade out as that can turn lights of that are on, if wanted
signed int led1_blink = 0;                      //LED1 blinking light
signed int led1_fade_in = 0;                    //LED1 fade in
signed int led1_fade_out = 0;                   //LED1 fade out
signed int led1_breath = 0;                     //LED1 blinking light
unsigned int blink_rate_1 = 16384;              //rate for the blinking light of LED D1, default period 1Hz
unsigned int light_flag_1 = 0;                  //breathing and fading light flag, brighter or darker,  brighter = 0, darker = 1;
unsigned int brightness_1 = 0;                  // Varies brightness between 0 and 32
unsigned int counter_val_1 = 0;                 //counter for the LED on time
unsigned int period_1 = 2048;                   //overall period for the fading and breathing light, default is 2Hz
int activated_led1[];                           //Array for activated timers for LED D1

//LED2
signed int led2_on = 0;                         //LED D2 on
unsigned int led2_active = 0;                   //LED2 active
unsigned int led2_fo_active = 0;                //own activation check for fade out as that can turn lights of that are on, if wanted
signed int led2_blink = 0;                      //LED2 blink
signed int led2_fade_in = 0;                    //LED2 fade in
signed int led2_fade_out = 0;                   //LED2 fade out
signed int led2_breath = 0;                     //LED2 breathing light
unsigned int blink_rate_2 = 16384;              //rate for the blinking light of LED D2, default period 1Hz
unsigned int light_flag_2 = 0;                  //breathing and fading light flag, brighter or darker,  brighter = 0, darker = 1;
unsigned int brightness_2 = 0;                  //varies brightness between 0 and 32
unsigned int counter_val_2 = 0;                 //LED counter
unsigned int period_2 = 2048;                   //period for flashing and breathing, default
int activated_led2[];                           //Array for activated timers for LED D2

//LED3
signed int led3_on = 0;                         //LED D3 on/off, colour light
unsigned int led3_active = 0;                   //LED3 active
unsigned int led3_fo_active = 0;                //own activation check for fade out as that can turn lights of that are on, if wanted
signed int led3_rot = 0;                        //rotate led3
signed int led3_dir = 0;                        //LED3 direction of rotation, -1 for backwards and 1 for forwards
signed int led3_blink = 0;                      //LE3 blink
unsigned int led3_blink_on = 0;                 //used to indicate on or off light
signed int led3_fade_in = 0;                    //LED3 fade in
signed int led3_fade_out = 0;                   //LED3 fade out
signed int led3_breath = 0;                     //LED3 blinking light
//define all the colours, based on improved code from past assignments
#define RED (BIT1)
#define GREEN (BIT3)
#define BLUE (BIT5)
#define YELLOW (BIT1 + BIT3)
#define PURPLE (BIT1 + BIT5)
#define WHITE (BIT1 + BIT3 + BIT5)
const int colours[] = {RED, GREEN, BLUE, YELLOW, PURPLE, WHITE};    //Array for the colours
unsigned int colour = 0;                        //The colour selected
unsigned int blink_rate_3 = 16384;              //rate for the blinking light for LED D3, or rotating, default 1Hz
signed int light_flag_3 = 0;                    //breathing and fading light flag, brighter or darker,  brighter = 1, darker = -1;
signed int brightness_3 = 0;                    //used for determining brightness, initially between 0 and 4
unsigned int flash_count_3 = 0;                 //LED counter
unsigned int counter_val_3 = 1;                 //duty cycle for breathing and flashing, default of 2048Hz
unsigned int period_3 = 2048;                   //period for flashing and breathing, default of 16Hz
int activated_led3[];                           //Array for activated timers for the colour led

//Buzzer
signed int buzzer_tone = 0;                     //buzzer on, for specific period
signed int buzzer_beep = 0;                     //buzzer beeping
unsigned int buzzer_active = 0;                 //boolean for indicating that the buzzer has been activated/is being used
unsigned int buzzer_on = 0;                     //boolean used for indicating if the buzzer is on during the beep one
unsigned int buzzer_duration = 2000;            //buzzer duration, in ms, will be divided to closest 100ms, 2000 is default
unsigned int buzzer_duration_off = 1000;        //second buzzer duration for the beeping, off period, to nearest 100ms, 1000 is default
unsigned int buzzer_count = 0;                  //counter variable for the on period of the buzzer
const int buzzer_period = 82;                   //timer for counting the required on period, 50ms = 20Hz
int activated_buzzer[];                         //Array for activated timer for the buzzer

/**
 * array for timer usage, 0 if not used, 1 if used 0, second row is to indicate how many functions are using the timers
 * buttons needs their own and are therefore -1 if they are using the timer ie second row value is -1
 * position as timer codes, UART holds A0_0, it is included here to verify UART timing, and to assist in timer denotions as specified previously
 */

signed int timers_used[2][6] = {{0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0}};
unsigned int timer0_activated = 0;              //Boolean used for checking if TA0 has been activated
unsigned int timer1_activated = 0;              //Boolean used for checking if TA1 has been activated
signed int activated_timers[2] = {-1, -1};      //array for activated timers for each activation

/**
 * Array for used lights If 1 means active, if 0 not active, for the breathing lights
 * Returns array of status for each light, pos0 = D1, pos1 = D2, pos3 = D3
 */
unsigned int lights_used[3] = {0, 0, 0};

//Helper Function Declaration, the functions are located at the bottom of the file
int activate_timer(int timer_no, int count);                        //function for activating a specific timer
int activate_free_timer(int registers, int counts[], int alone);    //function for activating required number of free timers
void deactivate_timer(int activated[], int len);                    //deactivate unused timers
int get_timer_code(int timers[]);                                   //get the specific timer code for 2 timers
void check_breath(void);                                            //function to check current active breathing lights to change period for increased numbers

//main function
int main(void)
 {
    //Base set up
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer


	/**
	 * Variables for testing
	 * Un-comment the features wanted, then select debug and run. Ensure all external devices are
	 * connected as per the diagram in the design specifications as these will not function otherwise.
	 *
	 * for features connected to inputs the required affecting output needs to be specified
	 * in the timer code for each corresponding input, these can be found in the timer interrupts later
	 * in this file. It is not necessary to specify the wanted output for each timer interrupt,
	 * these are dynamically allocated base on available timers and will therefore allocate the
	 * inputs based on the order in the list below, therefore for example if button 1 was selected,
	 * along with button 2. on this run Button 1 would be allocated Timer A0_1 and button 2 A0_2,
	 * therefore any required output should be activated in those corresponding timers.
	 * If only Button 2 was selected then this would be allocated A0_1.
	 *
	 * Please note that while any inputs wanted can be selected at once, however this will affect
	 * the timers and hence the availability of the outputs as there are only a possible number
	 * of 5 timer Capture Compare Registers of which 1 is required per each input, and a mix of
	 * 1 and 2 CCR for the outputs depending on functionality, please see the list below for details.
	 * LED is used without a digit here as it is the same for all LEDs
	 *
	 * Those with 1 CCR: LED blink, Buzzer Tone, Buzzer Beep
	 * Those with 2: LED fade in, LED Fade out, LED breathing
	 *
	 * Please note that Buzzer Tone, Fade in, and Fade out will deactivate their timer upon task completion.
	 * Default durations are set in the variable declaration above, the duration is specified in the comments.
	 *
	 * To deactivate any input or output simply assign them a value of -1
	 *
	 */

	//Inputs
	//button = 1;           //Button 1
	//button2 = 1;          //Button 2
	//pot = 1;              //Potentiometer
	//thermometer = 1;      //Thermometer

	//Outputs, these can be activated here or copied over to the timer interrupts to be activat3ed by input events
	//led1_on = 1;          //LED D1 (green) on, to turn off assign value -1
	//led1_blink = 1;       //LED D1 blink at default rate, this rate can be altered by altering the blink_rate_1 variable
	//led1_fade_in = 1;     //LED D1 fade in at default rate, this can be specified by assigning one of the blink_rate variables to period_1
	//led1_fad_out = 1'     //LED D1 fade out at default rate, can be specified in the same manner as fade in
	//led1_breath = 1;      //LED D1 breathing light, at default rate, can be specified in the same manner as fade

	//led2_on = 1;          //LED D2 (red) on, to turn off assign value -1
    //led2_blink = 1;       //LED D2 blink at default rate, this rate can be altered by altering the blink_rate_2 variable
    //led2_fade_in = 1;     //LED D2 fade in at default rate, this can be specified by assigning one of the blink_rate variables to period_2
    //led2_fad_out = 1'     //LED D2 fade out at default rate, can be specified in the same manner as fade in
    //led2_breath = 1;      //LED D2 breathing light, at default rate, can be specified in the same manner as fade

	//led3_on = 1;          //LED D3 colour can be specified by setting a number between 0 and 5, on, to turn off assign value -1
    //led3_rot = 1;         //LED D3 rotating light, direction is determined by dir, if dir = 0 then no rotation will take place dir = 1 is forward and dir = -1 backwards
	//led1_blink = 1;       //LED D3 blink at default rate, this rate can be altered by altering the blink_rate_2 variable, if dir is set to 1 or -1 this will also rotate
    //led1_fade_in = 1;     //LED D3 fade in at default rate, this can be specified by assigning one of the blink_rate variables to period_3
    //led1_fad_out = 1'     //LED D3 fade out at default rate, can be specified in the same manner as fade in
    //led1_breath = 1;      //LED D3 breathing light, at default rate, can be specified in the same manner as fade, if dir is set this will rotate
	//led3_dir = 1          //Used to set rotation direction for LED 3,

	//buzzer_tone = 1;      //Buzzer on for a specific duration, currently default duration, this can be specified by setting the buzzer_duration variable
	//buzzer_beep = 1;      //Buzzer beeping until stopped (highly recommended to use this in conjunction with an input to stop it as buzzer_beep needs to be set to -1 to deactivate
	                        //On duration and off duration of the beeps is set separately, currently default, can be changed by setting buzzer_duration, and buzzer_duration_off


	//UART initialisation
    //reserve timer A0_0 for UART, arbitrary nr for now:
    //returns 1 on success
    int uart_timer = activate_timer(0, 16384);
    //want it to be reserved
    timers_used[1][0] = -1;

    if(uart_timer != 1){
        //error occurred
        send_message = 1;
        send[0][0] = 1; //set flag
        send[1][0] = 1; //generic error

    }



	__bis_SR_register(GIE);                     //interrupt enabled

	//While loop for continuous running
    while(1){
        //read messages here


        //sending messages, based on set flags
        if(send_message){ //there is a message to send
            int i, to_send, value;
            for(i=0; i<5; i++){ //loop through and check for flag

                if(send[0][i] == 1){ //if the flag is set
                    to_send = i;      //say which one to send
                }

                switch(to_send){
                case 0:
                    //send error message here
                    //value = send[1][0]
                    //reset flag
                    send[0][0] = 0;
                    break;
                case 1:
                    //send message for button
                    //value is the time the button has been pressed for
                    //counts every 50ms so multiply by 50, to get total time in ms
                    value = send[1][1]*50;
                    //reset flag
                    send[0][1] = 0;
                    break;
                case 2:
                    //send message for second button, hold time in ms
                    value = send[1][2]*50;
                    send[0][2] = 0;
                    break;
                case 3:
                    //send message for potentiometer
                    //value stored in send[1][3]
                    send[0][3] = 0;
                    break;
                case 4:
                    //send message for temperature, in degrees C
                    //value stored in send[1][4]
                    send[0][4] = 0;
                    break;
                default:
                    //something went wrong
                    break;
                }
            }
            //reset the send message flag
            send_message = 0;
        }


        /**
         * Activation of inputs
         */

        /**
         * Button activation, selects first available timer
         */
        if (button > 0){
            if (!b_activated){ //if not already activated
                //button set up, from slides and previous challenge
                P1OUT |= BIT3;                        //pull up resistor on P1.3
                P1REN |= BIT3;                        //Enable it
                P1IES |= BIT3;                        //high to low edge
                P1IFG &= ~BIT3;                       //Clear flag
                P1IE |= BIT3;                         //Interrupt enable

                int counts[1] = {debounce_b};         //array set up for wanted CCR counts, set for debounce
                b_activated = activate_free_timer(1, counts, -1); //activate free timer, needs to be alone
                //record which ones were activated
                activated_button[0] = activated_timers[0];
                button = activated_button[0];            //assign to check properly for timer interrupts
                if (b_activated != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //send error
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic error
                    //disable button
                    button = -1;
                } else {
                        //turn of the timer for now, still being used, just doesn't need to trigger constantly, triggers by button press
                    switch (button){
                    case 1:
                        TA0CCR1 = 0;
                        break;
                    case 2:
                        TA0CCR2 = 0;
                        break;
                    case 3:
                        TA1CCR0 = 0;
                        break;
                    case 4:
                        TA1CCR1 = 0;
                        break;
                    case 5:
                        TA1CCR2 = 0;
                        break;
                    default:
                        //something went wrong, send error
                        send_message = 1;
                        send[0][0] = 1; //set flag
                        send[1][0] = 1; //generic error
                        button = -1; //disable button as wrong timer used
                        break;
                    }
                }

            }
        } else if(button == -1){    //disable the button
            b_activated = 0;                        //no longer active
            deactivate_timer(activated_button, 2);  //turn off timer
            P1IE &= ~BIT3;                          //and button interrupt
            P1REN |= BIT3;                          // and disable the pullup/pulldown
            button = 0;                             //set to default (off)
        }


        /**
         * Button 2, code copied from button 1, altered for correct pins
         */
        if (button2 > 0){
            if (!b2_activated){ //if not already activated
                P1OUT |= BIT5;                            //pull up resistor on P1.5
                P1REN |= BIT5;                            //Enable it
                P1IES |= BIT5;                            //high to low edge
                P1IFG &= ~BIT5;                           // Clear flag
                P1IE |= BIT5;                             //Interrupt enable

                int counts[1] = {debounce_b2};
                b2_activated = activate_free_timer(1, counts, -1); //single timer, alone
                //record which ones were activated
                activated_button2[0] = activated_timers[0];
                button2 = activated_button2[0];                //assign to check properly for timer interrupts
                if (b2_activated != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //send error
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic error
                    //disable button
                    button2 = -1;
                } else {
                    //turn of the timer for now
                    switch (button2){
                    case 1:
                        TA0CCR1 = 0;
                        break;
                    case 2:
                        TA0CCR2 = 0;
                        break;
                    case 3:
                        TA1CCR0 = 0;
                        break;
                    case 4:
                        TA1CCR1 = 0;
                        break;
                    case 5:
                        TA1CCR2 = 0;
                        break;
                    default:
                        //Something went wrong, send generic error message
                        send_message = 1;
                        send[0][0] = 1; //set flag
                        send[1][0] = 1; //generic error
                        button2 = -1; //disable button
                        break;
                    }
                }
            }

        } else if(button2 == -1){
            b2_activated = 0;                        //if button is no longer requested
            deactivate_timer(activated_button2, 2);   //turn off timer
            P1IE &= ~BIT5;                          //and button interrupt
            P1REN |= BIT5;                          // and disable the pullup/pulldown
            button2 = 0;
        }

        /**
         * Potentiometer, samples every 20ms, activates first available timer
         */
        if (pot > 0){
            if(!pot_activated){         //if not already activated
                ///based on previous assignments
                ADC10CTL0 = ADC10ON + CONSEQ_0 + SREF_0; // ADC10ON, single channel single sample, ref 0
                ADC10CTL1 = INCH_4 + ADC10SSEL_1;          // input A4, clock = ACLK
                ADC10AE0 |= BIT4;                          // PA.4 ADC option select

                int counts[1] = {10}; //stabilisation count
                pot_activated = activate_free_timer(1, counts, 0); //single timer, doesn't need to be alone
                activated_pot[0] = activated_timers[0];      //record which ones were activated
                pot = activated_pot[0];              //Set for proper check in interrupts
                if (pot_activated != 1) { //if return is 1, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable pot
                    pot = -1;
                } else {
                    stable_pot = 0;
                    sample_pot = 0;
                }
            } else {
                if(sample_pot) { //if sample is requested
                    if(t_activated){ //if thermometer is used as well
                        ADC10CTL0 = ADC10ON  + SREF_0;  //set ref to 0
                        ADC10CTL1 = INCH_4;             //Ensure right channel
                        ADC10AE0 |= BIT4;               // PA.4 ADC option select
                        ADC10CTL0 &= ~REFON;            //turn of reference

                        ADC10CTL0 |= ENC + ADC10SC;     // Sampling and conversion start
                        //ADC10MEM = conversion result
                        while ((ADC10CTL0 & ADC10IFG) == 0) {//wait for flag
                            //Wait for sample to take place
                        }
                        value = ADC10MEM;           //Just to make sure the value doesn't change during checking
                        //disable conversion
                        ADC10CTL0 &= ~ENC;
                        //clear flag
                        ADC10CTL0 &= ~ADC10IFG;

                    } else { //on its own, no need to reset INCH and SREF

                        ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
                        //ADC10MEM = conversion result
                        //wait for result completion,
                        while ((ADC10CTL0 & ADC10IFG) == 0) {//wait for flag
                            //Wait for sample to take place
                        }
                        value = ADC10MEM;           //Just to make sure the value doesn't change during checking

                        //clear flag
                        ADC10CTL0 &=  ~ADC10IFG;

                    }

                    if(value != orig_val){ //if value has changed
                        //check by how much
                        change = abs(orig_val - value);

                        if (change >= step_size){ //if by wanted step size or more
                            //send message
                            send_message = 1;
                            send[0][3] = 1; //set flag
                            send[1][3] = value; //value of pot

                            //change to new orig value
                            orig_val = value;
                            //testing light, need to be activated by setting P1DIR |= BIT6
                            //P1OUT ^= BIT6;
                        }

                    }

                    sample_pot = 0; //finished sampling, ready for another
                }
            }

        } else if(pot == -1){
            deactivate_timer(activated_pot, 1);
            pot_activated = 0;
            pot = 0;
        }

        /**
         * Thermometer, samples every second, activates first available timer
         * currently does not work when potentiometer is requested (changes when the potentiometer changes) making value invalid
         */
        if(thermometer > 0){
            //based on example adc10_temp.c from URL: http://dev.ti.com/tirex/explore/node?node=AEldOKIXgnT979MckUlLRw__IOGqZri__LATEST
            if(!t_activated){
                ADC10CTL0 = ADC10ON + SREF_1 + REFON + CONSEQ_0; // ADC10ON, vr+ = Vref+f, Vr- = Vss, reference on, ref 1
                ADC10CTL1 = INCH_10 + ADC10SSEL_1;    // temp sensor, clock = ACLK

                //set up a timer for stabilisation, use A1
                //need a stabilisation nr, only need 1 timer
                int counts[1] = {30};
                t_activated = activate_free_timer(1, counts, 0); //single timer, doesn't need to be alone
                activated_temp[0] = activated_timers[0];      //record which ones were activated
                thermometer = activated_temp[0];              //Set for proper check in interrupts
                if (t_activated != 1) { //if return is 1, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable temp
                    thermometer = -1;
                } else {
                    stable = 0;
                }
            }

            if (sample_temp){ //is sample was requested
                if(pot_activated){ //check if potentiometer is being used at the same time
                    //if pot is active reset these every time
                    ADC10CTL0 = ADC10ON + SREF_1 + REFON;
                    ADC10CTL1 = INCH_10;
                    ADC10AE0 &= ~BIT4;      //reset ADC input

                    ADC10CTL0 |= ENC + ADC10SC;         // Sampling and conversion start
                    //based on code for pot above
                    while ((ADC10CTL0 & ADC10IFG) == 0) {//wait for flag
                        //Wait for sample to take place
                    }
                    temp = ADC10MEM;           //Just to make sure the value doesn't change during checking
                    //disable conversion, to allow for reset of values
                    ADC10CTL0 &= ~ENC;
                    //clear flag
                    ADC10CTL0 &= ~ADC10IFG;

                } else {
                    ADC10CTL0 |= ENC + ADC10SC;         // Sampling and conversion start
                    //based on code for pot above
                    while ((ADC10CTL0 & ADC10IFG) == 0) {//wait for flag
                        //Wait for sample to take place
                    }
                    temp = ADC10MEM;           //Just to make sure the value doesn't change during checking

                    //clear flag
                    ADC10CTL0 &= ~ADC10IFG;
                }

                value_temp = ((temp - 673) * 423) / 1024;

                if(value_temp != orig_temp){ //if value has changed
                    //check by how much
                    change_t = abs(orig_temp - value_temp);

                    if (change_t >= step_size_t){
                        //send message
                        send_message = 1;
                        send[0][4] = 1; //set flag
                        send[1][4] = value_temp; //temp value
                        //change to new orig value
                        orig_temp = value_temp;
                        //testing light
                        P1OUT ^= BIT0;
                    }
                }

                //set sample to false
                sample_temp = 0;
            }

        } else if(thermometer == -1){
            t_activated = 0;
            deactivate_timer(activated_temp, 1);
            thermometer = 0;
        }


        /**
         * Outputs activation
         */

        /**
         * LED 1, turning it on, to turn off assign -1
         */
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

        /**
         * LED 1 blinking light, activates first available timer
         */
        if(led1_blink > 0){
            if(!led1_active){
                P1DIR |= BIT0;
                //need a timer for the blinking, only 1
                int counts[1] = {blink_rate_1};
                led1_active = activate_free_timer(1, counts, 0); //1 timer, doesn't need to be alone
                activated_led1[0] = activated_timers[0]; //record which ones were activated
                led1_blink = activated_led1[0];
                if (led1_active != 1) { //if return is 1, has now been activated
                    //something went wrong
                    //send generic error message
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the led
                    led1_blink = -1;
                }
                //toggle LED D1
                P1OUT ^= BIT0;
            }

        } else if (led1_blink == -1){
            led1_active = 0;
            P1OUT &= ~BIT0;
            deactivate_timer(activated_led1, 1);
            led1_blink = 0;
        }

        /**
         * LED 1 Fade in, allocates timer if 2 CCR are available in the same timer, else will not activate, will send error if this happens
         */
        if(led1_fade_in > 0){
            if(!led1_active){
                P1DIR |= BIT0;
                P1OUT &= ~BIT0; //don't want it on
                //need 2 timers for the fading in, doesn't need to be alone
                int counts[2] = {change_period, period_1};
                led1_active = activate_free_timer(2, counts, 0);
                activated_led1[0] = activated_timers[0]; //record which ones were activated, first
                activated_led1[1] = activated_timers[1]; //record second one
                led1_fade_in = get_timer_code(activated_led1);
                if (led1_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    //send generic error message
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the fade in
                    led1_fade_in = -1;
                } else {
                    light_flag_1 = 0; //reset this
                    counter_val_1 = 0; //and this
                }

            }

        } else if (led1_fade_in == -1){
            //led1_active = 0; - light is still active
            deactivate_timer(activated_led1, 2);
            led1_fade_in = 0;
        }

        /**
         * LED 1 fade out, allocates timer in the same manner as fade in
         */
        if(led1_fade_out > 0){
            if(!led1_fo_active){ //if light hasn't been activated, or if on
                P1DIR |= BIT0;
                //need 2 timers for the fading in, doesn't need to be alone
                int counts[2] = {change_period, period_1};
                led1_fo_active = activate_free_timer(2, counts, 0);
                activated_led1[0] = activated_timers[0]; //record which ones were activated, first
                activated_led1[1] = activated_timers[1]; //record second one
                led1_fade_out = get_timer_code(activated_led1);
                if (led1_fo_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the fade out
                    led1_fade_out = -1;
                } else {
                    led1_active = 1; //led is being used

                    //set the brightness to max
                    brightness_1 = max_brightness;
                    light_flag_1 = 1;
                    counter_val_1 = 0; //reset this
                    //need to turn on a light first, if not already on
                    P1OUT |= BIT0;
                }

            }

        } else if (led1_fade_out == -1){
            led1_fo_active = 0;
            led1_active = 0;
            P1OUT &= ~BIT0;
            deactivate_timer(activated_led1, 2);
            led1_fade_out = 0;
        }

        /**
         * LED 1 breathing light, allocates timers in the same manner as fading lights,
         * will share timer with other breathing lights if active at the same time
         */
        if(led1_breath > 0){
            if(!led1_active){
                P1DIR |= BIT0;
                //check active breathing lights
                check_breath();
                //need 2 timers for the breathing light
                int counts[2] = {change_period, period_1};
                led1_active = activate_free_timer(2, counts, 0);
                activated_led1[0] = activated_timers[0]; //record which ones were activated, first
                activated_led1[1] = activated_timers[1]; //record second one
                led1_breath = get_timer_code(activated_led1);
                if (led1_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the breathing light
                    led1_breath = -1;
                } else {
                     light_flag_1 = 0;       //ensure increasing brightness first
                     counter_val_1 = 0;      //and counter val 0
                     lights_used[0] = 1;    //indicate it is now active
                }

            }

        } else if (led1_breath == -1){
            led1_active = 0;
            P1OUT &= ~BIT0;
            deactivate_timer(activated_led1, 2);
            led1_breath = 0;
            lights_used[2] = 0;
        }

        /**
         * LED 2 code, based on that of LED 1
         *
         * LED 2 on
         */
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

        /**
         * LED 2 blinking light, allocates fist available timer
         */
        if(led2_blink > 0){
            if(!led2_active){
                P1DIR |= BIT6;
                //need a timer for the blinking, only 1
                int counts[1] = {blink_rate_1};
                led2_active = activate_free_timer(1, counts, 0);
                activated_led2[0] = activated_timers[0]; //record which ones were activated
                led2_blink = activated_led2[0];
                if (led2_active != 1) { //if return is 1, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the blinking light
                    led2_blink = -1;
                }
                //toggle LED D2
                P1OUT ^= BIT6;
            }

        } else if (led2_blink == -1){
            led2_active = 0;
            P1OUT &= ~ BIT6;
            deactivate_timer(activated_led2, 1);
            led2_blink = 0;
        }

        /**
         * LED 2, timer allocation as LED 1 fading lights
         */
        if(led2_fade_in > 0){
            if(!led2_active){
                P1DIR |= BIT6;
                P1OUT &= ~BIT6; //don't want it on
                //need 2 timers for the fading in
                int counts[2] = {change_period, period_2};
                led2_active = activate_free_timer(2, counts, 0);
                activated_led2[0] = activated_timers[0]; //record which ones were activated, first
                activated_led2[1] = activated_timers[1]; //record second one
                led2_fade_in = get_timer_code(activated_led2);
                if (led2_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the fade in
                    led2_fade_in = -1;
                } else {
                    light_flag_2 = 0; //reset light flag
                    counter_val_2 = 0;
                }

            }

        } else if (led2_fade_in == -1){
            //led2_active = 0; - light is still active
            //P1OUT &= ~BIT6;
            deactivate_timer(activated_led2, 2);
            led2_fade_in = 0;
        }

        /**
         * LED 2 fade out
         */
        if(led2_fade_out > 0){
            if(!led2_fo_active){ //if light hasn't been activated, or if on
                P1DIR |= BIT6;
                //need 2 timers for the fading in
                int counts[2] = {change_period, period_2};
                led2_fo_active = activate_free_timer(2, counts, 0);
                activated_led2[0] = activated_timers[0]; //record which ones were activated, first
                activated_led2[1] = activated_timers[1]; //record second one
                led2_fade_out = get_timer_code(activated_led2);
                if (led2_fo_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong'=
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the fade out
                    led2_fade_out = -1;
                } else {
                    led2_active = 1; //led is being used
                    //for testing
                    //P1OUT ^= BIT6;
                    //set the brightness to max
                    brightness_2 = max_brightness;
                    light_flag_2 = 1;
                    counter_val_2 = 0;
                    //need to turn on a light first, if not already on
                    P1OUT |= BIT6;
                }

            }

        } else if (led2_fade_out == -1){
            led2_fo_active = 0;
            led2_active = 0;
            P1OUT &= ~BIT6;
            deactivate_timer(activated_led2, 2);
            led2_fade_out = 0;
        }

        /**
         * LED 2 breathing light
         */
        if(led2_breath > 0){
            if(!led2_active){
                P1DIR |= BIT6;
                check_breath();
                //need 2 timers for the breathing light
                int counts[2] = {change_period, period_2};
                led2_active = activate_free_timer(2, counts, 0);
                activated_led2[0] = activated_timers[0]; //record which ones were activated, first
                activated_led2[1] = activated_timers[1]; //record second one
                led2_breath = get_timer_code(activated_led2);
                if (led2_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the breathing light
                    led2_breath = -1;
                } else {
                    light_flag_2 = 0;
                    counter_val_2 = 0;
                    lights_used[1] = 1;
                }

            }

        } else if (led2_breath == -1){
            led2_active = 0;
            P1OUT &= ~BIT6;
            deactivate_timer(activated_led2, 2);
            led2_breath = 0;
            lights_used[2] = 0;
        }

        /**
         * LED 3, based on LED 1 but with added functionality of colour selection
         */
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

        /**
         * LED 3 rotating light, first available timer will be allocated
         */
        if(led3_rot > 0){
            if(!led3_active){
                P2DIR |= BIT1 + BIT3 + BIT5;
                //need a timer for the rotation speed, only 1
                int counts[1] = {blink_rate_3};
                led3_active = activate_free_timer(1, counts, 0);
                activated_led3[0] = activated_timers[0]; //record which ones were activated
                led3_rot = activated_led3[0];
                if (led3_active != 1) { //if return is 1, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the rotation light
                    led3_rot = -1;
                } else {
                    //toggle LED D3, wanted colour
                    P2OUT = colours[colour];
                }
            }

        } else if(led3_rot == -1){
            led3_active = 0;
            P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
            deactivate_timer(activated_led3, 1);
            led3_rot = 0;

        }

        /**
         * LED 3 blinking light
         */
        if(led3_blink > 0){
            if(!led3_active){
                P2DIR |= BIT1 + BIT3 + BIT5;
                //need a timer for the rotation speed, only 1
                int counts[1] = {blink_rate_3};
                led3_active = activate_free_timer(1, counts, 0);
                activated_led3[0] = activated_timers[0]; //record which ones were activated
                led3_blink = activated_led3[0];
                if (led3_active != 1) { //if return is 1, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the blinking light
                    led3_blink = -1;
                } else {
                    //toggle LED D3, with start colour
                    P2OUT = colours[colour];
                    //indicate on
                    led3_blink_on = 1;
                }

            }

        } else if(led3_blink == -1){
            led3_active = 0;
            P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
            deactivate_timer(activated_led3, 1);
            led3_blink = 0;
        }

        /**
         * LED 3 fade in
         */
        if(led3_fade_in > 0){
            if(!led3_active){
                P2DIR |= BIT1 + BIT3 + BIT5;
                P2OUT &= ~(BIT1 + BIT3 + BIT5); //don't want it on
                //need 2 timers for the fading in
                int counts[2] = {change_period, period_3};
                led3_active = activate_free_timer(2, counts, 0);
                activated_led3[0] = activated_timers[0]; //record which ones were activated, first
                activated_led3[1] = activated_timers[1]; //record second one
                led3_fade_in = get_timer_code(activated_led3);
                if (led3_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable fade in
                    led3_fade_in = -1;
                } else {
                    light_flag_3 = 0;
                    counter_val_3 = 0;
                }
            }

        } else if (led3_fade_in == -1){
            deactivate_timer(activated_led3, 2);
            led3_fade_in = 0;
        }

        /**
         * LED 3 fade out
         */
        if(led3_fade_out > 0){
            if(!led3_fo_active){ //if light hasn't been activated, or if on
                P2DIR |= BIT1 + BIT3 + BIT5;
                //need 2 timers for the fading in
                int counts[2] = {change_period, period_3};
                led3_fo_active = activate_free_timer(2, counts, 0);
                activated_led3[0] = activated_timers[0]; //record which ones were activated, first
                activated_led3[1] = activated_timers[1]; //record second one
                led3_fade_out = get_timer_code(activated_led3);
                if (led3_fo_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable fade out
                    led3_fade_out = -1;
                } else {
                    led3_active = 1; //led is being used
                    //for testing
                    //set the brightness to max
                    brightness_3 = max_brightness;
                    light_flag_3 = 1;
                    counter_val_3 = 0;
                    //need to turn on a light first, if not already on, on the specified colour
                    P2OUT |= colours[colour];
                }

            }

        } else if (led3_fade_out == -1){
            led3_fo_active = 0;
            led3_active = 0;
            P2OUT &= ~(BIT1 + BIT3 + BIT5);
            deactivate_timer(activated_led3, 2);
            led3_fade_out = 0;
        }

        /**
         * LED 3 breathing light
         */
        if(led3_breath > 0){
            if(!led3_active){
                P2DIR |= BIT1 + BIT3 + BIT5;
                check_breath();
                //need 2 timers for the breathing light
                int counts[2] = {change_period, period_3};
                led3_active = activate_free_timer(2, counts, 0);
                activated_led3[0] = activated_timers[0]; //record which ones were activated, first
                activated_led3[1] = activated_timers[1]; //record second one
                led3_breath = get_timer_code(activated_led3);
                if (led3_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the breathing light
                    led3_breath = -1;
                } else {
                    light_flag_3 = 0;
                    counter_val_3 = 0;
                    lights_used[2] = 1;
                }
            }

        } else if (led3_breath == -1){
            led3_active = 0;
            P2OUT &= ~(BIT1 + BIT3 + BIT5); //turn off the light
            deactivate_timer(activated_led3, 2);
            led3_breath = 0;
            lights_used[2] = 0;
        }

        /**
         * Buzzer on for specific duration, allocated first available timer
         */
        if(buzzer_tone > 0){
            if(!buzzer_active){ //if it's not been activated
                //activate the pins used
                P2DIR |= BIT2 + BIT4;
                P2OUT |= BIT2;     //ensure one is on, the other off
                P2OUT &= ~ BIT4;
                buzzer_count = 0;   //ensure this is at 0;
                buzzer_duration = buzzer_duration/5; //ensure duration is proper as sampling at 5ms
                //need to have 1 timer
                int counts[1] = {buzzer_period};
                buzzer_active = activate_free_timer(1, counts, 0);
                activated_buzzer[0] = activated_timers[0]; //record which one were activated
                buzzer_tone = activated_buzzer[0]; //get the timer
                if (buzzer_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the buzzer
                    buzzer_tone = -1;
                }
            }
        } else if (buzzer_tone == -1){
            buzzer_active = 0;  //no longer on
            P2OUT &= ~BIT2;
            P2OUT &= ~BIT4;
            deactivate_timer(activated_buzzer, 1);
            buzzer_tone = 0;
        }

        /**
         * Buzzer beeping at specified rate, continues until turned off
         */
        if(buzzer_beep > 0){
            if(!buzzer_active){ //if it's not been activated
                //activate the pins used
                P2DIR |= BIT2 + BIT4;
                P2OUT |= BIT2;     //ensure one is on, the other off
                P2OUT &= ~ BIT4;
                buzzer_count = 0;   //ensure this is at 0;
                buzzer_duration = buzzer_duration/5; //ensure duration is proper
                buzzer_duration_off = buzzer_duration_off/5; //ensure duration is proper
                //need to have 1 timer
                int counts[1] = {buzzer_period};
                buzzer_active = activate_free_timer(1, counts, 0);
                activated_buzzer[0] = activated_timers[0]; //record which one were activated
                buzzer_beep = activated_buzzer[0];
                if (buzzer_active == 0) { //if return is anything but 0, has now been activated
                    //something went wrong
                    send_message = 1;
                    send[0][0] = 1; //set flag
                    send[1][0] = 1; //generic
                    //disable the buzzer
                    buzzer_beep = -1;
                }
            }

        } else if (buzzer_beep == -1){
            buzzer_active = 0;  //no longer on
            P2OUT &= ~BIT2;
            P2OUT &= ~BIT4;
            deactivate_timer(activated_buzzer, 1);
            buzzer_beep= 0;
        }

    }
}

/**interrupts,
 * duplicated code to allow for optimum timer usage
 *
 * timer A0 is reserved for UART
 */

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
{
    //TODO: insert uart code here
    //UART stuff here, generic timer nr for now
    TA0CCR0 += 16384;
}

//interrupt for timer A0_1,2 and overflow
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer0_A1(void){
    switch(TA0IV)
    {
    case  2:
        /**
         * Button interrupt, checks for duration of press and acts as debouncer
         */
        //check for how long button was pressed
        if ((button == 1) && (held == 1)) {
            if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
                timer_count += 1;
                //offset TA0CCR1 by the count number/period
                TA0CCR1 += count_b;
            } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
                //send message for how long the button was held for
                send_message = 1;
                send[0][1] = 1; //set flag
                send[1][1] = timer_count; //the time held

                //wanted output for testing should be set here, some were used during initial testing, uncomment for use
                //led2_breath = 1;
                //led1_fade_out = 1;
                //buzzer_beep = -1;
                held = 0;                               //button has now been released
                //set timer count to 0
                TA0CCR1 = 0;
            }

        } else if (button == 1){ //check that it is the button debouncing
            if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                 timer_count = 0;                     //Reset the timer count
                 pressed = 0;                        //reset pressed
                 held = 1;                           //Button is being held
                 //P1OUT ^= BIT6;
                 //offset TA0CCR0 by the count number/period
                 //change the value held in the array
                 timers_used[0][1] = count_b;
                 TA0CCR1 += count_b;

            } else {
                pressed = 0;                         //Button is not being pressed
            }

         }

        //button 2, copied code fom button 1
        if ((button2 == 1) && (held2 == 1)) {
            if ((held2 == 1) && !(P1IN & BIT5)) { //check how long its being held for
                timer_count2 += 1;
                //offset TA0CCR1 by the count number/period
                TA0CCR1 += count_b2;
            } else if ((held2 == 1) && (P1IN & BIT5)){   //if button was released
                //send message for how long the button was held for
                send_message = 1;
                send[0][1] = 1; //set flag
                send[1][1] = timer_count2; //the time held

                //testing outputs should be activated here
                //buzzer_beep = -1;

                held2 = 0;                               //button has now been released

                TA0CCR1 = 0;
            }

        } else if (button2 == 1){ //check that it is the button debouncing
            if ((pressed2 == 1) && !(P1IN & BIT5)){   //button was pressed properly
                 timer_count2 = 0;                    //Reset the timer count
                 pressed2 = 0;                        //reset pressed
                 held2 = 1;                           //Button is being held
                 P1OUT ^= BIT6;
                 //offset TA0CCR0 by the count number/period
                 //change the value held in the array
                 timers_used[0][1] = count_b2;
                 TA0CCR1 += count_b2;

            } else {
                pressed2 = 0;                         //Button is not being pressed
            }

         }

        /**
         * Potentiometer for stabilisation and sample rate
         */
        if(pot == 1){
            if (!stable_pot){

                //get original value, if possible here
                ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
                orig_val = ADC10MEM;

                //update timer value
                timers_used[0][1] = sample_time;
                stable_pot = 1;

            } else {
                sample_pot = 1;        //start a sample
            }

            TA0CCR1 += sample_time;
        }


        /**
         * Thermometer, for stabilisation and sampling rate
         */
        if(thermometer == 1){
            if (!stable){
                stable = 1;
                //get original value, if possible here
                ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
                temp = ADC10MEM;
                // oC = ((A10/1024)*1500mV)-986mV)*1/3.55mV = A10*423/1024 - 278, taken from example
                orig_temp = ((temp - 673) * 423) / 1024;

                //update timer value
                timers_used[0][1] = sample_temp_time;

            } else {
                sample_temp = 1;        //start a sample
            }

            TA0CCR1 += sample_temp_time;
        }

        /**
         * Output interrupts
         */
        /**
         * LED 1 blinking
         */
        if(led1_blink == 1){
            P1OUT ^= BIT0;
            TA0CCR1 += blink_rate_1;
        }

        /**
         * LED 1 fading in/out and breathing, part 1, can only be 10
         */
        if((led1_fade_in == 10) || (led1_fade_out == 10) || (led1_breath == 10)){
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

        /**
         * LED2 blinking
         */
        if(led2_blink == 1){
            P1OUT ^= BIT6;
            TA0CCR1 += blink_rate_2;
        }

        /**
         * LED 2 fading in/out and breathing, part 1, can only be 10
         */
        if((led2_fade_in == 10) || (led2_fade_out == 10) || (led2_breath == 10)){
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

        /**
         * LED3 rotating
         */
        if(led3_rot == 1){
            //verify boundaries
            if ((colour == 0) && (led3_dir == -1)){
                colour = 5;
            } else if ((colour == 5) && (led3_dir == 1)){
                colour = 0;
            } else {
                colour = colour + led3_dir; //change colour based on direction
            }
            //swap colour
            P2OUT = colours[colour];
            TA0CCR1 += blink_rate_3;
        }

        /**
         * LED3 blink
         */
        if(led3_blink == 1){
            if(led3_blink_on){
                P2OUT &= ~(BIT1 + BIT3 + BIT5);     //turn light off
                led3_blink_on = 0;
                TA0CCR1 += blink_rate_3;
            } else {
                if ((colour == 0) && (led3_dir == -1)){
                    colour = 5;
                } else if ((colour == 5) && (led3_dir == 1)){
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

        /**
         * LED 3 fading in/out and breathing, part 1, 10
         */
        if((led3_fade_in == 10) || (led3_fade_out == 10) || (led3_breath == 10)){
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

        /**
         * Buzzer tone
         */
        if(buzzer_tone == 1){
            buzzer_count += 1;  //increment count
            //alternate the pins
            P2OUT ^= BIT2;
            P2OUT ^= BIT4;

            //check if less or greater than the wanted duration
            if(buzzer_count >= buzzer_duration){
                buzzer_tone = -1; //turn it off
            }

            TA0CCR1 += buzzer_period;
        }

        /**
         * Buzzer Beeping
         */
        if(buzzer_beep == 1){
            buzzer_count += 1;  //increment count

            if(buzzer_on) { //if it is on, alternate
                //alternate the pins
                P2OUT ^= BIT2;
                P2OUT ^= BIT4;
                //check if less or greater than the wanted duration
                if(buzzer_count >= buzzer_duration){
                    buzzer_on = 0; //turn off
                    buzzer_count = 0; //reset count
                }

            } else { //if off, turn both off
                P2OUT &= ~BIT2;
                P2OUT &= ~BIT4;

                //check if less or greater than the wanted duration
                if(buzzer_count >= buzzer_duration_off){
                    buzzer_on= 1; //turn it on
                    //buzzer count reset
                    buzzer_count = 0;
                    //turn one of them on
                    P2OUT |= BIT2;
                }

            }


            TA0CCR1 += buzzer_period;
        }

        break;
    case 4:
        /**
         * Copied interrupts from above, with altered timer registers,
         * with some added functions for fading and breathing as they require 2 CCRs
         */
        //check for how long button was pressed
        if ((button == 2) && (held == 1)) {
            if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
                timer_count += 1;
                //offset TA0CCR0 by the count number/period
                TA0CCR2 += count_b;
            } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
                //send message for how long the button was held for
                send_message = 1;
                send[0][1] = 1; //set flag
                send[1][1] = timer_count; //the time held

                led3_rot = 1;
                //testing with light first
               // P1OUT ^= BIT0;
                held = 0;                               //button has now been released
                TA0CCR2 = 0;
            }

        } else if (button == 2){ //check that it is the button debouncing
            if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                 timer_count = 0;                     //Reset the timer count
                 pressed = 0;                        //reset pressed
                 held = 1;                           //Button is being held

                 //change the value held in the array
                 timers_used[0][2] = count_b;
                 TA0CCR2 += count_b;

            } else {
                pressed = 0;                         //Button is not being pressed
            }

         }

        //button 2, copied code
        if ((button2 == 2) && (held2 == 1)) {
            if ((held2 == 1) && !(P1IN & BIT5)) { //check how long its being held for
                timer_count2 += 1;
                //offset TA0CCR0 by the count number/period
                TA0CCR2 += count_b2;
            } else if ((held2 == 1) && (P1IN & BIT5)){   //if button was released
                //send timer count signal here
                send_message = 1;
                send[0][1] = 1; //set flag
                send[1][1] = timer_count2; //the time held

                //testing
                led3_breath = 1;
                //buzzer_beep = -1;
                held2 = 0;                               //button has now been released
                TA0CCR2 = 0;
            }

        } else if (button2 == 2){ //check that it is the button debouncing
            if ((pressed2 == 1) && !(P1IN & BIT5)){   //button was pressed properly
                 timer_count2 = 0;                     //Reset the timer count
                 pressed2 = 0;                        //reset pressed
                 held2 = 1;                           //Button is being held

                 //change the value held in the array
                 timers_used[0][2] = count_b2;
                 TA0CCR2 += count_b2;

            } else {
                pressed2 = 0;                         //Button is not being pressed
            }

         }

        //pot stuff
        if(pot == 2){
            if (!stable_pot){

                //get original value, if possible here
                ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
                orig_val = ADC10MEM;

                //update timer value
                timers_used[0][2] = sample_time;
                stable_pot = 1;

            } else {
                sample_pot = 1;        //start a sample
            }

            TA0CCR2 += sample_time;
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

                //update timer value
                timers_used[0][2] = sample_temp_time;

            } else {
                sample_temp = 1;        //start a sample
            }

            TA0CCR2 += sample_temp_time;
        }

        //outputs
        //LED1 blinking
        if(led1_blink == 2){
            P1OUT ^= BIT0;
            TA0CCR2 += blink_rate_1;
        }

        //fade in part 2, can only be 10
        if(led1_fade_in == 10) {
            if(light_flag_1 == 0){
                brightness_1 += 1;                //increase brightness
            }

            if(brightness_1 == max_brightness){               //if max brightness
                P1OUT |= BIT0;             //leave it on
                led1_fade_in = -1;          //completed task
            }

            TA0CCR2 += period_1;
        }

        //fade out part 2, can only be 10
        if(led1_fade_out == 10) {
            if(light_flag_1 == 1) {
                brightness_1 -= 1;                //decrease brightness
            }

             if (brightness_1 == 0) {       //reached lowest value
                P1OUT &= ~BIT0;                 //keep light off
                led1_fade_out = -1;          //completed task, deactivate it
            }

            TA0CCR2 += period_1;
        }

        //breathing part 2, can only be 10
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

            TA0CCR2 += period_1;
        }

        //LED2 blinking
        if(led2_blink == 2){
            P1OUT ^= BIT6;
            TA0CCR2 += blink_rate_2;
        }

        //fade in part 2, can onlt be 10
        if(led2_fade_in == 10) {
            if(light_flag_2 == 0){
                brightness_2 += 1;                //increase brightness
            }

            if(brightness_2 == max_brightness){               //if max brightness
                P1OUT |= BIT6;             //leave it on
                led2_fade_in = -1;          //completed task
            }

            TA0CCR2 += period_2;
        }

        //fade out part 2, can only be 10
        if(led2_fade_out == 10) {
            if(light_flag_2 == 1) {
                brightness_2 -= 1;                //decrease brightness
            }

             if(brightness_2 == 0) {       //reached lowest value
                P1OUT &= ~BIT6;                 //keep light off
                led2_fade_out = -1;          //completed task, deactivate it
            }

            TA0CCR2 += period_2;
        }

        //breathing part 2, can onlt be 10
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

            TA0CCR2 += period_2;
        }

        //LED3 rotating
        if(led3_rot == 2){
            //verify boundaries
            if ((colour == 0) && (led3_dir == -1)){
                colour = 5;
            } else if ((colour == 5) && (led3_dir == 1)){
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
                    colour = 5;
                } else if ((colour == 5) && (led3_dir == 1)){
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

        //fade in part 2, can only be 10
        if(led3_fade_in == 10) {
            if(light_flag_3 == 0){
                brightness_3 += 1;                //increase brightness
            }

            if(brightness_3 == max_brightness){               //if max brightness
                P2OUT |= colours[colour];             //leave it on
                led3_fade_in = -1;          //completed task
            }

            TA0CCR2 += period_3;
        }

        //fade out part 2, can only be 10
        if(led3_fade_out == 10) {
            if(light_flag_3 == 1) {
                brightness_3 -= 1;                //decrease brightness
            }

             if(brightness_3 == 0) {       //reached lowest value
                P2OUT &= ~(BIT1 + BIT3 + BIT5);    //keep light off
                led3_fade_out = -1;          //completed task, deactivate it
            }

            TA0CCR2 += period_3;
        }

        //breathing part 2, can only be 10
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
                    colour = 5;
                } else if ((colour == 5) && (led3_dir == 1)){
                    colour = 0;
                } else {
                    colour = colour + led3_dir; //change colour based on direction
                }
            }

            TA0CCR2 += period_2;
        }

        //buzzer
        if(buzzer_tone == 2){
            buzzer_count += 1;  //increment count
            //alternate the pins
            P2OUT ^= BIT2;
            P2OUT ^= BIT4;

            //check if less or greater than the wanted duration
            if(buzzer_count >= buzzer_duration){
                buzzer_tone = -1; //turn it off
            }

            TA0CCR2 += buzzer_period;
        }

        //buzzer beeping
        if(buzzer_beep == 2){
            buzzer_count += 1;  //increment count

            if(buzzer_on) { //if it is on, alternate
                //alternate the pins
                P2OUT ^= BIT2;
                P2OUT ^= BIT4;
                //check if less or greater than the wanted duration
                if(buzzer_count >= buzzer_duration){
                    buzzer_on = 0; //turn off
                    buzzer_count = 0; //reset count
                }

            } else { //if off, turn both off
                P2OUT &= ~BIT2;
                P2OUT &= ~BIT4;

                //check if less or greater than the wanted duration
                if(buzzer_count >= buzzer_duration_off){
                    buzzer_on= 1; //turn it on
                    //buzzer count reset
                    buzzer_count = 0;
                    //turn one of them on
                    P2OUT |= BIT2;
                }

            }


            TA0CCR2 += buzzer_period;
        }

        break;

    case 10: break;                             //Timer_A3 overflow, don't need to do anything

    }

}

//Interrupt for time A1, based on those above
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void)
{
    //button stuff
    //check for how long button was pressed
    if ((button == 3) && (held == 1)) {
        if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
            timer_count += 1;
            //offset TA0CCR0 by the count number/period
            TA1CCR0 += count_b;
        } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
            send_message = 1;
            send[0][1] = 1; //set flag
            send[1][1] = timer_count; //the time held

            led1_blink = 1;
            //testing with light first
           // P1OUT ^= BIT0;
            held = 0;                               //button has now been released
            TA1CCR0 = 0;
        }

    } else if (button == 3){ //check that it is the button debouncing
        if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
             timer_count = 0;                     //Reset the timer count
             pressed = 0;                        //reset pressed
             held = 1;                           //Button is being held
             P1OUT ^= BIT6;
             //offset TA0CCR0 by the count number/period
             //change the value held in the array
             timers_used[0][3] = count_b;
             TA1CCR0 += count_b;

        } else {
            pressed = 0;                         //Button is not being pressed
        }

     }

    //button 2, copied code
    if ((button2 == 3) && (held2 == 1)) {
        if ((held2 == 1) && !(P1IN & BIT5)) { //check how long its being held for
            timer_count2 += 1;
            //offset TA0CCR0 by the count number/period
            TA1CCR0 += count_b2;
        } else if ((held2 == 1) && (P1IN & BIT5)){   //if button was released
            //send timer count signal here
            send_message = 1;
            send[0][1] = 1; //set flag
            send[1][1] = timer_count2; //the time held

            held2 = 0;                               //button has now been released
            TA1CCR0 = 0;
        }

    } else if (button2 == 3){ //check that it is the button debouncing
        if ((pressed2 == 1) && !(P1IN & BIT5)){   //button was pressed properly
             timer_count2 = 0;                     //Reset the timer count
             pressed2 = 0;                        //reset pressed
             held2 = 1;                           //Button is being held
             //change the value held in the array
             timers_used[0][3] = count_b2;
             TA1CCR0 += count_b2;

        } else {
            pressed2 = 0;                         //Button is not being pressed
        }

     }

    //pot stuff
    if(pot == 3){
        if (!stable_pot){

            //get original value, if possible here
            ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
            orig_val = ADC10MEM;

            //update timer value
            timers_used[0][3] = sample_time;
            stable_pot = 1;

        } else {
            sample_pot = 1;        //start a sample
        }

        TA1CCR0 += sample_time;
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

            //update timer value
            timers_used[0][3] = sample_temp_time;

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

    //fading in/out and breathing, part 1, can be 11 or 12
    if(((led1_fade_in == 11) || (led1_fade_in == 12)) || ((led1_fade_out == 11) || (led1_fade_out == 12)) || ((led1_breath == 11) || (led1_breath == 12))){
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


    //LED2 blinking
    if(led2_blink == 3){
        P1OUT ^= BIT6;
        TA1CCR0 += blink_rate_2;
    }

    //fading in/out and breathing, part 1, can be 11 or 12
    if(((led2_fade_in == 11) || (led2_fade_in == 12)) || ((led2_fade_out == 11) || (led2_fade_out == 12)) || ((led2_breath == 11) || (led2_breath == 12))){
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

    //LED3 rotating
    if(led3_rot == 3){
        //verify boundaries
        if ((colour == 0) && (led3_dir == -1)){
            colour = 5;
        } else if ((colour == 5) && (led3_dir == 1)){
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
                colour = 5;
            } else if ((colour == 5) && (led3_dir == 1)){
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

    //fading in/out and breathing, part 1, can be 11 or 12
    if(((led3_fade_in == 11) || (led3_fade_in == 12)) || ((led3_fade_out == 11) || (led3_fade_out == 12)) || ((led3_breath == 11) || (led3_breath == 12))){
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


    //Buzzer
    if(buzzer_tone == 3){
        buzzer_count += 1;  //increment count
        //alternate the pins
        P2OUT ^= BIT2;
        P2OUT ^= BIT4;

        //check if less or greater than the wanted duration
        if(buzzer_count >= buzzer_duration){
            buzzer_tone = -1; //turn it off
        }

        TA1CCR0 += buzzer_period;
    }

    //buzzer beeping
    if(buzzer_beep == 3){
        buzzer_count += 1;  //increment count

        if(buzzer_on) { //if it is on, alternate
            //alternate the pins
            P2OUT ^= BIT2;
            P2OUT ^= BIT4;
            //check if less or greater than the wanted duration
            if(buzzer_count >= buzzer_duration){
                buzzer_on = 0; //turn off
                buzzer_count = 0; //reset count
            }

        } else { //if off, turn both off
            P2OUT &= ~BIT2;
            P2OUT &= ~BIT4;

            //check if less or greater than the wanted duration
            if(buzzer_count >= buzzer_duration_off){
                buzzer_on= 1; //turn it on
                //buzzer count reset
                buzzer_count = 0;
                //turn one of them on
                P2OUT |= BIT2;
            }

        }


        TA1CCR0 += buzzer_period;
    }
}

//interrupt for timerA1, based on previous timers
#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer1_A1(void){
    switch(TA1IV)
    {
    case  2:
        //check for how long button was pressed
        if ((button == 4) && (held == 1)) {
            if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
                timer_count += 1;
                //offset TA1CCR1 by the count number/period
                TA1CCR1 += count_b;
            } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
                //send timer count signal here
                send_message = 1;
                send[0][1] = 1; //set flag
                send[1][1] = timer_count; //the time held

                led1_blink = 1;
                //testing with light first
               // P1OUT ^= BIT0;
                held = 0;                               //button has now been released
                TA1CCR1 = 0;
            }

        } else if (button == 4){ //check that it is the button debouncing
            if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                 timer_count = 0;                     //Reset the timer count
                 pressed = 0;                        //reset pressed
                 held = 1;                           //Button is being held
                 P1OUT ^= BIT6;
                 //offset TA0CCR0 by the count number/period
                 //change the value held in the array
                 timers_used[0][4] = count_b;
                 TA1CCR1 += count_b;

            } else {
                pressed = 0;                         //Button is not being pressed
            }

         }

        //button 2, copied code
        if ((button2 == 4) && (held2 == 1)) {
            if ((held2 == 1) && !(P1IN & BIT5)) { //check how long its being held for
                timer_count2 += 1;
                //offset TA0CCR0 by the count number/period
                TA1CCR1 += count_b2;
            } else if ((held2 == 1) && (P1IN & BIT5)){   //if button was released
                //send timer count signal here
                send_message = 1;
                send[0][1] = 1; //set flag
                send[1][1] = timer_count2; //the time held

                held2 = 0;                               //button has now been released
                TA1CCR1 = 0;
            }

        } else if (button2 == 4){ //check that it is the button debouncing
            if ((pressed2 == 1) && !(P1IN & BIT5)){   //button was pressed properly
                 timer_count2 = 0;                     //Reset the timer count
                 pressed2 = 0;                        //reset pressed
                 held2 = 1;                           //Button is being held
                 //change the value held in the array
                 timers_used[0][4] = count_b2;
                 TA1CCR1 += count_b2;

            } else {
                pressed2 = 0;                         //Button is not being pressed
            }

         }

        //pot stuff
        if(pot == 4){
            if (!stable_pot){

                //get original value, if possible here
                ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
                orig_val = ADC10MEM;

                //update timer value
                timers_used[0][4] = sample_time;
                stable_pot = 1;

            } else {
                sample_pot = 1;        //start a sample
            }

            TA1CCR1 += sample_time;
        }

        //thermometer stuff
        if(thermometer == 4){
            if (!stable){
                stable = 1;
                //get original value, if possible here
                ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
                temp = ADC10MEM;
                // oC = ((A10/1024)*1500mV)-986mV)*1/3.55mV = A10*423/1024 - 278, taken from example
                orig_temp = ((temp - 673) * 423) / 1024;

                //update timer value
                timers_used[0][4] = sample_temp_time;

            } else {
                sample_temp = 1;        //start a sample
            }

            TA1CCR1 += sample_temp_time;
        }

        //outputs
        //LED1 blinking
        if(led1_blink == 4){
            P1OUT ^= BIT0;
            TA1CCR1 += blink_rate_1;
        }

        //fading in/out and breathing, part 1, can be 13
        if((led1_fade_in == 13) || (led1_fade_out == 13) || (led1_breath == 13)){
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

        //fade in part 2, can only be 11
        if(led1_fade_in == 11) {
            if(light_flag_1 == 0){
                brightness_1 += 1;                //increase brightness
            }

            if(brightness_1 == max_brightness){               //if max brightness
                P1OUT |= BIT0;             //leave it on
                led1_fade_in = -1;          //completed task
            }

            TA1CCR1 += period_1;
        }

        //fade out part 2, can only be 11
        if(led1_fade_out == 11) {
            if(light_flag_1 == 1) {
                brightness_1 -= 1;                //decrease brightness
            }

             if (brightness_1 == 0) {       //reached lowest value
                P1OUT &= ~BIT0;                 //keep light off
                led1_fade_out = -1;          //completed task, deactivate it
            }

            TA1CCR1 += period_1;
        }

        //breathing part 2, can only be 11
        if(led1_breath == 11){
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

        //fading in/out and breathing, part 1, can be 13
        if((led2_fade_in == 13) || (led2_fade_out == 13) || (led2_breath == 13)){
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
        if(led2_fade_in == 11) {
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
        if(led2_fade_out == 11) {
            if(light_flag_2 == 1) {
                brightness_2 -= 1;                //decrease brightness
            }

             if (brightness_2 == 0) {       //reached lowest value
                P1OUT &= ~BIT6;                 //keep light off
                led2_fade_out = -1;          //completed task, deactivate it
            }

            TA1CCR1 += period_2;
        }

        //breathing part 2
        if(led2_breath == 11){
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
                colour = 5;
            } else if ((colour == 5) && (led3_dir == 1)){
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
                    colour = 5;
                } else if ((colour == 5) && (led3_dir == 1)){
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

        //fading in/out and breathing, part 1, can be 13
        if((led3_fade_in == 13) || (led3_fade_out == 13) || (led3_breath == 13)){
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

        //fade in part 2, can be 11
        if(led3_fade_in == 11) {
            if(light_flag_3 == 0){
                brightness_3 += 1;                //increase brightness
            }

            if(brightness_3 == max_brightness){               //if max brightness
                P2OUT |= colours[colour];             //leave it on
                led3_fade_in = -1;          //completed task
            }

            TA1CCR1 += period_3;
        }

        //fade out part 2, can be
        if(led3_fade_out == 11) {
            if(light_flag_3 == 1) {
                brightness_3 -= 1;                //decrease brightness
            }

             if (brightness_3 == 0) {       //reached lowest value
                P2OUT &= ~(BIT1 + BIT3 + BIT5);   //keep light off
                led3_fade_out = -1;          //completed task, deactivate it
            }

            TA1CCR1 += period_3;
        }

        //breathing part 2, can be
        if(led3_breath == 11){
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
                    colour = 5;
                } else if ((colour == 5) && (led3_dir == 1)){
                    colour = 0;
                } else {
                    colour = colour + led3_dir; //change colour based on direction
                }
            }

            TA1CCR1 += period_3;
        }

        //buzzer
        if(buzzer_tone == 4){
            buzzer_count += 1;  //increment count
            //alternate the pins
            P2OUT ^= BIT2;
            P2OUT ^= BIT4;

            //check if less or greater than the wanted duration
            if(buzzer_count >= buzzer_duration){
                buzzer_tone = -1; //turn it off
            }

            TA1CCR1 += buzzer_period;
        }

        //buzzer beeping
        if(buzzer_beep == 4){
            buzzer_count += 1;  //increment count

            if(buzzer_on) { //if it is on, alternate
                //alternate the pins
                P2OUT ^= BIT2;
                P2OUT ^= BIT4;
                //check if less or greater than the wanted duration
                if(buzzer_count >= buzzer_duration){
                    buzzer_on = 0; //turn off
                    buzzer_count = 0; //reset count
                }

            } else { //if off, turn both off
                P2OUT &= ~BIT2;
                P2OUT &= ~BIT4;

                //check if less or greater than the wanted duration
                if(buzzer_count >= buzzer_duration_off){
                    buzzer_on= 1; //turn it on
                    //buzzer count reset
                    buzzer_count = 0;
                    //turn one of them on
                    P2OUT |= BIT2;
                }

            }


            TA1CCR1 += buzzer_period;
        }

        break;
    case 4:
        //check for how long button was pressed
        if ((button == 5) && (held == 1)) {
            if ((held == 1) && !(P1IN & BIT3)) { //check how long its being held for
                timer_count += 1;
                //offset TA0CCR0 by the count number/period
                TA1CCR2 += count_b;
            } else if ((held == 1) && (P1IN & BIT3)){   //if button was released
                //send timer count signal here
                send_message = 1;
                send[0][1] = 1; //set flag
                send[1][1] = timer_count; //the time held

                led1_blink = 1;
                held = 0;                               //button has now been released
                TA1CCR2 = 0;

            }

        } else if (button == 5){ //check that it is the button debouncing
            if ((pressed == 1) && !(P1IN & BIT3)){   //button was pressed properly
                 timer_count = 0;                     //Reset the timer count
                 pressed = 0;                        //reset pressed
                 held = 1;                           //Button is being held
                 P1OUT ^= BIT6;
                 //offset TA0CCR0 by the count number/period
                 //change the value held in the array
                 timers_used[0][5] = count_b;
                 TA1CCR2 += count_b;

            } else {
                pressed = 0;                         //Button is not being pressed
            }

         }

        //button 2, copied code
        if ((button2 == 5) && (held2 == 1)) {
            if ((held2 == 1) && !(P1IN & BIT5)) { //check how long its being held for
                timer_count2 += 1;
                //offset TA0CCR0 by the count number/period
                TA1CCR2 += count_b2;
            } else if ((held2 == 1) && (P1IN & BIT5)){   //if button was released
                //send timer count signal here
                send_message = 1;
                send[0][1] = 1; //set flag
                send[1][1] = timer_count2; //the time held

                held2 = 0;                               //button has now been released
                TA1CCR2 = 0;
            }

        } else if (button2 == 5){ //check that it is the button debouncing
            if ((pressed2 == 1) && !(P1IN & BIT5)){   //button was pressed properly
                 timer_count2 = 0;                     //Reset the timer count
                 pressed2 = 0;                        //reset pressed
                 held2 = 1;                           //Button is being held

                 //change the value held in the array
                 timers_used[0][5] = count_b2;
                 TA1CCR2 += count_b2;

            } else {
                pressed2 = 0;                         //Button is not being pressed
            }

         }

        //pot stuff
        if(pot == 5){
            if (!stable_pot){

                //get original value, if possible here
                ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
                orig_val = ADC10MEM;

                //update timer value
                timers_used[0][5] = sample_time;
                stable_pot = 1;

            } else {
                sample_pot = 1;        //start a sample
            }

            TA1CCR2 += sample_time;
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
                //update timer value
                timers_used[0][5] = sample_temp_time;

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

        //fade in part 2, can be 12 or 13
        if((led1_fade_in == 12) || (led1_fade_in == 13)) {
            if(light_flag_1 == 0){
                brightness_1 += 1;                //increase brightness
            }

            if(brightness_1 == max_brightness){               //if max brightness
                P1OUT |= BIT0;             //leave it on
                led1_fade_in = -1;          //completed task
            }

            TA1CCR2 += period_1;
        }

        //fade out part 2, can be 12 or 13
        if((led1_fade_out == 12) || (led1_fade_out == 13)) {
            if(light_flag_1 == 1) {
                brightness_1 -= 1;                //decrease brightness
            }

             if (brightness_1 == 0) {       //reached lowest value
                P1OUT &= ~BIT0;                 //keep light off
                led1_fade_out = -1;          //completed task, deactivate it
            }

            TA1CCR2 += period_1;
        }

        //breathing part 2, can be 12 or 13
        if((led1_breath == 12) || (led1_breath == 13)){
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

        //fade in part 2, can be 12 or 13
        if((led2_fade_in == 12) || (led2_fade_in == 13)) {
            if(light_flag_2 == 0){
                brightness_2 += 1;                //increase brightness
            }

            if(brightness_2 == max_brightness){               //if max brightness
                P1OUT |= BIT6;             //leave it on
                led2_fade_in = -1;          //completed task
            }

            TA1CCR2 += period_2;
        }

        //fade out part 2, can be 12 or 13
        if((led2_fade_out == 12) || (led2_fade_out == 13)) {
            if(light_flag_2 == 1) {
                brightness_2 -= 1;                //decrease brightness
            }

             if(brightness_2 == 0) {       //reached lowest value
                P1OUT &= ~BIT6;                 //keep light off
                led2_fade_out = -1;          //completed task, deactivate it
            }

            TA1CCR2 += period_2;
        }

        //breathing part 2, can be 12 or 13
        if((led2_breath == 12) || (led2_breath == 13)){
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
                colour = 5;
            } else if ((colour == 5) && (led3_dir == 1)){
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
                    colour = 5;
                } else if ((colour == 5) && (led3_dir == 1)){
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

        //fade in part 2, can be 12 or 13
        if((led3_fade_in == 12) || (led3_fade_in == 13)) {
            if(light_flag_3 == 0){
                brightness_3 += 1;                //increase brightness
            }

            if(brightness_3 == max_brightness){               //if max brightness
                P2OUT |= colours[colour];             //leave it on
                led3_fade_in = -1;          //completed task
            }

            TA1CCR2 += period_3;
        }

        //fade out part 2, can be 12 or 13
        if((led3_fade_out == 12) || (led3_fade_out == 13)) {
            if(light_flag_3 == 1) {
                brightness_3 -= 1;                //decrease brightness
            }

             if(brightness_3 == 0) {       //reached lowest value
                P2OUT &= ~(BIT1 + BIT3 + BIT5); //keep light off
                led3_fade_out = -1;          //completed task, deactivate it
            }

            TA1CCR2 += period_3;
        }

        //breathing part 2, can be 12 or 13
        if((led3_breath == 12) || (led3_breath == 13)){
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
                    colour = 5;
                } else if ((colour == 5) && (led3_dir == 1)){
                    colour = 0;
                } else {
                    colour = colour + led3_dir; //change colour based on direction
                }
            }

            TA1CCR2 += period_3;
        }

        //buzzer
        if(buzzer_tone == 5){
            buzzer_count += 1;  //increment count
            //alternate the pins
            P2OUT ^= BIT2;
            P2OUT ^= BIT4;

            //check if less or greater than the wanted duration
            if(buzzer_count >= buzzer_duration){
                buzzer_tone = -1; //turn it off
            }

            TA1CCR2 += buzzer_period;
        }

        //buzzer beeping
        if(buzzer_beep == 5){
            buzzer_count += 1;  //increment count

            if(buzzer_on) { //if it is on, alternate
                //alternate the pins
                P2OUT ^= BIT2;
                P2OUT ^= BIT4;
                //check if less or greater than the wanted duration
                if(buzzer_count >= buzzer_duration){
                    buzzer_on = 0; //turn off
                    buzzer_count = 0; //reset count
                }

            } else { //if off, turn both off
                P2OUT &= ~BIT2;
                P2OUT &= ~BIT4;

                //check if less or greater than the wanted duration
                if(buzzer_count >= buzzer_duration_off){
                    buzzer_on= 1; //turn it on
                    //buzzer count reset
                    buzzer_count = 0;
                    //turn one of them on
                    P2OUT |= BIT2;
                }

            }


            TA1CCR2 += buzzer_period;
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
        switch(button){//Current count plus the wanted debounce part
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


/**
 * Helper functions
 */

/**
 * function for activating a specific timer, takes in the wanted timer to activate and the CCR value to use
 * returns 1 if successful. else 0
 */
int activate_timer(int timer_no, int count){
    if(timer_no == 0){
        TA0CTL = TASSEL_1 + MC_2 + TAIE;          // ACLK, continuous mode, interrupt enabled
        TA0CCTL0 |= CCIE;                         // TA0CCR0 interrupt enabled
        TA0CCR0 = count;

        timers_used[0][0] = count;
        timers_used[1][0] = 1;
        timer0_activated = 1;
        return 1;
    } else if (timer_no == 1){
        TA1CTL = TASSEL_1 + MC_2 + TAIE;          // ACLK, continuous mode, interrupt enabled
        TA1CCTL0 |= CCIE;

        TA1CCR0 = count;
        timers_used[0][3] = count;
        timers_used[1][3] = 1;

        timer1_activated = 1;
        return 1;
    }

    return 0;

}

/**
 * function for checking which timer is free, if alone is 1 then requested CCR needs to be on its own
 * Prioritises activation of timers with same CCR value that are not specified as alone
 * returns 1 for success, otherwise 0. changes the activated_timers array to indicate which timers were activated
 */
int activate_free_timer(int registers, int counts[], int alone){
    //check which registers are free, if less than the register amount, return error
    int potential[2] = {}; //potential registers, second row is the value held in the register
    int to_activate[2] = {0, 0}; //registers to activate
    unsigned int index = 0;
    unsigned int i, r;

    //check how many registers there are, currently max is 2
    if(registers == 1) {
        //loop through and look first for registers with the same counts, unless the timer should be alone
        if(alone) {
            for(i=0; i<5; i++){
                if(timers_used[0][i] == 0){
                   to_activate[0] = i;
                   index = 1; //found one
                   break; //no need to look for others
               }
            }

            //check that one was actually found
            if(index == 0) {
                //error
                return 0;
            } //else its fine, just continue
        } else { //can look for evidence of same register value, if free
            for(i=0; i<5; i++){
                //check that the timer is available for others
               if((timers_used[0][i] == counts[0]) && (timers_used[1][i] != -1)){
                  to_activate[0] = i;
                  index = 1;
                  break; //break out as one was found
               }
            }
            //if none was found, check for free ones
            if(index == 0) {
                for(i=0; i<5; i++){
                    //check for first free timer
                   if(timers_used[0][i] == 0){
                      to_activate[0] = i;
                      index = 1;
                      break; //break out as one was found
                   }
                }
            }
            //check that one was actually found, otherwise return 0
            if(index == 0){
                return 0;
            }
        }

    } else if (registers == 2){ //shouldn't have any that needs to be alone
        //check registers for ones with same period
        for(r=0; r<2; r++){
            for(i=0; i<5; i++){
                //check that the timer is available for others
               if((timers_used[0][i] == counts[r]) && (timers_used[1][i] != -1)){
                   if(r == 0){ //if first register
                       //need to verify that it is either 1, 3, or 4
                       if((i == 1) || (i == 3) || (i == 4)){
                           potential[0] = i;
                           index++;
                           break; //break out as one was found
                       } //if not invalid option
                   } else {
                       //need to verify that it is either 2, 4, or 5
                      if((i == 2) || (i == 4) || (i == 5)){
                          potential[1] = i;
                          index++;
                          break; //break out as one was found
                      } //if not invalid option
                   }
               }
            }
            //if none were found check for free one
            if(r == 0) {
                if(index == 0){
                    for(i=0; i<5; i++){
                       if(timers_used[0][i] == 0){
                           if((i == 1) || (i == 3) || (i == 4)){
                              potential[0] = i;
                              index++;
                              break; //break out as one was found
                          } //if not, invalid option
                       }
                    }
                }
            } else {
                if(index == 1){
                    for(i=0; i<5; i++){
                       if(timers_used[0][i] == 0){
                           //also need to check what the first potential one
                           if(potential[0] == 1){
                               if(i == 2){
                                   potential[1] = i;
                                   index++;
                                   break; //break out as one was found
                               }
                           } else if(potential[0] == 3){
                               if((i == 4) || (i == 5)){
                                   potential[1] = i;
                                   index++;
                                   break; //break out as one was found
                               }

                          } else if(potential[0] == 4){
                              if(i == 5){
                                  potential[1] = i;
                                  index++;
                                  break; //break out as one was found
                              }
                          }
                       }
                    }
                }
            }
        }

        //need to verify that 2 was found
        if(index != 2){
            return 0;
        } else { //if 2 were actually found then verify them
            //allowed combo: 1+2, 3+4, 3+5, 4+5
            if(potential[0] == 1){
                if(potential[1] == 2){
                    to_activate[0] = 1;
                    to_activate[1] = 2;
                } else {
                    return 0; //no valid match
                }
            } else if(potential[0] == 3){
                if(potential[1] == 4){
                    to_activate[0] = 3;
                    to_activate[1] = 4;
                } else if(potential[1] == 5) {
                    to_activate[0] = 3;
                    to_activate[1] = 5;
                } else {
                    //invalid match
                    return 0;
                }
            } else if(potential[0] == 4){
                if(potential[1] == 5){
                    to_activate[0] = 4;
                    to_activate[1] = 5;
                } else {
                    //invalid combo
                    return 0;
                }
            } else {
                //something went wrong, should actually never reach this
                return 0;
            }

        }

    }

    //if reached here then its fine

    ///reset activated timers
    activated_timers[0] = -1;
    activated_timers[1] = -1;

    //activate them
    for (i=0; i < registers; i++){
        //need to ensure that if more than 1 register are needed that they are from the same timer
        switch (to_activate[i]){
        case 0: //should never be reached, reserved timer
            return 0;
        case 1:
            //activate A0_1
            //if A0_0 wasn't available then A0 was already activated so don't need to check again
            if(timers_used[0][1] == 0){
                TA0CCTL1 |= CCIE;
                TA0CCR1 = counts[i];
            }
            //set it as active
            timers_used[0][1] = counts[i];
            //update count
            if(alone){
                timers_used[1][1] = -1;
            } else {
                timers_used[1][1] = timers_used[1][1] + 1;
            }
            //set it as activated
            activated_timers[i] = 1;
            break;
        case 2:
            //activate A0_2
            if(timers_used[0][2] == 0){
                TA0CCTL2 |= CCIE;
                TA0CCR2 = counts[i];
            }
            //set it as active
            timers_used[0][2] = counts[i];
            //update count
            if(alone){
                timers_used[1][2] = -1;
            } else {
                timers_used[1][2] = timers_used[1][2] + 1;
            }
            //set it as activated
            activated_timers[i] = 2;
            break;
        case 3:
            //activate A1_0check if A1 is already active
            if(!timer1_activated){
                activate_timer(1, counts[i]);
            } else if(timers_used[0][3] == 0) {
                TA1CCTL0 |= CCIE;
                TA1CCR0 = counts[i];
                timers_used[1][3] = timers_used[1][3] + 1;
            } else {
                //if already active, just increase nr of functions
                timers_used[1][3] = timers_used[1][3] + 1;
            }
            //set it as active
            timers_used[0][3] = counts[i];
            //update count
            if(alone){
                timers_used[1][3] = -1;
            } //already been activated otherwise
            //set it as activated
            activated_timers[i] = 3;
            break;
        case 4:
            //activate A1_1
            if(timers_used[0][4] == 0){
               TA1CCTL1 |= CCIE;
               TA1CCR1 = counts[i];
            }

            //set it as active
            timers_used[0][4] = counts[i];
            //update count
            if(alone){
                timers_used[1][4] = -1;
            } else {
                timers_used[1][4] = timers_used[1][4] + 1;
            }
            //set it as activated
            activated_timers[i] = 4;
            break;
        case 5:
            //activate A1_2
            if(timers_used[0][5] == 0){
                TA1CCTL2 |= CCIE;
                TA1CCR2 = counts[i];
            }
            //set it as active
            timers_used[0][5] = counts[i];
            //update count
            if(alone){
                timers_used[1][5] = -1;
            } else {
                timers_used[1][5] = timers_used[1][5] + 1;
            }
            //set it as activated
            activated_timers[i] = 5;
            break;
        default:
            return 0;
        }
    }
    //success
    return 1;

}

/**
 * function to turn off the timers activated - or set the count value to 0,
 * check if each value is now zero so the full timer should be turned off
 * returns nothing, changes the timers_used array to reflect the deactivations
 */
void deactivate_timer(int activated[], int len){
    unsigned int i;
    for(i=0; i < len; i++){
        int deactivate = activated[i];
        //need to ensure that the timer isn't used by others, and need to see if it is designated as alone
        //so check for > 0, as -1 needs to be set to 0 as well

        switch (deactivate){ //deactivate specific timer count register
        case 0:
            //should not be reached
            break;
        case 1:
            if(timers_used[1][1] > 1){ //assuming at least 1 as it will be deactivated
                //reduce by 1
                timers_used[1][1] = timers_used[1][1] - 1;
            } else {
                TA0CCR1 = 0;
                timers_used[0][1] = 0;
                timers_used[1][1] = 0;
            }
            break;
        case 2:
            if(timers_used[1][2] > 1){ //assuming at least 1 as it will be deactivated
                //reduce by 1
                timers_used[1][2] = timers_used[1][2] - 1;
            } else {
                TA0CCR2 = 0;
                timers_used[0][2] = 0;
                timers_used[1][2] = 0;
            }
            break;
        case 3:
            if(timers_used[1][3] > 1){ //assuming at least 1 as it will be deactivated
                //reduce by 1
                timers_used[1][3] = timers_used[1][3] - 1;
            } else {
                TA1CCR0 = 0;
                timers_used[0][3] = 0;
                timers_used[1][3] = 0;
            }
            break;
        case 4:
            if(timers_used[1][4] > 1){ //assuming at least 1 as it will be deactivated
                //reduce by 1
                timers_used[1][4] = timers_used[1][4] - 1;
            } else {
                TA1CCR1 = 0;
                timers_used[0][4] = 0;
                timers_used[1][4] = 0;
            }
            break;
        case 5:
            if(timers_used[1][5] > 1){ //assuming at least 1 as it will be deactivated
                //reduce by 1
                timers_used[1][5] = timers_used[1][5] - 1;
            } else {
                TA1CCR2 = 0;
                timers_used[0][5] = 0;
                timers_used[1][5] = 0;
            }
            break;
        }
    }

    //check if all registers for A1 is 0
    int all_1 = 0;
        for(i=3; i<6; i++){
            if(timers_used[0][i] == 0){
                all_1++;
            }
        }

        if(all_1 == 3){
            TA1CTL = MC_0;      //turn off A0
            timer1_activated = 0;
        }
}

/**
 * Function that returns the "code"/designated number for those who uses 2 timers
 * returns 0 if something went wrong
 */
int get_timer_code(int timers[]){
    int first = timers[0];
    int second = timers[1];

    switch (first){ //check which timer is first
    //first timers can only be 1, 3, and 4
    case 1:
        //get second value, should be 2
        if(second == 2){
             return 10;
        } else {
            //something went wrong, send error message
            send_message = 1;
            send[0][0] = 1;
            send[1][0] = 1; //generic
        }
        break;
    case 3:
        //get second value
        if(second == 4){
            return 11;
        } else if (second == 5){
            return 12;
        } else {
            //something went wrong, send error message
            send_message = 1;
            send[0][0] = 1;
            send[1][0] = 1; //generic
        }
        break;
    case 4:
        if(second == 5){
            return 13;
        } else {
            //something went wrong, send error message
            send_message = 1;
            send[0][0] = 1;
            send[1][0] = 1; //generic
        }
        break;
    default:
        //something went wrong, send error message
        send_message = 1;
        send[0][0] = 1;
        send[1][0] = 1; //generic
        break;
    }
    return 0; //error occurred.
}

/**
 * Small function to assist with improving breathing light functionality
 * when 1 or more breathing lights are being used, returns nothing
 */
void check_breath(void){
    //check if more than 1 breathing light is currently active
    int i;
    int active = 0;
    for(i=0; i<3; i++){
        if(lights_used[i] != 0){
            active++;
        }
    }

    if(active == 1){
        //if one was active, change to second values
        max_brightness = max_brightness2;
        change_period = change_period2;
        //need to reflect this in the registers, need to find which register is being used first
        for(i=0; i<5; i++){ //check for original change period
            if(timers_used[0][i] == change_period1){
                timers_used[0][i] = change_period2;
            }
        }

    } //if other assume this has already been changed

}


