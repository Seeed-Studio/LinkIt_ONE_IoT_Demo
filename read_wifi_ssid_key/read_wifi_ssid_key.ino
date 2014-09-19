#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>

#define Drv LFlash

void setup()
{
    Serial.begin(9600);
    delay(3000);
    
    LTask.begin();
    Drv.begin();
    
    if(Drv.exists("wifi.txt"))
    {
        Serial.println("wifi.txt exist.........");
        
        LFile f = Drv.open("wifi.txt", FILE_READ);
        
            // read from the file until there's nothing else in it:
      //while (f.available()) 
      //Serial.write(f.read());
      
      f.seek(0);
      
      char __buf[100];
      int len = f.size();
      
      f.read(__buf, len);
      __buf[len] = '\0';
      Serial.print("file size: ");
      Serial.println(f.size());
      Serial.println("data:");
      Serial.println(__buf);
      
      f.close();
        
    }
    
}



void loop()
{
  
}
