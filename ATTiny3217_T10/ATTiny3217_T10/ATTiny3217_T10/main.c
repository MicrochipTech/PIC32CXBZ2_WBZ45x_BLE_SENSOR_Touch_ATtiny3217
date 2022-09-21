#include <atmel_start.h>
#include "led_driver.h"
#include <math.h>
extern volatile uint8_t measurement_done_touch;

int button_value,on_off_button=0,i=0;
int prev_button_val[4],status[4],curr_button_val[4];                        //Button variables
int scroller[3],SliderData[3],prev_slider_val[3],curr_slider_val[3];       //Slider variables
int button_led[4]={128,64,32,16};
int slider_led[8]={128,192,224,240,248,252,254,255};
uint16_t rgb_data;
char read_data_buffer[15];



#define LED_OFF 0
#define COMPARE    "B_On:0x"
#define COMPARE_SIZE   7

#define TOUCH_DATA_CR_POS   11
#define TOUCH_DATA_NL_POS   12

#define DATA_SHIFT_POS_1 12
#define DATA_SHIFT_POS_2 8
#define DATA_SHIFT_POS_3 4


int main(void)
{
	/* Initializes MCU, drivers and middle ware */
	atmel_start_init();
	#if ENABLE_LED== 1u
	init_led_driver();
	led_reset();
	#endif

	/* Replace with your application code */
	while (1)
	{
		touch_process();
		rgb_button(3);                   //ON_OF BUTTON
		rgb_button(0);					 //RED BUTTON
		rgb_button(1);                   //GREEN BUTTON
		rgb_button(2);                   //BLUE BUTTON
		read_BLE_data();
		
	}
}

void read_BLE_data(void)
{
	char rx_data='0';
	static char read_data_buffer[15];
	//static char read_data_buffer[15];
	while(USART_0_is_rx_ready())
	{
		rx_data= USART_0_get_data();
		if(rx_data=='B')
		{
			i=0;
		}
		else if(rx_data=='\n')
		{
			write_data(read_data_buffer);
			i=0;
		}
		read_data_buffer[i]=rx_data;
		i++;
	}
}



void write_data(char *buffer)                             
{
	uint8_t  r,g,b,button_state;
	uint16_t rgb_val;
	
	if(memcmp(buffer,COMPARE,COMPARE_SIZE) == 0)
	{
		if( *(buffer+TOUCH_DATA_CR_POS) == '\r' )
		{
			*(buffer+TOUCH_DATA_CR_POS) = 0;
			rgb_val = strtoul(buffer+COMPARE_SIZE, NULL, 16);
			
			r=((rgb_val & 0xF000)>>DATA_SHIFT_POS_1);
			g=((rgb_val & 0x0F00)>>DATA_SHIFT_POS_2);
			b=((rgb_val & 0x00F0)>>DATA_SHIFT_POS_3);
			button_state=(rgb_val & 0x000F);
			rgb_data=((button_state<<DATA_SHIFT_POS_1)|(r<<DATA_SHIFT_POS_2)|(g<<DATA_SHIFT_POS_3)|b);
			scroller[0]=slider_led[r-1];
			scroller[1]=slider_led[g-1];
			scroller[2]=slider_led[b-1];
			if(button_state==5)
			{
				led_gpio_update(button_led[3],LED_BUTTON);
				led_gpio_update(LED_OFF,LED_SCROLLER);
			}
			else
			{
				led_gpio_update(LED_OFF,LED_BUTTON);
				led_gpio_update(LED_OFF,LED_SCROLLER);
			}
		}
	}
}



void rgb_button(int i)
{
	uint8_t b1 = get_sensor_state(i) & KEY_TOUCHED_MASK;
	if(b1!=0)
	{
		button_value = i+1;
		prev_button_val[i]=curr_button_val[i];
		curr_button_val[i]=1;
	}
	else
	{
		curr_button_val[i]=0;
	}
	if((button_value==1 && i==0)|(button_value==2 && i==1)|(button_value==3 && i==2))
	{
		slider(i);
	}
	if((prev_button_val[i]==0 && curr_button_val[i]==1))
	{
		if(button_value==4 && on_off_button==1)
		{
			on_off_button=0;
		}
		else if(button_value==4 && on_off_button==0)
		{
			on_off_button=1;
		}
		else
		{
			on_off_button=1;
			status[3]=1;
		}
		touch_detected(i);
	}
	else if((prev_button_val[i]==1 && curr_button_val[i]==0))
	{
		prev_button_val[i]=0;
		curr_button_val[i]=0;     //touch released
	}
}

