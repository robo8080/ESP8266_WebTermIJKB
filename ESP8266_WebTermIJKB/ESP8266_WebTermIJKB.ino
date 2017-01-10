#include <Arduino.h>
#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <FS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <QueueArray.h>

#define LED1    12
#define LED2    4
#define LED3    5

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 1
const char* ssid = "******************";
const char* password = "******************";
bool WS_connected = false;
int WS_num = 0;
// create a queue of characters.
QueueArray <char> queue;

ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server = ESP8266WebServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);
void serverSetup();
void serial2ws(int select);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght);

void IJCodeSend(uint8_t num, uint8_t code);
int IJCodeSet(uint8_t *src, int len, uint8_t *dest);
void IJCodeEcho(uint8_t num, uint8_t code);
//char escCLS[4] = {0x1b, '[', '2','J'}; //CLS
char escCLS[10] = {0x1b, '[', '2','J', 0x1b, '[', '1', ';', '1', 'H'}; //CLS
char escLOCATE[20];
char escSCROLL_S[3] = {0x1b, '[', 'S'};  // コンソールを次にスクロールする
char escSCROLL_T[3] = {0x1b, '[', 'T'};  // コンソールを前にスクロールする
int escState = 0;
uint8_t escBuff[3];
int escBuffLen = 0;
uint8_t termCodeBuff[512];
int termCodeLen = 0;
uint8_t code0[2] = { 0, 0};
uint8_t code1[2] = { 0, 1};
uint8_t codeBS[6] = { 0x08, 0x00, 0x20, 0x00, 0x08, 0x00};

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  digitalWrite(LED1, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(LED2, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(LED3, LOW);    // turn the LED off by making the voltage LOW
  // put your setup code here, to run once:
  //Serial.begin(921600);
  Serial.begin(115200);
  serverSetup();
}

void loop() {
  // put your main code here, to run repeatedly:
  ArduinoOTA.handle();
  webSocket.loop();
  server.handleClient();
  serial2ws(1);
  while (!queue.isEmpty ()) {
    MJ_Command(queue.dequeue ());
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  else if(filename.endsWith(".ttf")) return "application/x-font-ttf";
  return "text/plain";
}

bool handleFileRead(String path){
//  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void serverSetup() {
//  Serial.setDebugOutput(true);
  Serial.setDebugOutput(false);
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
//      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
//    Serial.printf("\n");
  }

  Serial.println();
  Serial.println();
  Serial.println();

  for(uint8_t t = 4; t > 0; t--) {
      Serial.printf("'[SETUP] BOOT WAIT %d...\n", t);
      Serial.flush();
      delay(1000);
  }

  WiFiMulti.addAP(ssid, password);

  Serial.print("\n'");
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("'Connected to ");
  Serial.println(ssid);
  Serial.print("'IP address: ");
  Serial.println(WiFi.localIP());
/////////////////////////////////////////////////
  Serial.write("\r\n\r\n");
  delay(500);
  Serial.write("bps 19200\r\n");
  delay(1000);
//  Serial.end();
//  delay(500);
  Serial.begin(19200);
/////////////////////////////////////////////////  
  digitalWrite(LED1, HIGH);   // turn the LED on (HIGH is the voltage level)

  ArduinoOTA.begin();
  Serial.println("'OTA server started");
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("'WebSocket server started");

  if(MDNS.begin("esp8266")) {
      Serial.println("'MDNS responder started");
  }

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  server.begin();
  Serial.println("'HTTP server started\n");

  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);

}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            WS_connected = false;
//            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            WS_connected = true;
            WS_num = num;
            IPAddress ip = webSocket.remoteIP(num);
//            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
//            webSocket.sendTXT(num, "Connected");
        }
            break;
        case WStype_TEXT:
