// LinkIt ONE IoT demo
// get Sensor data value

#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <Wire.h>
#include <LGSM.h>
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>

#include "Suli.h"
#include "OLED_96x96_Arduino.h"
#include "OLED_96x96_Suli.h"
#include "DHT.h"

#define DBG     1               // If Debug


SeeedOLED oled;

char str_tmp[100];

const int pinLight = A2;
const int pinSound = A0;

const int pinLamp  = 5;
const int pinTh    = 2;

bool flg_temp_new = 0;
bool flg_humi_new = 0;


float __GTemp   = 0.0;
float __GHumi   = 0.0;
int   __GLight  = 0;
int   __GSound  = 0;


DHT dht(pinTh, DHT22);

#define getSound()  map(analogRead(pinSound), 0, 256, 0, 100)
#define getLight()  map(analogRead(pinLight), 0, 1023, 0, 100)


#define SITE_URL      "api.xively.com"      // Xively API URL
#define APIKEY         "HUQl7E2mC3tJbZoZ3NAPoUffxb2emxCCFJLNu1LROShfZy7u" // replace your xively api key here
#define FEEDID         59487691             // replace your feed ID
#define USERAGENT      "chrome"             // user agent is the project name

#define WIFI_AUTH LWIFI_WPA                 // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP according to your AP

/****************************************************************************
 * get ssid & key
 ****************************************************************************/
#define WIFI_AP __BUF_SSID
#define WIFI_PASSWORD __BUF_KEY

char __BUF_SSID[50];
char __BUF_KEY[50];

#define Drv LFlash

bool get_ssid_key()
{

    //delay(3000);
    LTask.begin();
    Drv.begin();

    if(Drv.exists("wifi.txt"))
    {
        LFile f = Drv.open("wifi.txt", FILE_READ);

        f.seek(0);

        char __buf[100];
        int len = f.size();

        f.read(__buf, len);
        __buf[len] = '\0';
        
#if DBG
        Serial.print("file size: ");
        Serial.println(f.size());
        Serial.println("data:");
        Serial.println(__buf);
#endif
        int len_ssid = 0;
        int __index = 5;
        while(1)        // set ssid
        {
            if(__buf[__index] != 0x0D)
            {
                __BUF_SSID[len_ssid++] = __buf[__index++];
            }
            else 
            {
                __BUF_SSID[len_ssid] = '\0';
                break;
            }
        }
        
        int len_key = 0;
        __index = 5+6+len_ssid;
        
        while(1)
        {
            if(__buf[__index] != 0x0D)
            {
                __BUF_KEY[len_key++] = __buf[__index++];
            }
            else 
            {
                __BUF_SSID[len_key] = '\0';
                break;
            }
        }
#if DBG
        Serial.println("GET SSID & KEY:");
        Serial.println(__BUF_SSID);
        Serial.println(__BUF_KEY);
#endif
        f.close();
        debug_oled(1, "READ SD OK");
        return true;
    }
    
    debug_oled(1, "READ SD NOK");
    while(1);
}


/****************************************************************************
 * Temperature
 ****************************************************************************/
const int numReadings = 10;

float   readings[numReadings];      // the readings from the analog input
int     __index = 0;                  // the __index of the current reading
float   total = 0;                  // the running total


void tempInit()
{

    for (int thisReading = 0; thisReading < numReadings; thisReading++)
    {
        readings[thisReading] = 0;
    }

    float h, t;

    while(1)
    {
        if(dht.readHT(&t, &h))
        {
            for(int i=0; i<10; i++)
            {
                pushTemp(t);
            }
            break;
        }
        delay(2500);
    }
}

void pushTemp(float __t)
{
    total = total - readings[__index];
    readings[__index] = __t;
    total = total + readings[__index];
    __index = __index + 1;
    if (__index >= numReadings)
    __index = 0;
    __GTemp = total / numReadings;
}


