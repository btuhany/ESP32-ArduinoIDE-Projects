/*
Arduino|ESP32   OLED      SI470X
3v3             vcc       vcc
gnd             gnd       gnd
i2c?sda|21      sda       sda
i2c?scl|22      scl       scl
?|18                      rst


*/
///////////Libraries////////////////
#include <SI470X.h>             //Si470X Lib
#include <SparkFunSi4703.h>     //This lib also used only for better RDS info. 

#include <Wire.h>
#include <Adafruit_GFX.h>    //OLED SSD1306 Lib
#include <Adafruit_SSD1306.h>

#include <WiFi.h>      //Webserver Lib
#include <WebServer.h>

//////////////////////////////////
//OLED Timer
unsigned long oled_elapsed=0;
//RDS Timer
unsigned long rds_elapsed = 0;  //rds1 timer
unsigned long rds2_elapsed = 0; //rds2 timer

//Wifi
const char* ssid = "wifi ssid";  //SSID
const char* password = "wifi_password";  //Password

#define MAX_DELAY_RDS 80   // rds1 40ms - polling method
#define DELAY_RDS 40       // rds2

//ESP32 Pins
#define RESET_PIN 25       
#define SDA_PIN   21         

// I2C bus pin on ESP32
#define ESP32_I2C_SDA 21
#define ESP32_I2C_SCL 22

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Si470x 
SI470X rx;
Si4703_Breakout radio(25, 21, 22, 18); //for rds data

int volume;
float rssi;
float freq;
char *rdsMsg;
char *stationName;
char *rdsTime;
int rdsAc;
char rdsBuffer[64]; 
int rdsinfo=0;
int Stereoinfo=0;
// Show current status
void showStatus()
{
  char aux[80];
  sprintf(aux,"\nYou are tuned on %u MHz | RSSI: %3.3u dbUv | Vol: %2.2u | %s ",rx.getFrequency(), rx.getRssi(), rx.getVolume(), (rx.isStereo()) ? "Yes" : "No" );
  Serial.print(aux);

  freq=rx.getFrequency();
  rssi=rx.getRssi();
  volume=rx.getVolume();
  
}

