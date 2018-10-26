#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>
#include <stdio.h>
#include <stdlib.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
//pipe addresses
const uint64_t pipes[2] = { 0x0000000048LL, 0x0000000049LL };
byte zero[3] = {0, 0, 0};
byte data_before [3];
byte data[3];
int data_array[24];
int x = 0;
int y = 0;
bool first = true;
int facing;

// Intialize strings to print
String xstring;
String ystring;
String west;
String east;
String north;
String south;
String robot;


void setup() {
  Serial.begin(9600);
  printf_begin();
  radio.begin();
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  radio.setChannel(0x50);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);    

  radio.startListening();
}
void loop() {
  // put your main code here, to run repeatedly:
    if ( radio.available() ) {
      bool done = false;
      while (!done) {
        done=radio.read(&data,sizeof(data));
        //if (!check_zeros()) {
          //if (check_data()){
             decipher();
          //}
          copy(data, data_before, 3);
          delay(20);
        //}
      }
    }
}

void decipher() {
// Parse byte array into 1D int array
  for (int i = 0; i<3; i++){
    for (int j = 0; j < 8; j++){
      int k = 8*i + j;
      data_array[k] = int(bitRead(data[i], 7-j));
     }
   }
   
   // Other Robot Information
   if (data_array[1]==1) {robot = "true";}
   else {robot = "false";}  

   // Wall information
   if (data_array[4]==1) {north = "true";}
   else {north = "false";}
   
   if (data_array[5]==1) {east = "true";}
   else {east = "false";}
   
   if (data_array[6]==1) {south = "true";}  
   else {south = "false";}
   
   if (data_array[7]==1) {west = "true";}
   else {west = "false";}

   // Start at 0 and mark first false
  if (first) {
    x = 0;
    y = 0;
    first = false;
   }
   
   xstring = String(x);
   ystring = String(y);


   Serial.println(ystring+","+xstring+","+"north="+north+","+"east="+east+","+"south="+south+","+"west="+west+","+"robot="+robot);

   if (data_array[2] == 0) {
     if (data_array[3] == 0) { facing = 0; }
     else { facing = 1; }
   }
  else {
    if (data_array[3] == 0) { facing = 2; }
    else { facing = 3; } 
  }

  switch(facing) {
    case 0: y--; break;
    case 1: x++; break;
    case 2: y++; break;
    case 3: x--; break;
  }

}

void copy(byte* src, byte* dst, int len) {
    memcpy(dst, src, sizeof(src[0])*len);
}

bool check_data(){
  if (first) {
    return true;
  }

  int last = int(bitRead(data_before[0],7));
  int now = int(bitRead(data[0],7));

  if (last != now) {
      return true;
  }
  else{
    return false; 
  }
}

bool check_zeros () {
 // Serial.println("checking if all zeros");
  byte zero = 0;
  for (int i=0; i<3; i++) {
    if (data[i] != zero) {
 //     Serial.println("not zero");
      return false;
    }
  }
 // Serial.println("all zero");
  return true;
}
