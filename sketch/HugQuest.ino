#include <SmartResponseXE.h>
#include "RadioFunctions.h"
#include <EEPROM.h>

char messages[6][26];
char command[32] = ">                      \x00R05e\x00";
char netbuff[26] = "";
unsigned int curs = 2;
int ncurs = 0;
int row=0;
uint8_t canaryOffset = 27;

unsigned long int time_loop=0;

// Operation mode constants
#define CONST_MODE_CONSOLE  0
#define CONST_MODE_HEX      1
#define CONST_MODE_RSSI     2
#define CONST_MODE_CHAT     3

// Special keyboard constants
#define CONST_KEY_CLEAR 0xF8
#define CONST_KEY_EXIT  0xF9

// EEPROM constants
#define CONST_MEM_NAME 0x0000000

struct global_t{
  int mode = CONST_MODE_CONSOLE;
  char command[26] = ">                      ";
  char canary[5] = "R05e";
  uint8_t name[256];
  uint16_t hexdumpaddr;
  uint32_t rfChannel;
  uint8_t rows;         // screen rows
  uint8_t columns;      // screen columns
} global;

void reset(){
  EEPROM.write(CONST_MEM_NAME,0xFF);
  getName();
}

void setup() {
  // put your setup code here, to run once:
  SRXEInit(0xe7, 0xd6, 0xa2); // initialize display
  SRXEWriteString(0,120,global.command, FONT_LARGE, 3, 0); // draw large black text at x=0,y=120, fg=3, bg=0
  global.rows = 6;

  for(int i = 0; i < global.rows; i++){
    strncpy(messages[i], "                        ",24);
  }
  resetInputBuffer();
  global.mode = CONST_MODE_CONSOLE;
  global.rfChannel = 11;
  rfBegin(global.rfChannel);

  // set up the name in memory
  if(getName())
  {
    // TODO print hugs
    submit("Welcome back!");
    submit("Press % or > to submit.");
  }

}

void clearScreen(){
  for(int i = 0; i < global.rows; i++){ submit("                       "); }
  row = 0;
}

int getName()
{
  char outbuff[24];

  // flash is totally empty, welcome the new user!
  if(EEPROM.read(CONST_MEM_NAME)==255)
  {
    submit("Welcome to HugQuest v0.1");
    submit("What is your name?");
    submit("type 'set name <name>'");
    submit("Press % or > to submit.");
    return false;
  }
  
  for(int i = 0; i < 4; i++)
  {
    char nameByte = EEPROM.read(CONST_MEM_NAME+i);
    //snprintf(outbuff,20,"%d %02x %03d : %c",i,nameByte,nameByte,nameByte);
    //submit(outbuff);

    if(nameByte >= 0x20 && nameByte <= 0x7A)
    {
      global.name[i] = nameByte;
      //break;
    }
    else
    {
      global.name[i] = 0x00;
    }
  }
  global.name[4]=0;

  return true;
}

int setName(char *name)
{
  for(int i = 0; i < 4; i++)
  {
    EEPROM.write(CONST_MEM_NAME+i,name[i]);
    global.name[i]=name[i];
    if(name[i] == 0x00)
    {
      break;
    }
  }
}

void resetInputBuffer()
{
  memcpy(global.command, ">                      ",26);
  curs = 2;
  SRXEWriteString(0,120,global.command, FONT_LARGE, 3, 0);
}

void redraw()
{
  SRXEWriteString(0,0  ,"                         ", FONT_LARGE, 3, 0);
  SRXEWriteString(0,20 ,"                         ", FONT_LARGE, 3, 0);   
  SRXEWriteString(0,40 ,"                         ", FONT_LARGE, 3, 0);
  SRXEWriteString(0,60 ,"                         ", FONT_LARGE, 3, 0);
  SRXEWriteString(0,80 ,"                         ", FONT_LARGE, 3, 0);
  SRXEWriteString(0,100,"                         ", FONT_LARGE, 3, 0); 

  SRXEWriteString(0,0  ,messages[(0+row)%global.rows], FONT_LARGE, 3, 0);
  SRXEWriteString(0,20 ,messages[(1+row)%global.rows], FONT_LARGE, 3, 0);   
  SRXEWriteString(0,40 ,messages[(2+row)%global.rows], FONT_LARGE, 3, 0);
  SRXEWriteString(0,60 ,messages[(3+row)%global.rows], FONT_LARGE, 3, 0);
  SRXEWriteString(0,80 ,messages[(4+row)%global.rows], FONT_LARGE, 3, 0);
  SRXEWriteString(0,100,messages[(5+row)%global.rows], FONT_LARGE, 3, 0); 
}

