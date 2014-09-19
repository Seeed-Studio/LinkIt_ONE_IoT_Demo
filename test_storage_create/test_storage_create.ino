#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>

#define Serial Serial1
#define Drv LFlash

void testRead(LFile &f, const char* pattern, int seekpos)
{
  char buf[20];
  
  f.seek(seekpos);
  f.read(buf, 5);
  buf[5] = 0;
  
  if(strcmp(buf, pattern)!=0)
  {
    Serial.print("Error! pattern not match:");
    Serial.print(f.name());
    Serial.println(f.size());
    
    Serial.print("read:");
    Serial.println(buf);
    Serial.print("pattern:");
    Serial.println(pattern);
  }

  if(f.position() != seekpos+5)
  {
    Serial.print(f.name());
    Serial.print("Error position is wrong:");
    Serial.println(f.position());
  }
  f.peek();
  f.peek();
  f.peek();
  f.peek();
  f.peek();
  if(f.position() != seekpos+5)
  {
    Serial.print(f.name());
    Serial.print("Error position is wrong:");
    Serial.println(f.position());
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  LTask.begin();
  Drv.begin();
  
  int i, j;
  char buf[64];
  
  Serial.println();
  Serial.println();
  Serial.println("===== test file creation =====");
  for(i=1;i<10;i++)
  {
    sprintf(buf, "%d.txt", i);
    // check if exist    
    if(!Drv.exists(buf))
    {
      LFile f = Drv.open(buf, FILE_WRITE);
      if(!f)
      {
        Serial.print("Error to create:");
        Serial.println(buf);
      }
      else
      {
        for(j=0;j<100;j++)
          f.print(buf);
        f.close();
      }
    }
  }

  Serial.println("===== test file read =====");
  for(i=1;i<10;i++)
  {
    sprintf(buf, "%d.txt", i);
    if(!Drv.exists(buf))
    {
      Serial.print(buf);
      Serial.println(" not exists...ERROR!");
      continue;
    }
    
    LFile f = Drv.open(buf, FILE_READ);
    
    testRead(f, buf, 0);
    testRead(f, buf, 250);
    testRead(f, buf, 495);
    
    f.close();    
  }
  
  Serial.println("===== test folder creation =====");
  // test create folder
  for(i=1;i<6;i++)
  {
    sprintf(buf, "dir%d", i);
    if(!Drv.exists(buf))
    {
      if(!Drv.mkdir(buf))
      {
        Serial.print("Error creating:");
        Serial.println(buf);
      }
    }
    for(j=1;j<4;j++)
    {
      sprintf(buf, "dir%d/sub%d", i, j);
      if(!Drv.exists(buf))
      {
        if(!Drv.mkdir(buf))
        {
          Serial.print("Error creating:");
          Serial.println(buf);
        }
      }
    }
  }

  // test create folder recursively
  sprintf(buf, "a/b/c");
  if(!Drv.mkdir(buf))
  {
    Serial.print("Error creating:");
    Serial.println(buf);
  }
  
  Serial.println("===== test file removal =====");
  for(i=6;i<10;i++)
  {
    sprintf(buf, "%d.txt", i);
    if(!Drv.remove(buf))
    {
      Serial.print(buf);
      Serial.println(" delete fail...ERROR!");
    }
    if(Drv.remove(buf))
    {
      Serial.print(buf);
      Serial.println(" double delete...ERROR!");
    }
  }  

  Serial.println("===== test folder removal =====");
  for(i=5;i<6;i++)
  {
    for(j=1;j<4;j++)
    {
      sprintf(buf, "dir%d/sub%d", i, j);
      if(!Drv.rmdir(buf))
      {
        Serial.print(buf);
        Serial.println(" delete fail...ERROR!");
      }
    }
    sprintf(buf, "dir%d", i);
    if(!Drv.rmdir(buf))
    {
      Serial.print(buf);
      Serial.println(" delete fail...ERROR!");
    }
  }
  
  // test create folder recursively
  sprintf(buf, "a/b/c");
  if(!Drv.rmdir(buf))
  {
    Serial.print("Error removal:");
    Serial.println(buf);
  }
  
  sprintf(buf, "a/b");
  if(!Drv.rmdir(buf))
  {
    Serial.print("Error removal:");
    Serial.println(buf);
  }
  
  sprintf(buf, "a");
  if(!Drv.rmdir(buf))
  {
    Serial.print("Error removal:");
    Serial.println(buf);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
}
