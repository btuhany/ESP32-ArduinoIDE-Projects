/*  ESP32 
 *  Kullanıcı, ESP32'ye bağlı halde bulunan farklı sensorlerden 
 hangisini/hangilerini calistirmak istiyorsa webserver üzerinden
 secim yapar. Bu sensorlerin pinlerini de istedigi gibi ayarlayabilir.
 Sensorler hangi pine baglılarsa bu bilgi de webserver üzerinden girilmelidir.
 İstege göre alarm pini oluşturulabilir. Bu pin high veya low durumda 
 webserver üzerinden uyarı verir.
 *  
 *  Proje iki adet sensor ile gerçekleştirildi(DS18b20,DHT11)
 *  Farklı sensorler ile genişletilebilir 
 *  
 *  
 *  
 *  
 */



/*Kütüphaneler*/
#include <DHT.h>      ///DHT11
#include <DHT_U.h>

#include <OneWire.h>             //Dallas Ds18b20
#include <DallasTemperature.h> 

#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);


const char* ssid = "wifi_ssid";
const char* password = "wifi_password";

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";

#define DHTTYPE DHT11
int highalarm=0;
int lowalarm=0;
int AlarmPin;
int alarmdeger=2;
int SicaklikDHTPin;
int DS18B20Pin;
int NemDHTPin;
float t=0; //DHT sicaklik
float h=0; //DHT nem
int bb;
int DS18B20PinKontrol=0; 
unsigned long lastTime=0;
int kontrol=0;
int sicaklikbutonkontrol=0;
int nemdhtbutonkontrol=0;
int ds18b20butonkontrol=0;
int alarmAnalogDeger;

void DHTcalistir(){               //DHT11 hem nem hem sicaklik ölçtüğü için ayrı ayrı stringler yerine tek fonksiyonda
  int DHTPIN=SicaklikDHTPin;
      DHT dht(DHTPIN,DHTTYPE);
      dht.begin();     
  if(nemdhtbutonkontrol==1){ 
    h = dht.readHumidity();
   
  }
   if(sicaklikbutonkontrol==1){
    t = dht.readTemperature(); 
   }
}

String nemdht(){        //nemdht=h
  DHTcalistir();
  return String(h);
}
String sicaklik(){      //sicaklik=t               
 DHTcalistir();
  return String(t);
}
String ds18b20(){   
  const int oneWireBus = DS18B20Pin; 
  OneWire oneWire(oneWireBus); 
  DallasTemperature sensors(&oneWire);                    
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  
  return String(temperatureC);
}


String KontrolAktif(){      //String KontrolAktif()=1;
  bb=1;
  return String(bb);
}

String alarm(){                 
  if (highalarm==1){
    alarmAnalogDeger=analogRead(AlarmPin);
    if(alarmAnalogDeger>2000){
       alarmdeger=1;
       Serial.println("ALERTTT3");
     }
    else{
      alarmdeger=0;
      Serial.println("ALERTTT1");
    }
    Serial.println("icerde");
    
  }
  if (lowalarm==1){
    Serial.println("ALERTTT4");
    alarmAnalogDeger=analogRead(AlarmPin);
    if(alarmAnalogDeger>2000){
      alarmdeger=0;
    }
    else{
      alarmdeger=1;
    }
  }
  Serial.println("icerde2");
  Serial.println("ALERTTT2 RETURN");
  Serial.println(analogRead(AlarmPin));
  return String(alarmdeger);
}



