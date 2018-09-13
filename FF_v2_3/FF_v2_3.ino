/*
FrostFree Master Control

Description:
  This code is designed to operate an Arduino-based control system for the FF-2.3 produced by Stanley Solutions.
  This code and the prototype device it is utilized in is inteded for non-sale personal use only.
  This code will interpret inputs based on engine operation status and radio communication input;
  based on these inputs, it will operate certain outputs to control the Block Heater power supply.

Version 2.3 Update:
  This project update relates software updates for hardware functionality for the FF-2.1.
  It maintains original segments of code from the FF-1.4 and introduces new functionality and patches.
  FF-2.1 Fixes include:
     -New startup procedure including:
        +Automated radio startup
        +Automatic radio feedback check
     -New communication features including:
        +New comm chanel
  FF-2.2 Fixes include:
     -External LED Output
        +Send error codes to dashpanel LEDs
  FF-2.3 Fixes include:
     -Voltage measurement correction
        +Analog input averaging
        +Test proven voltconst value
  
Design:
  This design was created by Joe Stanley for the FrostFree: Stanley Solutions FF-2.3.
  Solely for use in FF-2.3
  
Update:
  This project was last updated:  05/03/2017
  Version:   FF-2.3
  This project has been released for prototype service.
  This project's status is: Active Prototype.
  
Contact:
  Joe Stanley
  Electrical Engineering Student
  University of Idaho
  (208) 827-2594
  engineerjoe440@yahoo.com
  stan3926@vandals.uidaho.edu
  
For further specifications contact above individual(s).
*/


// Define All I/O Pin Allocation
const int radioRXpin = 0;
const int radioTXpin = 1;
const int radioDONEpin = 2;
const int timerOUTpin = 3;
const int solenoidRLYpin = 4;
const int inverterRINpin = 5;
const int inverterGINpin = 6;
const int markerREDpin = 7;
const int markerGRNpin = 8;
const int radioSTARTpin = 9;
const int indicatorPIN = 13;
const int timerREADpin = 11;
const int chargeRLYpin = 12;
const int errorLEDpin = 10;

// Define analog pins
const int voltageREADpin = A0;

// Define radio communication terms
#define radiopowerON 1
#define tenfourtone 2
#define heatercomplete 3
// Define LED monitor terms
#define heaterON 1
#define heaterOFF 2
#define altCONNECT 3

// Define voltage test constant
#define voltconst (245*32)


/* Initialization Function                                                *
 * This function performs all necessary initialization tasks.             */
void setup() {
  boolean problem = false; // Used for radio tests
  
  // Initialize I/O pins
  pinMode(radioRXpin, INPUT);
  pinMode(inverterRINpin, INPUT);
  pinMode(inverterGINpin, INPUT);
  pinMode(timerREADpin, INPUT);
  pinMode(voltageREADpin, INPUT);
  pinMode(radioTXpin, OUTPUT);
  pinMode(radioDONEpin, OUTPUT);
  pinMode(chargeRLYpin, OUTPUT);
  pinMode(solenoidRLYpin, OUTPUT);
  pinMode(markerREDpin, OUTPUT);
  pinMode(markerGRNpin, OUTPUT);
  pinMode(radioSTARTpin, OUTPUT);
  pinMode(indicatorPIN, OUTPUT);
  pinMode(timerOUTpin, OUTPUT);
  pinMode(errorLEDpin, OUTPUT);

  
  // Set Outputs to Normal positions
  digitalWrite(radioTXpin, HIGH);     // Controlled inversely by relay module
  digitalWrite(radioDONEpin, HIGH);   // Controlled inversely by relay module
  digitalWrite(chargeRLYpin, LOW);
  digitalWrite(solenoidRLYpin, HIGH); // Controlled inversely by relay module
  digitalWrite(markerREDpin, LOW);
  digitalWrite(markerGRNpin, LOW);
  digitalWrite(radioSTARTpin, HIGH);  // controlled inversely by relay module
  digitalWrite(indicatorPIN, LOW);
  
  // Turn on radio, if an error is detected, indicate it.
  if (radio_operate( radiopowerON )) { error_indicator( radiopowerON ); }
  
  
  // Transmit "ALIVE" signal via Radio, if an error is detected, indicate it.
  if (radio_operate( tenfourtone )) { error_indicator( tenfourtone ); }
}// setup breakpoint

/* MAIN Void Loop Function                                                    *
 * This is the main body of the program, it calls the necesary functions for  *
 * proper operation of the FrostFree system.                                  *
 * Function accepts no arguments and returns nothing.                         *
 * void loop()                                                                */
