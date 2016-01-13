#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

/* <START> server mode variable */
const char *ssid = "ESPap";
const char *password = "111111";
char *wifi_ssid = "";
char *wifi_password = "";

boolean client_mode_state = false;
/* <END> server mode variable */

/* <START> client mode variable */
int btnPin = 2;
int buttonState = 0;         // variable for reading the pushbutton status

byte mac[6];  // My MAC Addr
const char* host = "128.199.151.72"; // Host IP
/* <END> client mode variable */

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
  String body = String()
  + "<h1>You are connected</h1>"
  + "<form method='get' action='set_wifi'>"
  + "<label for='ssid'>SSID</label><input type='text' name='ssid' id='ssid'></input>"
  + "<label for='password'>PASS</label><input type='text' name='password' id='ssid'></input>"
  + "<input type='submit'>전송</button></form>";
	server.send(200, "text/html", body);
}

void set_wifi() {
  Serial.println("set_wifi 실행");
  
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";

    if(server.argName(i) == "ssid") {
      int len = server.arg(i).length();
      Serial.print(i);
      Serial.print(" SSID get value: ");
      Serial.println(server.arg(i));
      char temp[len];
      server.arg(i).toCharArray(temp, len);
      wifi_ssid = temp;
      
    } else if (server.argName(i) == "password") {
      int len = server.arg(i).length();
      Serial.print(i);
      Serial.print(" PASS get value: ");
      Serial.println(server.arg(i));
      char temp2[len];
      server.arg(i).toCharArray(temp2, len);
      wifi_password = temp2;
    }
      Serial.println("Setting: ");
      Serial.print(wifi_ssid);
      Serial.print(" AND ");
      Serial.println(wifi_password);

  }
  Serial.print("SSID: ");
  Serial.println(wifi_ssid);
  Serial.print("PASS: ");
  Serial.println(wifi_password);
  server.send(200, "text/plain", message);

  // 버튼 셋업
  pinMode(btnPin, INPUT);

  // WiFi 네트워크가 연결되며 실행
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  // WiFi 모드를 STA로 변경
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  WiFi.macAddress(mac);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.print("MAC: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
  Serial.println(WiFi.localIP());

  client_mode_state = true;
  
  // WiFiClient 클래스를 이용하여 TCP connection 생성
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return; 
  }
  
}

void setup() {
	delay(1000);
	Serial.begin(115200);
	Serial.println();
	Serial.print("Configuring access point...");
	/* You can remove the password parameter if you want the AP to be open. */
  WiFi.mode(WIFI_AP);
	WiFi.softAP(ssid, password);

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
	server.on("/", handleRoot);
  server.on("/set_wifi", set_wifi);
	server.begin();
	Serial.println("HTTP server started");
}

void loop() {
  if (client_mode_state) {
    
  Serial.println("*** Listening Button Click ***");
  delay(500);

  buttonState = digitalRead(btnPin);

  /** <START>
   *  버튼이 눌렸을때만 작동
   */
  if (buttonState == HIGH) {
    Serial.println("PUSH !!");
    Serial.print("connecting to ");
    Serial.println(host);
  
    // WiFiClient 클래스를 이용하여 TCP connection 생성
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return; 
    }
  
    // 연결한 호스트 URI 주소
    String url = "/clicky/ardu/click/";
  
    Serial.print("Requesting URL: ");
    Serial.println(url);
  
    // Send Data 설정
    String mac_addr = String() + mac[5]+":"+mac[4]+":"+mac[3]+":"+mac[2]+":"+mac[1]+":"+mac[0];
    String post_data = String("mac_addr=") + mac_addr;
    String req_header = String("POST ") + url + " HTTP/1.1\r\n" +
                        "Host: " + host + "\r\n" +
                        "Connection: close\r\n" +
                        "Content-Length:" + String(post_data.length()) + "\r\n" +
                        "Content-Type: application/x-www-form-urlencoded\r\n\r\n" +
                        post_data + "\r\n";
    Serial.print(req_header);
    client.print(req_header);
    delay(10);
  
    // 서버 응답 출력
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  
    Serial.println();
    Serial.println("closing connection");
  }
  /** <END> 버튼이 눌렸을때만 작동 */
  } else {
    server.handleClient();
  }
}
