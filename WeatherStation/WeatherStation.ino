#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <TextFinder.h>
#include "ThingSpeak.h"
#include <Wire.h>
#include <BME280I2C.h>
#include "SSD1306.h"
//#include "images.h"

/////////////////// OLED
const int I2C_DISPLAY_ADDRESS = 0x3c;
const int SDA_PIN = D3;
const int SDC_PIN = D4;

char showLine[4][25];
int showCount = 0;
char buff[25];
char pre[25];
char post[25];
int errorCount = 0;

SSD1306Wire display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);

/////////////////// Server Yahoo 
// IPAddress server(72,30,202,51);


///////////////// Wifi
const char* ssid     = "mixen-ap-f1"; 
const char* password = "mixensensagri";
WiFiClient client;
IPAddress ip;

/////////////// BME280

BME280I2C bme; 
bool metric = false;

/* ==== Prototypes ==== */
/* === Print a message to stream with the temp, humidity and pressure. === */
void printBME280Data(Stream * client);
/* === Print a message to stream with the altitude, and dew point. === */
void printBME280CalculatedData(Stream* client);
/* ==== END Prototypes ==== */

///////////////Weather UnderGround 
char host [] = "weatherstation.wunderground.com";  
char WEBPAGE [] = "GET /weatherstation/updateweatherstation.php?";
char ID [] = "IBANGKOK207";
char PASSWORD [] = "p0qayqgs";

// ThingSpeak

unsigned long myChannelNumber = 178977;
const char * myWriteAPIKey = "5KY2PPF182P6T71N";


TextFinder  finder( client );  

char place[50];
char hum[30];
float tempf = 0;
float tempc = 0;
float humidity =0;
float dewptf =0;
float dewptc =0;
float pressure =0;
float windspeed=0;
int winddir=0;
const int sleepTimeS = 300              ; //18000 for Half hour, 300 for 5 minutes etc.
int counter = 0;
char state[5] = "P";

void setup() {

  Serial.begin(115200);  

  // OLED Init
  display.init();
  display.clear();
  display.display();

  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);
  display.flipScreenVertically();  
  delay(1000);
  Serial.println();

 
  // Wifi Init
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
    display.clear();
    display.drawString(64, 15, "Connecting to WiFi");
    display.drawProgressBar(10, 40, 100, 10, counter*2);
    display.display();
    counter++;
    if (counter >= 50) {
      Serial.println("");
      Serial.println("Cannot connect to WiFi");
      Serial.println("Restarting...");
      delay(2000);
      ESP.restart();      
      } 
    delay(1000);
  }

  ip = WiFi.localIP();
  
  Serial.println();
  display.clear();
  display.drawString(64, 15, "IP Address");
  display.drawString(64, 30, String(ip[0])+"."+String(ip[1])+"."+String(ip[2])+"."+String(ip[3]));
  display.display();
  delay(1000);
  display.clear();

  // BME280 Init
  Wire.begin(0,2);
  screenAdd("BME280 Init");
  while(!Serial) {} // Wait
  while(!bme.begin()){
    Serial.println("Could not find BME280 sensor!");
    screenAdd("BME280 Error");
    delay(1000);
  }


  // Thingspeak Init 
  ThingSpeak.begin(client);


}

