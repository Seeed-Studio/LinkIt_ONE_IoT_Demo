#define Serial Serial1

#include <LFlash.h>
#include <LSD.h>

int mode = 0;
LDrive* pObjs[2];

void printRecur(LFile &dir, int level)
{
  while(true)
  {
    LFile entry =  dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }
 
    for(int i=0;i<level;i++)     // print padding
      Serial.print("  ");
 
    if (entry.isDirectory())
    {
      Serial.print(entry.name());   // folder will have "/"
      Serial.println("/");
      printRecur(entry, level+1);
    }
    else
    {
      Serial.printf("%s(%d)", entry.name(), entry.size());
      Serial.println();
    }
    entry.close();
  }
} 

void setup()
{
  int i, j, k;
  char buf[64];

  Serial.begin(115200);
  
  LFlash.begin();
  LSD.begin();

  while(!Serial)
    delay(100);

  pObjs[0] = &LFlash;
  pObjs[1] = &LSD;

  // create testing data
  for(k=0;k<2;k++)
  {
    for(i=1;i<6;i++)
    {
      sprintf(buf, "dir%d", i);
      pObjs[k]->mkdir(buf);
  
      for(j=1;j<4;j++)
      {
        sprintf(buf, "dir%d/sub%d", i, j);
        pObjs[k]->mkdir(buf);
      }

      for(j=1;j<4;j++)
      {
        sprintf(buf, "dir%d/sub2/%d.txt", i, j);
        LFile f = pObjs[k]->open(buf, FILE_WRITE);
        for(j=0;j<100;j++)
          f.print(buf);
        f.close();
      }
    }
  }  
}
  
void loop()
{
  if(mode % 2 == 0)
  {
    Serial.println("===== Starting to dir/s Flash... =====");
  }
  else
  {
    Serial.println("===== Starting to dir/s SD... =====");
  }

  LFile root = pObjs[mode % 2]->open("/");
  printRecur(root, 0);
  root.close();
  Serial.println("===== end =====");


  delay(5000);
  mode++;
}

