// This routine sends commands to a  modem that result in an SMS, with
// a message/text, to be sent to the given mobile phone number.

// INPUTS: -------------------------------------------------------------------

// modemDevName	- modem device/file descriptor name, usually /dev/ttyUSB0
// msgTxt 	- message text for the SMS
// phoneNum	- phone number for the SMS

// ---------------------------------------------------------------------------

// OUTPUTS: ------------------------------------------------------------------
// returns 0	- all ok
// returns 1	- error in one of the function/system calls

// ---------------------------------------------------------------------------
// History:

// 13sep2016    Simone Magri    Original routine
// 17sep2016	Simone Magri	convert to function from writeSerialPort.cpp

// End History.
// --------------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
 


int sendSMS(char *modemDevName, char *phoneNum,
	    char *msgTxt, int msgTxtLength){


  int ttyFD; // Serial port file descriptor, or unix file handle.
  int retval; // generic return value from function calls

  unsigned char echoMode[] = "ATE1\r\n";
  unsigned char setTxtMode[] = "AT+CMGF=1\r\n";
  //unsigned char phoneNum[] = "AT+CMGS=\"+61401858908\"\r\n";
  char phoneNumSMScmd[] = "AT+CMGS=\x22+61401858908\x22\r\n";
  //unsigned char msgTxt[] = "Payload has arrived.\r\n";
  unsigned char endMsg[] = "\x1A"; // == Control Z
  unsigned char charRead[8]; // character read from the serial port/ttyFD


  // Input serial port to communicate with as command line argument
  //printf("Please start with %s /dev/ttyUSB0 (for example)\n", argv[0]);
  fprintf(stderr, "dbg: sendSMS: Will open device %s\n", modemDevName);

     
  // Open the terminal/serial port stream: as read, write and blocking.
  ttyFD = open(modemDevName, O_RDWR);      
  if (ttyFD == -1){
    perror("sendSMS");
    fprintf(stderr, "sendSMS: error while opening modem device.\n");
    return 1;
  }

  // Open the terminal/serial port stream: as read, write and non-blocking.
  //  ttyFD = open(argv[1], O_RDWR | O_NONBLOCK);

  // Loop till stdin/console input is character 'q'.
  //while (readChar != 'q'){

  // Can make non-blocking with use of select() call here.


  // NOTE: The  best way to see  what is being sent  and recieved from
  // the serial  port is the 'strace' program;  use 'cat /dev/ttyUSB0'
  // to   see  what   you  are   sending  to   the  serial   port.  So
  // fprintf/printf's don't really show you all that is going one.

  // Start sending SMS:
  //
  // A couple of test commands to get everything in sync.
  if (  ( retval = write(ttyFD, "AT\r\n", 4) == -1 )  ){
    perror("sendSMS");
    fprintf(stderr,
	    "dbg: sendSMS: write echoMode returned error.%d\n", errno);
    return 1;
  }
  //sleep(1);
  retval = read(ttyFD, charRead, sizeof(charRead));

  if (  ( retval = write(ttyFD, "AT\r\n", 4) == -1 )  ){
    perror("sendSMS");
    fprintf(stderr,
	    "sendSMS: write echoMode returned error.%d\n", errno);
    return 1;
  }
  //sleep(1);

  // Reset the modem just to be sure.
  if (  ( retval = write(ttyFD, "ATZ\r\n", 5) == -1 )  ){
    perror("sendSMS");
    fprintf(stderr,
	    "sendSMS: write echoMode returned error.%d\n", errno);
    return 1;
  }
  //sleep(1);

  // Turn On 'Echo mode'.  So 'cat /dev/ttyUSB0' shows you what is
  // being sent to the modem.
  //
  // Note the count=( sizef(echoMode)-1  ) stops the string delimiter,
  // a NULL, from being sent to the modem.  The modem doesn't need the
  // null.
  if (  ( retval = write(ttyFD, echoMode, (sizeof(echoMode)-1)) == -1 )  ){
    perror("sendSMS");
    fprintf(stderr,
	    "sendSMS: write echoMode returned error.%d\n", errno);
    return 1;
  }
  //sleep(1);

  // Set 'Text Mode On'.
  if (  ( retval = write(ttyFD, setTxtMode, (sizeof(setTxtMode)-1)) ) == -1  ){
    perror("sendSMS");
    fprintf(stderr, "sendSMS: write setTxtMode returned error.\n");
    return 1;
  }
  sleep(1);

  // Send phone number where SMS will be sent too.
  //
  // Construct phone number SMS command eg: "AT+CMGS=\"+61401858908\"\r\n"
  strcpy(phoneNumSMScmd, "AT+CMGS=\"");
  strcat(phoneNumSMScmd, phoneNum);
  strcat(phoneNumSMScmd, "\"\r\n");
  fprintf(stderr, "dbg: sendSMS: New phoneNumSMScmd=%s\n", phoneNumSMScmd);

  if (  ( retval = write(ttyFD, phoneNumSMScmd,
			 (sizeof(phoneNumSMScmd)-1)) ) == -1  ){
    perror("sendSMS");
    fprintf(stderr, "sendSMS: write phoneNumSMScmd returned error.\n");
    return 1;
  }
  sleep(1);


  // if ( (retval = read(ttyFD, &readChar, sizeof(readChar))) == -1){
  //    perror("sendSMS");
  //    fprintf(stderr,
  // 	     "sendSMS: read greater than sign returned error.\n");
  //    return 1;
  // }
  // fprintf(stderr, "%c", readChar);
  // fprintf(stderr,"Read >, the  output from sending phone number.\n");


  // Write text of SMS.
  fprintf(stderr,"dbg: sendSMS: msgTxtLength %d\n", msgTxtLength);

  if (  ( retval = write(ttyFD, msgTxt, msgTxtLength) ) == -1  ){
    perror("sendSMS");
    fprintf(stderr, "sendSMS: write msgTxt returned error.\n");
    return 1;
  }
  //sleep(1);
  
  // Write Control-z to modem, which gives a SUB/EOF signal.
  if (  ( retval = write(ttyFD, endMsg, (sizeof(endMsg)-1)) == -1 )  ){
    perror("sendSMS");
    fprintf(stderr, "sendSMS: write endMsg returned error.\n");
    return 1;
  }
  //sleep(1);
  //sleep(20);

  close(ttyFD);

  fprintf(stderr,"sendSMS: All commands for SMS have been sent to 3G modem.\n");


  return 0; // success
}