void touch_detected(int i)
{
	if(status[i]==1)
	{
		if(on_off_button==0 && status[3]==1)
		{
			rgb_data=rgb_data & 0x0FFF;
			rgb_data=rgb_data|(1<<DATA_SHIFT_POS_1);
			printf("B_On:0x%3x\r\n",rgb_data);
			led_gpio_update(LED_OFF,LED_BUTTON);
			led_gpio_update(LED_OFF,LED_SCROLLER);
		}
		else
		{
			led_gpio_update(button_led[3],LED_BUTTON);
			led_gpio_update(LED_OFF,LED_SCROLLER);
		}
		status[i]=0;
	}
	else
	{
		rgb_data=rgb_data & 0x0FFF;
		rgb_data=rgb_data|(i+2<<DATA_SHIFT_POS_1);
		printf("B_On:0x%3x\r\n",rgb_data);
		if(i<3)
		{
			led_gpio_update(button_led[i]|button_led[3],LED_BUTTON);
			led_gpio_update(scroller[i], LED_SCROLLER);
		}
		else
		{
			led_gpio_update(button_led[i],LED_BUTTON);
			led_gpio_update(LED_OFF,LED_SCROLLER);
		}
		for(int a=0;a<3;a++)                  //To switch between buttons
		{
			if(status[a]==1 && a!=i)
			{
				status[a]=0;
			}
		}
		status[i]=1;
	}
}

void slider(int i)
{
	prev_slider_val[i]=curr_slider_val[i];
	slider_status(i);
	if(prev_slider_val[i]!=curr_slider_val[i])
	{
		if((prev_slider_val[i]==4)&&(curr_slider_val[i]==1))
		{
			curr_slider_val[i]=prev_slider_val[i]+1;
		}
		if((prev_slider_val[i]==1)&&(curr_slider_val[i]==4))
		{
			prev_slider_val[i]=curr_slider_val[i]+1;
		}
		inc_dec(curr_slider_val[i],prev_slider_val[i],i);
	}
}


void slider_status(int i)
{
	for (uint8_t k=4, j=1; k<8; k++,j++)
	{
		if(0u!=(get_sensor_state(k) & KEY_TOUCHED_MASK))
		{
			curr_slider_val[i]=j;
		}
	}
}

void inc_dec(int curr_button_val,int prev,int i)
{
	uint8_t value=1,value1=128,slider_stat=0;
	int slider_led[8]={128,192,224,240,248,252,254,255};
	int shift,slider_position=0;
	SliderData[i]=slider_position;
	if((curr_button_val>prev))
	{
		if((prev==4)&&(curr_button_val==5))
		{
			curr_button_val=1;
			curr_slider_val[i]=1;
			prev_slider_val[i]=0;
		}
		shift=curr_button_val-1;
		if(scroller[i]>=240)
		{
			shift=shift+4;
		}
		slider_stat=value << shift;
	}
	else if((curr_button_val<prev))
	{
		if((prev==5)&&(curr_button_val==4))
		{
			curr_button_val=4;
			curr_slider_val[i]=4;
			prev_slider_val[i]=0;
		}
		shift=3-(curr_button_val-1);
		if(scroller[i]<=248)
		{
			shift=shift+4;
		}
		slider_stat=value1 >> shift;
	}
	if(slider_stat>0)
	{
		slider_position=log(slider_stat)/log(2);
		led_gpio_update(slider_led[slider_position],LED_SCROLLER);
		scroller[i]=slider_led[slider_position];
		if(button_value==0x01 && i==0)
		{
			rgb_data=rgb_data & 0xF0FF;
			rgb_data=rgb_data|((slider_position+1)<<DATA_SHIFT_POS_2);
		}
		else if(button_value==0x02 && i==1)
		{
			rgb_data=rgb_data & 0xFF0F;
			rgb_data=rgb_data|((slider_position+1)<<DATA_SHIFT_POS_3);
		}
		else if(button_value==0x03 && i==2)
		{
			rgb_data=rgb_data & 0xFFF0;
			rgb_data=rgb_data|((slider_position+1));
		}
	}
	if(SliderData[i]!=(slider_position+1))
	{
		printf("B_On:0x%3x\r\n",rgb_data);
		SliderData[i]=slider_position+1;
	}

}





