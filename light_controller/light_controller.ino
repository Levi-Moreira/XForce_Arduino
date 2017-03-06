#include  <TimerOne.h>          //Timer library for coordinatin the opening and closing of the triac driver
#include <SoftwareSerial.h>     //SoftwareSerial library to use other pins as USART
#include <IRremote.h>           //Infrared library to enable recieving of infrared packets


/*Variables to controle the dimmer*/
volatile int i=0;                // Variable to use as a counter volatile as it is in an interrupt
volatile boolean zero_cross=0;   // Boolean to store a "switch" to tell us if we have crossed zero
int AC_pin = 3;                  // Output to Opto Triac
int dim = 0;                     // Dimming level (0-128)  0 = on, 128 = 0ff
int freqStep = 65;               //Time for eatch step  = 8333/128
int charNumber = 1;

/*Variables to control the infrared*/
int RECV_PIN = 11;               //Pin to receive the infrared data
boolean enableIR = true;         //When the correct infrared packet has been reeived, disables the infrared communication
IRrecv irrecv(RECV_PIN);         //Configures the infrared recieving
decode_results results;          //The results will be saved here

/*Variables to control the bluetooth*/
SoftwareSerial bluetooth(10, 9); /*RX and TX*/
boolean receivedCommand = false;  //When received a command, this will be true. Used only for reseting the arduino when receiving command 256
String cmd = "";  

int RST = 8;                      //Sending low to this pin will reset the arduino
int LED_INFRA = 13;               //Blue led indicates that infrared has benn paired
int EN_BT = 12;                   //Enable bluetooth pin


/*Preconfigurations, this will be run only once*/
void setup() {
  digitalWrite(RST,HIGH);                           //Imediatly pull reset pin to high, otherwise it will be reseting all the time
  digitalWrite(EN_BT,LOW);                          //turn off bluetooth
  digitalWrite(AC_pin,HIGH);                        //turn on lights
  cmd.reserve(6);

  pinMode(AC_pin, OUTPUT);                          // Set the Triac pin as output
  attachInterrupt(0, zero_cross_detect, RISING);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(dim_check, freqStep);      // initilize the time interrupt

  /*Configure ports as outputs*/
  pinMode(RST,OUTPUT);
  pinMode(LED_INFRA,OUTPUT);
  pinMode(EN_BT,OUTPUT);
  
  /*Setup the serial to communicate with the bt module*/
  bluetooth.begin(38400);  
  Serial.begin(9600); //hardware serial for debug
  Serial.println("XFORCE: Light controller"); 
  digitalWrite(LED_INFRA,LOW);
  irrecv.enableIRIn(); // Start the receiver      
}

/*Main loop*/
void loop() {                        

  //something new at the serial interface?
  if(bluetooth.available())
  {
    serialEvent();
  }

  //is the infrared enabled?
  if(enableIR)
  {
    //something new at the infrared interface
    if (irrecv.decode(&results)) {
     treatIRReceive();
    }
  }

  //a new command was received?
  if(receivedCommand)
  {
    onCommandReceived();
  }
}

/*Treats the receiving of a character through the serial-bluetooth interface*/
void serialEvent() {
  while (bluetooth.available()) {
    char inputChar = (char)bluetooth.read();
    cmd += inputChar;
    if (inputChar == '\n' || inputChar=='%') {
      receivedCommand = true;
      
    }
  }
}

/*When a infrared packet has been detected in the buffer*/
void treatIRReceive()
{
       //if it contains the right code
       if(results.value == 0x681cded5)
       {
         //disable the reeiving of more ifnrared packets for now
         enableIR = false;  

         //turn on the indication led
         digitalWrite(LED_INFRA,HIGH);

         //enable bluetooth
         digitalWrite(EN_BT,HIGH);
       }
       irrecv.resume(); // Receive the next value  
}

/*When some new command was received*/
void onCommandReceived()
{
    int neither = 0;
    receivedCommand = false;
    cmd.toUpperCase();

  
    String rec =cmd.substring(0,4);
    Serial.print("Received command: ");    
    Serial.println(rec); 
    
      //in case it's the indentify command
      if(rec=="CMDI")
      {
        bluetooth.println("CMDL%");
        neither = 1;
      }

      //in case it's the restart command
      if(rec=="CMDR")
      {
           digitalWrite(EN_BT,LOW);
           digitalWrite(LED_INFRA,LOW);
           bluetooth.flush();
           results.value = 0x00;
           enableIR = true;
           neither = 1;
      }

      //in case it's the light step command
      if(rec=="CMDS")
      {
        char answer[6];
        sprintf(answer,"S%d%%",charNumber);
        Serial.println(answer);
        bluetooth.println(answer);
        neither = 1;
      }
    

    //only when it's a number command
    if(!neither)
    {
      String newString = cmd.substring(3,6);
      Serial.println(newString);

      if((newString.indexOf("C")==-1) && (newString.indexOf("M")==-1) &&(newString.indexOf("D")==-1))
      {
        charNumber = newString.toInt();
        Serial.print("Received a Number: ");
        Serial.println(charNumber);
        if(charNumber>128) charNumber = 128;
        if(charNumber<1) charNumber = 1;
        dim = charNumber;
      }
    }
    cmd = "";
    i = 1;
}

/*Interrupt for when a zero crossing is detected in pin 2*/
void zero_cross_detect() {    
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  i=0;
  digitalWrite(AC_pin, LOW);       // turn off TRIAC (and AC)
  //Serial.println("Detect zero");
}                                 

/*Turn on the TRIAC at the appropriate time*/
void dim_check() {   

  if(zero_cross == true) { 
               
    if(i>dim) { 
        
      digitalWrite(AC_pin, HIGH); // turn on light       
      i=0;  // reset time step counter                         
      zero_cross = false; //reset zero cross detection
    } 
    else {
      i++; // increment time step counter                     
    }                                
  }                                  
}                                   