//Operating Commands:
void loop()
{
 // Try to connect to the alternator for charging
 alternator_connect();
 
 // If communication recieved from radio
 if (!digitalRead(radioRXpin))
 {
   time_delay(1750); // Wait for radio chatter to subside.
   // Turn on Inverter and Heater
   digitalWrite(solenoidRLYpin, LOW); // Inverter Relay is inversely controlled.
   // Transmit "RECEIVED" signal via Radio, if an error is detected, indicate it.
   if (radio_operate( tenfourtone )) { error_indicator( tenfourtone ); }
   // Output LED Monitor
   LED_monitor(heaterON);
 }//if breakpoint
 // If the inverter has shut the heater down
 else if (!digitalRead(inverterRINpin) && digitalRead(inverterGINpin))
 {
   // Turn off the Inverter
   digitalWrite(solenoidRLYpin, HIGH); // Inverter Relay is inversely controlled.
   // Transmit "HEATER COMPLETE" signal via Radio, if an error is detected, indicate it.
   if (radio_operate( heatercomplete )) { error_indicator( heatercomplete ); }
   // Output LED Monitor
   LED_monitor(heaterOFF);
 }// else if breakpoint
}//loop breakpoint



/* Radio Operate Function                                                     *
 * This function is designed to complete radio control operations.            *
 *                                                                            *
 * Function accepts one integer and returns a bool value.                     *
 *                                                                            *
 * boolean radio_transmit(int command);                                       */

boolean radio_operate( int command )
{
  // Two variables used for checking the radio's status
  boolean radiotalk = true;
  boolean timeout = false;
  boolean error_radio = false;
  
  do
  {
    error_radio = timeout; // Set error to timeout to allow error processing once
    // Switch based on what command was passed
    switch (command)
    {
      case radiopowerON:
        digitalWrite(radioSTARTpin, LOW);    // Turn on radio
        digitalWrite(timerOUTpin, HIGH);     // Turn on timer
        time_delay(100);                      // Wait for timer to turn on
        while ( (radiotalk) /*&& (!timeout)*/ ) // Wait for radio comm. or timeout
        {
          radiotalk = digitalRead( radioRXpin ); // Read the radio input
          timeout = digitalRead( timerREADpin ); // Read the timer input
        }// While breakpoint
        digitalWrite(radioSTARTpin, HIGH); // Stop radio turn on proceedure
        time_delay(100);
        break;
      
      
      case tenfourtone:
        digitalWrite(radioTXpin, LOW);       // Send radio message
        digitalWrite(timerOUTpin, HIGH);     // Turn on timer
        time_delay(1000);                      // Wait for timer to turn on
        while ( (radiotalk) && (!timeout) ) // Wait for radio comm. or timeout
        {
          radiotalk = digitalRead( radioRXpin ); // Read the radio input
          timeout = digitalRead( timerREADpin ); // Read the timer input
          digitalWrite(radioTXpin, HIGH); // Turn TX pin off
        }// While breakpoint
        time_delay(100);
        break;
      
      
      case heatercomplete:
        digitalWrite(radioDONEpin, LOW);     // Send radio message
        digitalWrite(timerOUTpin, HIGH);     // Turn on timer
        time_delay(100);                      // Wait for timer to turn on
        while ( (radiotalk) && (!timeout) ) // Wait for radio comm. or timeout
        {
          radiotalk = digitalRead( radioRXpin ); // Read the radio input
          timeout = digitalRead( timerREADpin ); // Read the timer input
          digitalWrite(radioDONEpin, HIGH); // Turn TX pin off
        }// While breakpoint
        time_delay(1750);
        break;
      
      
      default:
        // Toggle indicator LED once every second three times to indicate error.
        time_delay(1000);
        delay(1000);
        time_delay(1000);
        delay(1000);
        time_delay(1000);
        break;
      } // Switch breakpoint
  } while ( (timeout) && (!error_radio) ); //do-while breakpoint: test for timeout condition
  
  if (error_radio) error_radio = timeout; // Erase any incorrect errors. 
  digitalWrite(timerOUTpin, LOW); // Turn off timer
  time_delay(2500);               // Wait for chatter to stop
  return error_radio; // Return to main a value that describes if an error occured.
}// radio_operate breakpoint



/* Time Delay Control Function                                                *
 * This function is designed to perform a time delay and toggle an output     *
 * that will represent its activity.                                          *
 * Function accepts one integer and returns nothing.                          *
 *                                                                            *
 * void time_delay(unsigned int period);                                      */

void time_delay(unsigned int period)
{
  digitalWrite(indicatorPIN, HIGH); // Turn on indicator for instrumentation
  delay( period ); // Delay a certain period of time
  digitalWrite(indicatorPIN, LOW); // Turn indicator off
}



