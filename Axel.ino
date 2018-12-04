/*
******************************************************************************************************************************************************************
*
* Axel GEW-004 remote extension cord
* Sold by Motonet
* 
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* http://www.nrx-china.com/productsd.php?pid=21&tid=7
* 
* 
* HOW TO USE
* 
* Capture your remote button (at least two different codes) with
* RemoteCapture.ino and copy paste the 41 bit commands to Axel.ino
* for sendAxelCommand(). More info about this provided in RemoteCapture.ino.
* 
* 
* HOW TO USE WITH EXAMPLE COMMANDS
* 
* 1. Set the extension cord into pairing mode by holding down its power
*    button until the LED starts blinking.
* 2. Send a command, eg. "sendAxelCommand(AXEL_TOGGLE_1);", which stops
*    the pairing mode.
* 3. Now you can control the cord by sending alternating commands to it.
*    E.g. sendAxelCommand(AXEL_TOGGLE_1); (or AXEL_TOGGLE_2).
*
* 
* PROTOCOL DESCRIPTION
* 
* ON/OFF commands seem to be mostly consisting of random values.
* State (ON or OFF) changes when the next command differs from
* the one previously sent.
*
* All sample counts below listed with a sample rate of 44100 Hz
* (sample count / 44100 = microseconds).
*
* AGC is one HIGH of approx. 334 samples.
* 41 command bits are approx. 40 samples long, 10-30 or 30-10.
*
* Data 0 = LOW-LOW-LOW-HIGH (wire 0001)
* Data 1 = LOW-HIGH-HIGH-HIGH (wire 0111)
* 
* Ends with radio silence of approx. 10 samples.
* 
* 
* TOGGLE COMMAND EXAMPLES
* 
* These are just some of the commands I've captured from my
* power buttons:
* 
* Button 1:
* 0000000100100100100100000011101010110110
* 0001011010110111011000010000100111001001
* 0010000100000100101100000001101010010110
* 0100000101100100110100000111101011110110
* 0101011011110111001000010100100110001001
* 0110000101000100111100000101101011010110
* 1001011000110111111000011000100101001001
* 1010000110000100001100001001101000010110
* 1100000111100100010100001111101001110110
* 1101011001110111101000011100100100001001
* 1110000111000100011100001101101001010110
*
* Button 2:
* 0000111000101100101110000001001010011110
* 0001011000110100101000000000101010000110
* 0010011000000100100100000011101010110110
* 0011011000010100100000000010101010100110
* 0100111001101100111110000101001011011110
* 0101011001110100111000000100101011000110
* 0111011001010100110000000110101011100110
* 1001011010110100001000001000101000000110
* 1010011010000100000100001011101000110110
* 1011011010010100000000001010101000100110
* 1100111011101100011110001101001001011110
* 1101011011110100011000001100101001000110
* 1111011011010100010000001110101001100110
* 
* ******************************************************************************************************************************************************************
*/



// The extension cords don't seem to be picky about the commands received.
// These two commands are actually enough to control them (once paired to the receiver):
#define AXEL_TOGGLE_1   "1001001000110011111001011000110101001101"
#define AXEL_TOGGLE_2   "0010010110000011011101010001110111011101"

// Timings in microseconds (us). Get sample count by zooming all the way in to the waveform with Audacity.
// Calculate microseconds with: (samples / sample rate, usually 44100 or 48000) - ~15-20 to compensate for delayMicroseconds overhead.
// Sample counts listed below with a sample rate of 44100 Hz:
#define AXEL_AGC1_PULSE               7560   // 334 samples
#define AXEL_RADIO_SILENCE            220    // 10 samples
#define AXEL_PULSE_SHORT              220    // 10 samples
#define AXEL_PULSE_LONG               670    // 30 samples (approx. PULSE_SHORT * 3)
#define AXEL_COMMAND_BIT_ARRAY_SIZE   41     // Command bit count