void handleInput(char* cmd)
{
  char outbuff[26];
  
  if(!memcmp(cmd,"set",3))
  {
      char* item = strtok(cmd," ");
      if(item){
        item = strtok(NULL," ");
        if(item){
          if(!memcmp(item,"name",4)){
            item = strtok(NULL," ");
            setName(item);
            snprintf(outbuff,24,"Hello '%s', welcome!",item);
            submit(outbuff);
            getName();
          }else{
            submit("Item not recognized");
          }
        }else{
          submit("Malformed");
        }
      }
  }else if(!memcmp(cmd,"get",3)){
    char* item = strtok(cmd," ");
    if(item){
      item = strtok(NULL, " ");
      if(item){
        if(!memcmp(item,"name",4)){
          getName();
          submit((char*)global.name);
        }if(!memcmp(item,"timer",4)){
          snprintf(outbuff,20,"Timer is at %ul",time_loop);
          submit(outbuff);
        }else{
          submit("Item not recognized");
        }
      }else{
        submit("Malformed");
      }
    }
    }else if(!memcmp(cmd,"hex",3)){
    global.mode = CONST_MODE_HEX;
    global.hexdumpaddr = 0;
    // fill the first 6 lines of the screen
    while(global.hexdumpaddr < (6*4))
    {
        readHexLine(global.hexdumpaddr);
        global.hexdumpaddr += 4;
    }
  }
  else if(!memcmp(cmd,"rssi",4)){global.mode = CONST_MODE_RSSI;}
  else if(!memcmp(cmd,"help",4)){submit("there is no help");}
  else if(!memcmp(cmd,"resetforreals",13)){reset();}
  else if(!memcmp(cmd,"reset",5)){submit("Try 'resetforreals'");}
  else if(!memcmp(cmd,"ascii",4)){
    char outbuff[24];
    for(int x=0;x<256;++x)
    {
      snprintf(outbuff,20,"%02x %03d : %c",x,x,x);
      delay(1000);
      submit(outbuff);
    }
  }
  else if(!memcmp(cmd,"hug",3))
  {
    rfWrite('h');
    rfWrite('u');
    rfWrite('g');
    rfWrite('\n'); // write the last byte
    submit("Hug sent!!");
  }

  // doesn't look like a command, so let's blast it to the chat!
  else
  {
    char outputbuff[26];
    
    memset(outbuff,0,25);
    snprintf(outbuff,24,"%-3s:%s.",global.name,global.command+2);
 
    // transmit
    for(int i=0;i<curs+2;++i){
      rfWrite(outbuff[i]);
    }
    rfWrite('\n'); // write the last byte
        
    submit(outbuff);
  }
}

