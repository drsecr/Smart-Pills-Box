/*
Smart Pills Box Project
by Doron Raz & Amir Atia
*/

 #include <ThingSpeak.h>                 // Allow us connect to thinkspeak server
 #include <TimeLib.h>                    // Time library

// ############################## global variables ##############################
 bool detect = 0;                        // Flag
 const int gateTime[3] = {7,12,19};      // Definition of taking pills hours
 String tcp = "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80"; // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 long int clearBufferTime;
 char c;
 int led = 13;                           // Pin 13 = LED 
 String sms1 = "GET https://api.thingspeak.com/apps/thinghttp/send_request?api_key=HR3GV7VIPLL0G6KN";   // Sending reminder SMS
 String sms2 = "GET https://api.thingspeak.com/apps/thinghttp/send_request?api_key=VCOWDZ4S2VYDSA1C";   // Sending last reminder SMS
 String sms3 = "GET https://api.thingspeak.com/apps/thinghttp/send_request?api_key=TTNCDEELCGTWBZT8";   // Sending SMS the pills were taken


// #################################### Setup ####################################
 void setup()
 { 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Hall Effect ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Serial.begin(115200);
   Serial1.begin(115200);
   attachInterrupt(14, magnet_detect, RISING);  // Initialize the interrupt pin (Teensy digital pin 14)
  
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Time ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   setTime(18, 59 ,30,15,8,2019);          // Current time&date definition (H,M,S,D,M,Y)

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ LED ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   pinMode(led, OUTPUT);                   // initialize the digital pin as an output

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ESP8266 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Serial.println("Start...");
   Serial1.print("AT+RST\r\n");            // Reset the ESP8266 
   delay(1000); 
   if (Serial1.available() > 0)            // Read ESP8266 data buffer
     while (Serial1.available() > 0)
     {
      char ch = Serial1.read();
      Serial.print(ch);
     }

   Serial1.print("AT+CWMODE=3\r\n");            // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
   delay(1000); 
   if (Serial1.available() > 0)            // Read ESP8266 data buffer
    while (Serial1.available() > 0)
    {
      char ch = Serial1.read();
      Serial.print(ch);
    }
    
   Serial1.println("AT+CWJAP=\"Doron\",\"1qaz1qaz\"");  // Connect to network
   delay(3000);
   if (Serial1.available() > 0)            // Read ESP8266 data buffer
    while (Serial1.available() > 0)
      {
        char ch = Serial1.read();
        Serial.print(ch);
      }

   Serial1.println("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80");   // Connect to ThinkSpeak site's servers
   delay(7000);
   if (Serial1.available() > 0)           // Read ESP8266 data buffer
    while (Serial1.available() > 0)
    {
      char ch = Serial1.read();
      Serial.print(ch);
    }
 }

// ##################################### Main Loop ########################################
void loop()
{
  for (int i = 0; i < 3; i++)             // Waiting for taking pills time 
    if (gateTime[i] == hour() && minute() == 0)
      magnet_loop(); 
      
  Serial.print(hour());                   // print the hour hh:mm
  Serial.print(":");  
  Serial.print(minute());  
  Serial.print("\n");  
  delay (1000*60);                        // wait 1 minute
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~ Waiting user to take pills ~~~~~~~~~~~~~~~~~~~~~~~~~~
void magnet_loop()  
{ 
  int samplingTime = hour();
  int massageNum = 0;                     // Initialization the number of massage has been send
  detect = 0;               
  digitalWrite(led, HIGH);                // Turn the LED on (HIGH is the voltage level)
  Serial.println("Time to take pills");   // Inform the pills need to be taken 

  while(detect == 0)                      // Check if pills have been taken
  {
    // in a case that the pills have been forgotten to taken after full hour
    if (hour() == (samplingTime + 1) && minute() == 0)  
    {  
      samplingTime = hour();              // Updating the sampling hour
      massageNum += 1;                    // Updating the number of massages sent
      ESP8266write();                     // Updating Thinkspeak site for long follow-up
      Serial.println("The pills didn't taken");  // inform the pills doesnt taken
      if (massageNum < 3)
      {
        TSS (sms1);                       // Sending reminder SMS
      }
      else                                // Third massage
      {
        detect = 1;                       // break the loop
        digitalWrite(led, LOW);           // turn the LED off by making the voltage LOW
        TSS (sms2);                       // Sending last reminder SMS
      }
    }
    delay (1000*60);                      // wait 1 minute
  }      
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Magnet interruped detected ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void magnet_detect()                      // This function is called whenever a magnet interrupt detected
{
   Serial.println("Detect, pills have been taken!");
   detect = 1;
   digitalWrite(led, LOW);                // turn the LED off by making the voltage LOW
   TSS (sms3);                            // Sending SMS the pills were taken
   ESP8266write();                        // Updating Thinkspeak site for long follow-up
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Sending data to Thingspeak ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ESP8266write() 
{ 
	char buf1[32];
  float pillsIndector = 0;
  
  if (detect == 1)
    pillsIndector = 1;
  else
    pillsIndector = 0;
    
	String str_pillsIndector = dtostrf(pillsIndector, 2, 1, buf1); // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
   
  // Establish TCP connection 
	clearBuffer(); 
  Serial1.println(tcp);                   // Connect to ThingSpeak server
  delay(3000);  

	// Prepare GET string 
	String getStr2 = "GET https://api.thingspeak.com/update?api_key=18NU7AGY9DBUDLG2&field1="; 
  getStr2 += String(str_pillsIndector);
  getStr2 += "\r\n\r\n";

	// Send data length to esp8266
	String cmd2 = "AT+CIPSEND="; 
	cmd2 += String(getStr2.length()); 
	Serial1.println(cmd2);                  // Sending the length of the data
  delay(3000);                            // Wait 3 second

  // Sending data to thingspeak through esp8266
	Serial1.print(getStr2);
  delay(5000);
  Serial.print("data sent\r\n");
}

 void TSS(String myString)
 {
  // Establish TCP connection 
  clearBuffer(); 
  Serial1.println(tcp);                   // Connect to ThingSpeak server 
  delay(3000);                            // Wait 3 second

  // Prepare GET string 
  String getStr = myString; 
  getStr += "\r\n";

  // Send data length 
  String cmd = "AT+CIPSEND="; 
  cmd += String(getStr.length()); 
  Serial1.println(cmd);                    // Sending the length of the data
  delay(3000); 

  // Sending data to thingspeak through esp8266
  Serial1.print(getStr);
  delay(5000);
  Serial.print("data sent\r\n");          // Reset ESP8266
}


 
void clearBuffer() // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
{ 
	clearBufferTime = millis(); 
	if ((millis() - clearBufferTime < 2000) && Serial1.available() > 0) 
	{ 
		Serial.println("Buffer:"); 
		while ((millis() - clearBufferTime < 2000) && Serial1.available() > 0) 
		{ 
			c = Serial1.read(); 
			Serial.print(c); 
		} 
		Serial.println(); 
		Serial.println("Untill here.\n"); 
	} 
} 
