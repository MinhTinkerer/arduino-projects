/*
	Script for DDNS for Xboard
	Todo:
		turn it into a library
		clean up and comment
	
	Licence CC = CC BY-SA
*/


#include <Ethernet.h>
#include <EEPROM.h>
#include <SPI.h>


/************ ETHERNET STUFF ************/
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 
  192, 168, 3, 222 };
byte gateway[] = { 
  192, 168, 1, 10 };   //your router's IP address
byte subnet[] = { 
  255, 255, 255, 0 };    //subnet mask of the network 
EthernetServer server(80);
EthernetClient client;

/************ DDNS stuff ************/
IPAddress ipcheck(208,85,241,107); // dyndns ip checker
IPAddress ddns(204,16,170,42); // http://nic.ChangeIP.com/nic/update
char hostname[ ]   = "username";
char userpwdb64[ ] = "x";  //http://www.functions-online.com/base64_encode.html    use->   username:password
IPAddress ipadsl;
IPAddress ipadslold;
IPAddress newip;   //= String(51);
int counter = 0;
int ipi = 0;
int codetemp=0;

void setup() {

  /************* web server and sd conf **************/

  pinMode(10, OUTPUT);                       // set the SS pin as an output (necessary!)
  digitalWrite(10, HIGH);                    // but turn off the W5100 chip!


  //root.ls(LS_DATE | LS_SIZE);
  Serial.println();

  // Debugging complete, we start the server!
  Ethernet.begin(mac, ip); 
  server.begin();
  //Serial.println("end");
  getip(); // debug
}


/*
  These are the main functions for the Dynamic DNS client for arduino/smarthome
 
 */


void loop() {
  
}

///////////////////////////////////////////////Get current IP//////////////////////////////////////
void getip()
{
  int timeout=0;
  int skip=0;

  Serial.print("getting current IP \n");
  Serial.print("connecting to ");
  Serial.print(ipcheck);

  if (client.connect(ipcheck, 8245)) 
  {
    Serial.println("\n       client connected ");
    client.println("GET / HTTP/1.0");
    client.println();
  } 
  else {
    Serial.println("\nconnection failed");
  }

  while (!client.available() && timeout<50)
  {
    timeout++;
    Serial.print("Time out ");
    Serial.println(timeout);
    delay(100);
  }

  while (client.available())
  {
    if (client.available())
    { 
      char c = client.read();
      if (c == '\n')
      {
        counter++;
      } 
      if (counter == 9 && c != '\n')
      {
        if(c == '.' )
        {
          ipadsl[ipi] = codetemp;
          codetemp = 0;
          ipi++;
        } 
        else {
          codetemp = codetemp *10 + (c-0x30);
        }
        ipadsl[3] = codetemp;
      }
    }
  }
  client.flush();

  if (!client.connected())
  {
    Serial.println("disconnecting. \n");
    client.stop();
    delay (1000);
    if (ipadsl != '0,0,0,0')
    {
      Serial.print("New IP: >");
      Serial.print(ipadsl);
      Serial.println("<");             
      for (int i=0; i<4; i++) {    //save in memory
        ipadslold[i] = EEPROM.read(i);
      }
      Serial.print("Old IP: >");
      Serial.print(ipadslold);
      Serial.println("<");
      if (ipadsl != ipadslold)
      {
        Serial.println("IP different from PUBLIC IP");
        ddns_changer();
      }
      else
      {
        Serial.println("same IP");
      }

      ipadslold = ipadsl;
      for (int i=0; i<4; i++)
      {
        EEPROM.write(i, ipadsl[i]);                // http://www.arduino.cc/en/Reference/EEPROMWrite
      }
      Serial.println("IP saved !");
    }
  }
}
///////////////////////////////////////////////Change current IP//////////////////////////////////////

void ddns_changer()
{
  int timeout=0;
  Serial.print("connecting to");
  Serial.println(ddns);

  Serial.print("Public IP");
  Serial.println(ipadsl);

  if (client.connect(ddns, 80)) {
    Serial.println("connected");
    client.print("GET /nic/update?hostname=");
    client.print(hostname);
    client.print("&myip=");
    client.print(ipadsl);
    client.println(" HTTP/1.1 ");
    client.print("Host: ");
    client.println(ddns);

    client.print("Authorization: Basic ");
    client.println(userpwdb64);

    client.println("User-Agent: DFRobot - jose at dfrobot.com");
    client.println();
  } 
  else {
    Serial.println("connection failed");
  }

  while (!client.available() && timeout<50)
  {
    timeout++;
    Serial.print("Time out");
    Serial.println(timeout);
    delay(100);
  }

  while (client.available())
  {
    if (client.available())
    {
      char c = client.read();
      Serial.print(c);
    }
  }

  if (!client.connected())
  {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
}
