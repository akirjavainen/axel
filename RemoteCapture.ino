/*
******************************************************************************************************************************************************************
*
* Axel GEW-004 remote extension cord
* 
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* Use this code to capture the commands from your remotes. Outputs to serial.
* What you need for mastering your cords are 41 bits commands.
* 
* 
* HOW TO USE
* 
* Plug a 433.92MHz receiver to digital pin 2 and start pressing buttons
* from your original remotes (copy pasting them to Axel.ino). You need
* a minimum of two different codes to control the extension cord.
*
******************************************************************************************************************************************************************
*/



// Plug your 433.92MHz receiver to digital pin 2:
#define RECEIVE_PIN   2

// Enable debug mode if there's no serial output or if you're modifying this code for
// another protocol/device. However, note that serial output delays receiving, causing
// data bits capture to fail. So keep debug disabled unless absolutely required:
#define DEBUG         false
#define ADDITIONAL    false    // Display some additional info after capture

#define COMMAND_LENGTH  41



// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  pinMode(RECEIVE_PIN, INPUT);
  Serial.begin(9600);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
  int i = 0;
  unsigned long t = 0;
  String command = "";


  // *********************************************************************
  // Wait for the AGC bit:
  // *********************************************************************
  // HIGH between 7300-7800 us
  // *********************************************************************
  
  while (t < 7300 || t > 7800) {
    t = pulseIn(RECEIVE_PIN, LOW, 1000000); // Waits for a HIGH waveform spike (low-HIGH-low)
  }

  if (DEBUG) {
    Serial.print("AGC: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Command bits, locate them simply by HIGH waveform spikes:
  // *********************************************************************  
  // 0 = 150-330 us
  // 1 = 600-800 us
  // *********************************************************************

  while (i < COMMAND_LENGTH) {
    t = pulseIn(RECEIVE_PIN, LOW, 1000000); // Waits for a HIGH waveform spike (low-HIGH-low)
    
    if (DEBUG) {
      Serial.print(t);
      Serial.print(": ");
    }

    if (t > 150 && t < 330) { // Found 0
      command += "0";
      if (DEBUG) Serial.println("0");

    } else if (t > 600 && t < 800) { // Found 1
      command += "1";
      if (DEBUG) Serial.println("1");
      
    } else { // Unrecognized bit, finish
      if (DEBUG) Serial.println("INVALID BIT");
      i = 0;
      break;
    }

    i++;
  }

  // *********************************************************************
  // Done! Display results:
  // *********************************************************************  

  // Correct data bits length is 41 bits, dismiss bad captures:
  if (command.length() != COMMAND_LENGTH) {

    // These buttons cause many failed captures, so only show error
    // messages if the user really wants to see them:
    if (ADDITIONAL || DEBUG) {
      Serial.print("Bad capture, invalid command length ");
      Serial.println(command.length());
      Serial.println("Invalid command: " + command);
      Serial.println();
    }
    
  } else {
    Serial.println("Successful capture, command is: " + command);
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
