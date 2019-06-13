#include "SmartResponseXE.h"
#include "RadioFunctions.h"

char messages[6][24];
char command[24] = ">                      ";
char netbuff[24] = "";
int curs = 2;
int ncurs = 0;
int row=0;

void setup() {
  // put your setup code here, to run once:

  SRXEInit(0xe7, 0xd6, 0xa2); // initialize display
  SRXEWriteString(0,120,command, FONT_LARGE, 3, 0); // draw large black text at x=0,y=120, fg=3, bg=0

  strcpy(messages[0], "");
  strcpy(messages[1], "");
  strcpy(messages[2], "");
  strcpy(messages[3], "");
  strcpy(messages[4], "");
  strcpy(messages[5], "");

  rfBegin(11);

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
    if(k==3)
    {
      if(curs>3)
      {
      submit(command+2);
     
      // transmit
      for(int i=2;i<curs;++i)
      {
        rfWrite(command[i]); 
      }
      rfWrite(3); // write the last byte
      }
      
      strcpy(command, ">                      ");
      curs = 2;
    }
    else
    {
      //sprintf(command,"> %d",k);
    
      command[curs]=k;
      curs = (curs+1)%24;
    }
  }
  redraw();
}
