#include <reg51.h>
#define dataport P2

sbit rs=P2^0;
sbit rw=P2^1;
sbit e=P2^2;

sbit DHT11=P1^7;                     //Connect DHT11 output Pin to P1.7 Pin 

int I_RH,D_RH,I_Temp,D_Temp,CheckSum;

int x;
unsigned char cmd;
int i,j;

//////////////////////////////////Delay program
void delay(unsigned int msec)
{
  for(i=0;i<msec;i++)
  for(j=0;j<1000;j++);
}
////////////////////////////////////20 Milisecond delay function
void delay20ms()		
{
	TMOD = 0x01;			/* Timer0 delay function */
	TH0 = 0xB8;				/* Load higher 8-bit in TH0 */
	TL0 = 0x0C;				/* Load lower 8-bit in TL0 */
	TR0 = 1;					/* Start timer0 */
	while(TF0 == 0);	/* Wait until timer0 flag set */
	TR0 = 0;					/* Stop timer0 */
	TF0 = 0;					/* Clear timer0 flag */
}


////////////////////////////////////30 Microsecond delay function
void delay30us()						
{
	TMOD = 0x01;			/* Timer0 mode1 (16-bit timer mode) */
	TH0 = 0xFF;				/* Load higher 8-bit in TH0 */
	TL0 = 0xE4;				/* Load lower 8-bit in TL0 */
	TR0 = 1;					/* Start timer0 */
	while(TF0 == 0);	/* Wait until timer0 flag set */
	TR0 = 0;					/* Stop timer0 */
	TF0 = 0;					/* Clear timer0 flag */
}
////////////////////////////// Function to send command to LCD
void lcd_cmd(unsigned char item) 
{ 
	////////////////////////////Sending upper nibble of command
	unsigned char a;
	a=item&0xf0;
	dataport=a;
  rs=0;
  rw=0;
  e=1;
  delay(1);
  e=0;
	////////////////////////////Sending lower nibble of command
	a=(item<<4)&0xf0;
	dataport=a;
	rs=0;
  rw=0;
  e=1;
  delay(1);
  e=0;
}
//////////////////////////////// Function to send data to LCD

void lcd_data(unsigned char item) 
{ 
	////////////////////////////Sending upper nibble of data_
  unsigned char a;
	a=item&0xf0;
	dataport=a;
  rs=1;
  rw=0;
  e=1;
  delay(1);
  e=0;
	////////////////////////////Sending lower nibble of data_
	a=(item<<4)& 0xf0;
	dataport=a;
	rs=1;
  rw=0;
  e=1;
  delay(1);
  e=0;  
}
//////////////////////////////// Function to send more than one character/data
void message(unsigned char *s)
{
	while(*s)
		lcd_data(*s++);
}
///////////////////////////LCD initialising command
void lcdinit()
{
	lcd_cmd(0x33); // These two commands (0x33 and ox32) are used in 4 bit mode of LCD 
	lcd_cmd(0x32);
	lcd_cmd(0x28); //4 bit mode of LCD
	lcd_cmd(0x06); //display from left to right
	lcd_cmd(0x0C); //display ON, cursor hide
}
///////////////////////////Microcontroller send request
void Request()		
{
	DHT11 = 0;					//set DH11 pin low
	delay20ms();				//wait for 20ms
	DHT11 = 1;					//set DH11 pin High
}
//////////////////////////Function for Receive response from DHT11
void Response()		
{
	while(DHT11==1);		//set DH11 pin High
	while(DHT11==0);		//set DH11 pin low
	while(DHT11==1);		//set DH11 pin High
}	
////////////////////////// Function to Receive data(0,1) from DH11 
int Receive_data()	
{
	int q,c=0;							///q is used to process 8 bit of data at a time and 'c' is used to store 0 or 1 
	for (q=0; q<8; q++)
	{
		while(DHT11==0);			// check received bit 0 or 1
		delay30us();
		if(DHT11 == 1)				//If high pulse is greater than 30ms
		c = (c<<1)|(0x01);		//Then its logic HIGH i.e c=1
		else		              //otherwise its logic LOW i.e c=0
		c = (c<<1);
		while(DHT11==1);
	}
	return c;
}


///////////////////////////Main function
void main()
{   
		unsigned int a, b, c, d;
		lcdinit();
	  lcd_cmd(0x01);
    lcd_cmd(0x80);
		message("Initialising");
		lcd_cmd(0x8c);
		lcd_data('.');
		delay(50);
		lcd_cmd(0x8d);
		lcd_data('.');
		delay(50);
		lcd_cmd(0x8e);
		lcd_data('.');
		delay(50);
		lcd_cmd(0x8f);
		lcd_data('.');
		delay(50);
		lcd_cmd(0x01);	
  while(1)
	{  
		lcdinit();
		Request();	//send start pulse 
		Response();	//receive response 
		I_RH=Receive_data();			//store first eight bit in I_RH 
		D_RH=Receive_data();			//store next eight bit in D_RH 
		I_Temp=Receive_data();		//store next eight bit in I_Temp
		D_Temp=Receive_data();		//store next eight bit in D_Temp 
		CheckSum=Receive_data();	//store next eight bit in CheckSum
		
		if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)    // check wheather data is valid or invalid
		{
			lcd_cmd(0x80);
			message("  Invalid Data  ");
			lcd_cmd(0x01);	
		}
		
		else
		{
			lcd_cmd(0x01);
			//////////////////////////////To display humidity value
			lcd_cmd(0x80);
			message("Humidity=");			
			a=(I_RH/10)+48; 					//10th digit of humidity is stored in a
			b=(I_RH%10)+48;					  // Unit digit of humidity is store in b
			lcd_cmd(0x89);
			lcd_data(a);
			lcd_cmd(0x8A);
			lcd_data(b);
			lcd_cmd(0x8B);
			lcd_data('%');		
			//////////////////////////////To display temp. value
			lcd_cmd(0xc0);
			message("  Temp. =");	
			c=(I_Temp/10)+48;					//10th digit of humidity is stored in c
			d=(I_Temp%10)+48;					// Unit digit of humidity is store in d
			lcd_cmd(0xc9);
			lcd_data(c);
			lcd_cmd(0xcA);
			lcd_data(d);
			lcd_cmd(0xcB);
			lcd_data('C');	
		}		
		delay(1000);
  } 
}