bool getTempHumi()
{
    static long timer_th = millis();
    if(millis() - timer_th < 2000)return false;
    timer_th = millis();

    float h = 0.0;
    float t = 0.0;

    if(dht.readHT(&t, &h))
    {
        pushTemp(t);
        __GHumi = h;
        flg_temp_new = 1;
        flg_humi_new = 1;
        return true;
    }

    return false;
}

/****************************************************************************
 * Wi-Fi Initialize
 ****************************************************************************/


bool wifi_init()
{
    LWiFi.begin();

#if DBG
    Serial.print("Connecting to WiFi AP:");
    Serial.println(WIFI_AP);
#endif


    unsigned long timer_w = millis();
    
    while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
    {
        if(millis() - timer_w > 30000)
        {
#if DBG
            Serial.println("\r\nConnect wo AP Fail, Time OUT....");
#endif
            return false;
        }
        
        checkSms();
#if DBG
        Serial.print(".");
#endif
    }
#if DBG
    Serial.println();
#endif
    delay(1000);
    
    return true;
}

/****************************************************************************
 * Oled
 ****************************************************************************/
void oled_init()
{
    oled.init();                            // initialze SEEED OLED display
    oled.clearDisplay();                    // clear the screen and set start position to top left corner
    oled.setNormalDisplay();                // Set display to normal mode (i.e non-inverse mode)
    oled.setVerticalMode();                 // Set to vertical mode for displaying text
}


void displaySensor()
{
    static int __first_here__ = 1;
    
    if(__first_here__)
    {
        __first_here__ = 0;
        for(int i=0; i<12; i++)
        {
            oled.setTextXY(0, i);
            oled.putString("            ");
        }
    }
    
    static unsigned long timer_ds = millis();
    if(millis()-timer_ds < 1000)return;
    
    timer_ds = millis();

#if DBG
    Serial.println("Refresh sensor value");
#endif 
    char tmp[20];
    
    
    oled.setTextXY(0, 2);
    
    sprintf(tmp, "Temp:  %.2f", __GTemp);
    oled.putString(tmp);
    
    oled.setTextXY(0, 4);
    sprintf(tmp, "Humi:  %.2f", __GHumi);
    oled.putString(tmp);
    
    oled.setTextXY(0, 6);
    sprintf(tmp, "Sound: %d  ", getSound());
    oled.putString(tmp);
    
    oled.setTextXY(0, 8);
    sprintf(tmp, "Light: %d  ", getLight());
    oled.putString(tmp);
}


void debug_oled(int row, char *str)
{

    int len = 0;
    char tmp[20];

    for(len = 0; *str; len++)
    {
        if(str[len] == '\0')break;
    }
    
    for(int i=0; i<len; i++)
    {
        tmp[i] = str[i];
    }

    if(len>12)tmp[12] = '\0';
    else tmp[len] = '\0';

    oled.setTextXY(0, row);
    oled.putString(tmp);

}


/****************************************************************************
 * setup
 ****************************************************************************/
void setup()
{

#if DBG
    Serial.begin(115200);
#endif
    
    oled_init();
    
    debug_oled(0, "START");
    delay(1000);
    pinMode(pinLamp, OUTPUT);               // initialize io of lamp
    digitalWrite(pinLamp, LOW);             // lamp off

    
    get_ssid_key();
    tempInit();
    
    while(!LSMS.ready())
    {
        delay(1000);
    }

#if DBG
    Serial.println("SIM ready for work!");
#endif

    debug_oled(2, "SIM READY");
    
    

    debug_oled(3, "CONNECT...");
    
    if(wifi_init())
    {

        debug_oled(4, "CONNECTED!");
        debug_oled(6, "--INIT OK---");
#if DBG
        Serial.println("Wifi Init Ok");
        Serial.println("Init OK..........");
#endif
        delay(1000);
    }
    else
    {
#if DBG
        Serial.println("Wifi Init Fail");
        Serial.println("Init Fail...........");
#endif
        debug_oled(4, "CONNECT FAIL");  
        debug_oled(6, "-WiFi Fail-");
        
        delay(5000);
    }

    
}