//////////////////////////////Webserver/////////////////////////////////////
WebServer server(80);
String ptr;
String SendHTML(float freq,float rssi,int volume,char *rdsBuffer){
  ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  
  ptr +="<title>Si470x Radio</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 40px;} h1 {color: #e63946;margin: 50px auto 30px;text-align: center;} h3 {color: #18717A;margin-bottom: 2px;}\n";
 
  ptr +=".button {display: inline-block;width: 50px;background-color: #3498db;border:  1px solid black;color: #000000;padding: 15px 15px;text-decoration: none;font-size: 18px;margin: 0px auto 35px;cursor: pointer;border-radius: 15px;text-align: center;}\n";
  ptr +=".button2 {display: block;width: 370px;background-color: #D04A4A;border:  1px solid black;color: black;padding: 10px 5px;text-decoration: none;font-size: 20px;margin: 0px auto 0px;cursor: pointer;border-radius: 10px;text-align: center;}\n";
  ptr +=".button3 {display: inline-block;width: 60px;background-color: #3498db;border:  1px solid black;color: #444444;padding: 7px 10px;text-decoration: none;font-size: 17px;margin: 0px auto 35px;cursor: pointer;border-radius: 10px;text-align: center;}\n";
  ptr +=".button-on {background-color: #A8DADC;}\n";
  ptr +=".button-on:active {background-color: #e63946;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +=".button2-on {background-color: #EB8E8E;}\n";
  ptr +=".button2-on:active {background-color: #e63946;}\n";
  
  ptr +=".divider{ width:5px;height:auto;display:inline-block;}\n";
  ptr +=".divider2{ width:23px;height:auto;display:inline-block;}\n";
  ptr +=".divider3{ width:44px;height:auto;display:inline-block;}\n";
  ptr +=".divider4{ width:10px;height:auto;display:inline-block;}\n";
  ptr +="p1 {font-size: 19px;color: #00AEBD;margin-bottom: 10px;}\n";
  ptr +="p2 {font-size: 17px;color: #37838A;margin-bottom: 10px;}\n";
  ptr +="p3 {font-size: 28px;color: #e63946;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  
  ptr +="<script>\n";
  ptr +="setInterval(loadDoc,600);\n";
  ptr +="function loadDoc() {\n";
  ptr +="var xhttp = new XMLHttpRequest();\n";
  ptr +="xhttp.onreadystatechange = function() {\n";
  ptr +="if (this.readyState == 4 && this.status == 200) {\n";
  ptr +="document.getElementById(\"webpage\").innerHTML =this.responseText}\n";
  ptr +="};\n";
  ptr +="xhttp.open(\"GET\", \"/\", true);\n";
  ptr +="xhttp.send();\n";
  ptr +="}\n";
  ptr +="</script>\n";

  
  ptr +="</head>\n";
  ptr +="<body bgcolor= #F6FFFB style=color:white>\n";
  ptr +="<body>\n";
  
  ptr +="<div id=\"webpage\">\n";
  ptr +="<div style=\"display: inline-block; text-align: left;\">";
  ptr +="<h1>Si470x Radio</h1>\n";

  ptr +="<p1>RDS:";
  ptr +="</p1>";
  ptr +="<p3>";
  if (rdsinfo==1){
  ptr +=rdsBuffer;}
  else{
  ptr +="no rds";
  }
  ptr +="</p3>";
  
  
  ptr += "<br/><br/>\n";
  
  ptr +="<p1>FREQUENCY: ";
  ptr +="</p1>";
  ptr +="<p3>";
  ptr +=(float)freq/100;
  ptr +=" Mhz";
  ptr +="</p3>";

  ptr += "<br/><br/>\n";
  
  ptr +="\n\n<p1>VOLUME: ";
  ptr +="</p1>";
  ptr +="<p3>";
  ptr +=(int)volume;
  ptr +="/15";
  ptr +="</p3>";
  

  ptr += "<br/><br/>\n";

  
  ptr +="<p1>\nSIGNAL POWER(RSSI): ";
  ptr +="</p1>";
  ptr +="<p3>";
  ptr +=(float)rssi;
  ptr +=" dbUv";
  ptr +="</p3>";
  

  ptr += "<br/><br/>\n";

  ptr +="\n<p1>\nSTEREO: ";
  ptr +="</p1>";
  ptr +="<p3>";
  if(Stereoinfo==1){
  ptr +="YES"; 
  }
  else{
  ptr +="NO";
  }
  ptr +="</p3>";
  ptr += "<br/><br/><br/>\n";

  ptr +="<p2>Frequency Up/Down: ";
  ptr +="</p2>";
  
  ptr +="<a class=\"button button-on\" href=\"/freqIncrease\">Up</a>";
  ptr +="<div class=\"divider\">";
  ptr +="</div>";
  ptr +="<a class=\"button button-on\" href=\"/freqDecrease\">Down</a>\n";
  
  ptr += "<br/>\n";

  ptr +="<p2>Seek Up/Down: ";
  ptr +="</p2>";

  ptr +="<div class=\"divider3\">";
  ptr +="</div>";

  ptr +="<a class=\"button button-on\" href=\"/seekUp\">Up</a>";
  ptr +="<div class=\"divider\">";
  ptr +="</div>";
  ptr +="<a class=\"button button-on\" href=\"/seekDown\">Down</a>\n";
  
  ptr += "<br/>\n";
  ptr +="<p2>Volume Up/Down: ";
  ptr +="</p2>";
  
  ptr +="<div class=\"divider2\">";
  ptr +="</div>";
  
  ptr +="<a class=\"button button-on\" href=\"/volumeUp\">Up</a>";
  ptr +="<div class=\"divider\">";
  ptr +="</div>";
  ptr +="<a class=\"button button-on\" href=\"/volumeDown\">Down</a>";
  
  ptr += "<br/>\n";
  ptr += "<br/>\n";
  ptr +="<h3>Favourite FM Stations: ";
  ptr +="</h3>";
  ptr += "<br/>\n";
  ptr +="<a class=\"button2 button2-on\" href=\"/fmStation1\">89.6  Kafa Radyo</a>";

  ptr +="<div class=\"divider4\">";
  ptr +="</div>";
  ptr +="<a class=\"button2 button2-on\" href=\"/fmStation2\">92.0 Kral FM</a>";

  ptr +="<div class=\"divider4\">";
  ptr +="</div>";
  ptr +="<a class=\"button2 button2-on\" href=\"/fmStation4\">95.6 TRT Radyo</a>";

  ptr +="<div class=\"divider4\">";
  ptr +="</div>";
  ptr +="<a class=\"button2 button2-on\" href=\"/fmStation3\">106.2 Virgin Radio</a>";


  ptr +="<div class=\"divider4\">";
  ptr +="</div>";
  ptr +="<a class=\"button2 button2-on\" href=\"/fmStation5\">106.0 Pal FM</a>";

  
ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
  
  
}
void handle_OnConnect (){server.send(200, "text/html", SendHTML(freq,rssi,volume,rdsBuffer));}
void handle_NotFound(){server.send(404, "text/plain", "Not found");}
void handle_freqIncrease(){
  rx.setFrequencyUp();
  Serial.println("\nFrequency Increased");
  showStatus();
  server.send(200, "text/html", SendHTML(freq,rssi,volume,rdsBuffer)); 
}
void handle_freqDecrease(){
  rx.setFrequencyDown();
  Serial.println("\nFrequency Decreased");
  showStatus();
  server.send(200, "text/html", SendHTML(freq,rssi,volume,rdsBuffer)); 
}

void handle_seekUp(){
  rx.seek(SI470X_SEEK_WRAP, SI470X_SEEK_UP);
  showStatus();
  server.send(200, "text/html", SendHTML(freq,rssi,volume,rdsBuffer)); 
}
void handle_seekDown(){
  rx.seek(SI470X_SEEK_WRAP, SI470X_SEEK_DOWN);       
  showStatus();
  server.send(200, "text/html", SendHTML(freq,rssi,volume,rdsBuffer)); 
}
void handle_volumeUp(){
  rx.setVolumeUp();
  Serial.println("\nVolume Increased");
  showStatus();
  server.send(200, "text/html", SendHTML(freq,rssi,volume,rdsBuffer));
  
}
void handle_volumeDown(){
  rx.setVolumeDown();
  Serial.println("\nVolume Decreased");
  showStatus();
  server.send(200, "text/html", SendHTML(freq,rssi,volume,rdsBuffer));
}
void handle_fmStation1(){
  rx.setFrequency(8960);;
  Serial.println("\nFrequency Set 89.6");
  showStatus();
  server.send(200, "text/html", SendHTML(freq,rssi,volume,rdsBuffer));
  
}

void handle_fmStation2(){
  rx.setFrequency(9200);;
  Serial.println("\nFrequency Set 92.0");
  showStatus();
  server.send(200, "text/html", SendHTML(freq,rssi,volume,rdsBuffer));
  
}
void handle_fmStation3(){
  rx.setFrequency(10620);;
  Serial.println("\nFrequency Set 106.2");
  showStatus();
  server.send(200, "text/html", SendHTML(freq,rssi,volume,rdsBuffer));
  
}
void handle_fmStation4(){
  rx.setFrequency(9560);;
  Serial.println("\nFrequency Set 95.6");
  showStatus();
  server.send(200, "text/html", SendHTML(freq,rssi,volume,rdsBuffer));
  
}
void handle_fmStation5(){
  rx.setFrequency(10600);;
  Serial.println("\nFrequency Set 106.0");
  showStatus();
  server.send(200, "text/html", SendHTML(freq,rssi,volume,rdsBuffer));
  
}

////////////////////////////////////////////////////////////////////////////
void showHelp()
{
  Serial.println("\nType U to increase and D to decrease the frequency");
  Serial.println("Type S or s to seek station Up or Down");
  Serial.println("Type + or - to volume Up or Down");
  Serial.println("Type 0 to show current status");
  Serial.println("Type ? to this help.");
  Serial.println("Type r to see RDS data (R to off)");
  Serial.println("==================================================");
  delay(500);
}




void setup()
{
    Serial.begin(115200);
/******************************OLED DISPLAY**********************/      
    while (!Serial) ;
      if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
/******************************Si470x RADIO**********************/  
    // The line below may be necessary to setup I2C pins on ESP32
    Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);

    
    rx.setup(RESET_PIN, ESP32_I2C_SDA);
    rx.setVolume(9);             // INITIAL VOLUME
    rx.setRDS(true);
    delay(500);



    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    
    // Select a station with RDS service in your place
    Serial.print("Starting Radio");
    display.println("Starting Radio");
    display.display();
    rx.setFrequency(10740); // It is the frequency you want to select in MHz multiplied by 100.

    // Enables SDR
    rx.setRds(true);
    rx.setRdsMode(1); 
    rx.setMono(false);
    rx.setSeekThreshold(30); // Sets RSSI Seek Threshold (0 to 127)

    showHelp();
    showStatus();
/******************************WEBSERVER**********************/    
    Serial.println("Starting WebServer");
    
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 20);
    display.println("Starting WebServer");
    display.display();
    delay(100);
    Serial.println("Connecting to ");
    display.setCursor(0, 30);
    display.println("Trying to connect:");
    Serial.println(ssid);
    display.setCursor(0, 40);
    display.println(ssid);
    //connect to your local wi-fi network
    WiFi.begin(ssid, password);
    display.display();

    //check wi-fi is connected to wi-fi network
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    }
    delay(1400);
    Serial.println("");
    Serial.println("WiFi connected..!");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("WiFi connected..!");
    
    
    display.setCursor(0, 10);
    Serial.print("Got IP: ");
    display.setCursor(0, 20);
    display.println("WiFi connected..!");  
    display.setCursor(0, 30);
    display.println("Got IP: "); 
    Serial.println(WiFi.localIP());
    display.setCursor(0, 40);
    display.println(WiFi.localIP()); 
    display.display();
    delay(2000);
    server.on("/", handle_OnConnect);
    server.onNotFound(handle_NotFound);
    server.on("/freqIncrease", handle_freqIncrease);
    server.on("/freqDecrease", handle_freqDecrease);
    server.on("/seekUp", handle_seekUp);
    server.on("/seekDown", handle_seekDown);
    server.on("/volumeUp", handle_volumeUp);
    server.on("/volumeDown", handle_volumeDown);
    server.on("/fmStation1", handle_fmStation1);
    server.on("/fmStation2", handle_fmStation2);
    server.on("/fmStation3", handle_fmStation3);
    server.on("/fmStation4", handle_fmStation4);
    server.on("/fmStation5", handle_fmStation5);
    server.begin();
    Serial.println("HTTP server started");
}


