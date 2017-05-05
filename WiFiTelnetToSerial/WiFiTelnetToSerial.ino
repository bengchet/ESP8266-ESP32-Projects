/* 
  WiFiTelnetToSerial - Example Transparent UART to Telnet Server for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WiFi library for Arduino environment.
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <ESP8266WiFi.h>

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 1
const char* ssid = "Cytron-R&D";
const char* password = "f5f4f3f2f1";

WiFiServer server(23);
WiFiServer disp_server(8080);
WiFiClient serverClients[MAX_SRV_CLIENTS];
String data = "";

void setup() {
  Serial1.begin(115200);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to "); Serial.println(ssid);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if(i == 21){
    Serial.print("Could not connect to"); Serial.println(ssid);
    while(1) delay(500);
  }
  //start UART and the server
  
  server.begin();
  disp_server.begin();
  server.setNoDelay(true);
  disp_server.setNoDelay(true);
  
  Serial.print("Ready! Use 'telnet ");
  Serial.print(WiFi.localIP());
  Serial.println(" 23' to connect");
}

void loop() {
  uint8_t i;
  //check if there are any new clients
  if (server.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        //clear gibberish words upon connected
        serverClients[i].readString();
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++){
    if (serverClients[i] && serverClients[i].connected()){
      if(serverClients[i].available()){
        //get data from the telnet client and push it to the UART
        while(serverClients[i].available()){
          if(serverClients[i].peek() < 128)
            data += (char)serverClients[i].read();
          else
            serverClients[i].read(); //discard
          delay(5);
        }
        
        data += "<br>";
        Serial.println(data);
      }
    }
  }

  if(disp_server.hasClient()){
    WiFiClient client = disp_server.available();
    if (client) {  //display web page if there is client connected to port 8080

      // Wait for data from client to become available
      while(client.connected() && !client.available()){
        delay(1);
      }
      
      String req = client.readStringUntil('\r');

      // First line of HTTP request looks like "GET /path HTTP/1.1"
      // Retrieve the "/path" part by finding the spaces
      int addr_start = req.indexOf(' ');
      int addr_end = req.indexOf(' ', addr_start + 1);
      if (addr_start != -1 && addr_end != -1) {
        req = req.substring(addr_start + 1, addr_end);
        Serial.print("Request: ");
        Serial.println(req);
        client.flush();

        String s;
        if (req == "/")
        {
          IPAddress ip = WiFi.localIP();
          String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
          s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
          s += ipStr;
          s += "<br><br>Data Received from telnet:<br>";
          s += data;
          s += "</html>\r\n\r\n";
          Serial.println("Sending 200");
        }
        else
        {
          s = "HTTP/1.1 404 Not Found\r\n\r\n";
          Serial.println("Sending 404");
        }
        client.print(s);
      }   
    }
  }
  
  //check UART for data
  if(Serial.available()){
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //push UART data to all connected telnet clients
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        serverClients[i].write(sbuf, len);
        delay(1);
      }
    }
  }
}