/* Error Signal Function                                                      *
 * This function is designed to output a flashing LED code to indicate        *
 * various types of errors.                                                   *
 * Function accepts one integer and returns nothing.                          *
 *                                                                            *
 * void error_indicator(unsigned int err_cod)                                 */
void error_indicator(int err_cod)
{
  switch (err_cod)
  {
    
    case radiopowerON:  // An error has occured in the radio startup:
      while (1) // Lock into error handling mode
      {
        // Flash Error Signal
        digitalWrite(errorLEDpin, HIGH);
        digitalWrite(markerREDpin, HIGH);
        delay(1000);
        digitalWrite(errorLEDpin, LOW);
        digitalWrite(markerREDpin, LOW);
        delay(1000);
      }// while breakpoint
      break; // The error handler should never reach this point.
      
      
    case tenfourtone:  // An error has occured in the radio transmission
      while (1) // Lock into error handling mode
      {
        // Flash Error Signal
        digitalWrite(errorLEDpin, HIGH);
        digitalWrite(markerREDpin, HIGH);
        delay(1000);
        digitalWrite(errorLEDpin, LOW);
        digitalWrite(markerREDpin, LOW);
        delay(100);
      }// while breakpoint
      break; // The error handler should never reach this point.
      
      
    case heatercomplete:  // An error has occured in the heatercomplete transmission
      while (1) // Lock into error handling mode
      {
        // Flash Error Signal
        digitalWrite(errorLEDpin, HIGH);
        digitalWrite(markerREDpin, HIGH);
        delay(5000);
        digitalWrite(errorLEDpin, LOW);
        digitalWrite(markerREDpin, LOW);
        delay(1000);
      }// while breakpoint
      break; // The error handler should never reach this point.
      
  }// Switch Statement Breakpoint
}//error_indicator breakpoint



/* Alternator Connection Function                                     *
 * This function determies whether the vehicle's alternator is        *
 * active, while it is, this function turns off the heater, activates *
 * the charging circuit, and outputs a signal to the user LED in the  *
 * vehicle.                                                           *
 * Function has no input, and returns nothing.                        *
 * void alternator_connect(void)                                      */
void alternator_connect(void)
{
  static unsigned voltarr[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  static unsigned short arrptr = 0;
  unsigned long voltage = 0;
  unsigned int input, ind;
  
  // Read and store analog voltage
  voltarr[arrptr] = analogRead(voltageREADpin);
  for(ind = 0; ind < 32; ind++)
  {
    voltage = voltage + voltarr[ind];
  }
  // Increment and mask array pointer
  arrptr = (0x1F & (1 + arrptr));
  
  // While the voltage is greater than 13V, connect to alternator
  if(voltage > voltconst)
  {
    // Disable Inverter, output is inverted by relay module
    digitalWrite(solenoidRLYpin, HIGH);
    // Output to LED Monitor
    LED_monitor(altCONNECT);
    // Connect to the Alternator Input
    digitalWrite(chargeRLYpin, HIGH);
    // Read voltage again
    voltage = analogRead(voltageREADpin);
  }//if breakpoint
  else // voltage is less than voltconst
  {
    // Clear LED Monitor if still charging
    if (digitalRead(chargeRLYpin)) { LED_monitor(0); }
    // Turn off charching circuit
    digitalWrite(chargeRLYpin, LOW);
  }
}//alternator_connect breakpoint



/* User LED Monitor Control                                           *
 * This function controlls the LED Monitor display in the vehicle's   *
 * cab. It follows the following output scheme:                       *
 * Green  : Inverter Active, Heater Enabled                           *
 * Red    : Inverter Active, Heater Disabled                          *
 * Yellow : Alternator Connection Active                              *
 * If any other value is passed, this function will clear the LED     *
 * Function acceptes one integer and returns nothing.                 *
 * void LED_monitor(int mon)                                          */
void LED_monitor(int mon)
{
  switch(mon)
  {
   case heaterON: // Set the output LED Green
     digitalWrite(markerGRNpin, HIGH);
     digitalWrite(markerREDpin, LOW);
     break;
   case heaterOFF: // Set the output LED Red
     digitalWrite(markerGRNpin, LOW);
     digitalWrite(markerREDpin, HIGH);
     break;
   case altCONNECT: // Set the output LED Yellow
     digitalWrite(markerGRNpin, HIGH);
     digitalWrite(markerREDpin, HIGH);
     break;
   default: // Clear the output LED
     digitalWrite(markerGRNpin, LOW);
     digitalWrite(markerREDpin, LOW);
     break;
  }// Switch-case breakpoint
}// LED_monitor Breakpoint

