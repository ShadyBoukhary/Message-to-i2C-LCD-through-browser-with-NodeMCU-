// This program for the NodeMCU displays a message to an LCD wirelessly using any web browser
// When the message is received, a buzzer will beep and a red LED will blink until a button is 
// pressed to indicated that the message has been read
// This works on any device that has a web browser

#include <ESP8266WiFi.h>                                        // wifi library for esp8266 
#include <LiquidCrystal_I2C.h>                                  //LCD library using I2C (serial)

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* ssid = "Boukhary";                                  // Wifi SSID
const char* password = "QWEasd123";                             // Wifi Password
char * parameter;                                               // pointer to a char to be used in array notation to hold message
byte button = 13;                                               // D7 Pin on NodeMCU connected to the button mapped to PIN 13 on Adruino IDE
byte buzzer = 14;                                               // D5 Pin on NodeMCU connected to buzzer mapped to PIN 14
byte led = 15;                                                  // D8 Pin on NodeMCU connected to LED mapped to PIN 15


WiFiServer server(80);                                          // Start wifi server


void setup()
{
  // setup led, button, and buzzer
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(button, INPUT);
  Serial.begin(115200);                     // begin serial communication baud 115200

  delay(100);
  // connect to wifi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)     // wait until wifi is connected
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  
 // lcd.begin();                                                // initializing the LCD
  lcd.backlight();                                            // Enable or Turn On the backlight 
  lcd.setCursor(0, 0);                                        // set cursor to x = 0 y = 0


  lcd.print("REQUESTING FROM");                               // print to LCD
  lcd.setCursor(0,1);                                         // Set cursor on LCD to x = 0 y = 1
  lcd.print(WiFi.localIP());                                  // Print IP Address to be connected to
}



void loop()

