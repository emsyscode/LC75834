/*
This code is not clean and far from perfect, that's just
a reference to extract ideas and adapt to your solution.
you can replace the BIN values with HEX... I leave it in BIN
because it is easier to relate the segment number with
  the position of the bit in BIN.
Of course, a library can be created for this purpose! But I won't 
take the time to do that, I'll leave it up to you!
*/

/*
 * Note: DD ... Direction data
 * •CCB address...............46H
 * •D1 to D136.................Display data (At the LC75834JE, the display data D33 to D36, D69 to D72, D105 to D108, D133 to D136 must be set to 0.
 * •P0 to P3......................Segment output port/general-purpose output port switching control data
 * •DR..............................1/2-bias drive or 1/3-bias drive switching control data
 * •SC...............................Segments on/off control data
 * •BU..............................Normal mode/power-saving mode control dataNo. 5597-8/18LC75834E, 75834W, 75834JE
 * 
 * 0, 0, 0, P0, P1, P2, P3, DR, SC, BU, DD, DD;
 */

//#include <Arduino.h>
//#include <stdio.h>
////#include <math.h>
//#include <stdbool.h>

void send_char(unsigned char a);
void send_data(unsigned char a);
void segments();
void buttonReleasedInterrupt();  

#define LCD_in 8  // This is the pin number 8 on Arduino UNO
#define LCD_clk 9 // This is the pin number 9 on Arduino UNO
#define LCD_CE 10 // This is the pin number 10 on Arduino UNO

//unsigned int numberSeg = 0;  // Variable to supporte the number of segment
//unsigned int numberByte = 0; // Variable to supporte the number byte 
unsigned int shiftBit=0;
unsigned int nBitOnBlock=0; // Used to count number of bits and split to 8 bits... (number of byte)
unsigned int nByteOnBlock=0; 
unsigned int sequencyByte=0x00;
byte Aa,Ab,Ac,Ad,Ae,Af,Ag;
byte blockBit =0x00;


// constants won't change. They're used here to set pin numbers:
//const int buttonPin = 7;  // the number of the pushbutton pin
const int ledPin = 12;    // the number of the LED pin

#define BUTTON_PIN 2 //Att check wich pins accept interrupts... Uno is 2 & 3
volatile byte buttonReleased = false;

// variables will change:
int buttonState = 0;  // variable for reading the pushbutton status

bool forward = false;
bool backward = false;
bool isRequest = true;
bool allOn=false;
bool cycle=false;
/*
#define BIN(x) \
( ((0x##x##L & 0x00000001L) ? 0x01 : 0) \
| ((0x##x##L & 0x00000010L) ? 0x02 : 0) \
| ((0x##x##L & 0x00000100L) ? 0x04 : 0) \
| ((0x##x##L & 0x00001000L) ? 0x08 : 0) \
| ((0x##x##L & 0x00010000L) ? 0x10 : 0) \
| ((0x##x##L & 0x00100000L) ? 0x20 : 0) \
| ((0x##x##L & 0x01000000L) ? 0x40 : 0) \
| ((0x##x##L & 0x10000000L) ? 0x80 : 0))
*/

//ATT: On the Uno and other ATMEGA based boards, unsigned ints (unsigned integers) are the same as ints in that they store a 2 byte value.
//Long variables are extended size variables for number storage, and store 32 bits (4 bytes), from -2,147,483,648 to 2,147,483,647.

