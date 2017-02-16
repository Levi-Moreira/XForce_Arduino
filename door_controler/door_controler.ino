#include <SoftwareSerial.h>           //For bluetooth communication though other pins in the serial
#include <IRremote.h>                 //Infrared library for receiving packets

/*Infrared variables*/
int RECV_PIN = 11;
boolean enableIR = true;
IRrecv irrecv(RECV_PIN);
decode_results results;

/*Serial bluetooth comunication*/
SoftwareSerial bluetooth(10, 9); /*RX and TX*/
String cmd = "";  
boolean receivedCommand = false; 


int i = 1;


void setup()
{
  digitalWrite(8,HIGH);
  /*Saves 4 bytes for the received commands*/
  cmd.reserve(4);

  /*Start the pin as output for the transistor that turn the relay on and off*/
  pinMode(2,OUTPUT);
  
  /*The LED of the board will imitate the LED of the relay*/
  pinMode(13,OUTPUT);

  pinMode(8,OUTPUT);
  pinMode(7,OUTPUT);
  /*Setup the serial to communicate with the bt module*/
  bluetooth.begin(38400);  
  bluetooth.println("XFORCE: Door controller"); 
  digitalWrite(7,LOW);
  irrecv.enableIRIn(); // Start the receiver
}

void loop()
{

  if(receivedCommand)
  {
    onCommandReceived();
  }

  if(bluetooth.available())
  {
    serialEvent();
  }

  if(enableIR)
  {
    if (irrecv.decode(&results)) {
     treatIRReceive();
    }
  }

}

void treatIRReceive()
{
    
       if(results.value == 0x681cded5)
       {
         bluetooth.println(0x01,HEX);
         enableIR = false;  
         digitalWrite(7,HIGH);
       }
       irrecv.resume(); // Receive the next value  
}
void onCommandReceived()
{
    receivedCommand = false;
    cmd.toUpperCase();
    
    cmd=cmd.substring(0,4);
    bluetooth.print("Received command: ");    
    bluetooth.println(cmd); 
    
    if(cmd=="CMD1")
    {
  
      bluetooth.println("Turn on");
      digitalWrite(2,HIGH);  
      digitalWrite(13,HIGH);
      while(i<=3)
      {
        bluetooth.println(i++);
        delay(1000);  
      }
      
      bluetooth.println("Turn off");
      digitalWrite(2,LOW);  
      digitalWrite(13,LOW);
      
    } 

    if(cmd=="CMDR")
    {
       digitalWrite(8,LOW);
     }
    cmd = "";
    i = 1;
}
void serialEvent() {
  while (bluetooth.available()) {
    char inputChar = (char)bluetooth.read();
    cmd += inputChar;
    if (inputChar == '\n' || inputChar=='%') {
      receivedCommand = true;
    }
  }
}
