# Control Axel GEW-004 433.92MHz remote extension cord from Arduino

http://www.nrx-china.com/productsd.php?pid=21&tid=7


# How to use
1. Load up RemoteCapture.ino and plug a 433.92MHz receiver to digital pin 2.
2. Open up Tools -> Serial Monitor in Arduino IDE and start pressing the remote controller (button).
3. Copy paste the 41 bit commands to Axel.ino for sendAxelCommand(). At least two different codes are needed.


# How to use with example commands
1. Set the extension cord into pairing mode by holding down its power button until the LED starts blinking.
2. Send a command, eg. "sendAxelCommand(AXEL_TOGGLE_1);", which stops the pairing mode.
3. Now you can control the cord by sending alternating commands to it. E.g. sendAxelCommand(AXEL_TOGGLE_1); (or AXEL_TOGGLE_2).
