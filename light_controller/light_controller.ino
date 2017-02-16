#include  <TimerOne.h>          //Timer library for coordinatin the opening and closing of the triac driver
#include <SoftwareSerial.h>     //SoftwareSerial library to use other pins as USART
#include <IRremote.h>           //Infrared library to enable recieving of infrared packets


/*Variables to controle the dimmer*/
volatile int i=0;                // Variable to use as a counter volatile as it is in an interrupt
volatile boolean zero_cross=0;   // Boolean to store a "switch" to tell us if we have crossed zero
int AC_pin = 3;                  // Output to Opto Triac
int dim = 0;                     // Dimming level (0-128)  0 = on, 128 = 0ff
int freqStep = 65;               //Time for eatch step  = 8333/128

/*Variables to control the infrared*/
int RECV_PIN = 11;               //Pin to receive the infrared data
boolean enableIR = true;         //When the correct infrared packet has been reeived, disables the infrared communication
IRrecv irrecv(RECV_PIN);         //Configures the infrared recieving
decode_results results;          //The results will be saved here

/*Variables to control the bluetooth*/
SoftwareSerial bluetooth(10, 9); /*RX and TX*/
boolean receivedCommand = false;  //When received a command, this will be true. Used only for reseting the arduino when receiving command 256


int RST = 8;                      //Sending low to this pin will reset the arduino
int LED_BLUE = 7;                 //Blue led indicates that infrared has benn paired



/*Preconfigurations, this will be run only once*/
void setup() {
  digitalWrite(RST,HIGH);                           //Imediatly pull reset pin to high, otherwise it will be reseting all the time

  pinMode(AC_pin, OUTPUT);                          // Set the Triac pin as output
  attachInterrupt(0, zero_cross_detect, RISING);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(dim_check, freqStep); 
  
  pinMode(RST,OUTPUT);
  pinMode(LED_BLUE,OUTPUT);
  /*Setup the serial to communicate with the bt module*/
  bluetooth.begin(38400);  
  bluetooth.println("XFORCE: Light controller"); 
  digitalWrite(LED_BLUE,LOW);
  irrecv.enableIRIn(); // Start the receiver      
}

/*Main loop*/
void loop() {                        

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

  if(receivedCommand)
  {
    onCommandReceived();
  }
}

/*When something is detected in the serial*/
void serialEvent() {
  while (bluetooth.available()) {
    int inputChar = (int)bluetooth.parseInt();
    if (inputChar == 256) {
      receivedCommand = true;
    }else
    {
      dim = inputChar;
      }
  }
}

/*When a infrared packet has been detected in the buffer*/
void treatIRReceive()
{
       if(results.value == 0x681cded5)
       {
         bluetooth.println(0x02,HEX);
         enableIR = false;  
         digitalWrite(LED_BLUE,HIGH);
       }
       irrecv.resume(); // Receive the next value  
}

/*When a reset command has been received*/
void onCommandReceived()
{
    receivedCommand = false;
    bluetooth.print("Received command Reset ");    
    digitalWrite(RST,LOW);

}

/*Interrupt for when a zero crossing is detected in pin 2*/
void zero_cross_detect() {    
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  i=0;
  digitalWrite(AC_pin, LOW);       // turn off TRIAC (and AC)
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