//            Serial.printf("[%u] get Text: %s\n", num, payload);
//            webSocket.sendTXT(num, payload, lenght);
//            Serial.write((char*)payload, lenght);
          // enqueue all the message's characters to the queue.
          digitalWrite(LED3, HIGH);   // turn the LED on (HIGH is the voltage level)
          for (int i = 0; i < lenght; i++) {
            //queue.enqueue ((char)payload[i]);
            if(payload[i] == 0x7f){
              webSocket.sendTXT(num, "\b \b");
              Serial.write(0x08);
            } else {
              webSocket.sendTXT(num, payload+i, 1);
              Serial.write((char)payload[i]);
            }
            if(payload[i] == 0x0d) delay(250);
            else delay(20);
          }
          digitalWrite(LED3, LOW);    // turn the LED off by making the voltage LOW
            break;
        case WStype_BIN:
//            Serial.printf("[%u] get binary lenght: %u\n", num, lenght);
//            hexdump(payload, lenght);

            // send message to client
//             webSocket.sendBIN(num, payload, lenght);
          digitalWrite(LED3, HIGH);   // turn the LED on (HIGH is the voltage level)
          for (int i = 0; i < lenght; i++) {
            //queue.enqueue ((char)payload[i]);
            if(payload[i] == 0x08){
              webSocket.sendBIN(num, codeBS, 6);
              Serial.write(0x08);
            } else {
              IJCodeEcho(num, payload[i]);
              Serial.write((char)payload[i]);
            }
            if(payload[i] == 0x0d) delay(250);
            else delay(20);
            serial2ws(0);
          }
          digitalWrite(LED3, LOW);    // turn the LED off by making the voltage LOW
            break;
    }
}

void IJCodeConv(uint8_t code) {
  switch(escState) {
    case 0:
      if(code == 0x13){ //CLS?
        escBuff[escBuffLen++] = code;
        escState = 1;
      } else if (code == 0x15) {  // LOCATE,SCROLL?
        escBuff[escBuffLen++] = code;
        escState = 2;
      } else {
       termCodeLen += IJCodeSet(&code, 1, &termCodeBuff[termCodeLen]);        
      }
      break;
    case 1: //CLS
      escBuff[escBuffLen++] = code;
      if(code == 0x0c){ //CLS
        termCodeLen += IJCodeSet((uint8_t*)&escCLS, 10, &termCodeBuff[termCodeLen]);        
        escBuffLen = 0;   
        escState = 0;
      } else {
        termCodeLen += IJCodeSet(escBuff, 2, &termCodeBuff[termCodeLen]);
        escBuffLen = 0;   
        escState = 0; 
      }
      break;
    case 2: // LOCATE,SCROLL
      escBuff[escBuffLen++] = code;
      if((code >= 0x20)&&(code <= 0x3f)){ //X
        escState = 3;
      } else if(code == 0x1e){ // SCROLL UP
        termCodeLen += IJCodeSet((uint8_t*)&escSCROLL_S, 3, &termCodeBuff[termCodeLen]);        
        escBuffLen = 0;   
        escState = 0;
      } else if(code == 0x1f){ // SCROLL DOWN
        termCodeLen += IJCodeSet((uint8_t*)&escSCROLL_T, 3, &termCodeBuff[termCodeLen]);        
        escBuffLen = 0;   
        escState = 0;
      } else {
        termCodeLen += IJCodeSet(escBuff, 2, &termCodeBuff[termCodeLen]);
        escBuffLen = 0;   
        escState = 0; 
      }
      break;
    case 3:
      escBuff[escBuffLen++] = code;
      if((code >= 0x20)&&(code <= 0x37)){ //Y
        sprintf(escLOCATE,"\033[%d;%dH",escBuff[2] - 0x20 + 1,escBuff[1] - 0x20 + 1);
        termCodeLen += IJCodeSet((uint8_t*)&escLOCATE, strlen(escLOCATE), &termCodeBuff[termCodeLen]);        
        escBuffLen = 0;   
        escState = 0;
      } else {
        termCodeLen += IJCodeSet(escBuff, 3, &termCodeBuff[termCodeLen]);
        escBuffLen = 0;   
        escState = 0; 
      }
      break;
  }
}