///////////////////////////////////////////////////.........ANA SAYFA...........///////////////////////////////////////////////////////////////////////
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP32 Test-Sensor Secimi</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} 
  body{margin-top: 60px;} 
  h1 {font-size: 46px;color: #166273; margin: 10px auto 30px; text-align: center;}
  p1 {font-size: 25px;color: black;margin-bottom: 10px;}
  p2 {font-size: 18px;color: black;margin-bottom: 10px;}
  p3 {font-size: 22px;color: black;margin-bottom: 10px;}
  .divider{ width:172px;height:auto;display:inline-block;}
  .divider2{ width:500px;height:auto;display:block;}
  .divider3{ width:228px;height:auto;display:inline-block;}
  input {
     display: inline-block;
     align-items: center;
     justify-content: center;
     margin: 0 auto;
   } 
   legend {
      background-color: #166273;
      color: white;
      padding: 5px 10px;
      font-size: 28px;
   }
  </style>
  
  <script type="text/javascript"> 
              function display_c(){
              var refresh=1000; // Refresh rate in milli seconds
              mytime=setTimeout('display_ct()',refresh)
              }                                                     
              function display_ct() {
              var x = new Date()
              var x1=x.getMonth()+1+ "/" + x.getDate() + "/" + x.getFullYear(); 
              x1 = x1 + " - " +  x.getHours( )+ ":" +  x.getMinutes() + ":" +  x.getSeconds();
              document.getElementById('ct').innerHTML = x1;
              display_c();
               }       
  </script>
  </head>
  
  <body>
  <body onload=display_ct(); bgcolor="#E0F1F1" style="color:black">
    <div style="display: inline-block; text-align: left;">             

      <div class="divider2">
      </div>

      <h1>ESP32 Sensor Testi</h1>
  
      <div style="display: block; text-align: right;">
      <p2> 
      <span id='ct' ></span></body></p2>
      </div>

      <legend>Sensor/Modul Secimi:
      </legend>

      <br>
        <div style="display: inline-block; text-align: left;">
            <form  action="/check" onsubmit="return handle_form_submission()"  >                 
              <input type="checkbox" name="message" value="NemDhtBaslat" style="width: 20px; height: 20px;"> <p1>Nem(DHT11)</p1>
                
              <br><br>
              <input type="checkbox" name="message" value="SicaklikBaslat" style="width: 20px; height: 20px;"> <p1>Sicaklik(DHT11)</p1>
              <div class="divider3">
              </div>  
              <p3>DHT11 Pin:</p3> <input type="text" name="input2" maxlength="2" style="width: 28px; height: 25px;font-size:20px;" required>
              <br><br>
              <input type="checkbox" name="message" value="Ds18b20Baslat" style="width: 20px; height: 20px;"> <p1>Sicaklik(DS18B20)</p1>
              <div class="divider">
                </div>     
              <p3>DS18B20 Pin:</p3> <input type="text" name="input1" maxlength="2" style="width: 28px; height: 25px;font-size:20px;" required>
              <br><br>
              
              <legend for="alarms">Alarm Ekle:</legend>
              <br>
                 <p1>Alarm Pin:</p1> <input type="text" name="input3" maxlength="2" style="width: 28px; height: 25px;font-size:20px;">
                 <br>
                 <input type="radio" id="HIGH" name="alarm" value="yok" style="width: 21px; height: 21px;">
                 <p1>Alarm Yok</p1><br>
                 <input type="radio" id="HIGH" name="alarm" value="HIGH" style="width: 21px; height: 21px;">
                 <p1>HIGH konumdayken</p1><br>
                 <input type="radio" id="LOW" name="alarm" value="LOW" style="width: 21px; height: 21px;">
                 <p1>LOW konumdayken</p1><br>
              <br><br>
              <input type="submit" value="Baslat" style="width: 100px; height: 38px;font-size:27px;"/>      
            
            </form>
        </div>
      <br>
       <br>
      <p2> *Input ve Output GPIO Port pinleri:<br> 2,4,5,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33<br>
         Sadece Input GPIO pinleri: 34,35,36,39 <br><br>*ADC1 Pinleri (Alarm pini icin): 36,39,34,35,32,33 
      </p2>                    
    </div>
  </body>
</html>)rawliteral";
/////////////////////////////////////////////////////////////OLCUM SAYFASI///////////////////////////////////////////////////////////
const char index_html2[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
<title>TEST</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} 
  body{margin-top: 60px;} 
  h1 {font-size: 52px;color: #166273; margin: 10px auto 30px; text-align: center;}
  p1 {font-size: 60px;color: black;margin-bottom: 10px;}
  p2 {font-size: 25px;color: #00AEBD;margin-bottom: 10px;}
  p3 {font-size: 14px;color: black;margin-bottom: 10px;}
  p4 {font-size: 25px;color: #0C282E;margin-bottom: 10px;display: none;}
  p5 {font-size: 25px;color: #0C282E;margin-bottom: 10px;display: none;}
  p6 {font-size: 20px;color: purple;margin-bottom: 10px;display: none;}
  p7 {font-size: 25px;color: #0C282E;margin-bottom: 10px;display: none;}
  .divider{ width:350px;height:auto;display:inline-block;}
  </style>



<body bgcolor="#E0F1F1" style="color:black">
 
 <h1> ESP32 Olcumleri </h1>
    
      <div style="display: inline-block; text-align: left;"> 
      
        <p4 id="Nemtextid">
        Nem(DHT11):
        <span id="temp_celcius">%NEMDHT%</span>&#37;</p4>
        
        <p5 id="Sicakliktextid">
        Sicaklik(DHT11):
        <span id="htmlsicaklikdegeriid">%htmlsicaklikdegeri%</span> &#8451;</p5>
        
        <p7 id="Ds18b20textid">
        Sicaklik(DS18B20):
        <span id="htmlds18b20degeriid">%htmlds18b20degeri%</span> &#8451;</p7>
          
        <p6 id="kontrol">
        <span id="htmlnemdhtkontrolid">%htmlnemdhtkontrol%</span>
        <span id="htmlsicaklikkontrolid">%htmlsicaklikkontrol%</span>
        <span id="htmlds18b20kontrolid">%htmlds18b20kontrol%</span>

        <span id="alarmvoltageid">%alarmvoltage%</span>
        </p6>
       </div>
</body>

<script>
          setInterval(function ( ) {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
              if (this.readyState == 4 && this.status == 200) {
                document.getElementById("temp_celcius").innerHTML = this.responseText;
              }
              if(document.getElementById("htmlnemdhtkontrolid").innerHTML=="1"){
                document.getElementById("Nemtextid").style.display="block";
              }
            };
            xhttp.open("GET", "/nemdht", true);
            xhttp.send();
          }, 1000 ) ;
          //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          setInterval(function ( ) {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
              if (this.readyState == 4 && this.status == 200) {
                document.getElementById("htmlsicaklikdegeriid").innerHTML = this.responseText;
                
              }
              if(document.getElementById("htmlsicaklikkontrolid").innerHTML=="1"){
                document.getElementById("Sicakliktextid").style.display="block";
              }
            };
            
          
            xhttp.open("GET", "/sicaklik", true);
            xhttp.send();
          }, 1000 ) ;
          //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          setInterval(function ( ) {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
              if (this.readyState == 4 && this.status == 200) {
                document.getElementById("htmlds18b20degeriid").innerHTML = this.responseText;
                
              }
              if(document.getElementById("htmlds18b20kontrolid").innerHTML=="1"){
                document.getElementById("Ds18b20textid").style.display="block";
              }
            };
            
          
            xhttp.open("GET", "/ds18b20", true);
            xhttp.send();
          }, 1000 ) ;
          //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          setInterval(function ( ) {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {

              if (this.readyState == 4 && this.status == 200) {
                document.getElementById("alarmvoltageid").innerHTML = this.responseText;
                if(document.getElementById("alarmvoltageid").innerHTML=="1"){
                 alert('UYARI!');
              }
              }
              
            };
            
          
            xhttp.open("GET", "/alarm", true);
            xhttp.send();
          }, 2000 ) ;
