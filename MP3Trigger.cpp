/*
MP3Trigger.cpp
@author David Wicks, additions by Carl Jensen
@url	sansumbrella.com

Supports Quiet Mode
Doesn't handle status report from the trigger at this point
*/

#include "MP3Trigger.h"

MP3Trigger::MP3Trigger()
{
	mDoLoop = false;
	mPlaying = false;
}

MP3Trigger::~MP3Trigger()
{
	s->flush();
	s = NULL;
	quickModeCallback = NULL;
}

void MP3Trigger::setup()
{
	//Only for HardwareSerial, remove this line for SofwareSerial use
	setup(&Serial);
}

void MP3Trigger::setup(HardwareSerial* serial)
//void MP3Trigger::setup(SoftwareSerial* serial)
{
	s = serial;
	s->begin(38400);
}

// 
// Looping functions
// 
void MP3Trigger::setLooping(bool doLoop, byte track)
{
	mDoLoop = doLoop;
	mLoopTrack = track;
	
	if(!mPlaying && mDoLoop)
	{
		loop();
	}
}

void MP3Trigger::setLoopingTrack(byte track)
{
	mLoopTrack = track;
}

void MP3Trigger::update()
{
	if( s->available() )
	{
		int data = s->read();
		
		if(char(data) == 'X')
		{
			if(mDoLoop)
			{	
				loop();
			} else
			{
				mPlaying = false;
			}
		} else if(char(data) == 'E')
		{
			mPlaying = false;
		} else if(char(data) == 'M')
		{
			byte reads = 0;
			byte retries = 0;
			byte i;
			while(reads < 3 && retries < 1000) {
				if(s->available()) {
					data = (byte) s->read();
					if(data) {
						for(i = 0; i < 8; i++) {
							if( (data >> i) & B00000001 ) {
								quickModeCallback(i + 8*(2-reads) + 1);
							}
						}
					}
					reads++;
				}
				else {
					retries++;
				}
			}
		}
	}
}

void MP3Trigger::loop()
{
	trigger(mLoopTrack);
}

void MP3Trigger::stop()
{
	mDoLoop = false;
	
	if(mPlaying)
	{
		play();
	}
}

// 
// Single-byte built-in functions
// 

void MP3Trigger::play()
{
	s->write('O');
	mPlaying = !mPlaying;
}

void MP3Trigger::forward()
{
	s->write('F');
}

void MP3Trigger::reverse()
{
	s->write('R');
}

//
// Built-in two-byte functions
// 

void MP3Trigger::trigger(byte track)
{
	s->write('t');
	s->write(track);
	mPlaying = true;
}

void MP3Trigger::play(byte track)
{
	s->write('p');
	s->write(track);
	mPlaying = true;
}

void MP3Trigger::setVolume(byte level)
{
	// level = level ^ B11111111;	//flip it around, so the higher number > higher volume
	s->write('v');
	s->write(level);
}

// 1 for on, 0 for off, callback function
void MP3Trigger::quietMode(boolean onoff, void (*callback)(int))
{
	s->write('Q');
	s->write( '0' + onoff );
	quickModeCallback = callback;
}

// 
// Response functions
// 

void MP3Trigger::statusRequest()
{
	s->println("MP3Trigger::statusRequest is not yet implemented");
	s->flush();
	s->write('S');
	s->write('1');
	delay(5);
	s->read();
	//will need to work on this one to make it useful
	// if (Serial.available() > 0)
	// {
	// 	// read the incoming byte:
	// 	int incomingByte = Serial.read();
	// 
	// 	// say what you got:
	// 	Serial.print("I received: ");
	// 	Serial.println(incomingByte, DEC);
	// }
}