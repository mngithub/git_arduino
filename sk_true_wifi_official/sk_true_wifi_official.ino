#include <SPI.h>
#include <WiFi.h>
//#include <SD.h>
#include <SdFat.h>
SdFat SD;

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// IMPORTANT !!!!!
// Must burn with Arduino 1.0.2 IDE only
// Wifi cannot make the connection on other IDE version !!!

// Becareful when using Serial.println(); may cause memory leak 
// and unexpected behavior.

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// DEFAULT VALUE FOR CONFIGURATION

// 0: on production nothing out from serial port
// 1: on dev the program will waiting for serial connection 
int CONFIG_DEBUG_MODE                   = 1;


String CONFIG_WIFI_SSID                 = "STEP_N1";
String CONFIG_WIFI_PASS                 = "Qwerty123?";
// 0: WPA/WPA2
// 1: WEP
unsigned int CONFIG_WIFI_MODE           = 0;
// your network key Index number (needed only for WEP)
unsigned int CONFIG_WIFI_KEYINDEX       = 0;


// Point to Cm Proxy Service IP
IPAddress CONFIG_CMPROXY_IP            (192, 168, 1, 135);
// Point to Cm Proxy Service port
// in some case firewall will block the connection 
// then disable firewall or allow connection on specific port
unsigned int CONFIG_CMPROXY_PORT        = 13002;

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// Infrared 1
String CONFIG_IN1_COMMAND               = "";
String CONFIG_IN1_SECOND                = "";
String CONFIG_IN1_PRIORITY              = "";
int CONFIG_IN1_PIN                      = 0;

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// Infrared 2
String CONFIG_IN2_COMMAND               = "";
String CONFIG_IN2_SECOND                = "";
String CONFIG_IN2_PRIORITY              = "";
int CONFIG_IN2_PIN                      = 0;

// ----------------------------------------------------
// ----------------------------------------------------
// ---------------------------------------------------
// Infrared 3
String CONFIG_IN3_COMMAND               = "";
String CONFIG_IN3_SECOND                = "";
String CONFIG_IN3_PRIORITY              = "";
int CONFIG_IN3_PIN                      = 0;

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// Infrared 4
//String CONFIG_IN4_COMMAND               = "";
//String CONFIG_IN4_SECOND                = "";
//String CONFIG_IN4_PRIORITY              = "";
//int CONFIG_IN4_PIN                      = 0;

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// CONST

// Debug LED
// build-in LED on wifi shield
const int L9                            = 9;
// Command Separator
String CONFIG_CMPROXY_COMMAND_PREFIX    = "|||";

// Infrared
// delay after sent command, in milliseconds
unsigned long CONFIG_INFRARED_INTERVAL  = 1000L; 
unsigned int CONFIG_INFRARED_COUNT_HIGH = 5000;

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// VARIABLE

// Initialize the Wifi client library
WiFiClient client;

int status = WL_IDLE_STATUS;

// count total command sent
unsigned int commandCount               = 0;

unsigned long VAR_IN1_CNT_HIGH          = 0;
unsigned long VAR_IN1_TIMESTAMP         = 0;
unsigned long VAR_IN2_CNT_HIGH          = 0;
unsigned long VAR_IN2_TIMESTAMP         = 0;
unsigned long VAR_IN3_CNT_HIGH          = 0;
unsigned long VAR_IN3_TIMESTAMP         = 0;
//unsigned long VAR_IN4_CNT_HIGH          = 0;
//unsigned long VAR_IN4_TIMESTAMP         = 0;

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------