uint8_t dbuf[256];
void serial2ws(int select) {
  //check UART for data
//  if(Serial.available()){
  while(Serial.available()){
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    if (WS_connected){
      digitalWrite(LED2, HIGH);   // turn the LED on (HIGH is the voltage level)
      for (int i= 0 ; i< len ;i++) {
          IJCodeConv(sbuf[i]);
       }
       webSocket.sendBIN(WS_num, termCodeBuff, termCodeLen);
       termCodeLen = 0;
      if(select != 0) { 
        for (int i= 0 ; i< len ;i++) {
            MJ_Command((char)sbuf[i]);
         }
      } else {
        for (int i= 0 ; i< len ;i++) {
            queue.enqueue ((char)sbuf[i]);
         }        
      }
     
      digitalWrite(LED2, LOW);    // turn the LED off by making the voltage LOW
    }
  }
}

void IJCodeEcho(uint8_t num, uint8_t code) {
  if(code == 0x08){
    webSocket.sendBIN(num, codeBS, 6);
  } else if((code == 0x0a) || (code == 0x1b)){
    code0[0] = code;
    webSocket.sendBIN(num, code0, 2);
  } else if ((code >= 0x20) && (code <= 0x7e)){
    code0[0] = code;
    webSocket.sendBIN(num, code0, 2);    
  } else if ((code >= 0x1c) && (code <= 0x1f)){
    ;
  } else {
    code1[0] = code;
    webSocket.sendBIN(num, code1, 2);      
  }
}

void IJCodeSend(uint8_t num, uint8_t code) {
  if((code == 0x0a) || (code == 0x1b)){
    code0[0] = code;
    webSocket.sendBIN(num, code0, 2);
  } else 
  if ((code >= 0x20) && (code <= 0x7e)){
    code0[0] = code;
    webSocket.sendBIN(num, code0, 2);    
  } else {
    code1[0] = code;
    webSocket.sendBIN(num, code1, 2);      
  }
}

int IJCodeSet(uint8_t *src, int len, uint8_t *dest) {
  int j = 0;
  for(int i = 0; i < len; i++){
    if((src[i] == 0x0a) || (src[i] == 0x1b)){
      dest[j++] = src[i];
      dest[j++] = 0x00;     
    } else 
    if ((src[i] >= 0x20) && (src[i] <= 0x7e)){
      dest[j++] = src[i];
      dest[j++] = 0x00;     
    } else {
      dest[j++] = src[i];
      dest[j++] = 0x01;     
    }
  }
  return j;
}

void IJCodeSendString(uint8_t num, String st) {
  int len = st.length();
  uint8_t buf[len];
  uint8_t sbuf[len*2];
  st.toCharArray((char *)buf, len);
  IJCodeSet(buf, len, sbuf);
  webSocket.sendBIN(num, sbuf, len*2);
}

//------------------------------------------------------------------------------
/*
 *  MicJack by Michio Ono.
 *  IoT interface module for IchigoJam with ESP-WROOM-02
 *  CC BY
 */
// modified by robo8080
 
const String MicJackVer="MicJack-0.8.0";
const String MJVer="MixJuice-1.2.0";
String inStr = ""; // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

int httpPort = 80;

int posttype;
boolean postmode=false;
enum { HTML_GET, HTML_GETS, 
       HTML_POST, HTML_POSTS };
String postaddr="";
String postdata="";
String myPostContentType="";

#define kspw  20
#define kspn  30
int spw=kspw;
int spn=kspn;

/*** Use access Status LED ***/
//#define useMJLED
#ifdef useMJLED
  const int connLED=12; //Green
  const int postLED=4;  //Yellow
  const int getLED=5;   //Red
#endif

String homepage="mj.micutil.com";
String lastGET=homepage;

/***************************************
 *  Reset POST parameters
 *  POST用パラメータをリセット
 *  
***************************************/
void ResetPostParam() {
  postmode=false;
  postaddr="";
  postdata="";
}

void SetPCT(String s) {
  int n=s.length();
  if(n<=0) {
    myPostContentType="";
  } else {
    //Content-Type: application/x-www-form-urlencoded;\r\n
    myPostContentType="Content-Type: "+s+"\r\n";
  }
}

