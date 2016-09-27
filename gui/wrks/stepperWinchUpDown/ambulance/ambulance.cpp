// c++ based:
#include <iostream>

// c based:
#include <unistd.h>
#include <stdio.h>

// local:
#include "ambulance.hpp"
#include "ui_ambulance.h"



ambulance::ambulance(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ambulance){

  ui->setupUi(this);
  
  fprintf(stderr, "Hello Ambulance World!\n");

  // Initialises SMS variables.
  strcpy(modemDevName, "/dev/ttyUSB0"); // default modem device
  strcpy(phoneNum, "+61401858908"); // default SMS phone number(smagri)
  strcpy(msgTxt, "Payload has arrived.\r\n");//default payload arrival msg
  //              123451234512345123451 2 3, NULL is 23rd char

  fprintf(stderr, "Default device to be opened is %s\n", modemDevName);
  fprintf(stderr, "Default phone number for SMS=%s\n", phoneNum);
  fprintf(stderr, "Default payload delivered message sent is=%s\n", msgTxt);

  fprintf(stderr, "Number of chars in message=%ld\n", sizeof(msgTxt));
  //
  for (int i=0; i<(MAX_msgTxt_CHARS+1); i++){
    if (msgTxt[i] == '.'){
      fprintf(stderr, "full stop is at index %d\n", (i+1));
    }
    else if (msgTxt[i] == '\r'){
      fprintf(stderr, "CR at index %d\n", (i+1));
    }
    else if (msgTxt[i] == '\n'){
      fprintf(stderr, "CR+LF, or newline at index %d\n", (i+1));
    }
    else if(msgTxt[i] == '\0'){
      msgTxtLength = i + 1;
      fprintf(stderr,"c str terminating null char at index %d\n", msgTxtLength);
      break;
    }
  }

  // Other initialisations.
  dirRotation='1'; // clockwise rotation, winch down
  ui->winchDownRadioButton->setChecked(1);
 
  numRotation=3; // default number of rotations is 3
  ui->stepperNumRotations->setText("3");

  ui->messageTextTextEdit->setText("Payload has arrived.\r\n");

}


ambulance::~ambulance(){

  delete ui;
}


void ambulance::on_deliverPayloadPushButton_clicked(){

  fprintf(stderr, "deliverPayloadPushButton_clicked\n");

}


void ambulance::on_sendSMSPushButton_clicked(){

  fprintf(stderr, "sendSMSPushButton_clicked\n");

  
  sendSMS(modemDevName, phoneNum, msgTxt, msgTxtLength);

}


void ambulance::on_modemDevNameComboBox_activated(const QString &arg1){

  string str; // standard c++ string
  const char *mdn; // temporary

  fprintf(stderr, "modemDevNameComboBox_activated\n");

  str = arg1.toStdString(); // QString member function to c++ 'string'
  mdn = str.c_str();// c++ 'string' to pointer to char(ie char *)
  strcpy(modemDevName, mdn);
  fprintf(stderr, "QString pointer, &arg1=modemDevName %s\n", modemDevName);
  fprintf(stderr, "QString pointer, &arg1=mdn %s\n", mdn);

}


// void ambulance::on_stepperNumRotations_cursorPositionChanged(int arg1,
// 							     int arg2){

//  fprintf(stderr, "stepperNumRotations_cursorPositionChanged\n");

//  fprintf(stderr, "Current cursor  postn is in front of position arg2\n");
//  fprintf(stderr, "Gone from arg1 to arg2 position narg1=%d, arg2=%d\n",
// 	 arg1, arg2);
  
// }