void setup() {
  
  if(CONFIG_DEBUG_MODE == 1){
    //Initialize serial and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo only
    }
  }
  //Serial.println("Start...");
  
  // DEBUG LED
  pinMode(L9, OUTPUT);
  digitalWrite(L9, LOW);
  
  readConfig();
 
  digitalWrite(L9, HIGH);
   
  // infrared
  if(CONFIG_IN1_PIN > 0) pinMode(CONFIG_IN1_PIN,INPUT);
  if(CONFIG_IN2_PIN > 0) pinMode(CONFIG_IN2_PIN,INPUT);
  if(CONFIG_IN3_PIN > 0) pinMode(CONFIG_IN3_PIN,INPUT);
  //if(CONFIG_IN4_PIN > 0) pinMode(CONFIG_IN4_PIN,INPUT);
  

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    //if(CONFIG_DEBUG_MODE == 1) Serial.println("SH");
    // don't continue:
    while (true);
  }
  
  /*
  String fv = WiFi.firmwareVersion();
  if ( fv != "1.1.0" ){
    if(CONFIG_DEBUG_MODE == 1){ 
        //Serial.print("Please upgrade the firmware to 1.1.0 .... ");
        //Serial.println(fv);
    }
  }
  */
}
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
void loop() {
  
  
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    
    digitalWrite(L9, HIGH);
   
    //if(CONFIG_DEBUG_MODE == 1) Serial.print("Attempting to connect to SSID: ");
    //if(CONFIG_DEBUG_MODE == 1) Serial.println(CONFIG_WIFI_SSID);
    //Serial.println("1");
     char  ssid[CONFIG_WIFI_SSID.length()+1];
     CONFIG_WIFI_SSID.toCharArray(ssid,CONFIG_WIFI_SSID.length()+1);
     
     char  pass[CONFIG_WIFI_PASS.length()+1];
     CONFIG_WIFI_PASS.toCharArray(pass,CONFIG_WIFI_PASS.length()+1);
     //Serial.println(ssid);
     //Serial.println(pass);
     //   Serial.println(CONFIG_WIFI_SSID.length());
        //Serial.println("2");
     if(CONFIG_WIFI_MODE == 0){
        
        //Serial.println(CONFIG_WIFI_SSID);
        // Connect to WPA/WPA2 network.
        status = WiFi.begin(ssid, pass);
        //client.connect(CONFIG_CMPROXY_IP, CONFIG_CMPROXY_PORT);
        
     }else{
        
        
        
        // Connect to WEP network.
        status = WiFi.begin(ssid, CONFIG_WIFI_KEYINDEX, pass);
        //client.connect(CONFIG_CMPROXY_IP, CONFIG_CMPROXY_PORT);
     }
     //Serial.println("3");
     // wait for connection:
     delay(1000);
    
     // you're connected now, so print out the status:
     printWifiStatus();
  }
  
  
  if( status == WL_CONNECTED){ 
      
      digitalWrite(L9, LOW);
    
      // ------------------------------------------
      // ------------------------------------------
      // Connected 
      
      if(CONFIG_IN1_PIN > 0) {
      
          int IN1_STATE = digitalRead(CONFIG_IN1_PIN); 
           
          
          if(IN1_STATE==HIGH){
              //Serial.println(IN1_STATE);
           
              if (millis() - VAR_IN1_TIMESTAMP > CONFIG_INFRARED_INTERVAL) {  
              
                 if(VAR_IN1_CNT_HIGH > CONFIG_INFRARED_COUNT_HIGH){
                    //Serial.println("send");
                    sendTcpMessage(1);
                    
                 }else{
                   
                    // have the obstacle
                    VAR_IN1_CNT_HIGH++;
                 }
              }
              
          }else{
              // nothing in front of sensor
              VAR_IN1_CNT_HIGH = 0;
          }
      }
       
      if(CONFIG_IN2_PIN > 0) {
      
          int IN2_STATE = digitalRead(CONFIG_IN2_PIN); 
          //Serial.println("IN2_STATE : " + IN2_STATE);
          if(IN2_STATE==HIGH){
          
              if (millis() - VAR_IN2_TIMESTAMP > CONFIG_INFRARED_INTERVAL) {  
              
                 if(VAR_IN2_CNT_HIGH > CONFIG_INFRARED_COUNT_HIGH){
                       
                    sendTcpMessage(2);
                    
                 }else{
                   
                    // have the obstacle
                    VAR_IN2_CNT_HIGH++;
                 }
              }
              
          }else{
              // nothing in front of sensor
              VAR_IN2_CNT_HIGH = 0;
          }
      }
      if(CONFIG_IN3_PIN > 0) {
      
          int IN1_STATE = digitalRead(CONFIG_IN3_PIN); 
          
          if(IN1_STATE==HIGH){
          
              if (millis() - VAR_IN3_TIMESTAMP > CONFIG_INFRARED_INTERVAL) {  
              
                 if(VAR_IN3_CNT_HIGH > CONFIG_INFRARED_COUNT_HIGH){
                       
                    sendTcpMessage(3);
                    
                 }else{
                   
                    // have the obstacle
                    VAR_IN3_CNT_HIGH++;
                 }
              }
              
          }else{
              // nothing in front of sensor
              VAR_IN3_CNT_HIGH = 0;
          }
      }
      /*
      if(CONFIG_IN4_PIN > 0) {
      
          int IN1_STATE = digitalRead(CONFIG_IN4_PIN); 
          
          if(IN1_STATE==HIGH){
          
              if (millis() - VAR_IN4_TIMESTAMP > CONFIG_INFRARED_INTERVAL) {  
              
                 if(VAR_IN4_CNT_HIGH > CONFIG_INFRARED_COUNT_HIGH){
                       
                    sendTcpMessage(4);
                    
                 }else{
                   
                    // have the obstacle
                    VAR_IN4_CNT_HIGH++;
                 }
              }
              
          }else{
              // nothing in front of sensor
              VAR_IN4_CNT_HIGH = 0;
          }
          
      }  
      */
      // ------------------------------------------
      // ------------------------------------------
       
      // Toggle LED while Wifi connected
      unsigned long statusL9 = millis();
      if(statusL9 % 3000 > 1500) digitalWrite(L9, LOW); else digitalWrite(L9, HIGH);
     
      // ------------------------------------------
      // ------------------------------------------
      
  }else{
      // ------------------------------------------
      // ------------------------------------------
      // wait to try new connection
      // wait 10 seconds for connection:
      delay(10000);
      // ------------------------------------------
      // ------------------------------------------
  }
  
  
}

