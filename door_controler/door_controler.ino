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
int RST = 3;
int EN_BT = 8;
int LED_INFRA = 7;
int RELAY_EN = 12;

void setup()
{
  digitalWrite(RST,HIGH);
  digitalWrite(EN_BT,LOW);
  /*Saves 4 bytes for the received commands*/
  cmd.reserve(4);

  /*Start the pin as output for the transistor that turn the relay on and off*/
  pinMode(RELAY_EN,OUTPUT);
  
  /*The LED of the board will imitate the LED of the relay*/
  pinMode(13,OUTPUT);

  pinMode(RST,OUTPUT);
  pinMode(LED_INFRA,OUTPUT);
  pinMode(EN_BT,OUTPUT);
  /*Setup the serial to communicate with the bt module*/
  bluetooth.begin(38400);  
  Serial.begin(9600);
  Serial.println("XFORCE: Door controller"); 
  digitalWrite(LED_INFRA,LOW);
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
         enableIR = false;  
         digitalWrite(LED_INFRA,HIGH);
         digitalWrite(EN_BT,HIGH);
       }
       irrecv.resume(); // Receive the next value  
}
void onCommandReceived()
{
    receivedCommand = false;
    cmd.toUpperCase();
    
    cmd=cmd.substring(0,4);
    Serial.print("Received command: ");    
    Serial.println(cmd); 
    
    if(cmd=="CMD1")
    {
  
      Serial.println("Turn on");
      digitalWrite(RELAY_EN,HIGH);  
      digitalWrite(13,HIGH);
      while(i<=3)
      {
        Serial.println(i++);
        delay(1000);  
      }
      
      Serial.println("Turn off");
      digitalWrite(RELAY_EN,LOW);  
      digitalWrite(13,LOW);
      
    } 

    if(cmd=="CMDR")
    {
       digitalWrite(RST,LOW);
     }

     if(cmd=="CMDS")
      {
        digitalWrite(LED_INFRA,LOW);
     }

     if(cmd=="CMDI")
     {
       bluetooth.println("CMDD%");
     }
    cmd = "";
    i = 1;
}
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