void MJ_Command(char inChar) {

//  while (Serial.available()) {
//    // get the new byte:
//    char inChar = (char)Serial.read();

    //Check end of line 
    if(inChar=='\n' || inChar=='\r') {
      stringComplete = true;
      if(!postmode) Serial.flush(); //読み飛ばし "OK"など
//      break;
    } else {
      inStr += inChar;
    }
    
//  }

  if(stringComplete) {  //データあり
    stringComplete=false;
    String cs=inStr;//文字をコピー
    cs.toUpperCase();//大文字に

    if(postmode) {
      /**** POSTモード 処理 ****/
      if(cs.startsWith("MJ POST END")) {
        /* POST通信開始 */
        MJ_HTML(posttype,postaddr);
        ResetPostParam();
      } else if(cs.startsWith("MJ POSTS END")) {
        /* POST通信開始 */
        MJ_HTMLS(posttype,postaddr);
        ResetPostParam();
      } else if(cs.startsWith("MJ POST CANCEL") || cs.startsWith("MJ POSTS CANCEL") ||
                cs.startsWith("MJ POST STOP") || cs.startsWith("MJ POSTS STOP") ||
                cs.startsWith("MJ POST ESC") || cs.startsWith("MJ POSTS ESC")) {
        /*POST/POSTSデータリセット*/
        ResetPostParam();
        
      } else {
        /* POSTコンテンツを記録 */
        if(inStr.endsWith("\n")) inStr=inStr.substring(0,inStr.length()-1);//最後の"\n"を外す
        postdata+=inStr;
        postdata+="\n";

      }
      
   } else if(cs.startsWith("MJ GET ")) {
      /*** GET通信 ****/
      MJ_HTML(HTML_GET,inStr.substring(7));
     
    } else if(cs.startsWith("MJ GETS ")) {
      /**** HTMLS GET通信 ****/
      MJ_HTMLS(HTML_GETS,inStr.substring(8));
     
 //   } else if(cs.startsWith("MJ FUJI")) {
 //     /*** FUJI ***/ MJ_HTMLS(HTML_GETS,"raw.githubusercontent.com/bokunimowakaru/petit15term/master/fuji.txt");
    
    } else if(cs.startsWith("MJ POST START ")) {
      /*** POST START通信 ***/
      MJ_POST_START(HTML_POST,inStr.substring(14));
      
    } else if(cs.startsWith("MJ POSTS START ")) {
      /*** HTMLS POST通信 ***/
      MJ_POST_START(HTML_POSTS,inStr.substring(15));
      
    } else if(cs.startsWith("MJ PCT ")) {
      /*** PCT ***/ SetPCT(inStr.substring(7));
    
    } else if(cs.startsWith("MJ MJVER")) {
      /*** バージョン ***/ Serial.println("'"+MicJackVer);
      IJCodeSendString(WS_num, "'"+MicJackVer);
      IJCodeSend(WS_num, 0x0a);
      // Verで、エラー表示で、MixJuiceのバージョンを表示
      // MJVerで、MicJackのバージョンを表示
      
    } else if(cs.startsWith("MJ ")) {
      /*** NG ***/ Serial.println("'NG: "+MJVer);
      IJCodeSendString(WS_num, "'NG: "+MJVer);
      IJCodeSend(WS_num, 0x0a);
    }

    inStr="";
  }

  
}


/***************************************
 *  MJ GET / POST
 *  HTTP GET/POST 通信
 *  
***************************************/