/****************************************************************************
 * loop
 ****************************************************************************/
void loop()
{
    sendXively();
    getTempHumi();
    checkSms();
    displaySensor();
    delay(1000);
}


/****************************************************************************
 * check sms & control the light
 ****************************************************************************/
void checkSms()
{

    static unsigned long time_sms = millis();

    if(millis()-time_sms < 500)return;
    time_sms = millis();

    char p_num[20];

    int len = 0;
    char dtaget[500];

    if(LSMS.available()) // Check if there is new SMS
    {
    
        LSMS.remoteNumber(p_num, 20); // display Number part
#if DBG
        Serial.println("There is new message.");
        
        Serial.print("Number:");
        Serial.println(p_num);
        Serial.print("Content:"); // display Content part
#endif        

        while(true)
        {
            int v = LSMS.read();
            if(v < 0)
            break;

            dtaget[len++] = (char)v;
#if DBG
            Serial.print((char)v);
#endif
        }
        
#if DBG
        Serial.println();
#endif
        LSMS.flush(); // delete message

        
        if((dtaget[0] == 'O' && dtaget[1] == 'N') || (dtaget[0] == 'o' && dtaget[1] == 'n'))
        {
            digitalWrite(pinLamp, HIGH);        // lamp on
        }

        else if((dtaget[0] == 'O' && dtaget[1] == 'F' && dtaget[2] == 'F') || (dtaget[0] == 'o' && dtaget[1] == 'f' && dtaget[2] == 'f'))
        {
            digitalWrite(pinLamp, LOW);         // lamp off
        }
    }

}

/****************************************************************************
 * send data to Xively
 ****************************************************************************/
void sendXively()
{
    static unsigned long timer_sx = millis();

    /*
     * 0: temperature
     * 1: humidity
     * 2: light
     * 3: sound
     */
    static int which_to_send = 0;

    if(millis() - timer_sx < 10000)return;              // send data per 10s
    timer_sx = millis();

    switch(which_to_send)
    {
        case 0:     // temperature

        if(flg_temp_new)
        {
            flg_temp_new = 0;
            sendDtaXivery("Temperature,", String(__GTemp));
        }
        break;

        case 1:     // humidity

        if(flg_humi_new)
        {
            flg_humi_new = 0;
            sendDtaXivery("Humidity,", String(__GHumi));
        }
        break;

        case 2:     // light

        sendDtaXivery("Light,", String(getLight()));
        break;

        case 3:     // sound

        sendDtaXivery("Sound,", String(getSound()));
        break;

        default:;

    }

    which_to_send++;
    which_to_send = (which_to_send>3) ? 0 : which_to_send;

}


void sendDtaXivery(String device_name, String value)
{

    LWiFiClient c;
    while (!c.connect(SITE_URL, 80))
    {

#if DBG
        Serial.println("connect fail");
#endif
        delay(1000);
        c.stop();
        delay(100);

#if DBG
        Serial.println("wifi end");
#endif
        LWiFi.end();
        delay(2000);

#if DBG
        Serial.println("reconnect to wifi again");
#endif
        wifi_init();
#if DBG
        Serial.println("reconnect to wifi ok");
#endif
        return;
    }

    String data = device_name+value;
#if DBG
    Serial.println("send PUT request");
    Serial.println(data);
#endif
    // construct a HTTP PUT request
    // and set CSV data to the Xively feed.
    c.print("PUT /v2/feeds/");
    c.print(FEEDID);
    c.println(".csv HTTP/1.1");
    c.println("Host: api.xively.com");
    c.print("X-ApiKey: ");
    c.println(APIKEY);
    c.print("User-Agent: ");
    c.println(USERAGENT);
    c.print("Content-Length: ");
    int thisLength = data.length();
    c.println(thisLength);
    c.println("Content-Type: text/csv");
    c.println("Connection: close");
    c.println();
    c.println(data);
    delay(1000);
    c.stop();
}

/****************************************************************************
 * END
 ****************************************************************************/