/*
FrostFree Master Control

Description:
  This code is designed to operate an Arduino-based control system for the FF-1.4 produced by Stanley Solutions.
  This code and the prototype device it is utilized in is inteded for non-sale personal use only.
  This code will interpret inputs based on engine operation status and radio communication input;
  based on these inputs, it will operate certain outputs to control the Block Heater power supply.
  
Design:
  This design was created by Joe Stanley for the FrostFree: Stanley Solutions FF-1.4.
  Solely for use in FF-1.4
  
Update:
  This project was last updated:  12/06/2016
  Version:   FF-1.4
  This project has been released as a "Functional Prototype".
  
Contact:
  Joe Stanley
  Electrical Engineering Student
  University of Idaho
  (208) 827-2594
  engineerjoe440@yahoo.com
  
For further specifications contact above individual(s).
*/


// Declare All I/O Pins
const int radioRXpin = 2;
const int radioTXpin = 7;
const int ignitionPin = 4;
const int engineSTApin = 5;
const int chargeRLYpin = 10;
const int solenoidRLYpin = 6;

// Initialization Commands:
void setup() {
  // Initialize I/O pins
  pinMode(radioRXpin, INPUT);
  pinMode(engineSTApin, INPUT);
  pinMode(ignitionPin, INPUT);
  pinMode(radioTXpin, OUTPUT);
  pinMode(chargeRLYpin, OUTPUT);
  pinMode(solenoidRLYpin, OUTPUT);
  
  // Set Outputs to Normal positions
  digitalWrite(radioTXpin, HIGH);
  digitalWrite(chargeRLYpin, LOW);
  digitalWrite(solenoidRLYpin, HIGH);
  
  // Transmit "ALIVE" signal via Radio
  delay(1500);
  digitalWrite(radioTXpin, LOW);
  delay(800);
  digitalWrite(radioTXpin, HIGH);
  delay(8000);
  
  // Set additional Outputs to Normal positions
  digitalWrite(chargeRLYpin, LOW);
  digitalWrite(solenoidRLYpin, HIGH);
}


//Operating Commands:
void loop(){
  // Activate Charging when ignition is ON and battery light is OFF
  // These inputs are LOW when they are active; i.e. they are inverted
  while((digitalRead(ignitionPin) == LOW) && (digitalRead(engineSTApin) == LOW)){
    // If the charging circuit output hasn't been set, then set it
    if (digitalRead(chargeRLYpin) == LOW){
      digitalWrite(chargeRLYpin, HIGH);
    }
    // If the power inversion system hasn't been shut off, do so
    if (digitalRead(solenoidRLYpin) == LOW){
      digitalWrite(solenoidRLYpin, HIGH);
    }
  }
  // Turn the charging circuit off when the conditions are no longer met
  if (digitalRead(chargeRLYpin) == HIGH){
    digitalWrite(chargeRLYpin, LOW);
  }
  
  // Activate the power inversion circuit when signal is recieved from the radio and the ignition is OFF
  if ((digitalRead(radioRXpin) == LOW) && (digitalRead(ignitionPin) == HIGH)){
    // Turn on power inversion circuit
    digitalWrite(solenoidRLYpin, LOW);
    // Delay 3 seconds then send radio transmission to signal system response
    delay(3000);
    digitalWrite(radioTXpin, LOW);
    delay(500);
    digitalWrite(radioTXpin, HIGH);
    delay(8000);
  }
  
  // When the ignition is turned on, turn the power inversion system OFF
  if (digitalRead(ignitionPin) == LOW){
    digitalWrite(solenoidRLYpin, HIGH);
  }
}