void MJ_HTML(int type, String addr) {
  int sc=addr.length();
  if(sc<=0) return;

  String host="";
  String url="/";
  String prm="";
  
  int ps;
  ps=addr.indexOf("/");
  if(ps<0) {
    host=addr;
  } else {
    host=addr.substring(0,ps);  // /より前
    url=addr.substring(ps);     // /を含んで後ろ
  }
  //Serial.println("'"+host);
  //Serial.println("'"+url);

  //------プロキシ-----
  String httpServer=host;
//  if(useProxy) httpServer=httpProxy;

  //------ポート-----
  int port=httpPort;
  ps=host.indexOf(":");
  if(ps>0) {
    host=host.substring(0,ps);  // :より前
    port=host.substring(ps+1).toInt();     // :より後ろ
  }
  
  //------せつぞく-----
  WiFiClient client;
  if(!client.connect(httpServer.c_str(), port)) {
    return;
  }
    
  //------リクエスト-----
  switch(type) {
    case HTML_GET:
    case HTML_GETS:
      #ifdef useMJLED
        digitalWrite(getLED, HIGH);
      #endif
      client.print(String("GET ") + url + " HTTP/1.0\r\n" + 
                   "Host: " + host + "\r\n" + 
                   "User-Agent: " + MJVer + "\r\n" + 
                   //"Connection: close\r\n" +
                   "\r\n");
                   //"Accept: */*\r\n" + 
      break;

    case HTML_POST:
    case HTML_POSTS:
      #ifdef useMJLED
        digitalWrite(postLED, HIGH);
      #endif
      prm=postdata;//postdata.replace("\n","\r\n");//改行を置き換え

      client.print(String("POST ") + url + " HTTP/1.0\r\n" + 
                  "Accept: */*\r\n" + 
                  "Host: " + host + "\r\n" + 
                  "User-Agent: " + MJVer + "\r\n" + 
                  //"Connection: close\r\n" +
                  myPostContentType +
                  "Content-Length: " + String(prm.length()) + "\r\n" +
                  "\r\n" + 
                  prm);
                  //"Content-Type: application/x-www-form-urlencoded;\r\n" +
     
      break;

    default:
      return;
  }

  //------待ち-----
  int64_t timeout = millis() + 5000;
  while (client.available() == 0) {
    if (timeout - millis() < 0) {
      Serial.println("'Client Timeout !");
      client.stop();
      return;
    }
  }

  //------レスポンス-----
  switch(type) {
    case HTML_GET:
    case HTML_GETS:
      /* ヘッダー 読み飛ばし */
      while(client.available()){
        String line = client.readStringUntil('\n');
        if(line.length()<2) break;
      }
       
      /*
       * プログラム入力時は、改行後、50msくらい,
       * 文字は、20msはあけないと、
       * IchigoJamは処理できない
      */
      if(spw<=1) {
        while(client.available()){
          String line = client.readStringUntil('\n');
          Serial.print(line);
          Serial.print('\n');
          delay(spn);
        }
      } else {
        while(client.available()){
          char c = client.read();
          switch(c){
          case 0:
            break;
          case '\r':
            break;
          case '\n':
            Serial.print('\n');
            IJCodeSend(WS_num, (uint8_t)'\n');
            delay(spn);
            break;
          default:
            Serial.print(c);
            IJCodeSend(WS_num, (uint8_t)c);
            delay(spw);
            break;
          }
          serial2ws(0);
        }
      }
      if(!addr.equals(homepage)) lastGET=addr;
      #ifdef useMJLED
        digitalWrite(getLED, LOW);
      #endif
      break;

    case HTML_POST:
    case HTML_POSTS:
      Serial.println("'POST OK!");
      IJCodeSendString(WS_num, "'POST OK!");
      IJCodeSend(WS_num, 0x0a);
      #ifdef useMJLED
        digitalWrite(postLED, LOW);
      #endif
      break;
      
    default:
      /* 読み飛ばし */
      while(client.available()){
        String line = client.readStringUntil('\r');
      }
      break;
  }

}

/***************************************
 *  MJ GETS / POSTS
 *  HTTP GETS/POSTS 通信
 *  
***************************************/

