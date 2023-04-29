#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define ALDL_PIN 2
#define BUFSIZ 64

#define PROM1 11
#define PROM2 235

// Number of bytes per ALDL frame (19 for a 1227747) + 1 header sync + 1 trailer sync + 1 for a  null
#define FRAMESIZ 22

// Number of bits per ALDL byte, and the total if they are all set to 1
#define NBITS 8
#define FULLBITS 511

// Minimum microsecs for a complete packet
#define MIN_PACKET_TIME 5915

// Maximum microsecs for a complete packet
#define MAX_PACKET_TIME 5925

// Approx "start bit" max microsecs when transmitting a "0"
#define MIN_LENGTH_0 360
#define MAX_LENGTH_0 370

// Approx "start bit" min microsecs when transmitting a "1"
#define MIN_LENGTH_1 1850
#define MAX_LENGTH_1 1899

// Input parsing
short nbit = 0;
unsigned short byt = 0;
unsigned short nbyt = 0;
unsigned char data[BUFSIZ];
unsigned char * pdata;
unsigned int now = 0;
unsigned int prev = 0;
unsigned int interval = 0;
bool synced = false;

// Processing
unsigned short syncs = 0;
unsigned char frame[BUFSIZ];
unsigned char * pframe;
float h2o = 0;
float tps = 0;
float iac = 0;
float mapx = 0;
float vel = 0;
float cli = 0;
float o2v = 0;
float bat = 0;
float o2x = 0;
float rpm = 0;
unsigned long cycles = 0;

// Output for LCD
char obuf1[BUFSIZ];
char obuf2[BUFSIZ];
char obuf3[BUFSIZ];
char obuf4[BUFSIZ];
LiquidCrystal_I2C lcd(0x27, 20, 4);


void setup() {
    lcd.init();
    lcd.backlight();
    lcdx("1227747\0","Syncing...\0","\0","\0");
    Serial.begin(9600);
    // Clear the buffers
    memset(&data,0,sizeof(data));
    memset(&frame,0,sizeof(frame));
    pdata = data;
    pframe = frame;
    pinMode(ALDL_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(ALDL_PIN), isr, CHANGE);
    // Some output
    Serial.println("# OK here we go...");
  
  // test values
  for (int i=0; i<FRAMESIZ; i++){
    *(pframe + i) = 99;
}
  
}


void lcdx(char * p1, char * p2, char * p3, char * p4){
 
    lcd.setCursor(0,0);
    lcd.print(p1);
    lcd.setCursor(0,1);
    lcd.print(p2);
    lcd.setCursor(0,2);
    lcd.print(p3);
    lcd.setCursor(0,3);
    lcd.print(p4);
 }

/**
 * This runs anytime the state on pin ALDL_PIN changes. 
 * We will examine the time between 
 */
void isr() {

    now = micros();
    interval = now - prev;
    //Serial.println("# ?");

    // eliminate noise
    if (interval < MIN_LENGTH_0) {
        // Too short to be a bit
        return;
    }

    if (interval > MAX_LENGTH_1) {
        // Too long to be a bit
        prev = now;
        return;
    }
    
    // To get to here, its a bit but we dont know which yet. Bitshift regardless.
    byt = byt << 1;
    // Add a 1 if the new bit is a 1
    if (interval <= MAX_LENGTH_1 && interval >= MIN_LENGTH_1) {
        //Serial.println("# 1");
        byt += 1;
    } else {
        //Serial.println("# 0");
    }
    
    // restrict byt to max 9 bits
    byt = byt & 511;
    //Serial.println(byt, BIN);

    // a train of 9x1 "111111111" means a sync character; flush the buffers.
    if (byt==511){
        memset(&frame,0,sizeof(frame));
        memcpy(&frame,&data,sizeof(frame));
        memset(pdata,0,sizeof(data));
        synced=true;
        byt=0;
        nbyt=0;
        nbit=-1;
    }

    // once synced we can start counting bits and filling the buffers
    if (synced){
      if (nbit == NBITS){
        *(pdata + nbyt)=byt;
        nbit=-1;
        nbyt++;
        if (nbyt>FRAMESIZ){
          nbyt=0;
        }
      }
    }
    nbit++;

    prev = now;
}



char * flags(unsigned char byt1, unsigned char byt2, unsigned char byt3){
  static char flags[101];
  memset(&flags,0,sizeof(flags));
  char * p = flags;   
  if (byt1&128) { strcpy(p, "PU "); p+=3;   }
  if (byt1&64)  { strcpy(p, "ES "); p+=3;   }
  if (byt1&32)  { strcpy(p, "RE "); p+=3;   }
  if (byt1&16)  { strcpy(p, "FA "); p+=3;   }
  if (byt1&8)   { strcpy(p, "DI "); p+=3;   }
  if (byt1&4)   { strcpy(p, "AL "); p+=3;   }
  if (byt1&2)   { strcpy(p, "1s "); p+=3;   }
  if (byt1&1)   { strcpy(p, "ID "); p+=3;   }

  if (byt2&128) { strcpy(p, "CL "); p+=3;   }
  if (byt2&64)  { strcpy(p, "LR "); p+=3;   }
  if (byt2&32)  { strcpy(p, "BA "); p+=3;   }
  if (byt2&16)  { strcpy(p, "43 "); p+=3;   }
  if (byt2&8)   { strcpy(p, "AS "); p+=3;   }
  if (byt2&4)   { strcpy(p, "OH "); p+=3;   }
  if (byt2&2)   { strcpy(p, "RI "); p+=3;   }
  if (byt2&1)   { strcpy(p, "CL "); p+=3;   }
  
  if (byt3&128) { strcpy(p, "AR "); p+=3;   }
  if (byt3&64)  { strcpy(p, "AD "); p+=3;   }
  if (byt3&32)  { strcpy(p, "O3 "); p+=3;   }
  if (byt3&16)  { strcpy(p, "TC "); p+=3;   }
  if (byt3&8)   { strcpy(p, "NE "); p+=3;   }
  if (byt3&4)   { strcpy(p, "#4 "); p+=3;   }
  if (byt3&2)   { strcpy(p, "FJ "); p+=3;   }
  if (byt3&1)   { strcpy(p, "AC "); p+=3;   }

 //truncate
  p = flags;
  *(p+21)=0;
  //Serial.println(flags);
  return flags;
} 
           