// ----------------------------------------------------
// this method makes a HTTP connection to the server:
void sendTcpMessage(int device) {

    boolean isConnected = client.connected();

    if(!isConnected){
     
      digitalWrite(L9, HIGH);
      
      //if(CONFIG_DEBUG_MODE == 1) Serial.println("Try to reconnect...");
      
      client.flush();
      client.stop();
      //Serial.println(CONFIG_CMPROXY_IP);
      //  Serial.println(CONFIG_CMPROXY_PORT);
      isConnected = client.connect(CONFIG_CMPROXY_IP, CONFIG_CMPROXY_PORT);
      
      if(isConnected){ 
      
        if(CONFIG_DEBUG_MODE == 1) Serial.println("Connected...");
        digitalWrite(L9, LOW);  
      
      }else{
        
        WiFi.disconnect(); 
        status = WL_IDLE_STATUS;
        if(CONFIG_DEBUG_MODE == 1) Serial.println("Cannot connect to cmproxy2 service... disconnect Wifi and try to connect again");
      }
    }
  
   //Serial.println("connected + " + isConnected);
  
  if (isConnected) {
    
      commandCount++;
      //Serial.println(CONFIG_CMPROXY_COMMAND_PREFIX);
      
      
      client.print(CONFIG_CMPROXY_COMMAND_PREFIX);
      String msg = "";
      
      if(device == 1){
          
          msg = CONFIG_IN1_COMMAND + "#" + CONFIG_IN1_SECOND + "#" + CONFIG_IN1_PRIORITY;
          client.print(msg);
          VAR_IN1_TIMESTAMP = millis();
      
      }else if(device == 2){
          
          msg = CONFIG_IN2_COMMAND + "#" + CONFIG_IN2_SECOND + "#" + CONFIG_IN2_PRIORITY;
          client.print(msg);
          VAR_IN2_TIMESTAMP = millis();
          
      }else if(device == 3){
          
          msg = CONFIG_IN3_COMMAND + "#" + CONFIG_IN3_SECOND + "#" + CONFIG_IN3_PRIORITY;
          client.print(msg);
          VAR_IN3_TIMESTAMP = millis();
          
      //}else if(device == 4){
      //    
      //    msg = CONFIG_IN4_COMMAND + "#" + CONFIG_IN4_SECOND + "#" + CONFIG_IN4_PRIORITY;
      //    client.print(msg);
      //    VAR_IN4_TIMESTAMP = millis();
      }
      // Serial.println(msg);
      
      if(CONFIG_DEBUG_MODE == 1){ 
          //Serial.print("Sent ["+String(commandCount)+"] >> ");
          Serial.println("#["+String(commandCount)+"] " + msg);
      }
      
    
  }else {
      // if you couldn't make a connection:
      //if(CONFIG_DEBUG_MODE == 1)Serial.println("Connection failed");
  }
  
  //digitalWrite(L9, LOW);
}
// ----------------------------------------------------
void printWifiStatus() {
  
  //if(CONFIG_DEBUG_MODE == 0) return;
  
  // print the SSID of the network you're attached to:
  //Serial.print("SSID: ");
  //Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  //IPAddress ip = WiFi.localIP();
  //Serial.print("IP Address: ");
  //Serial.println(ip);

  // print the received signal strength:
  //long rssi = WiFi.RSSI();
  //Serial.print("signal strength (RSSI):");
  //Serial.print(rssi);
  //Serial.println(" dBm");
}

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
void readConfig(){
    
    //if(CONFIG_DEBUG_MODE == 1) Serial.println("Initializing SD card...");
    
    // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
    // Note that even if it's not used as the CS pin, the hardware SS pin 
    // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
    // or the SD library functions will not work. 
    pinMode(10, OUTPUT);
     
    if (!SD.begin(4)) {
        //if(CONFIG_DEBUG_MODE == 1) Serial.println("initialization failed!");
        return;
    }
    File file = SD.open("config.txt");
    
    if (file) {
      
      String  line = "";
    
      // read from the file until there's nothing else in it:
      while (file.available()) {
          
          char c = file.read();
          //Serial.print(c);
          
          if(c == '\r' || c == '\n'){
              
              processConfigLine(line);
              line = "";
             
          }else{
              line += c;
          }
      }
      if(!line.equals("")) processConfigLine(line);
      
      // close the file:
      file.close();
      
      //if(CONFIG_DEBUG_MODE == 1) Serial.println("initialization done.");
      
    } else {
      // if the file didn't open, print an error:
      //if(CONFIG_DEBUG_MODE == 1) Serial.println("error opening file");
    }
    
}
void processConfigLine(String line){
    
   line.trim();
   
   
   if(line.equals("")) return;
   if(line.startsWith("----")) return;
   
    // process line
    int equalIndex = line.indexOf('=');
    if(equalIndex == -1) return;
   
    Serial.println("> " + line);
   
    String key = line.substring(0, equalIndex);
    String value = line.substring(equalIndex, line.length());
    value.replace("=","");
    
    //Serial.print(">");
    //Serial.print(key);
    //Serial.print(" : ");
    //Serial.println(value);
    
    if(!key.equals("") && !value.equals("")){
        
        if(key.equals("debug_mode")){
              
              if(value.equals("1") || value.equals("true"))
                CONFIG_DEBUG_MODE = 1;
              else 
                CONFIG_DEBUG_MODE = 0;
                
        }else if(key.equals("wifi_ssid")){
              
              CONFIG_WIFI_SSID = value;
              
        }else if(key.equals("wifi_password")){
          
              CONFIG_WIFI_PASS = value;
              
        }else if(key.equals("wifi_mode")){
          
              CONFIG_WIFI_MODE = value.toInt();
              
        }else if(key.equals("wifi_key_index")){
          
              CONFIG_WIFI_KEYINDEX = value.toInt();
              
              
              
        }else if(key.equals("in1_pin")){
          
              CONFIG_IN1_PIN = value.toInt();
        
        }else if(key.equals("in1_cmd")){
    
              CONFIG_IN1_COMMAND = value;
              
        }else if(key.equals("in1_second")){
 
              CONFIG_IN1_SECOND = value;
              while(CONFIG_IN1_SECOND.length() < 3) CONFIG_IN1_SECOND = ("0"+CONFIG_IN1_SECOND);
        
        }else if(key.equals("in1_priority")){
 
              CONFIG_IN1_PRIORITY = value;
              while(CONFIG_IN1_PRIORITY.length() < 3) CONFIG_IN1_PRIORITY = ("0"+CONFIG_IN1_PRIORITY);
        
        
        
        
        }else if(key.equals("in2_pin")){
          
              CONFIG_IN2_PIN = value.toInt();
              
        }else if(key.equals("in2_cmd")){
    
              CONFIG_IN2_COMMAND = value;
              
        }else if(key.equals("in2_second")){
 
              CONFIG_IN2_SECOND = value;
              while(CONFIG_IN2_SECOND.length() < 3) CONFIG_IN2_SECOND = ("0"+CONFIG_IN2_SECOND);
        
        }else if(key.equals("in2_priority")){
 
              CONFIG_IN2_PRIORITY = value;
              while(CONFIG_IN2_PRIORITY.length() < 3) CONFIG_IN2_PRIORITY = ("0"+CONFIG_IN2_PRIORITY);
              
        }else if(key.equals("in3_pin")){
          
              CONFIG_IN3_PIN = value.toInt();
        
        }else if(key.equals("in3_cmd")){
    
              CONFIG_IN3_COMMAND = value;
              
        }else if(key.equals("in3_second")){
 
              CONFIG_IN3_SECOND = value;
              while(CONFIG_IN3_SECOND.length() < 3) CONFIG_IN3_SECOND = ("0"+CONFIG_IN3_SECOND);
        
        }else if(key.equals("in3_priority")){
 
              CONFIG_IN3_PRIORITY = value;
              while(CONFIG_IN3_PRIORITY.length() < 3) CONFIG_IN3_PRIORITY = ("0"+CONFIG_IN3_PRIORITY);
        /*      
        }else if(key.equals("in4_pin")){
          
              CONFIG_IN4_PIN = value.toInt();
        
        }else if(key.equals("in4_second")){
 
              CONFIG_IN4_SECOND = value;
              while(CONFIG_IN4_SECOND.length() < 3) CONFIG_IN4_SECOND = ("0"+CONFIG_IN4_SECOND);
        
        }else if(key.equals("in4_priority")){
 
              CONFIG_IN4_PRIORITY = value;
              while(CONFIG_IN4_PRIORITY.length() < 3) CONFIG_IN4_PRIORITY = ("0"+CONFIG_IN4_PRIORITY);
        */     
        }else if(key.equals("cmproxy_ip")){
            
              // ----------------------------------------
              // parse IP Address
              
              String tempIP = value;
              int tempIndex = tempIP.indexOf('.');
              String ip1, ip2, ip3, ip4;
              
              // ip1
              if(tempIndex != -1){
                  
                  ip1 = tempIP.substring(0, tempIndex);
                  tempIP = tempIP.substring(tempIndex + 1);
                  
                  // ip2
                  tempIndex = tempIP.indexOf('.');
                  if(tempIndex != -1){
                    
                      ip2 = tempIP.substring(0, tempIndex);
                      tempIP = tempIP.substring(tempIndex + 1);
                      
                      // ip3
                      tempIndex = tempIP.indexOf('.');
                      if(tempIndex != -1){
                        
                          ip3 = tempIP.substring(0, tempIndex);
                          ip4 = tempIP.substring(tempIndex + 1);
                          
                          byte tempByteArray [4] = { ip1.toInt(), ip2.toInt(), ip3.toInt(), ip4.toInt()};
                          CONFIG_CMPROXY_IP  = tempByteArray;
                      }
                    
                  }
              }
              // ----------------------------------------
        
        }else if(key.equals("cmproxy_port")){
            
              CONFIG_CMPROXY_PORT = value.toInt();                                 
        }
        
    } // end key:value
} // end processConfigLine
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------


