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

// Operation mode constants
#define CONST_MODE_CHAT 0
#define CONST_MODE_HEX  1
#define CONST_MODE_RSSI 2

// Special keyboard constants
#define CONST_KEY_CLEAR 0xF8
#define CONST_KEY_EXIT  0xF9

// EEPROM constants
#define CONST_MEM_NAME 0x00000000

struct global_t{
  int mode = CONST_MODE_CHAT;
  char command[26] = ">                      ";
  char canary[5] = "R05e";
  uint8_t name[256];
  uint16_t hexdumpaddr;
  uint32_t rfChannel;
} global;

void reset(){
  //EEPROM.write(CONST_MEM_NAME,0xFF);
  setName("");
  readName();
}

void setup() {
  // put your setup code here, to run once:
  SRXEInit(0xe7, 0xd6, 0xa2); // initialize display
  SRXEWriteString(0,120,global.command, FONT_LARGE, 3, 0); // draw large black text at x=0,y=120, fg=3, bg=0
  strcpy(messages[0], "");
  strcpy(messages[1], "");
  strcpy(messages[2], "");
  strcpy(messages[3], "");
  strcpy(messages[4], "");
  strcpy(messages[5], "");
  resetInputBuffer();
  global.mode = CONST_MODE_CHAT;
  global.rfChannel = 11;
  rfBegin(global.rfChannel);

  // Device is ready to go; check if this was a reboot or new use
  strcpy((char*)global.name, "uninitialized");
  //setName("Bob");   // uncomment this for testing
  //reset();
  readName();
  if(global.name[0]==NULL){
    // device is unininitialized
    submit("Hello!");
    submit("What is your name?");
    // TODO: fall into limited mode waiting for username and write to EEPROM
  }
}

void clearScreen(){
  for(int i = 0; i < 6; i++){ submit("                       "); }
  row = 0;
}

void readName()
{
  for(int i = 0; i < 256; i++)
  {
    char nameByte = EEPROM.read(CONST_MEM_NAME+i);
    if(nameByte == 0xFF || nameByte == 0x00){
      global.name[i] = 0x00;
      break;
    }
    global.name[i] = nameByte;
  }
}

void setName(char *name)
{
  for(int i = 0; i < 256; i++)
  {
    if(name[i] == 0x20)
    {
      name[i] = 0x00; // no spaces
    }
    
    EEPROM.write(CONST_MEM_NAME+i,name[i]);
    if(name[i] == 0x00)
    {
      break;
    }
  }
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

void handleInput(char* cmd)
{
  if(!memcmp(cmd,"set",3))
  {
      char* item = strtok(cmd," ");
      if(item){
        item = strtok(NULL," ");
        if(item){
          if(!memcmp(item,"name",4)){
            item = strtok(NULL," ");
            setName(item);
            readName();
          }else{
            submit("Item not recognized");
          }
        }else{
          submit("Malformed");
        }
      }
  }else if(!memcmp(cmd,"read",4)){
    char* item = strtok(cmd," ");
    if(item){
      item = strtok(NULL, " ");
      if(item){
        if(!memcmp(item,"name",4)){
          submit((char*)global.name);
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
  }else if(!memcmp(cmd,"rssi",4)){
    global.mode = CONST_MODE_RSSI;
  }else if(!memcmp(cmd,"help",4)){
    submit("there is no help");
  }else{
    submit(cmd);
  }
}

// submit(char*)
//  Prints the designated string to the screen buffer and forces a readraw
void submit(char* submission)
{  
  strcpy(messages[row],"                       ");
  strncpy(messages[row],submission, 24);
  row = (row+1)%6;
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

// mode_chat_loop(byte)
//  This is the loop function for when the device is in chat (default) mode
void mode_chat_loop(byte k){
  if(k)
  {
    // submit on "return" (key right of 'Sym', box line box)
    if(k==0x0D)
    {
      if(curs>3)
      {
      //submit(global.command+2);
      handleInput(global.command+2);
     
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
      curs = 2;
    }
    else
    {
      
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
    //snprintf(buf,24,"Ch %d : %d   ",i,rssiRaw);
    //submit(buf);
  }
  //else
  //{
    snprintf(buf,24,"Ch %d: %d  ",i,rssiRaw);
  //  submit(buf);
  //}

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
  if(k){
    if(k == CONST_KEY_EXIT){
      global.mode = CONST_MODE_CHAT;
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
    case CONST_MODE_CHAT:
    default:
      mode_chat_loop(k);
  }
  redraw();
}