//*************************************************//
void setup() {
  pinMode(LCD_clk, OUTPUT);
  pinMode(LCD_in, OUTPUT);
  pinMode(LCD_CE, OUTPUT);

  pinMode(13, OUTPUT);
  
// initialize the LED pin as an output:
//pinMode(ledPin, OUTPUT);
// initialize the pushbutton pin as an input:
//pinMode(buttonPin, INPUT);  //Next line is the attach of interruption to pin 2
pinMode(BUTTON_PIN, INPUT);

 attachInterrupt(digitalPinToInterrupt(BUTTON_PIN),
                  buttonReleasedInterrupt,
                  FALLING);

//Dont insert any print inside of interrupt function!!!
//If you run the search function, please active the terminal to be possible print lines,
//Other way the run will be blocked!
//
  Serial.begin(115200);
  
  /*CS12  CS11 CS10 DESCRIPTION
  0        0     0  Timer/Counter1 Disabled 
  0        0     1  No Prescaling
  0        1     0  Clock / 8
  0        1     1  Clock / 64
  1        0     0  Clock / 256
  1        0     1  Clock / 1024
  1        1     0  External clock source on T1 pin, Clock on Falling edge
  1        1     1  External clock source on T1 pin, Clock on rising edge
 */
  
// Note: this counts is done to a Arduino 1 with Atmega 328... Is possible you need adjust
// a little the value 62499 upper or lower if the clock have a delay or advnce on hours.

  digitalWrite(LCD_CE, LOW);
  delayMicroseconds(5);
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
}
void send_char(unsigned char a){
 unsigned char transmit = 15; //define our transmit pin
 unsigned char data = 170; //value to transmit, binary 10101010
 unsigned char mask = 1; //our bitmask
  data=a;
  // the validation of data happen when clk go from LOW to HIGH.
  // This lines is because the clk have one advance in data, see datasheet of sn74HC595
  // case don't have this signal instead of "." will se "g"
  digitalWrite(LCD_CE, LOW); // When strobe is low, all output is enable. If high, all output will be set to low.
  delayMicroseconds(5);
  digitalWrite(LCD_clk,LOW);// need invert the signal to allow 8 bits is is low only send 7 bits
  delayMicroseconds(5);
  for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
  digitalWrite(LCD_clk,LOW);// need invert the signal to allow 8 bits is is low only send 7 bits
  delayMicroseconds(5);
    if (data & mask){ // if bitwise AND resolves to true
      digitalWrite(LCD_in, HIGH);
      //Serial.print(1);
    }
    else{ //if bitwise and resolves to false
      digitalWrite(LCD_in, LOW);
      //Serial.print(0);
    }
    digitalWrite(LCD_clk,HIGH);// need invert the signal to allow 8 bits is is low only send 7 bits
    delayMicroseconds(5);
    //
    digitalWrite(LCD_CE, HIGH); // When strobe is low, all output is enable. If high, all output will be set to low.
  delayMicroseconds(5);
  }
}
// I h've created 3 functions to send bit's, one with strobe, other without strobe and one with first byte with strobe followed by remaing bits.
void send_char_without(unsigned char a){
 //
 unsigned char data = 0x00; //value to transmit, binary 10101010
 unsigned char mask = 1; //our bitmask
  data=a;
  //Serial.println(":");
  for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
  digitalWrite(LCD_clk, LOW);
  delayMicroseconds(5);
    if (data & mask){ // if bitwise AND resolves to true
      digitalWrite(LCD_in, HIGH);
      //Serial.print(1);
    }
    else{ //if bitwise and resolves to false
      digitalWrite(LCD_in, LOW);
      //Serial.print(0);
    }
    digitalWrite(LCD_clk,HIGH);// need invert the signal to allow 8 bits is is low only send 7 bits
    delayMicroseconds(5);
  }
}
void send_char_8bit_stb(unsigned char a){
 unsigned char data = 0x00; //value to transmit, binary 10101010
 unsigned char mask = 1; //our bitmask
 int i = -1;
  data=a;
  //Serial.println(":");
  digitalWrite(LCD_CE, LOW);
  delayMicroseconds(1);
  // This lines is because the clk have one advance in data, see datasheet of sn74HC595
  // case don't have this signal instead of "." will se "g"
      for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
       i++;
       digitalWrite(LCD_clk, LOW);
      delayMicroseconds(5);
        if (data & mask){ // if bitwise AND resolves to true
          digitalWrite(LCD_in, HIGH);
          //Serial.print(1);
        }
        else{ //if bitwise and resolves to false
          digitalWrite(LCD_in, LOW);
          //Serial.print(0);
        }
        digitalWrite(LCD_clk,HIGH);// need invert the signal to allow 8 bits is is low only send 7 bits
        delayMicroseconds(1);
            if (i==7){
            //Serial.println(i);
            digitalWrite(LCD_CE, HIGH);
            delayMicroseconds(2);
            }
      }
}
//
void allON(){
//Bit function: 0, 0, 0, P0, P1, P2, P3, DR, SC, BU, DD, DD;
 for(int i=0; i<4;i++){   // 
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          
          send_char_without(0B11111111);  send_char_without(0B11111111);  //   8:1     -16:9// 
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  24:17    -32:25// 
          send_char_without(0B00000000);    //  40:33    //the next switch send reamaining bits -41:48// 
              switch (i){ //Last 3 bits is "DD" data direction, and is used
                case 0: send_char_without(0B00000000); break;
                case 1: send_char_without(0B10000000); break;
                case 2: send_char_without(0B01000000); break;
                case 3: send_char_without(0B11000000); break;
              }
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
      }
}
void allOFF(){
//Bit function: 0, 0, 0, P0, P1, P2, P3, DR, SC, BU, DD, DD;
 for(int i=0; i<4;i++){   // 
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          
          send_char_without(0B00000000);  send_char_without(0B00000000);  //   8:1      -16:9// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  24:17    -32:25// 
          send_char_without(0B00000000);    //  40:33   //the next switch send reamaining bits -41:48//  
              switch (i){ //Last 3 bits is "DD" data direction, and is used
                case 0: send_char_without(0B00000000); break;
                case 1: send_char_without(0B10000000); break;
                case 2: send_char_without(0B01000000); break;
                case 3: send_char_without(0B11000000); break;
              }
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
      }
}
void searchOfSegments(){
// put your main code here, to run repeatedly:
  int group = 0x00;
  byte nBit =0x00;
  byte nMask = 0b00000001;
  unsigned int block =0;
  byte nSeg=0x00;
  Serial.println();
  Serial.println("We start the test of segments!");
  for(block=0; block<4; block++){  //This is the last 2 bit's marked as DD, group: 0x00, 0x01, 0x10, 0x11;
for( group=0; group<5; group++){   // Do until n bits 5*36 bits
        //for(int nBit=0; nBit<8; nBit++){
          for (nMask = 0b00000001; nMask>0; nMask <<= 1){
            Aa=0x00; Ab=0x00; Ac=0x00; Ad=0x00; Ae=0x00;
                  switch (group){
                    case 0: Aa=nMask; break;
                    case 1: Ab=nMask; break;//atoi(to integer)
                    case 2: Ac=nMask; break;
                    case 3: Ad=nMask; break;
                    case 4: Ae=nMask; break;
                  }
            
           nSeg++;
           if((nSeg >=0) && (nSeg<37)){
            blockBit=0;
            }
            if((nSeg >=37) && (nSeg<73)){
            blockBit=1;
            }
            if((nSeg >=73) && (nSeg<109)){
            blockBit=2;
            }
            if((nSeg >=109) && (nSeg<137)){
            blockBit=3;
            }
            if (nSeg >137){
              nSeg=0;
              group=0;
              block=0;
              break;
            }
          
      //This start the control of button to allow continue teste! 
                      while(1){
                            if(!buttonReleased){
                              delay(200);
                            }
                            else{
                              delay(15);
                               buttonReleased = false;
                               break;
                               }
                         }
               
                     segments();
            Serial.print(nSeg, DEC); Serial.print(", group: "); Serial.print(group, DEC);Serial.print(", BlockBit: "); Serial.print(blockBit, HEX);Serial.print(", nMask: "); Serial.print(nMask, BIN);Serial.print("   \t");
            Serial.print(Ae, HEX);Serial.print(", ");Serial.print(Ad, HEX);Serial.print(", ");Serial.print(Ac, HEX);Serial.print(", ");Serial.print(Ab, HEX);Serial.print(", ");Serial.print(Aa, HEX); Serial.print("; ");
            
            Serial.println();
            delay (400);  
                }         
           }        
      }
  }
