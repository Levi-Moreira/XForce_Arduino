#include <SoftwareSerial.h>           //For bluetooth communication though other pins in the serial
#include <IRremote.h>                 //Infrared library for receiving packets

/*Infrared variables*/
int RECV_PIN = 11;        //The receive pin
boolean enableIR = true;  //Enable infrared receive
IRrecv irrecv(RECV_PIN);  //Configure infrared
decode_results results;   //Where the results from the infrared will arrive

/*Serial bluetooth comunication*/
SoftwareSerial bluetooth(10, 9);    //Rx and TX
String cmd = "";                    //String of received command
boolean receivedCommand = false;    //Received a command


int i = 1;                          //helper counter
int RST = 3;                        //reset pin
int EN_BT = 8;                      //enable bluetooth pin
int LED_INFRA = 7;                  //LED infra pin (waring LED)
int RELAY_EN = 12;                  //relay enable pin

void setup()
{
  digitalWrite(RST,HIGH);           //set te reset pin to high so arduino wont be reseting all the time
  digitalWrite(EN_BT,LOW);          //disable bluetooth
  /*Saves 4 bytes for the received commands*/
  cmd.reserve(4);

  /*Start the pin as output for the transistor that turn the relay on and off*/
  pinMode(RELAY_EN,OUTPUT);
  
  /*The LED of the board will imitate the LED of the relay*/
  pinMode(13,OUTPUT);

  //configure all used pins as output
  pinMode(RST,OUTPUT);
  pinMode(LED_INFRA,OUTPUT);
  pinMode(EN_BT,OUTPUT);
  
  /*Setup the serial to communicate with the bt module*/
  bluetooth.begin(38400);  

  //make sure infra led is turned off
  digitalWrite(LED_INFRA,LOW);
  irrecv.enableIRIn(); // Start the infrared receiver
}

void loop()
{
  //has a command been processed
  if(receivedCommand)
  {
    onCommandReceived();
  }

  //has anything arrived through the bluetooth serial interface
  if(bluetooth.available())
  {
    serialEvent();
  }

  //is the infrared enabled
  if(enableIR)
  {
    //has anything arrived through the infrared interface
    if (irrecv.decode(&results)) {
     treatIRReceive();
    }
  }

}

//treats a received package from the infrared
void treatIRReceive()
{
        //if the received package is the correct one
       if(results.value == 0x681cded5)
       {
        //disable the receiving of more packages
         enableIR = false;  
         //enable the infrared LED
         digitalWrite(LED_INFRA,HIGH);
         //turn on the bluetooth
         digitalWrite(EN_BT,HIGH);
       }
       irrecv.resume(); // Receive the next value  
}

//in case a command has been received and processed from the bluetooth serial
void onCommandReceived()
{
    //disable the receiving of more commands for now
    receivedCommand = false;

    //treat the command string
    cmd.toUpperCase();
    cmd=cmd.substring(0,4);

    //if received the turn on command
    if(cmd=="CMD1")
    {
  
      Serial.println("Turn on");
      digitalWrite(RELAY_EN,HIGH);  
      digitalWrite(13,HIGH);     
    } 

    //if recieved the turn off command
    if(cmd=="CMD0")
    {
  
      Serial.println("Turn on");
      digitalWrite(RELAY_EN,LOW);  
      digitalWrite(13,LOW);     
    } 

    //if received the reset command
    if(cmd=="CMDR")
    {
       digitalWrite(RST,LOW);
     }

    //if received the suspend command
     if(cmd=="CMDS")
      {
        digitalWrite(LED_INFRA,LOW);
     }

    //if received the identify command
     if(cmd=="CMDI")
     {
       bluetooth.println("CMDD%");
     }
    cmd = "";
    i = 1;
}

//in case there's something new in the bluetooth serial 
void serialEvent() {
  while (bluetooth.available()) {
    char inputChar = (char)bluetooth.read();
    cmd += inputChar;
    Serial.println(inputChar);
    if (inputChar == '\n' || inputChar=='%') {
      receivedCommand = true;
      
    }
  }
}
