/*
Expressive Wearable - distracted noise
Mandeep Mangat
Wearable Computing, GDES-2013-001 
OCAD University
Created on [Feb 16]

Based on:
"Trigger" example sketch for LilyPad MP3 Player, by Mike Grusin, http://www.sparkfun.com
SdFAT library, William Greiman, http://code.google.com/p/sdfatlib/
SFEMP3Shield library, Bill Porter, http://www.billporter.info/
*/

// This sketch will play a specific audio file when one of the two trigger
// inputs (labeled T1 - T2) is momentarily grounded with a softswitch.

#include <SPI.h>            // Include SPI library...allows for talking to the SD card and MP3 chip
#include <SdFat.h>          // Include SdFat.H library ....access SD card file system
#include <SFEMP3Shield.h>   // Include SFEMP3Shield.h library ....MP3 decoder chip

const int TRIG1 = A0;  // Trigger input for audio 1
const int TRIG2 = A4;  // Trigger input for audio 2
int trigger[2] = {TRIG1, TRIG2};  // Trigger inputs arranged in an array.

const int EN_GPIO1 = A2; // Output = Amp enable + MIDI/MP3 mode select
const int SD_CS = 9;     // Ouput = Chip Select for SD card

// Create library objects:
SFEMP3Shield MP3player; 
SdFat sd;

boolean debugging = false; // from previous sketches - removed the coding for debugging status messages that would be sent to the seiral port.
boolean interrupt = false;  // Triggered file will play all the way to the end, without interruption from new triggered file.
boolean interruptself = false;  // Triggers on the same file will not start the file over.

char filename[2][13];  // The two filenames will be stored as arrays of characters.   // CHANGED [5] TO [2]


void setup()
{
  int x, index;
  SdFile file;
  byte result; // 8-bit unsigned number, from 0 to 255.
  char tempfilename[13];

  for (x = 0; x <= 1; x++) // Setting up the two trigger pins as inputs.
  {
    pinMode(trigger[x],INPUT); 
    digitalWrite(trigger[x],HIGH); //Internal pullup resistors.
  }
  
  pinMode(EN_GPIO1,OUTPUT); //pin selects mode the MP3 chip will start up in.
  digitalWrite(EN_GPIO1,LOW);  // MP3 mode (not MIDI) / amp off

  result = sd.begin(SD_CS, SPI_HALF_SPEED);  //Initializes the SD card, pin 9, at half speed.
  
  result = MP3player.begin(); //Start up the MP3 library

  sd.chdir("/",true);  // acessing SD card, looking for audio files, 
  while (file.openNext(sd.vwd(),O_READ))  //look through all files sequentially. 
  {
    file.getFilename(tempfilename); //retrieve filename.  
    if (tempfilename[0] >= '1' && tempfilename[0] <= '3')  // sorting filename, char '1' through '3'
    {
      index = tempfilename[0] - '1';  //Subtract char '1' to get an index of 0 through 2
      strcpy(filename[index],tempfilename); //data copied
    }
    else
    file.close();
  }
  MP3player.setVolume(25,25); //Setting the volume. 0 = loudest, 255 = off.
  digitalWrite(EN_GPIO1,HIGH);  // Turn on amplifier chip
  delay(5); 
}


void loop()
{
  int t;              // current trigger
  static int last_t;  // previous (playing) trigger
  int x;
  byte result;

  for(t = 1; t <= (debugging ? 3 : 5); t++)  //checking trigger inputs, when there is no connection the internal pullup resistors will keep them HIGH
  {
    if (digitalRead(trigger[t-1]) == LOW)  //reading pin, checking if it is LOW (ie, triggered).
    {
      x = 0;
      while(x < 50)  // Wait for trigger to return high for a solid 50ms
      {
        if (digitalRead(trigger[t-1]) == HIGH)
          x++;
        else
          x = 0;
        delay(1);
      } 
      if (filename[t-1][0] == 0)
      {
        if (interrupt && MP3player.isPlaying() && ((t != last_t) || interruptself))  //is file is playing, playback will not be triggered by a new file.
        {
         MP3player.stopTrack();
        }
        result = MP3player.playMP3(filename[t-1]);  //Play filename associated with the trigger number.  
        if (result == 0) last_t = t;  // Save playing trigger

      }
    }
  }
}