void segments(){
//Bit function: 0, 0, 0, P0, P1, P2, P3, DR, SC, BU, DD, DD;
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75834 the message have first 5*8 bits (last byte is control Bit function: 0, 0, 0, P0, P1, P2, P3, DR, SC, BU, DD, DD;)
          send_char_without(Aa);  send_char_without(Ab);  //   1:8      -9:16// 
          send_char_without(Ac);  send_char_without(Ad);  //  17:24    -25:32// 
          send_char_without(Ae);  //  33:40    
          //The next switch finalize the burst of bits -41:48//  
              switch (blockBit){ //Last 2 bits is "DD" data direction, and is used to mark the 4 groups of 36 bits, 00, 01, 10, 11.                                 
                case 0: send_char_without(0B00000000); break; //Block 00
                case 1: send_char_without(0B10000000); break; //Block 01
                case 2: send_char_without(0B01000000); break; //Block 10
                case 3: send_char_without(0B11000000); break; //Block 11
              }
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); //                   
}
void testModeAllGroups(){
//Bit function: 0, 0, 0, P0, P1, P2, P3, DR, SC, BU, DD, DD;
Serial.print(Aa, HEX);Serial.print(Ab, HEX);Serial.print(Ac, HEX);Serial.print(Ad, HEX);Serial.print(Ae, HEX);Serial.print(Af, HEX);Serial.println(Ag, HEX);
//Serial.print(Ba, HEX);Serial.print(Bb, HEX);Serial.print(Bc, HEX);Serial.print(Bd, HEX);Serial.print(Be, HEX);Serial.print(Bf, HEX);Serial.println(Bg, HEX);
 for(int i=0; i<4;i++){   // 
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75834 the message have first 5*8 bits (last byte is control Bit function: 0, 0, 0, P0, P1, P2, P3, DR, SC, BU, DD, DD;)
          send_char_without(Aa);  send_char_without(Ab);  //   1:8      -9:16// 
          send_char_without(Ac);  send_char_without(Ad);  //  17:24    -25:32// 
          send_char_without(Ae);  send_char_without(Af);  //  33:40    -41:48//  
          send_char_without(Ag);  send_char_without(0B00000000);  //  49-56
              switch (i){ //Last 3 bits is "DD" data direction, and is used
                case 0: send_char_without(0B00000000); break;
                case 1: send_char_without(0B10000000); break;
                case 2: send_char_without(0B01000000); break;
                case 3: send_char_without(0B11000000); break;
              }
      // to mark the 3 groups of 57 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
      }
}
//
void portTest(){
//Bit function: 0, 0, 0, P0, P1, P2, P3, DR, SC, BU, DD, DD;
digitalWrite(LCD_CE, LOW); //
delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //   8:1      -16:9// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  24:17    -32:25// 
          send_char_without(0B00000000);    //  40:33  33 until 36 must be 0 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
//
void seg01_32_HI(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //   8:1      -16:9// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  24:17    -32:25// 
          send_char_without(0B00000000);    //  40:33  33 until 36 must be 0 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg37_72_HI(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75834 the message have first 5*8 bits more 1 byte control
          send_char_without(0B00000000);  send_char_without(0B00110100);  //  44:37   -52:45// 
          send_char_without(0B01100000);  send_char_without(0B00000111);  //  60:53   -68:61// 
          send_char_without(0B00000000);    //  76:69   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg73_102_HI(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  80:73     -88:81// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  96:89    -104:97// 
          send_char_without(0B00000000);    //  112:105 //105 to 108 must be 0  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg103_136_HI(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  116:109    -124:117// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  132:125    -139:132// 
          send_char_without(0B00000000);    //  140:133   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
//
void seg01_32_FOL(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //   8:1      -16:9// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  24:17    -32:25// 
          send_char_without(0B00000000);    //  40:33  33 until 36 must be 0 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg37_72_FOL(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75834 the message have first 5*8 bits more 1 byte control
          send_char_without(0B00000000);  send_char_without(0B00110100);  //  44:37   -52:45// 
          send_char_without(0B00000001);  send_char_without(0B10010110);  //  60:53   -68:61// 
          send_char_without(0B00000000);    //  76:69   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg73_102_FOL(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B01100000);  send_char_without(0B10000110);  //  80:73     -88:81// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  96:89    -104:97// 
          send_char_without(0B00000000);    //  112:105 //105 to 108 must be 0  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg103_136_FOL(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  116:109    -124:117// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  132:125    -139:132// 
          send_char_without(0B00000000);    //  140:133   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
//
void seg01_32_KS(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //   8:1      -16:9// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  24:17    -32:25// 
          send_char_without(0B00000000);    //  40:33  33 until 36 must be 0 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg37_72_KS(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75834 the message have first 5*8 bits more 1 byte control
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  44:37   -52:45// 
          send_char_without(0B00000000);  send_char_without(0B00100110);  //  60:53   -68:61// 
          send_char_without(0B00000000);    //  76:69   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg73_102_KS(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000101);  send_char_without(0B10100010);  //  80:73     -88:81// 
          send_char_without(0B00000101);  send_char_without(0B00000000);  //  96:89    -104:97// 
          send_char_without(0B00000000);    //  112:105 //105 to 108 must be 0  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg103_136_KS(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  116:109    -124:117// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  132:125    -139:132// 
          send_char_without(0B00000000);    //  140:133   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
//
void seg01_32_bar1(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //   8:1      -16:9// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  24:17    -32:25// 
          send_char_without(0B00000000);    //  40:33  33 until 36 must be 0 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg37_72_bar1(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75834 the message have first 5*8 bits more 1 byte control
          send_char_without(0B00000000);  send_char_without(0B00001000);  //  44:37   -52:45// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  60:53   -68:61// 
          send_char_without(0B00000000);    //  76:69   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg73_102_bar1(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  80:73     -88:81// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  96:89    -104:97// 
          send_char_without(0B00000000);    //  112:105 //105 to 108 must be 0  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg103_136_bar1(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B10000000);  send_char_without(0B00000000);  //  116:109    -124:117// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  132:125    -139:132// 
          send_char_without(0B00000000);    //  140:133   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}  
//
void seg01_32_bar2(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //   8:1      -16:9// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  24:17    -32:25// 
          send_char_without(0B00000000);    //  40:33  33 until 36 must be 0 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg37_72_bar2(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75834 the message have first 5*8 bits more 1 byte control
          send_char_without(0B00000000);  send_char_without(0B10000000);  //  44:37   -52:45// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  60:53   -68:61// 
          send_char_without(0B00000000);    //  76:69   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg73_102_bar2(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  80:73     -88:81// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  96:89    -104:97// 
          send_char_without(0B00000000);    //  112:105 //105 to 108 must be 0  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg103_136_bar2(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00001000);  send_char_without(0B00000000);  //  116:109    -124:117// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  132:125    -139:132// 
          send_char_without(0B00000000);    //  140:133   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}  
//
void seg01_32_bar3(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //   8:1      -16:9// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  24:17    -32:25// 
          send_char_without(0B00000000);    //  40:33  33 until 36 must be 0 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg37_72_bar3(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75834 the message have first 5*8 bits more 1 byte control
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  44:37   -52:45// 
          send_char_without(0B00001000);  send_char_without(0B00000000);  //  60:53   -68:61// 
          send_char_without(0B00000000);    //  76:69   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg73_102_bar3(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  80:73     -88:81// 
          send_char_without(0B00000000);  send_char_without(0B10000000);  //  96:89    -104:97// 
          send_char_without(0B00000000);    //  112:105 //105 to 108 must be 0  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg103_136_bar3(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  116:109    -124:117// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  132:125    -139:132// 
          send_char_without(0B00000000);    //  140:133   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
//
void seg01_32_bar4(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //   8:1      -16:9// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  24:17    -32:25// 
          send_char_without(0B00000000);    //  40:33  33 until 36 must be 0 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg37_72_bar4(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75834 the message have first 5*8 bits more 1 byte control
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  44:37   -52:45// 
          send_char_without(0B10000000);  send_char_without(0B00000000);  //  60:53   -68:61// 
          send_char_without(0B00000000);    //  76:69   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg73_102_bar4(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  80:73     -88:81// 
          send_char_without(0B00000000);  send_char_without(0B00001000);  //  96:89    -104:97// 
          send_char_without(0B00000000);    //  112:105 //105 to 108 must be 0  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg103_136_bar4(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  116:109    -124:117// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  132:125    -139:132// 
          send_char_without(0B00000000);    //  140:133   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}    
//
void seg01_32_bar5(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //   8:1      -16:9// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  24:17    -32:25// 
          send_char_without(0B00000000);    //  40:33  33 until 36 must be 0 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg37_72_bar5(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75834 the message have first 5*8 bits more 1 byte control
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  44:37   -52:45// 
          send_char_without(0B00000000);  send_char_without(0B00001000);  //  60:53   -68:61// 
          send_char_without(0B00000000);    //  76:69   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg73_102_bar5(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00001000);  //  80:73     -88:81// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  96:89    -104:97// 
          send_char_without(0B00000000);    //  112:105 //105 to 108 must be 0  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg103_136_bar5(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  116:109    -124:117// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  132:125    -139:132// 
          send_char_without(0B00000000);    //  140:133   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}  
//
void seg01_32_bar6(){
//Bit function: 0, 0, 0, P0, P1, P2, P3, DR, SC, BU, DD, DD;
digitalWrite(LCD_CE, LOW); //
delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //   8:1      -16:9// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  24:17    -32:25// 
          send_char_without(0B10000000);    //  40:33  33 until 36 must be 0 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000111);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg37_72_bar6(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75834 the message have first 5*8 bits more 1 byte control
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  44:37   -52:45// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  60:53   -68:61// 
          send_char_without(0B00000000);    //  76:69   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg73_102_bar6(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00001000);  send_char_without(0B00000000);  //  80:73     -88:81// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  96:89    -104:97// 
          send_char_without(0B00000000);    //  112:105 //105 to 108 must be 0  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg103_136_bar6(){
digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  116:109    -124:117// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  132:125    -139:132// 
          send_char_without(0B00000000);    //  140:133   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}  
//
void loop() {
long randNumber;
//buttonState = digitalRead(buttonPin);
//read the state of the pushbutton value:
//buttonState = digitalRead(buttonPin);

while(1){
      for(unsigned int c=0; c<3; c++){
        for(int w=0; w<4; w++){
          allON(); // All on
          delay(500);
          allOFF(); // All off
          delay(500);
        }
      seg01_32_HI();
      seg37_72_HI();
      seg73_102_HI();
      seg103_136_HI();
      delay(500);
      seg01_32_FOL();
      seg37_72_FOL();
      seg73_102_FOL();
      seg103_136_FOL();
      delay(500);
      seg01_32_KS();
      seg37_72_KS();
      seg73_102_KS();
      seg103_136_KS();
      delay(500);
      seg01_32_bar1();
      seg37_72_bar1();
      seg73_102_bar1();
      seg103_136_bar1();
      delay(300);
      seg01_32_bar2();
      seg37_72_bar2();
      seg73_102_bar2();
      seg103_136_bar2();
      delay(250);
      seg01_32_bar3();
      seg37_72_bar3();
      seg73_102_bar3();
      seg103_136_bar3();
      delay(200);
      seg01_32_bar4();
      seg37_72_bar4();
      seg73_102_bar4();
      seg103_136_bar4();
      delay(150);
      seg01_32_bar5();
      seg37_72_bar5();
      seg73_102_bar5();
      seg103_136_bar5();
      delay(100);
      seg01_32_bar6();
      seg37_72_bar6();
      seg73_102_bar6();
      seg103_136_bar6();
      delay(500);
      //searchOfSegments(); // This is the line of interrupt button to advance one step on the search of segments!
     }
}
 
//Uncomment this two lines to proceed identification of segments on this driver... adapt to other if necessary!
//Please don't forget of activation of Serial Monitor of IDE Arduino, to allow printing of running correctley,other way will block!
allOFF();
searchOfSegments(); //Uncomment this line if you want run the find segments
}

void buttonReleasedInterrupt() {
  buttonReleased = true; // This is the line of interrupt button to advance one step on the search of segments!
}