void loop() {


  strcpy(state,"P");

/* Get Yahoo Weather Data

  Serial.println("Connect to Yahoo Weather...");
  screenAdd("Connecting to Y!Weather");
  
if (client.connect(server, 80))
  {
    // Call Wetter-API
    // w: ID from your City
    // http://weather.yahooapis.com/forecastrss?w=12893459&u=c
    ///
    client.println("GET /v1/public/yql?q=select%20*%20from%20weather.forecast%20where%20woeid%20in%20(select%20woeid%20from%20geo.places(1)%20where%20text%3D%22Nonthaburi%2CTH%22)&format=xml&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys HTTP/1.0");
    client.println("HOST:query.yahooapis.com\n\n");
    client.println();
    Serial.println("Connected...");
    screenAdd("Connected");
     
  } 
  else
  {
    strcpy(state,"F");
    Serial.println(" connection failed");
    screenAdd("Failed");
    errorCount++;
    
  } 

  delay(500);

  if (client.connected())
  {  
    
    screenAdd("Get Data from Yahoo!");
    
    
    // Wind Direction
    if(finder.find("direction=") )
    {
     winddir = finder.getValue();
     Serial.print("Wind Direction :  ");
     Serial.println(winddir);
    } 

    // Windspeed
    if(finder.find("speed=") )
    {
     windspeed = finder.getValue()*0.621371192;
     Serial.print("Windspeed mph:  ");
     Serial.println(windspeed);
    }  
    
    // Humidity
   if ( finder.find("humidity=") )
    {
     humidity = finder.getValue();
     Serial.print("Humidity:  ");
     Serial.println(humidity);
    }  

     // Pressure
    if(finder.find("pressure=") )
    {
     pressure = finder.getValue()*0.02953;
     Serial.print("Pressure In Hg:  ");
     Serial.println(pressure);
    }

   
    // Temperature
    if(finder.find("temp=") )
    {
     tempf = finder.getValue();
     Serial.print("Temp F:  ");
     Serial.println(tempf);
    }
    
         
  // END XML
  }
  else
  {
    Serial.println("Disconnected"); 
  }
*/

// BME Read
  
  screenAdd("Read Sensor Data");
  
  float temp(NAN), hum(NAN), pres(NAN);
  uint8_t pressureUnit(2);                                           // unit: B000 = Pa, B001 = hPa, B010 = Hg, B011 = atm, B100 = bar, B101 = torr, B110 = N/m^2, B111 = psi
  bme.read(pres, temp, hum, metric, pressureUnit);                   // Parameters: (float& pressure, float& temp, float& humidity, bool celsius = false, uint8_t pressureUnit = 0x0)

  tempf= temp;
  humidity= hum;
  pressure= pres;
  
// WU Section

  screenAdd("Connect to WU"); 

  tempc =  ((tempf - 32)/9)*5;
  //float tempf =  (tempc * 9.0)/ 5.0 + 32.0; 
  dewptf = (dewPoints(tempf, humidity)); 
  dewptc = ((dewptf - 32)/9)*5;
  
  //print sensor data
  Serial.println("+++++++++++++++++++++++++");
  Serial.print("temp *F = ");
  Serial.println(tempf);
  Serial.print("temp *C = ");
  Serial.println(tempc);
  Serial.print("dew point *F= ");
  Serial.print(dewptf);
  Serial.print("humidity % = ");
  Serial.println(humidity);
  Serial.print("Pressure inHg= ");
  Serial.println(pressure);
  //Serial.print("Wind Direction = ");
  //Serial.println(winddir);
  //Serial.print("Wind Speed mph = ");
  //Serial.println(windspeed);


  
  //Send data to Weather Underground  

  screenAdd("Post Data to WU");

  Serial.println("Post Data to WU");
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    strcpy(state,"F");
    errorCount++;
    Serial.println(" WU Connection Failed");
    screenAdd(" WU Connection Failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/weatherstation/updateweatherstation.php?ID=";
  url += ID;
  url += "&PASSWORD=";
  url += PASSWORD;
  url += "&dateutc=now";
  url += "&tempf=";
  url += tempf;
  url += "&dewptf=";
  url += dewptf;
  url += "&humidity=";
  url += humidity;
//  url += "&winddir=";
//  url += winddir;
//  url += "&windspeedmph=";
//  url += windspeed;
  url += "&baromin=";
  url += pressure;
  url += "&weather=&clouds=&softwaretype=Arduino-ESP8266&action=updateraw";
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print("WU Read Data");
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");


  dtostrf(tempc,4,2,buff);
  strcpy(pre,"Temp C: ");
  strcpy(post,buff);
  screenAdd(strcat(pre,post));
  

  dtostrf(humidity,4,2,buff);
  strcpy(pre,"Humidity %: ");
  strcpy(post,buff);
  screenAdd(strcat(pre,post));


  dtostrf(pressure,4,2,buff);
  strcpy(pre,"Pressure inHg: ");
  strcpy(post,buff);
  screenAdd(strcat(pre,post));

  dtostrf(dewptc,4,2,buff);
  strcpy(pre,"Dew Point C: ");
  strcpy(post,buff);
  screenAdd(strcat(pre,post));

  // Thingspeak

  ThingSpeak.setField(1,tempc);
  ThingSpeak.setField(2,humidity);
  ThingSpeak.setField(3,pressure);
  ThingSpeak.setField(4,dewptc);

  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  


  display.display();
  sleepMode(); 

}

double dewPoints(double tempf, double humidity) //Calculate dew Point
{
  double A0= 373.15/(273.15 + tempf);
  double SUM = -7.90298 * (A0-1);
  SUM += 5.02808 * log10(A0);
  SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
  SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
  SUM += log10(1013.246);
  double VP = pow(10, SUM-3) * humidity;
  double T = log(VP/0.61078);   
  return (241.88 * T) / (17.558-T);
}

void sleepMode(){
  Serial.println("Sleeping...");
  //delay(sleepTimeS*1000);
  ESP.deepSleep(sleepTimeS * 1000000);
}

void screenAdd(char text[25]) {
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  
  //Top
  display.clear();
  display.drawString(1, 1, "IP:");
  display.drawString(25, 1, String(ip[0])+"."+String(ip[1])+"."+String(ip[2])+"."+String(ip[3]));
  display.drawString(100, 1, String(state[0]));
  display.drawString(110, 1, String(errorCount));

  // Line move up
  if (showCount==0){
    strcpy(showLine[0], text);  
  }else if (showCount==1){
    strcpy(showLine[1], text);  
  }else if (showCount==2){
    strcpy(showLine[2], text); 
  }else if (showCount==3){
    strcpy(showLine[3], text); 
  }else{
    strcpy(showLine[0], showLine[1]);
    strcpy(showLine[1], showLine[2]);
    strcpy(showLine[2], showLine[3]);
    strcpy(showLine[3], text);
  }
  // Content
  display.drawString(5, 15, showLine[0]);
  display.drawString(5, 25, showLine[1]);
  display.drawString(5, 35, showLine[2]);
  display.drawString(5, 45, showLine[3]);
  
  display.display();
  showCount++;
  delay(1000);
  
}