void ambulance::on_stepperNumRotations_editingFinished(){

  // *_editingFinished is called on CR or moving the cursor outside
  // the box via the mouse or keyboard commands(eg tab).
  //
  fprintf(stderr, "stepperNumRotations_editingFinished\n");
  //char nrc[4];

  numRotation = ui->stepperNumRotations->text().toInt();


  // if (numRotationInt > 127){
  //   // Improvements need to be made to the arduino server, if possible.
  //   fprintf(stderr, "Currently, stepper motor number of rotations > 127"
  // 	    " are invalid.\n");
  //   return;
  // }

  fprintf(stderr, "  unsigned int numRotation=%u\n", numRotation);
  
  // Only  works for numbers  that represent  characters in  the ascii
  // character set, otherwise you just get funny chars on the screen.
  fprintf(stderr, "  char numRotation=%c\n", numRotation);

  // sprintf(nrc, "%d", numRotation);
  // fprintf(stderr, "  string nrc=%s\n", nrc);
  // fprintf(stderr, "  char nrc[0]=%c\n", nrc[0]);
  // fprintf(stderr, "  char nrc[1]=%c\n", nrc[1]);
  // fprintf(stderr, "  char nrc[2]=%c\n", nrc[2]);
  // fprintf(stderr, "  char nrc[3]=%c\n", nrc[3]);
  //fprintf(stderr, "  ascii string of characters=%s\n", nrc);

}


void ambulance::on_microAdjPushButton_clicked(){

  int retval; // return value from xferas() fn

  fprintf(stderr, "microAdjPushButton_clicked\n");
  
  //strcpy(serverID, "192.168.43.200"); // servo motor
  strcpy(serverID, "192.168.43.201"); // stepper motor
  
  fprintf(stderr, "  unsigned int numRotation=%u\n", numRotation);
  fprintf(stderr, "  char numRotation=%c\n", numRotation);
  fprintf(stderr, "  dirRotation=%c\n", dirRotation);
  fprintf(stderr, "  serverID=%s\n", serverID);
  
  retval = xfer2as(dirRotation, numRotation, serverID);
  if (retval){
    fprintf(stderr, "Error returned from xfer2as()\n");
  }
  sleep(2); // stepper needs shorter wait
  // sleep(6); // servo needs longer wait
  
  // fprintf(stderr, "Reverse Motor Direction, dirRotation=%c\n", dirRotation);
  // dirRotation = '0';
  // retval = xfer2as(dirRotation, numRotation, serverID);
  // sleep(12);
  // sleep(6);
  
  // wrks for servo motor:
  //
  // retval = xfer2as(dirRotation, numRotation, serverID);
  // //sleep(12);// wait for servo to stop before given next command, min=11sec
  // sleep(6);
  //
  // fprintf(stderr, "Reverse Motor Direction, dirRotation=%c\n", dirRotation);
  // dirRotation = '0';
  // retval = xfer2as(dirRotation, numRotation, serverID);
  // //sleep(12);
  // sleep(6);
   
  //fprintf(stderr, "microAdjPushButton_clicked: xfer2as retval=%d\n", retval);
}


void ambulance::on_winchDownRadioButton_clicked(){

    if (ui->winchDownRadioButton->isChecked()){
        dirRotation = '1';
        fprintf(stderr, "winchDownRadioButton: dirRotation=%c\n", dirRotation);
    }
}


void ambulance::on_winchUpRadioButton_clicked(){

  if (ui->winchUpRadioButton->isChecked()){
    dirRotation = '0';
    fprintf(stderr, "winchUpRadioButton: dirRotation=%c\n", dirRotation);
  }

}

void ambulance::on_mobileNumLineEdit_editingFinished(){

  // Set new phone number for the SMS message.
  
  const char *phoneNumPtr; // temporary pointer for c string, includes
			   // null('\0') string delimiter


  phoneNumPtr = ui->mobileNumLineEdit->text().toStdString().c_str();
  strcpy(phoneNum, phoneNumPtr);
  fprintf(stderr, "New phone number for SMS=%s\n", phoneNum);

}

void ambulance::on_messageTextTextEdit_textChanged(){

  // New SMS message text has arrived.

  const char *msgTxtPtr; // temporary pointer for c string(=>NULL terminated)

  msgTxtPtr = ui->messageTextTextEdit->toPlainText().toStdString().c_str();

  strcpy(msgTxt, msgTxtPtr);

  for (int i=0; i<(MAX_msgTxt_CHARS+1); i++){
    if(msgTxt[i] == '\0'){
      msgTxt[i] = '\r';
      msgTxt[i+1] = '\n';
      msgTxt[i+2] = '\0';
      msgTxtLength = i + 3;
      fprintf(stderr,"msgTxtLength %d\n", msgTxtLength);
      break;
    }
  }

  fprintf(stderr, "New SMS message text is=%s\n", msgTxt);

}