
/*
Measuring AC Current Using ACS712
www.circuits4you.com
*/
#include <LiquidCrystal.h> // include Arduino LCD library
#include <math.h>

//GSM MODULE FUNCTIONS
#include <SoftwareSerial.h>
SoftwareSerial sim800l(0, 1); // RX,TX for Arduino and for the module it's TXD RXD, they should be inverted
//#define button1 7 //Button pin, on the other pin it's wired with GND
//bool button_State; //Button state


// LCD module connections (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

//PIR SENSOR
int a,b;

//current measurements
const int sensorIn = A0;
int mVperAmp = 120; // use 100 for 20A Module and 66 for 30A Module 185A
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;



//pulsewidth
float pulsewidth =0;
float powerfactor=0;
float phase = 0;
int x = 0; 


//energymeasurements
float e1,e2;


void setup()
{ 
  
//GSM STARTING COMMAND
  sim800l.begin(9600);




  
 Serial.begin(9600);
 lcd.begin(16, 4);           // set up the LCD's number of columns and rows
 lcd.setCursor(0, 0);
 lcd.print("L&T PRESENTATION ON SMART GRID   ");
 lcd.println();
 delay(2000);
 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print(" INITIAL ENERGY      100kWh");
 delay(2000);
 lcd.clear();

  //voltage measurements
 // lcd.setCursor(0, 0);
 // lcd.print("Voltage:");
 // analogReference(INTERNAL);  // set ADC positive reference voltage to 1.1V (internal)

 //energy measurements
 e1=0;e2=100.00;

 //relay 
 pinMode(10,OUTPUT);
}

void loop()
{

                     //GSM MODULE
                     if(e2<99.90)
                     { 
                      Serial.println("Your energy limit is over"); //Shows this message on the serial monitor
                      Serial.println("so we are informing to the HPSEBL.");
                      Serial.println("RECHARGE PAYTM NUMBER 7018351675 ");
                      delay(200);                         //Small delay to avoid detecting the button press many times
                      SendSMS();                          //And this function is called

                     }
                     if (sim800l.available())
                     {            //Displays on the serial monitor if there's a communication from the module
                                  Serial.write(sim800l.read());
                                  switch(Serial.read())
                                           {
                                                case '1':
                                                e2=100.00;
                                                break;
                                                case '2':
                                                e2=200.00;
                                                break;
                                           } 
                     }
                       

                           


                       //PIR SENSOR and relay settings where e2 is the minimum amount of energy needed to work which is 0.5kWh in our case.
                      int pirstate=digitalRead(9);
                      if(pirstate==LOW&&e2>=0.50)
                          {
                            Serial.println("Motion Not Detected");delay(100);digitalWrite(10,LOW);digitalWrite(12,LOW);
                          }

                     else if(pirstate==HIGH||e2<0.50)
                          {
                            Serial.println("Motion Detected");delay(100);digitalWrite(10,HIGH);digitalWrite(12,HIGH);
                          }
                       

                    

                          
                          //pulsewidth and power factor
                         if(x==0)
                          { 
                            pulsewidth = pulseIn (11, HIGH) ;
                            lcd.setCursor(0,2);
                            lcd.print("pf is ");
                             powerfactor = cos(pulsewidth*0.314/1000);
                            //lcd.print(cos(pulsewidth*0.314/1000));
                             lcd.print(powerfactor);
                            //delay(2000);
                            lcd.println(); x++;
                          }
                          else
                             {
                             lcd.setCursor(0,2);
                             lcd.print("pf is ");
                             //lcd.print(cos(pulsewidth*0.314/1000));
                             lcd.print(powerfactor);
                             
                             } 
 
 
 
                          //voltage measurements
                         
                          // get amplitude (maximum - or peak value)
                          uint32_t v = get_max();
                          // get actual voltage (ADC voltage reference = 5V which is default)
                          v = v * 5000/(2*1023);
                          // calculate the RMS value ( = peak/√2 )
                          v /= sqrt(2);
                          
                          lcd.setCursor(8, 0);
                          lcd.setCursor(0,0);
                          lcd.print("Voltage(V): ");
                          lcd.print(v);
                          delay(100);
 
 
 //current measurements
 analogReference(DEFAULT);
 Voltage = getVPP();
 VRMS = (Voltage/2.0) *0.707;  //root 2 is 0.707
 AmpsRMS = (VRMS * 1000)/mVperAmp;
 lcd.setCursor(0,1);
 lcd.print("current(A): ");
 lcd.print(AmpsRMS);
 //lcd.print(Voltage);
 //delay(1000);
 //lcd.clear();
 //Serial.println(" Amps RMS");

                         
                         


//energy measurements
 
 e1+=AmpsRMS*v*powerfactor*0.000278;
 //e1=198;
 e2=100.00-e1;
 //e2=1.00/10;
 

 lcd.setCursor(0,3);
 
 lcd.print("Left(kWh):");
 lcd.print(e2);
 delay(2000);





}



//external functions

//voltagemeasurements
// get maximum reading value
         uint16_t get_max() 
         {
            uint16_t max_v = 0;
                  for(uint8_t i = 0; i < 100; i++) 
                  {
                   uint16_t r = analogRead(A3);  // read from analog channel 3 (A3)
                   if(max_v < r) max_v = r;
                   delayMicroseconds(200);
                   }
                    return max_v-15.8;
         }

//current measurements
float getVPP()
{
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensorIn);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the minimum sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 5.0)/1024.0;
     //return readValue;
   return result;
 }


//GSM MODULE
void SendSMS()
{
  Serial.println("Sending SMS...");               //Show this message on serial monitor
  sim800l.print("AT+CMGF=1\r");                   //Set the module to SMS mode
  delay(100);
  sim800l.print("AT+CMGS=\"+917018351675\"\r");  //Your phone number don't forget to include your country code, example +212123456789"
  delay(500);
  sim800l.print("HOUSE number 1342 has reached its energy limit");       //This is the text to send to the phone number, don't make it too long or you have to modify the SoftwareSerial buffer
  delay(500);
  sim800l.print((char)26);// (required according to the datasheet)
  delay(500);
  sim800l.println();
  Serial.println("Text is Sent.");
  delay(500);

}