void MJ_HTMLS(int type, String addr) {
  int sc=addr.length();
  if(sc<=0) return;

  String host="";
  String url="/";
  String prm="";
  
  int ps;
  ps=addr.indexOf("/");
  if(ps<0) {
    host=addr;
  } else {
    host=addr.substring(0,ps);  // /より前
    url=addr.substring(ps);     // /を含んで後ろ
  }
  //Serial.println("'"+host);
  //Serial.println("'"+url);

  //------プロキシ-----
  String httpServer=host;
//  if(useProxy) httpServer=httpProxy;

  //------ポート-----
  int port=httpPort;
  ps=host.indexOf(":");
  if(ps>0) {
    host=host.substring(0,ps);  // :より前
    port=host.substring(ps+1).toInt();     // :より後ろ
  }
  
  switch(type) {
    case HTML_GET:
    case HTML_POST:
      break;
    case HTML_POSTS:
    case HTML_GETS:
      if(port==80) port=443;
      break;
  }
  
  //------セキュアーなせつぞく-----
  WiFiClientSecure client;
  if(!client.connect(httpServer.c_str(), port)) {
    return;
  }

  //------リクエスト-----
  switch(type) {
    case HTML_GET:
    case HTML_GETS:
      #ifdef useMJLED
        digitalWrite(getLED, HIGH);
      #endif
      client.print(String("GET ") + url + " HTTP/1.0\r\n" + 
                   "Host: " + host + "\r\n" + 
                   "User-Agent: " + MJVer + "\r\n" + 
                   //"Connection: close\r\n" + 
                   "\r\n");
                   //"Accept: */*\r\n" + 
      break;

    case HTML_POST:
    case HTML_POSTS:
      #ifdef useMJLED
        digitalWrite(postLED, HIGH);
      #endif
      prm=postdata;//postdata.replace("\n","\r\n");//改行を置き換え

      client.print(String("POST ") + url + " HTTP/1.0\r\n" + 
                  "Accept: */*\r\n" + 
                  "Host: " + host + "\r\n" + 
                  "User-Agent: " + MJVer + "\r\n" + 
                  //"Connection: close\r\n" +
                  myPostContentType +
                  "Content-Length: " + String(prm.length()) + "\r\n" +
                  "\r\n" + 
                  prm);
                  //"Content-Type: application/x-www-form-urlencoded;\r\n" +
     
      break;

    default:
      return;
  }

  //------待ち-----
  int64_t timeout = millis() + 5000;
  while (client.available() == 0) {
    if (timeout - millis() < 0) {
      Serial.println("'Client Timeout !");
      client.stop();
      return;
    }
  }

  //------レスポンス-----
  switch(type) {
    case HTML_GET:
    case HTML_GETS:
      /* ヘッダー 読み飛ばし */
      while(client.available()){
        String line = client.readStringUntil('\n');
        if(line.length()<2) break;
      }
       
      /*
       * プログラム入力時は、改行後、50msくらい,
       * 文字は、20msはあけないと、
       * IchigoJamは処理できない
      */
      if(spw<=1) {
        while(client.available()){
          String line = client.readStringUntil('\n');
          Serial.print(line);
          Serial.print('\n');
          delay(spn);
        }
      } else {
        while(client.available()){
          char c = client.read();
          switch(c){
          case 0:
            break;
          case '\r':
            break;
          case '\n':
            Serial.print('\n');
            IJCodeSend(WS_num, (uint8_t)'\n');
            delay(spn);
            break;
          default:
            Serial.print(c);
            IJCodeSend(WS_num, (uint8_t)c);
            delay(spw);
            break;
          }
          serial2ws(0);
        }
      }
      if(!addr.equals(homepage)) lastGET=addr;
      #ifdef useMJLED
        digitalWrite(getLED, LOW);
      #endif
      break;

    case HTML_POST:
    case HTML_POSTS:
      Serial.println("'POST OK!");
      IJCodeSendString(WS_num,"'POST OK!");
      IJCodeSend(WS_num, 0x0a);
      #ifdef useMJLED
        digitalWrite(postLED, LOW);
      #endif
      break;
      
    default:
      /* 読み飛ばし */
      while(client.available()){
        String line = client.readStringUntil('\r');
      }
      break;
  }

}

/***************************************
 *  MJ POST START addr
 *  POST/POSTS 開始
 *  
***************************************/

void MJ_POST_START(int type, String addr) {
  int n=addr.length();
  if(n>0) {
    posttype=type;
    postaddr=addr;
    postdata="";
    postmode=true;
  }
}


