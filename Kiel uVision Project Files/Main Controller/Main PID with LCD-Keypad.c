/*
 * Title: Servo Controller for DC Motors
 * Author: Osama Mazhar
 * Date: 28th December 2013
 * Email: osamazhar@yahoo.com
 */
 
#include <reg51.h>

#define KEYPAD P1												
#define ldata P0 											

sbit rs = P3^5;
sbit rw = P3^6;
sbit en = P3^7;
sbit busy = P0^7;
sbit Clock = P3^0;
sbit Status = P3^1;


void lcdready();
void lcdcmd(unsigned char);
void lcddata(unsigned char);
void MSDelay(unsigned int);
unsigned char KeypadRead();
unsigned int GetInput();
void inttoLCD(unsigned int);

unsigned char code keypad[4][4] = {'7','8','9','/','4','5','6','X','1','2','3','-','.','0','=','+'};

unsigned int cnt;

void Encoderpulse() interrupt 0
	{
	 cnt++;
	}

void main()
	{
	 unsigned int actual_position, previous_error; 
	 unsigned char dt;
	 float Ki, Kp, I, Prop, D, Kd, error, setpoint, PWM;
	 Status = 0;
	 KEYPAD = 0xF0;											// make higher bits of keypad port that is column bits input and low row bits outputs
	 P2 = 0x00;
	 Clock = 0;
	 lcdcmd(0x38);											// 2 lines and 5X7 matrix
	 lcdcmd(0x0C);											// Display ON, Cursor Blinking = 0x0E But this command is Display ON, Cursor OFF
	 lcdcmd(0x01);											// Clear Display Screen
	 lcdcmd(0x80);											// Force Cursor to the beginning of the First Line
	 lcdcmd(0x06);
	 IE = 0x81;
	 TCON = 0x01;
	 setpoint = GetInput();
	 I = 0;
	 dt = 5;
	 Ki = 0.00001;
	 Kp = 5;
	 Kd = 1;
	 error = 0;
	 while(1)														//2 MC
		{
		 actual_position = cnt;
		 previous_error = error;
		 error = setpoint - actual_position;
		 
		 /**********Propotional Controller*************/

		 Prop = Kp * ((error * 255) / setpoint);	
		
		 //this error value is actually whats determining how much PWM should the controller generate.
		 //this is being divided by variable "setpoint" and multiplied by 255 inorder to scale the max 
		 //PWM to 255. As the other controller generating PWM works on the range of 0-255 for PWM.
		 //For faster response increase Kp and for smaller use smaller Kp
		 
		 if(Prop > 255)			
		 Prop = 255;				

		 // For max speed, when Kp is increase and fast response is required, max PWM is set until the 
		 // error value becomes so small that it forces Prop/PWM to be less than 255 following actual PID graph.
		 
		 else if(Prop < 1)					
		 		goto Stop;
		 
		 // this prevents Prop to become negative as Prop is a float variable, this will ultimately forces
		 // controller to an undetermined condition. So this is the solution :p
		 	
		 /***************Integral Controller**************/

		 MSDelay(dt);
		 I = I + Ki * error * dt;

		 /**********Differential Controller*************/

		 D = Kd * (error - previous_error) / dt;


		 lcdcmd(0xC0);								// for development purposes
		 inttoLCD(PWM); 							// same
		 inttoLCD(cnt);								// same
		 PWM = Prop + I + D;
		 if(PWM > 255)
		 	PWM = 255;
		 P2 = PWM;										// transfering the value of PWM to other controller
		 
		}							

		// stop routine when the target value is achieved by the Propotional Controller
		// this routine may change when Integral and Derivative controllers will be implemented.
		
Stop: lcdcmd(0xC0);									
	  Prop = 0;										
	  inttoLCD(Prop); 
	  P2 = 0;
	  while(1);
	}																// main braces

unsigned int GetInput()
	{
	 unsigned char L, cnt, Key[5];
	 unsigned int Entry = 0;
	 cnt = 0;
	 do
		{
Re:		 Key[cnt] = KeypadRead();
		 L = Key[cnt];
		 if(L == '+' | L == '-')		  // my beloved sryinge pump routine to re-enter data if mistakenly entered wrong
		 	goto Re;
		 if(L == '=')
		 	continue;
		 lcddata(Key[cnt]);
		 Key[cnt] = Key[cnt] - 48;
		 cnt++;

		}
	 while(L != '=' & cnt != 5);
	 switch(cnt)
	 	{
		 case(1):
		 	{
			 Entry = Key[0];
			 break;
			}
		 case(2):
		 	{
			 Entry = (Key[0] * 10) + Key[1];
			 break;
			}
		 case(3):
		 	{
			 Entry = (Key[0] * 100) + (Key[1] * 10) + Key[2];
			 break;
			}
		 case(4):
		 	{
			 Entry = (Key[0] * 1000) + (Key[1] * 100) + (Key[2] * 10) + Key[3];
			 break;
			}
		 case(5):
		 	{
			 Entry = (Key[0] * 10000) + (Key[1] * 1000) + (Key[2] * 100) + (Key[3] * 10) + Key[4];
			 break;
			}
		}
	 return Entry;
	}