{
  int state = 0;                                             // Holds the state of the button (0 for not being pressed)

 // Check if a client has connected
  WiFiClient client = server.available();                    // Wait for a client to be connected to the Web Server
  if (!client) {
    return;                                                  // return to loop if no connection was found
  }
 
  // Wait until the client sends some data
  Serial.println("new client");                              // Print when someone connects
  while(!client.available()){                                // Await message
    delay(1);
  }
   char* req = new char[100];                                // pointer to a char to be used as an array to hold request
                                                             // char is used instead of string to facilitate splitting using strtok()

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);                                   // Print request to the Serial Monitor for debugging purposes
  client.flush();
  if (request.indexOf("/favicon.ico") != -1)                 // This is to prevent an error when a message is sent from an android device 
    goto BREAK;                                              // where the browser requests an icon for the webpage, messing up our message
                                                             // since it will be deleted because we will extract it from the request itself
                                          
  for (int x = 0; x < request.length(); x++)                 // copy the string holding the request to an array of characters to facilitate splitting (previously mentioned)
  {
    req[x] = request[x];
  }

  parameter = strtok(req, "=");                             // split the array req (pointer in array notation) at "=" character and store in array of char parameter
  parameter = strtok(NULL, " ");                            // now we have the message and some addons at the end. The addons and the message are separated by a space
                                                            // So, we will split the array of characters at the space and store only the message in parameter
                                                            
  if (parameter == NULL)                                    // When the user sends an empty message, the resulting parameter at this point would be NULL
  {                                                         // We will set it to "none" for debugging purposes. The reason we can't do this earlier is
    parameter = "none";                                     // the request itself would not be NULL but it would be something like /HTTP etc etc..
  }                                                         // So after we extract the message (parameter) we will set it to none in case it is NULL                                             
  else                                                      // Not setting it to NULL WILL result in a CRASH when the program tries to do things with NULL characters
  {
    Serial.print("Message before adding spaces: ");         // The spaces in a message will first be represented by "+". So, we will print that to the Serial Monitor
    Serial.println(parameter);                              // before making any changes for debugging purposes
    Serial.print("Message character count: ");              // We will also display the message character count for debugging purposes as the LCD can only 
    Serial.println(strlen(parameter));                      // display 32 characters (16 per line) since it is a 16x2
    for (int x = 0; x < strlen(parameter); x++)             // Now, we will replace all the "+" with spaces using a loop to retain the original message
    {
      if (parameter[x] == '+')
        parameter[x] = ' ';
    }
  } 
  Serial.print("Message after adding spaces: ");           // Then, we will display the result of the previous code for debugging purposes
  Serial.println(parameter);
  delay(100);
  
  // On android, the resulting parameter if an empty message was sent would not be NULLl but rather would be HTTP....
  // So, this is added to prevent errors  
  if (!((parameter[0] == 'H' && parameter[1] == 'T' && parameter[2] == 'T') || (parameter == "none")))
  {                                                       // if the message sent was not an empty message, we will display it to the LCD. Otherwise, we will leave it unchanged
    lcd.clear();                                          // Simply clears the LCD
    lcd.setCursor(0,0);                                   // Set the cursor back to x = 0 y = 0 on the LCD
    
    if (strlen(parameter) > 16)                           // if the message is longer than 16 characters and cannot be displayed on a single line,
    {
      for (int x = 0; x < 16; x++)                        // print the first 16 characters of the message on the first line
        lcd.print(parameter[x]);
      lcd.setCursor(0,1);                                 // Move the cursor to the start of the second line x = 0 y = 1
      for (int x = 16; x < strlen(parameter); x++)
        lcd.print(parameter[x]);                         // print the rest of the message. NOTE: if the message does not fit on the LCD, the first 32 characters of it will be displayed
    }
    else
      lcd.print(parameter);                              // If it DOES fit on a single line, just print the whole thing on there

      //LED AND BUZZER TO INDICATE MESSAGE ARRIVAL
      delay(100);
      state = digitalRead(button);                       // Read the value from the button (1 for being pressed)  
      Serial.println("Value returned from button: ");    // This is for debugging purposes
      while (state != HIGH)                              // while button is not pressed keep beeping (HIGH is 1).
      {
        analogWrite(buzzer, 100);                        // Using Pulse Width Modulation, we will send the buzzer a value between 0 and 255 where 0 is no sound (0V) and 255 is max volume(5V) 
        digitalWrite(led, HIGH);                         // Turn the LED on
        delay(500);                                      // wait 500ms
        analogWrite(buzzer, 0);                          // turn buzzer off
        digitalWrite(led, LOW);                          // turn LED off
        delay(500);                                      // wait 500ms. When looping, this will create blinking of LED and beeping every 500ms until the button is pressed
        state = digitalRead(button);                     // Read the state of the button again. NOTE: since we have a second delay between checking the state, the button will need
                                                         // to be long-pressed in order to compensate for the delay. Otherwise, the button will need to be pressed exactly at this point
        Serial.println(state);                           // Print the state of the button again (0 or 1) for debugging purposes
      }

  }
  
    // Return the response
  client.println("HTTP/1.1 200 OK");                     // Print all this to the webserver. Notice that this HTML
  client.println("Content-Type: text/html");             // Printing HTML to the webserver would create an interface in which the client 
  client.println(""); //  do not forget this one            could enter the message in a text box and hit submit to send the message
  client.println("<!DOCTYPE HTML>");                     // to /action_page.php. Then, when the response is returned and is stored in "request" object,
  client.println("<html>");                              // it will be copied to req array as shown above in the code. Since the request will be stored
  client.print("<body>");                                // and used above before this segment of code is reached, we compensated for that by setting parameter
  client.println("<form action=\"/action_page.php\">");  // to NULL and desregarding everything when the message starts with "HTTP". Either one of these will be 
  client.println("<br><br>");                            // the case every time a new client enters the webserver e.g 192.168.1.70. If we hadn't compensated for all
  client.println("Enter Message:<br>");                  // of that an error would have occured causing nodeMCU to crash and restart as it tries to do operations with
  client.println("<input type=\"text\"");                // NULL characters. We are bascically extracting the message from the URL in simple words. So, when the URL does
  client.println("name=\"/entermessage\"><br>");         // not contain a message we need a handler for that, which basically is the if statement that sets parameter to none
  client.println("<input type=\"submit\"");
  client.println("value=\"Submit\">");
  client.println("</form>");
  client.println("</body>");  

  client.println("</html>");
BREAK:
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
  delete [] req;                                        // since we used dynamic memory allocation, we have to return the memory we used back to the system using the delete command
  req = NULL;                                           // finally, we will set that pointer to NULL

 }