#define TRANSMIT_PIN 13 // We'll use digital 13 for transmitting
#define REPEAT_COMMAND 10 // How many times to repeat the same command: original remote repeats as long as button is down


// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600); // Used for error messages
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {
  
  sendAxelCommand(AXEL_TOGGLE_1);
  delay(3000);
  sendAxelCommand(AXEL_TOGGLE_2);
  delay(3000);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendAxelCommand(String command) {

  // Prepare for transmitting and check for validity
  pinMode(TRANSMIT_PIN, OUTPUT); // Prepare the digital pin for output
  
  // rcswitch adds an ending zero, so we retain command
  // compatibility with the library by adding it here:
  command = command + "0";

  if (command.length() < AXEL_COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendAxelCommand(): Invalid command (too short), cannot continue.");
    return;
  }
  if (command.length() > AXEL_COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendAxelCommand(): Invalid command (too long), cannot continue.");
    return;
  }

  // Declare the array (int) of command bits
  int command_array[AXEL_COMMAND_BIT_ARRAY_SIZE];

  // Processing a string during transmit is just too slow,
  // let's convert it to an array of int first:
  convertStringToArrayOfInt(command, command_array, AXEL_COMMAND_BIT_ARRAY_SIZE);
  
  // Repeat the command:
  for (int i = 0; i < REPEAT_COMMAND; i++) {
    doTriStateSend(command_array, AXEL_COMMAND_BIT_ARRAY_SIZE, AXEL_AGC1_PULSE, AXEL_RADIO_SILENCE, AXEL_PULSE_SHORT, AXEL_PULSE_LONG);
  }

  // Disable output to transmitter to prevent interference with
  // other devices. Otherwise the transmitter will keep on transmitting,
  // which will disrupt most appliances operating on the 433.92MHz band:
  digitalWrite(TRANSMIT_PIN, LOW);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void doTriStateSend(int *command_array, int command_array_size, int pulse_agc1, int pulse_radio_silence, int pulse_short, int pulse_long) {
  if (command_array == NULL) {
    errorLog("doTriStateSend(): Array pointer was NULL, cannot continue.");
    return;
  }

  // Starting (AGC) bits:
  transmitWaveformHigh(pulse_agc1);

  // Transmit command:
  for (int i = 0; i < command_array_size; i++) {

    // If current command bit is 0, transmit LOW-LOW-LOW-HIGH:
    if (command_array[i] == 0) {
      transmitWaveformLow(pulse_long);
      transmitWaveformHigh(pulse_short);
    }

    // If current command bit is 1, transmit LOW-HIGH-HIGH-HIGH:
    if (command_array[i] == 1) {
      transmitWaveformLow(pulse_short);
      transmitWaveformHigh(pulse_long);
    }
   }

  // Radio silence at the end.
  // It's better to rather go a bit over than under required length.
  transmitWaveformLow(pulse_radio_silence);

  digitalWrite(TRANSMIT_PIN, LOW);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitWaveformHigh(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, LOW); // Digital pin low transmits a high waveform
  //PORTB = PORTB D13low; // If you wish to use faster PORTB commands instead
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitWaveformLow(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, HIGH); // Digital pin high transmits a low waveform
  //PORTB = PORTB D13high; // If you wish to use faster PORTB commands instead
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
int convertStringToInt(String s) {
  char carray[2];
  int i = 0;
  
  s.toCharArray(carray, sizeof(carray));
  i = atoi(carray);

  return i;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void convertStringToArrayOfInt(String command, int *int_array, int command_array_size) {
  String c = "";

  if (int_array == NULL) {
    errorLog("convertStringToArrayOfInt(): Array pointer was NULL, cannot continue.");
    return;
  }
 
  for (int i = 0; i < command_array_size; i++) {
      c = command.substring(i, i + 1);

      if (c == "0" || c == "1") {
        int_array[i] = convertStringToInt(c);
      } else {
        errorLog("convertStringToArrayOfInt(): Invalid character " + c + " in command.");
        return;
      }
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void errorLog(String message) {
  Serial.println(message);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