char * dtcs(unsigned char byt1, unsigned char byt2, unsigned char byt3){
  static char dtcs[101];
  memset(&dtcs,0,sizeof(dtcs));
  char * p = dtcs;
  if (byt1&128) { strcpy(p, "24 "); p+=3;   }
  if (byt1&64)  { strcpy(p, "23 "); p+=3;   }
  if (byt1&32)  { strcpy(p, "22 "); p+=3;   }
  if (byt1&16)  { strcpy(p, "21 "); p+=3;   }
  if (byt1&8)   { strcpy(p, "15 "); p+=3;   }
  if (byt1&4)   { strcpy(p, "14 "); p+=3;   }
  if (byt1&2)   { strcpy(p, "13 "); p+=3;   }
  if (byt1&1)   { strcpy(p, "12 "); p+=3;   }
  
  if (byt2&128) { strcpy(p, "42 "); p+=3;   }
  if (byt2&64)  { strcpy(p, "41 "); p+=3;   }
  if (byt2&32)  { strcpy(p, "35 "); p+=3;   }
  if (byt2&16)  { strcpy(p, "34 "); p+=3;   }
  if (byt2&8)   { strcpy(p, "33 "); p+=3;   }
  if (byt2&4)   { strcpy(p, "32 "); p+=3;   }
  if (byt2&2)   { strcpy(p, "31 "); p+=3;   }
  if (byt2&1)   { strcpy(p, "25 "); p+=3;   }
  
  if (byt3&128) { strcpy(p, "55"); p+=3;   }
  if (byt3&64)  { strcpy(p, "54"); p+=3;   }
  if (byt3&32)  { strcpy(p, "53"); p+=3;   }
  if (byt3&16)  { strcpy(p, "52"); p+=3;   }
  if (byt3&8)   { strcpy(p, "51"); p+=3;   }
  if (byt3&4)   { strcpy(p, "45"); p+=3;   }
  if (byt3&2)   { strcpy(p, "44"); p+=3;   }
  if (byt3&1)   { strcpy(p, "43"); p+=3;   }

 //truncate
  p = dtcs;
  *(p+21)=0;
  //Serial.println(dtcs);
  return dtcs;
}


void loop(){

  // parse the buffer once every 500ms or so
  if (++cycles%10000==0){
    // user the PROM ID and realistic BATT range to validate
    if (*(pframe+1)==PROM1 && *(pframe+2)==PROM2 && *(pframe+15)>80 && *(pframe+15)<150) {
 
      h2o = 0.5 + ((231.0 - *(pframe + 4)) * 0.525) ;       // test == 231-99 *.025 == 69.3C
      tps = 0.5 + (*(pframe + 8) * 0.0196) * 100;               // test == 99 * .0196   == 1.94V
      iac = 0.5 + *(pframe + 3);                           // test == 99  
      mapx= 0.5 + (*(pframe + 6) * 0.0196) * 100;               // test == 99 * .0196   == 1.94
      vel = 0.5 + *(pframe + 5);                           // test == 99 
      cli = 0.5 + *(pframe + 9);                          // test == 99 
      o2v = 0.5 + *(pframe + 10);                          // test == 99 
      bat = 0.5 + (float)*(pframe + 15) * 0.1 * 10;             // test == 99 * .1      == 9.9V
      o2x = 0.5 + *(pframe + 19);                          // test == 99
      rpm = 0.5 + *(pframe + 7) * 25;                      // test == 99 * 25      == 2475
  
      for (int i=0; i<FRAMESIZ; i++){
        Serial.print(*(pframe+i));
        Serial.print(',');
      }
      Serial.println("#");
      
       sprintf(obuf1, "%3dC %3dv %3d' %3d>\0", (int)h2o, (int)tps, (int)iac, (int)vel);
       sprintf(obuf2, "%3d^ %3dV %4dR %3dx\0", (int)mapx, (int)bat, (int)rpm, (int)o2x);
       sprintf(obuf3, "%s\0",  flags(*(pframe + 0),*(pframe + 14),*(pframe + 16) ) );
       sprintf(obuf4, "%s\0",  dtcs(*(pframe + 11),*(pframe + 12),*(pframe + 13) ) );
    
       lcdx(obuf1,obuf2,obuf3,obuf4);
    }
    
  }
}

