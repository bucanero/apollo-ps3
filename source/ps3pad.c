#include <string.h>
#include <io/pad.h>

#define ANALOG_CENTER       0x78
#define ANALOG_THRESHOLD    0x68
#define ANALOG_MIN          (ANALOG_CENTER - ANALOG_THRESHOLD)
#define ANALOG_MAX          (ANALOG_CENTER + ANALOG_THRESHOLD)

padData paddata[MAX_PADS];

extern int idle_time;

//Pad stuff
static padInfo padinfo;
static padData padA[MAX_PADS];
static padData padB[MAX_PADS];

static int pad_time = 0, rest_time = 0, pad_held_time = 0, rest_held_time = 0;

int readPad(int port)
{
	ioPadGetInfo(&padinfo);
	int off = 2;
	int retDPAD = 0, retREST = 0;
	u8 dpad = 0, _dpad = 0;
	u16 rest = 0, _rest = 0;
	
	if(padinfo.status[port])
	{
		ioPadGetData(port, &padA[port]);
		
		if (padA[port].ANA_L_V < ANALOG_MIN)
			padA[port].BTN_UP = 1;
			
		if (padA[port].ANA_L_V > ANALOG_MAX)
			padA[port].BTN_DOWN = 1;
			
		if (padA[port].ANA_L_H < ANALOG_MIN)
			padA[port].BTN_LEFT = 1;
			
		if (padA[port].ANA_L_H > ANALOG_MAX)
			padA[port].BTN_RIGHT = 1;

		//new
		dpad = ((char)*(&padA[port].zeroes + off) << 8) >> 12;
		rest = ((((char)*(&padA[port].zeroes + off) & 0xF) << 8) | ((char)*(&padA[port].zeroes + off + 1) << 0));
		
		//old
		_dpad = ((char)*(&padB[port].zeroes + off) << 8) >> 12;
		_rest = ((((char)*(&padB[port].zeroes + off) & 0xF) << 8) | ((char)*(&padB[port].zeroes + off + 1) << 0));
		
		if (dpad == 0 && rest == 0)
			idle_time++;
		else
			idle_time = 0;
		
		//Copy new to old
		//memcpy(&_paddata[port].zeroes + off, &paddata[port].zeroes + off, size);
		memcpy(paddata, padA, sizeof(padData));
		memcpy(padB, padA, sizeof(padData));
		
		//DPad has 3 mode input delay
		if (dpad == _dpad && dpad != 0)
		{
			if (pad_time > 0) //dpad delay
			{
				pad_held_time++;
				pad_time--;
				retDPAD = 0;
			}
			else
			{
				//So the pad speeds up after a certain amount of time being held
				if (pad_held_time > 180)
				{
					pad_time = 2;
				}
				else if (pad_held_time > 60)
				{
					pad_time = 5;
				}
				else
					pad_time = 20;
				
				retDPAD = 1;
			}
		}
		else
		{
			pad_held_time = 0;
			pad_time = 0;
		}
		
		//rest has its own delay
		if (rest == _rest && rest != 0)
		{
			if (rest_time > 0) //rest delay
			{
				rest_held_time++;
				rest_time--;
				retREST = 0;
			}
			else
			{
				//So the pad speeds up after a certain amount of time being held
				if (rest_held_time > 180)
				{
					rest_time = 2;
				}
				else if (rest_held_time > 60)
				{
					rest_time = 5;
				}
				else
					rest_time = 20;
				
				retREST = 1;
			}
		}
		else
		{
			rest_held_time = 0;
			rest_time = 0;
		}
		
	}
	
	if (!retDPAD && !retREST)
		return 0;
	
	if (!retDPAD)
	{
		paddata[port].BTN_LEFT = 0;
		paddata[port].BTN_RIGHT = 0;
		paddata[port].BTN_UP = 0;
		paddata[port].BTN_DOWN = 0;
	}
	
	if (!retREST)
	{
		paddata[port].BTN_L2 = 0;
		paddata[port].BTN_R2 = 0;
		paddata[port].BTN_L1 = 0;
		paddata[port].BTN_R1 = 0;
		paddata[port].BTN_TRIANGLE = 0; 
		paddata[port].BTN_CIRCLE = 0;
		paddata[port].BTN_CROSS = 0;
		paddata[port].BTN_SQUARE = 0;
		paddata[port].BTN_SELECT = 0;
		paddata[port].BTN_L3 = 0;
		paddata[port].BTN_R3 = 0;
		paddata[port].BTN_START = 0;
	}
	
	return 1;
}
