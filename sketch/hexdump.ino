#include <SmartResponseXE.h>
#include <EEPROM.h>
char messages[6][24];
char command[24] = "> ";
uint32_t addr = 0x00;
int row = 0;

void redraw()
{
  SRXEWriteString(0,0  ,messages[(0+row)%6], FONT_LARGE, 3, 0);
  SRXEWriteString(0,20 ,messages[(1+row)%6], FONT_LARGE, 3, 0);   
  SRXEWriteString(0,40 ,messages[(2+row)%6], FONT_LARGE, 3, 0);
  SRXEWriteString(0,60 ,messages[(3+row)%6], FONT_LARGE, 3, 0);
  SRXEWriteString(0,80 ,messages[(4+row)%6], FONT_LARGE, 3, 0);
  SRXEWriteString(0,100,messages[(5+row)%6], FONT_LARGE, 3, 0); 
  SRXEWriteString(0,120,command, FONT_LARGE, 3, 0);
}

void readLine(uint32_t addr)
{
  //uint8_t foo[5];
  byte foo[4];
  //memset(foo,0x00,5);
  //char foo;
  snprintf(command, 24, "> 0x%x", addr);
  for(int i = 0; i < 5; i++){ strcpy(messages[i],messages[i+1]); }
  for(int j = 0; j < 4; j++){ foo[j] = EEPROM.read(addr+j); }
  //foo = EEPROM.read(addr);
  snprintf(messages[5], 24, "%04x %02x%02x%02x%02x %c%c%c%c", addr, foo[0],foo[1],foo[2],foo[3], foo[0],foo[1],foo[2],foo[3]);
  redraw();
}

void setup() {
  // put your setup code here, to run once:
  SRXEInit(0xe7, 0xd6, 0xa2); // initialize display
  SRXEWriteString(0,120,command, FONT_LARGE, 3, 0); // draw large black text at x=0,y=120, fg=3, bg=0
  for(int i = 0; i < 4*8; i++)
  {
    /*EEPROM.write(n*i+0,0xde);
    EEPROM.write(n*i+1,0xad);
    EEPROM.write(n*i+2,0xbe);
    EEPROM.write(n*i+3,0xef);*/
    EEPROM.write(i,'A');
    //delay(500);
  }
  while(addr < (6*4))
  {
    readLine(addr);
    addr += 4;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  byte k = SRXEGetKey();
  if(k)
  {
    readLine(addr);
    if(addr <= (4*1024))
    {
      addr += 4;
    }
  }
}