</script>
</html>

)rawliteral";

String processor(const String& var){
  
  //Serial.println(var);
  if(var == "NEMDHT"){
    return nemdht();
  }                                           
  else if(var == "htmlsicaklikdegeri"){                   
    return sicaklik();
  }
  else if(var == "htmlds18b20degeri"){                  
    return ds18b20();
  }
  if(nemdhtbutonkontrol==1){
    if(var == "htmlnemdhtkontrol"){
      return KontrolAktif();
    }
  }
  if(sicaklikbutonkontrol==1){
    if(var == "htmlsicaklikkontrol"){
      return KontrolAktif();
    }
  }
  if(ds18b20butonkontrol==1){
    if(var == "htmlds18b20kontrol"){
      return KontrolAktif();
    }
  }

  if(var == "alarmvoltage"){
    return alarm();
  }
  return String();
}
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

/*void handle_check(AsyncWebServerRequest *request){
  Serial.print("deneme");
}*/
//////////////////////////////////////////////////////////////////SETUP/////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected..!\n");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  server.on("/alarm", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", alarm().c_str());        //?????????????????????????????????????????????
  });
  server.on("/nemdht", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", nemdht().c_str());
  });
 server.on("/sicaklik", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", sicaklik().c_str());
  });
  server.on("/ds18b20", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", ds18b20().c_str());
  });
  
  server.on("/check", HTTP_GET, [](AsyncWebServerRequest *request){
    String message;
    int params = request->params();
    int i;

    for(int i=0;i<params;i++){
        
    AsyncWebParameter* p = request->getParam(i);
    //Serial.printf("HEADER[%s]: %s\n", p->name().c_str(), p->value().c_str());
      //  Serial.println("OK");
    //Serial.println(p->value().c_str()[0]);
        if(p->value().c_str()[0]=='N'){
          nemdhtbutonkontrol=1;
          
          }
        if(p->value().c_str()[0]=='S'){
          sicaklikbutonkontrol=1;
          }
        if(p->value().c_str()[0]=='D'){
          ds18b20butonkontrol=1;
          }
        if(p->value().c_str()[0]=='H'){
          //Serial.println("HIGHken alarm");
          highalarm=1;
          Serial.println("highalarm esittir 1");
          
          }
        if(p->value().c_str()[0]=='L'){
          //Serial.println("LOWken alarm");
          lowalarm=1;
          }     
    }
         
      String inputMessage;
      String inputParam;

      String inputMessage2;
      String inputParam2;

      String inputMessage3;
      String inputParam3;
      
        // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
        if (request->hasParam(PARAM_INPUT_1)) {
          inputMessage = request->getParam(PARAM_INPUT_1)->value();
          inputParam = PARAM_INPUT_1;
          DS18B20Pin=inputMessage.toInt();   
        }
        else{
          DS18B20PinKontrol=0; 
        }
        // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
        if (request->hasParam(PARAM_INPUT_2)) {
          inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
          inputParam2 = PARAM_INPUT_2;
          SicaklikDHTPin=inputMessage2.toInt();
        }
        else {
          inputMessage = "No message sent";
          inputParam = "none";
        }        
        if (request->hasParam(PARAM_INPUT_3)) {
          inputMessage3 = request->getParam(PARAM_INPUT_3)->value();
          inputParam3 = PARAM_INPUT_3;
          AlarmPin=inputMessage3.toInt();
        }
        else {
          
        }            
        request->send_P(200, "text/html", index_html2,processor);


  });
  server.onNotFound(notFound);
  server.begin();
}

void loop(){
}
