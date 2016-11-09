#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <TextFinder.h>


// Server Yahoo
IPAddress server(72,30,202,51);

const char* ssid     = "RonAP"; 
const char* password = "1234567890";
const int sleepTimeS = 600; //18000 for Half hour, 300 for 5 minutes etc.


///////////////Weather//////////////////////// 
char host [] = "weatherstation.wunderground.com";  
char WEBPAGE [] = "GET /weatherstation/updateweatherstation.php?";
char ID [] = "IOMKRET2";
char PASSWORD [] = "4j19zv3n";

WiFiClient client;

TextFinder  finder( client );  

char place[50];
char hum[30];

float tempf = 0;
float tempc = 0;
float humidity =0;
float dewptf =0;
float pressure =0;
float windspeed=0;
int winddir=0;

void setup() {

  Serial.begin(115200);

  
  delay(1000);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

}

void loop() {

if (client.connect(server, 80))
  {
    // Call Wetter-API
    // w: ID from your City
    // http://weather.yahooapis.com/forecastrss?w=12893459&u=c
    ///
    Serial.println("Connect to Yahoo Weather...");
    client.println("GET /v1/public/yql?q=select%20*%20from%20weather.forecast%20where%20woeid%20in%20(select%20woeid%20from%20geo.places(1)%20where%20text%3D%22Nonthaburi%2CTH%22)&format=xml&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys HTTP/1.0");
    client.println("HOST:query.yahooapis.com\n\n");
    client.println();
    Serial.println("Connected...");

    
  } 
  else
  {
    Serial.println(" connection failed");
  } 

  if (client.connected())
  {  

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


  
// WU Section

  //float tempf = 30; 
  tempc =  ((tempf - 32)/9)*5;
  //float tempf =  (tempc * 9.0)/ 5.0 + 32.0; 
  //float humidity = 28; 
  dewptf = (dewPoints(tempf, humidity)); 

  //check sensor data
  Serial.println("+++++++++++++++++++++++++");
  Serial.print("tempF= ");
  Serial.print(tempf);
  Serial.println(" *F");
  Serial.print("tempC= ");
  Serial.print(tempc);
  Serial.println(" *C");
  Serial.print("dew point= ");
  Serial.print(dewptf);
  Serial.println(" *F");
  Serial.print("humidity= ");
  Serial.println(humidity);
  Serial.print("Pressure= ");
  Serial.print(pressure);
  Serial.println(" In Hg");
  Serial.print("Wind Direction= ");
  Serial.println(winddir);
  Serial.print("Wind Speed= ");
  Serial.print(windspeed);
  Serial.println(" mph");  

  //Send data to Weather Underground  

  Serial.println("Send to WU Sensor Values");
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
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
  url += "&winddir=";
  url += winddir;
  url += "&windspeedmph=";
  url += windspeed;
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
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");

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
  delay(sleepTimeS*1000);
  //ESP.deepSleep(sleepTimeS * 1000000);
}

