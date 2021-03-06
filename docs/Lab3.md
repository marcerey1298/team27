
# Lab 3
[Home](./index.md)

Materials:  
  * Arduino Uno (2)
  * Nordic nRF24L01+ transceiver (2)
  * 2 Breakout Boards w/ Headers
  * Electret microphone
  * Phototransistor
  
The purpose of this lab is to integrate the individual components completed in the previous labs and milestones. The objectives of this milestone are as follows:

 * Have the robot navigate a small maze and map the maze by wirelessly communicating to a base station
 * Integrate the microphone circuit from Lab 2 to start the robot on a 660 Hz tone
 * Successfully avoiding other robots while ignoring decoys

## Testing the Radios:

We began ensuring the functionality of the RF24 transceivers by running the provided Getting Started sketch.  This sketch ensures that the transceivers are able to pass a timestamp to each other and print it to the Serial Monitor. These sample printouts are shown below.

SHOW SCREENSHOTS OF SERIAL PRINTOUTS FOR GETTING STARTED SKETCH

## Encoding the Maze and Updating the GUI:

We created the below 4X3 maze to demonstrate our robot's new capabilities:

SHOW DIAGRAM OF MAZE/PHOTO OF MAZE

Since the maximum payload for the transceivers is 32 bytes, we decided that a 3 byte encoding scheme would be efficient without restricting the base station's ability to interpret and process accurate information. We decided to package our information payload in a byte array, since the first byte is related specifically to wall information, while the last two bytes will be related to treasure information. A diagram of the encoding scheme of the first byte is shown below:

![Byte 0 Bitmap](./media/lab3/BitMap.PNG)

These payloads are transferred each time the robot enters a new square, and stored on the base station in a 9X9 2D array.  Despite the actual starting location of the robot, the GUI is updated and information is handled such that the robot begins in the 'Northwest' corner of the map, facing 'East'.  It is understood that the directions are ordinal, and do not have any relation to the Cardinal directions of the same names.

The transmission code is shown below:

The receiving code is shown below:
~~~ c
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
        if (!check_zeros()) {
          if (check_data()){
             decipher();
          }
          copy(data, data_before, 3);
          delay(20);
        }
      }
    }
}
~~~

The helper function *decipher* parses the received byte array into a 1D array of ints, such that its elements can be easily accessed through indexing in constant time according to the encoding scheme detailed above:

~~~ c
void decipher() {
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
}
~~~
The function check data checks the parity bit of the instruction in order to detect whether new data is received. The data is transmitted multiple times per square in order to account for possible dropped packets. The data is only parsed when the parity bit is flipped compared to the last received data in order to indicate that the robot is on a new square.

~~~ c
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
~~~

A video of the maze simulation and GUI updating is shown below:

<iframe width="560" height="315" src="https://www.youtube.com/embed/u13swiZ8vGE" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

## Audio Detection and Robot Activation

The second part of this lab was to integrate the microphone circuit that was built in Lab 2.  The purpose of this circuit, as expressed in the past, is to detect a 660 Hz tone and susequently activate the robot.  We implemented this circuit using a multiplexer to switch between the optical and audio circuits in order to save on analog pins beng used. The select bit starts low until the tone is heard, and then the select bit is switched high for the rest of the time to accomplish robot detection. The modifications to the FFT detection code are shown below and the code to start on the 660 Hz tone is shown below:

~~~c
void fft_detect() {
      cli();
    
      for (int i = 0 ; i < 512 ; i += 2) {
        fft_input[i] = analogRead(A5); // <-- NOTE THIS LINE
        fft_input[i + 1] = 0;
      }
    
      fft_window();
      fft_reorder();
      fft_run();
      fft_mag_log();
      sei();
        if (!start) {
          if (fft_log_out[3] > 70){
            l = l + 1;
          }
          else {
            l = 0;
          }
         if (l >= 10) {
           start = 1;
           drive_straight();
           digitalWrite(2, HIGH);  //flip MUX select bit
           //Serial.println("660 HURTS !!!!!");
          }
}
~~~

        

A video of the robot starting on the 660 Hz tone is shown below:

SHOW VIDEO OF ROBOT STARTING ON 660 HZ TONE

Moreover, in order to encode the maze information using the encoding scheme detailed above, information is recorded by reading the wall sensors and updating the byte array using the built-in Arduino functions relating to the byte type. The transmitting code from the robot is shown below:

SHOW CODE FOR TRANSMITTING INFO FROM ROBOT TO BASE STATION

In order to improve upon wall detection from Milestone 2, we decided to add a third wall sensor to the left side of the robot. Not only would this improve turning and decision making at intersections, but it reduces the number of turns needed in a square in order to map all sides. We also used a multiplexer to switch between the analog outputs of the three wall sensors, accounting for the fact that this switching between select bits occurs on the scale of nanoseconds, and shouldn't slow down our processing speed too much. This modified code is shown below:

SHOW WALL SENSOR CODE

A video of our robot navigating the test maze is shown below. The robot successfully stops upon detecting another robot and ignores decoys:

SHOW VID

A video of the robot wirelessly communicating the maze information to the base station and updating the GUI is shown below: 

SHOW VID


