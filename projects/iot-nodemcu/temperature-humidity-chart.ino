#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define DHTTYPE DHT11
#define LCDADDR 0x27

/* Server */
const char *ssidPublic = /*SSID*/;
const char *pwdPublic = /*PASSWORD*/;
ESP8266WebServer server(80);

/* Web Socket */
WebSocketsServer webSocket = WebSocketsServer(81);

/* Sensor : Temperature and Humidity */
uint8_t DHTPin = D6;
float Temperature;
float Humidity;
String temp_str;
String humid_str;
DHT dht(DHTPin, DHTTYPE);
uint32_t time_poll = 0;
uint32_t dht_poll = 0;

/* LCD display */
LiquidCrystal_I2C lcd(LCDADDR, 16, 2);
bool displayOnLCD = false;

char webpage[] PROGMEM = R"==(
<!DOCTYPE html>
<html lang="en">

  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=1, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <title>Jtm | Environment monitoring system</title>

    <link rel="stylesheet" href='https://cdnjs.cloudflare.com/ajax/libs/chartist/0.11.4/chartist.min.css'>
    <style>
      html {
          margin: 0px auto;
          text-align: center;
      }

      #chart-humidity .ct-line, #chart-humidity .ct-point {
          stroke: #349eeb;
      }

    </style>

    <script src='https://cdnjs.cloudflare.com/ajax/libs/chartist/0.11.4/chartist.min.js'></script>

  </head>

  <body onload="javascript:init()">

    <div>
      <h3>Air Temperature and Humidity</h3>
      <div class="ct-chart" id="chart-temperature"></div>
      <p>Temperature: <span id="valueTemp"></span>°C</p>
      <div class="ct-chart" id="chart-humidity"></div>
      <p>Humidity: <span id="valueHumid"></span>%</p>
    </div>

    <script>
    
      var dataTemp = {
        labels: [],
        series: [
          []
        ]
      };
      var dataHumid = {
        labels: [],
        series: [
          []
        ]
      };
      var options = {
        fullWidth: true
      };

      var chartTemp = new Chartist.Line('#chart-temperature', dataTemp, options);
      var chartHumid = new Chartist.Line('#chart-humidity', dataHumid, options);

      var dataSec;
      var valueTemp;
      var valueHumid;
      var maxDataPoint = 20;

      function init()
      {
        var webSocket = new WebSocket('ws://' + window.location.hostname + ':81/');
        webSocket.onmessage = function(event)
        {
          var index = event.data.charAt(0);
          switch(index) {
            case '1':
              updateValue("valueTemp", event);
              break;
            case '2':
              updateValue("valueHumid", event);
              break;
          }
          var today = new Date();
          var sec = today.getSeconds();

          if(dataTemp.labels.length > maxDataPoint) removeData(dataTemp);
          if(dataHumid.labels.length > maxDataPoint) removeData(dataHumid);
          if(dataSec!=sec)
          {
            dataSec = sec;

            dataTemp.labels.push(dataSec);
            dataTemp.series[0].push(valueTemp);
            chartTemp.update(dataTemp);

            dataHumid.labels.push(dataSec);
            dataHumid.series[0].push(valueHumid);
            chartHumid.update(dataHumid);
          }
        }
      }

      function updateValue(id, event)
      {
        var value = event.data.slice(2);
        window[id] = parseFloat(value);
        document.getElementById(id).innerHTML = value;
      }

      function removeData(data)
      {
        data.labels.shift();
        data.series[0].shift();
      }
      
    </script>

  </body>

</html>
)==";

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
    }
    Serial.println("");

    pinMode(DHTPin, INPUT);
    dht.begin();

    connectToExistingWiFiNetwork(true);

    server.on("/", []() {
        server.send_P(200, "text/html", webpage);
    });
    server.onNotFound(handle_NotFound);

    startServer();
    startLCD();
}

void loop()
{
    webSocket.loop();
    server.handleClient();
    getTemperatureAndHumidity();
    if (time_poll <= millis())
    {
        webSocket.broadcastTXT("1|" + temp_str);
        webSocket.broadcastTXT("2|" + humid_str);
        time_poll = millis() + 1000;
    }
    if (displayOnLCD)
    {
        lcd.setCursor(0, 0);
        lcd.print("Temp.: ");
        lcd.print(Temperature);
        lcd.print(" C");
        lcd.setCursor(0, 1);
        lcd.print("Humi.: ");
        lcd.print(Humidity);
        lcd.print(" %");
    }
}

void connectToExistingWiFiNetwork(bool sta)
{
    if (sta)
    {
        /* Station Mode */
        Serial.print("Establishing connection to ");
        Serial.println(ssidPublic);
        WiFi.begin(ssidPublic, pwdPublic);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(1000);
            Serial.print(".");
        }
        Serial.println("Connected to IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        /* Access Point Mode */
        const char *ssid = "Jtm NodeMCU";
        const char *pwd = "8characters";
        IPAddress local_ip(192, 168, 1, 1);
        IPAddress gateway(192, 168, 1, 1);
        IPAddress subnet(255, 255, 255, 0);

        WiFi.softAP(ssid, pwd);
        WiFi.softAPConfig(local_ip, gateway, subnet);
    }
}

void startServer()
{
    server.begin();
    Serial.println("Server Started");

    webSocket.begin();
    Serial.println("Web Socket Started");
    webSocket.onEvent(webSocketEvent);
}

int scanI2C()
{
    Serial.println("I2C Scanning ...");
    byte count = 0;
    Wire.begin();
    for (byte i = 8; i < 120; i++)
    {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0)
        {
            Serial.print("Found address: ");
            Serial.print(i, DEC);
            Serial.print(" (0x");
            Serial.print(i, HEX);
            Serial.println(")");
            count++;
        }
    }
    Serial.print("Found ");
    Serial.print(count, DEC);
    Serial.println(" device(s).");
    return count;
}

void startLCD()
{
    if (scanI2C())
    {
        lcd.begin();
        lcd.backlight();
        displayOnLCD = true;
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
}

void getTemperatureAndHumidity()
{
    if (dht_poll <= millis())
    {
        Temperature = dht.readTemperature();
        temp_str = String(Temperature);
        Humidity = dht.readHumidity();
        humid_str = String(Humidity);

        dht_poll = millis() + 1000;
    }
}

void handle_NotFound()
{
    server.send(404, "text/plain", "Not Found");
}