void inttoLCD(unsigned int value)
	{
	 unsigned int x, y, z, d[5];
	 char l;
	 x = value / 10; 											// => 6553
	 d[0] = (value % 10) + 48; 						// => 6 (LSD) *
	 d[1] = (x % 10) + 48;	 							// => 3 *
	 y = x / 10; 													// => 655
	 d[2] = (y % 10) + 48; 								// => 5 *
	 z = y / 10; 													// => 65
	 d[3] = (z % 10) + 48;								// => 5 *
	 d[4] = (z / 10) + 48;								// => 6 (MSD) *
	 if(d[4] == 48 & d[3] == 48 & d[2] == 48 & d[1] == 48)
		{
		  lcddata(d[0]);
		  for(l=0; l<4; l++)
		 	 lcddata(' ');
		}
	 if(d[4] == 48 & d[3] == 48 & d[2] == 48 & d[1] != 48)
		{
		 for(l=1; l>=0; l--)
	 	 	lcddata(d[l]);
		 for(l=0; l<3; l++)
		 	 lcddata(' ');
		}
	 if(d[4] == 48 & d[3] == 48 & d[2] != 48)
		{
		 for(l=2; l>=0; l--)
	 	 	lcddata(d[l]);
		 for(l=0; l<2; l++)
		 	 lcddata(' ');
		}
	 if(d[4] == 48 & d[3] != 48)
	 	{
		 for(l=3; l>=0; l--)
	 	 	lcddata(d[l]);
		 lcddata(' ');
		}
	 if(d[4] != 48)
	 	{
		 for(l=4; l>=0; l--)
		 lcddata(d[l]);
		}
	 }	

void MSDelay(unsigned int count) 
	{
     unsigned int i;		       		
     while(count)
		{
         i = 122; 
		 while(i>0)
		 	i--;
         count--;
    	}
	}

void lcdready()
	{														
		busy = 1;	  										// make the busy pin an input
		rs = 0;
		rw = 1;
		while(busy == 1)								// wait here for busy flag
			{
				en = 0;										  // strobe the enable pin
				MSDelay(1);
				en = 1;
			}
		return;					
	 }
	 	  
void lcdcmd(unsigned char value)
	{
		lcdready();
		ldata = value;
		rs = 0;
		rw = 0;
		en = 1;
		MSDelay(1);
		en = 0;
		return;
	}


void lcddata(unsigned char value)
	{
		lcdready();
		ldata = value;
		rs = 1;
		rw = 0;
		en = 1;
		MSDelay(1);
		en = 0;
		return;
	}

unsigned char KeypadRead()
	{
		unsigned char colloc, rowloc; 		
		do
		   {												
		   KEYPAD = 0xF0;									// ground all rows at once
		   colloc = KEYPAD;								// read the port for columns
		   colloc &= 0xF0;								// mask row bits
		   }
		while(colloc != 0xF0);						// check until all keys are released
		
		do
		   {
		  	 do
		   		{
				MSDelay(8);  									// call delay
				colloc = KEYPAD;							// see if any key is pressed
				colloc &= 0xF0;								// mask unsused bits
				}
		 	 while(colloc == 0xF0);					// keep checking for keypress

			MSDelay(8);											// call delay for debounce
			colloc = KEYPAD;								// read columns
			colloc &= 0xF0;									// mask unused bits
			}
			while(colloc == 0xF0);					// wait for keypress

			while(1)
				{
					KEYPAD &= 0xF0;							// masking row bits
					KEYPAD |= 0x0E;							// now ground row 0 0E = 00001110b ORing won't affect column data
					colloc = KEYPAD;						// read columns
					colloc &= 0xF0;							// mask row bits
					if(colloc != 0xF0)					// column detected
						{
							rowloc = 0;							// save row location
							break;
						}
					KEYPAD &= 0xF0;
					KEYPAD |= 0x0D;							// ground row 1 0D = 00001101b  ORing won't affect column data
					colloc = KEYPAD;						// read columns
					colloc &= 0xF0;							// mask row bits
					if(colloc != 0xF0)					// column detected
						{
							rowloc = 1;							// save row location
							break;
						}
					KEYPAD &= 0XF0;
					KEYPAD |= 0x0B;							// ground row 2 0B = 00001011b
					colloc = KEYPAD;						// read columns
					colloc &= 0xF0;							// mask row bits
					if(colloc != 0xF0)					// column detected
						{
							rowloc = 2;							// save row location
							break;
						}
					KEYPAD &= 0XF0;
					KEYPAD |= 0x07;							// ground row 3 07 = 00000111b
					colloc = KEYPAD;						// read columns
					colloc &= 0xF0;							// mask row bits
					if(colloc != 0xF0)					// column detected
					rowloc = 3;									// save row location
					break;
				}
				
		// check columns and send result to LCD
				
		if(colloc == 0xE0)								//0E = 00001110
			return keypad[rowloc][0];
		else if(colloc == 0xD0)						//0D = 00001101
			return keypad[rowloc][1];
		else if(colloc == 0xB0)						//0B = 00001011
			return keypad[rowloc][2];
		else
			return keypad[rowloc][3];
		}