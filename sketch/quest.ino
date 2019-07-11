#include <SmartResponseXE.h>
#include "RadioFunctions.h"
#include <EEPROM.h>

char messages[6][24];
char command[32] = ">                      \x00R05e\x00";
char netbuff[24] = "";
unsigned int curs = 2;
int ncurs = 0;
int row=0;
uint8_t canaryOffset = 27;

#define NAME_ADDR 0x00000000

struct global_t{
  char command[26] = ">                      ";
  char canary[5] = "R05e";
  uint8_t name[256];
} global;

void setup() {
  // put your setup code here, to run once:

  SRXEInit(0xe7, 0xd6, 0xa2); // initialize display
  SRXEWriteString(0,120,global.command, FONT_LARGE, 3, 0); // draw large black text at x=0,y=120, fg=3, bg=0
  strcpy((char*)global.name, "uninitialized");

  //setName("Bob");
  readName();
  strcpy(messages[0], "Hi there...");
  //strcpy(messages[1], "");
  memcpy(messages[1], global.name, 24);
  strcpy(messages[2], "");
  strcpy(messages[3], "");
  strcpy(messages[4], "");
  strcpy(messages[5], "");
  resetInputBuffer();
  rfBegin(11);
}

void readName()
{
  //SRXEFlashRead(NAME_ADDR,global.name,256);
  for(int i = 0; i < 256; i++)
  {
    char nameByte = EEPROM.read(NAME_ADDR+i);
    if(nameByte == 0x00){
      break;
    }
    global.name[i] = nameByte;
  }
}

void setName(char *name)
{
  /*uint8_t foo[256];
  memset(foo,0x00,256);
  strcpy((char*)foo,name);
  bool result = SRXEFlashWritePage(NAME_ADDR,foo);
  delay(100);*/
  memset(global.name,0x00,256);
  for(int i = 0; i < 256; i++)
  {
    if(name[i] == 0x00)
    {
      break;
    }
    EEPROM.write(NAME_ADDR+i,name[i]);
  }
  //return(result);
}

void resetInputBuffer()
{
  /*memcpy(command,">",1);
  memcpy(command+1," ",30);
  memcpy(command+27,"R05e",4);
  memcpy(command+31,0x00,1);
  memcpy(command+25,0x00,1);*/
  
  //memcpy(command, ">                      \x00R05e\x00",26);
  memcpy(global.command, ">                      ",26);
}

void redraw()
{
  SRXEWriteString(0,0  ,messages[(0+row)%6], FONT_LARGE, 3, 0);
  SRXEWriteString(0,20 ,messages[(1+row)%6], FONT_LARGE, 3, 0);   
  SRXEWriteString(0,40 ,messages[(2+row)%6], FONT_LARGE, 3, 0);
  SRXEWriteString(0,60 ,messages[(3+row)%6], FONT_LARGE, 3, 0);
  SRXEWriteString(0,80 ,messages[(4+row)%6], FONT_LARGE, 3, 0);
  SRXEWriteString(0,100,messages[(5+row)%6], FONT_LARGE, 3, 0); 

  SRXEWriteString(0,120,global.command, FONT_LARGE, 3, 0);
}

void submit(char* submission)
{  
  strcpy(messages[row],"                       ");
  strncpy(messages[row],submission, 26);
  row = (row+1)%6;
  checkCanary();
  setName(submission);
  readName();
  strncpy(messages[1],(char*)global.name, 26);
  resetInputBuffer();
  redraw();
}

void checkCanary()
{
  //if(memcmp(command+canaryOffset,"R05e",4) != 0) {
  if(memcmp(global.canary,"R05e",4) != 0) {
    for(int i = 0; i < 6; i++){ strcpy(messages[i],"OMGH@XXE!"); }
    //redraw();
  }
}

void loop() {

  // if we hear something on the radio, build up the net buffer
  if (rfAvailable())  
  {
    byte n = rfRead();
    if(n)
    {   
      if(n==3)
      {
        submit(netbuff);
        strcpy(netbuff, "                       ");
        ncurs = 0;
      }
      else
      {    
        netbuff[ncurs]=n;
        ncurs = (ncurs+1)%24;
      }
    }
  }

  // otherwise, just take data from the keyboard
  byte k = SRXEGetKey();
  if(k)
  {
    // submit on "return" (key right of 'Sym', box line box)
    if(k==0x0D)
    {
      if(curs>3)
      {
      submit(global.command+2);
     
      // transmit
      for(int i=2;i<curs;++i)
      {
        rfWrite(global.command[i]); 
      }
      rfWrite(3); // write the last byte
      }

      //checkCanary();
      
      resetInputBuffer();
      memcpy(global.command, ">                      ",26);//canaryOffset-1);
      //memcpy(command, ">                      \x00R05e\x00",26);//canaryOffset-1);
      curs = 2;
      /*if(memcmp(command+60,"R05e",4) == 0) {
        memcpy(command, ">                      ",26);//canaryOffset-1);
        curs = 2;
      }else{
        //strcpy(command, "OMGH@XXE!");
        submit("OMGH@XXE!");
      }*/
    }
    else
    {
      //sprintf(command,"> %d",k);
      //sprintf(command,"> %02X",k);
      
      if(k >= 0x20 && k <= 0x7A){
        //is it printable?
        global.command[curs]=k;
        //curs = (curs+1)%24;
        if(curs < 31){  //22) {
          curs++;
        }
      }else if(k == 0x08){
        // backspace?
        if(curs > 2){
          curs--;
        }
        global.command[curs] = 0x20; //space is your blank character
      }
    }
  }
  redraw();
}
