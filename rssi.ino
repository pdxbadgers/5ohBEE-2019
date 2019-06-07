#include <SmartResponseXE.h>
#include "RadioFunctions.h"

char messages[6][24];
char command[24] = "> ";
char netbuff[24] = "";
int curs = 2;
int ncurs = 0;
int row=0;

void setup() {
  // put your setup code here, to run once:

  SRXEInit(0xe7, 0xd6, 0xa2); // initialize display
  //SRXEWriteString(0,120,command, FONT_LARGE, 3, 0); // draw large black text at x=0,y=120, fg=3, bg=0

  strcpy(messages[0], "");
  strcpy(messages[1], "");
  strcpy(messages[2], "");
  strcpy(messages[3], "");
  strcpy(messages[4], "");
  strcpy(messages[5], "");

  //rfBegin(24);

}

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

void submit(char* submission)
{  
  strcpy(messages[row],"                       ");
  strcpy(messages[row],submission);
  row = (row+1)%6;

  redraw();
}

void loop() {

int x;
int y;

for(int i=11;i<=26;++i)
{
  rfBegin(i);
  delay(100);

  y=(i-11)%8*17;
  if(i<19){ x = 0;}
  else { x = 201; }
  
  if(rfAvailable())
  {
    while(0>rfRead()){} // burn through the reads
    sprintf(command,"Ch %d : %d   ",i,rssiRaw);
    SRXEWriteString(x,y ,command, FONT_LARGE, 3, 0);
  
  }
  else
  {
    sprintf(command,"Ch %d : %d   ",i,rssiRaw);
    SRXEWriteString(x,y ,command, FONT_LARGE, 3, 0);
  }

}


}