void loop()
{ 
  rssi=rx.getRssi();
  server.handleClient();  //Webserver

/*//////////////////////////RDS////////////////////////////////////*/
  if((millis()-rds2_elapsed)>DELAY_RDS) //RDS timer
  {
    radio.readRDS(rdsBuffer, 400);  //400ms timeout
    rds2_elapsed = millis();

    if(rdsAc==1)      //show RDS in SerialPort?
    {
      Serial.println("\nRDS heard:");
    if(rdsBuffer[0] == '\0') //RDS data control
       {Serial.println("no rds");
       }
    else
      {
        Serial.println("\n");
        Serial.print(rdsBuffer);
      }
    }   
  }


  if (Serial.available() > 0)
  {
    char key = Serial.read();
    if (key == '+')           /////VOLUME UP
    {
      rx.setVolumeUp();
      showStatus();
    }
    if (key == '-')            /////VOLUME DOWN
    {
      rx.setVolumeDown();
      showStatus();
    }
    else if (key == 'U' || key =='u')       /////FREQUENCY UP
    {
      rx.setFrequencyUp();
      showStatus();
      
    }
    else if (key == 'D' || key =='d')     /////FREQUENCY DOWN
    {
      rx.setFrequencyDown();
      showStatus();
    }
    else if (key == 'S')             /////SEEK UP
    {
      rx.seek(SI470X_SEEK_WRAP, SI470X_SEEK_UP);
      showStatus();
    }
    else if (key == 's')                /////SEEK DOWN
    {
      rx.seek(SI470X_SEEK_WRAP, SI470X_SEEK_DOWN);       
      showStatus();
    }
    else if (key == '0')          /////SHOW STATUS
    {
      showStatus();
    }
    else if (key == '?')         /////SHOW HELP
    {
      showHelp();
      showStatus();
    }
    else if (key == 'r')           /////RDS DATA IN SERIALPORT ON
    {
      rdsAc=1;
    }
    else if (key == 'R')            /////RDS DATA IN SERIALPORT OFF
    {
      rdsAc=0;
    }    
     

    
     delay(200);
  }
 
  delay(5);


//////////////////////////////////OLED///////////////////////////////////
  if (millis() - oled_elapsed > 500) //oled timer
  { 
    freq = rx.getFrequency();
    volume =rx.getVolume();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 20);
    display.println("Frekans:");
    display.setCursor(75, 20);
    display.println(freq/100, 1);
    display.setCursor(106, 20);
    display.println("Mhz");
    display.setCursor(0, 30);
    display.println("Ses:");
    display.setCursor(75, 30);
    if (volume<10)
    {
      display.println("0");
      display.setCursor(82, 30);
      display.println(volume);
    }
    else
    {
      display.println(volume);
    }
    display.setCursor(88, 30);
    display.println("/15");
    display.setCursor(0, 40);
    display.println("Sinyal Gucu:");
    display.setCursor(75, 40);
    display.println(rx.getRssi());
    display.setCursor(90, 40);
    display.println("dbUv"); 
    display.setTextSize(1);
    display.setCursor(0, 8);
    display.println("RDS:");
    display.setTextSize(2);
    display.setCursor(30, 0);  
    if(rdsBuffer[0] == '\0') //RDS data control
    {
     display.println("no rds");
     rdsinfo=0;
    }
    else
    {rdsinfo=1;
      display.println(rdsBuffer);
    }
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.println("Stereo:");
    display.setCursor(75,50);
    if(rx.isStereo()){
      display.println("Yes");
      Stereoinfo=1;
    }
    else{
      display.println("no rds");
      Stereoinfo=0;
    }
    display.setCursor(0, 60);
    
    display.println(WiFi.localIP()); 
    display.display();
    
   }

  
  
}