// submit(char*)
//  Prints the designated string to the screen buffer and forces a redraw
int submit(char* submission)
{
  time_loop=0; // reset the sleep timer
  if(strlen(submission)<3)return false;
  if(strlen(submission)>24)return false;
   
  memcpy(messages[row],"                       ",24);
  messages[row][24]=0;
  memcpy(messages[row],submission, 24);
  row = (row+1)%global.rows;
  //checkCanary();
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

// readHexLine(uint32_t)
//  Read 4 bytes starting from the specified address and print it to the string formatted as:
//     <address> <hex bytes> <character bytes>
void readHexLine(uint32_t addr)
{
  byte foo[4];
  char bar[26];
  for(int j = 0; j < 4; j++){ foo[j] = EEPROM.read(addr+j); }
  snprintf(bar, 24, "%04x %02x%02x%02x%02x %c%c%c%c", addr, foo[0],foo[1],foo[2],foo[3], foo[0],foo[1],foo[2],foo[3]);
  bar[24] = 0x00;
  submit(bar);
}

void updateInputBuffer(byte k){
  if(k >= 0x20 && k <= 0x7A){
    //is it printable?
    if(curs <= 20)
    {
      global.command[curs]=k;
      curs++;
    }
    
  }else if(k == 0x08){
    // backspace?
    if(curs > 2){
      curs--;
    }
    global.command[curs] = 0x20; //space is your blank character
  }

  SRXEWriteString(0,120,global.command, FONT_LARGE, 3, 0);
}

// mode_hex_loop(byte)
//  This is the loop function for when the device is in hex-dump mode.
//  TODO:
//    - Enable reading backwards
void mode_hex_loop(byte k){
  if(k){
    readHexLine(global.hexdumpaddr);
    if(global.hexdumpaddr <= (4*1024))
    {
      global.hexdumpaddr += 4;
    }
  }
}

// mode_console_loop(byte)
//  This is the loop function for when the device is in console (default) mode
void mode_console_loop(byte r, byte k){

  if(r){  
    if(r=='\n')
    {
      // HERE IS WHERE WE ACCEPT COMMANDS FROM THE NETWORK
      if(!memcmp(netbuff,"hug",3)){submit("OMG YOU'VE BEEN HUGGED!");}
      else if(!memcmp(netbuff,"wannahug",8)){submit("OH NO HAX!");}
      else submit(netbuff);
      strncpy(netbuff,"                       ",24);
      ncurs = 0;
    }
    else
    {    
      netbuff[ncurs]=r;
      ncurs = (ncurs+1)%24;
    } 
  }
  
  
  if(k){
    // submit on "return" (key right of 'Sym', box line box)
    if(k==0x0D || k==0x03)
    {
      if(curs>3)
      {
        handleInput(global.command+2);
        resetInputBuffer();
      }
    }else{
      //update input buffer
      updateInputBuffer(k);
    }
  }
}

// mode_rssi_loop()
//  This is a port of earlier RSSI code
void mode_rssi_loop(){
  int x;
  int y;
  char buf[24];
  char buf2[12];
  int i = global.rfChannel;
  
  rfBegin(i);
  delay(100);

  y=(i-11)%8*17;
  if(i<19){ x = 0;}
  else { x = 201; }
  
  if(rfAvailable())
  {
    while(0>rfRead()){} // burn through the reads
  }
  snprintf(buf,24,"Ch %d: %d  ",i,rssiRaw);
  
  global.rfChannel++;
  i = global.rfChannel;
  
  rfBegin(i);
  delay(100);

  y=(i-11)%8*17;
  if(i<19){ x = 0;}
  else { x = 201; }
  
  if(rfAvailable())
  {
    while(0>rfRead()){} // burn through the reads
  }
  snprintf(buf2,12,"Ch %d: %d",i,rssiRaw);
  strncat(buf,buf2,24);
  submit(buf);

  global.rfChannel++;
  if(global.rfChannel > 26){
    global.rfChannel = 11;
  }
}

void loop() {

  time_loop+=1;
  if(time_loop>50000) // maybe a minute or so?
  {
    time_loop=0;
    SRXESleep();
    redraw();
    resetInputBuffer();
  }

  
  // if we hear something on the radio, build up the net buffer
  byte r = NULL;
  if (rfAvailable())  
  {
    r = rfRead();
  }

  // otherwise, just take data from the keyboard
  byte k = SRXEGetKey();
  if(k){
    if(k == CONST_KEY_EXIT){
      global.mode = CONST_MODE_CONSOLE;
    }else if(k == CONST_KEY_CLEAR){
      clearScreen();
      k = NULL;   // prevents subprograms from operating on this input
    }
  }
  
  // figure out our mode
  switch(global.mode)
  {
    case CONST_MODE_HEX:
      mode_hex_loop(k);
      break;
    case CONST_MODE_RSSI:
      mode_rssi_loop();
      break;
    case CONST_MODE_CONSOLE:
    default:
      mode_console_loop(r,k);
  }
  //redraw();
}
