/**************************************
* Testing project for my own board
* Snail Universe Board rev 1.0
* Contact me: voloh@land.ru
**************************************/
// Same libs for chips on board
// PIC24 - PinOut, Interfeses, etc.
#include <desc_pic24fj64gb004.h>
// Onboard clock
#include <..\lib\m41t81.c>
// Onboard flash
#include <..\lib\at45db0081.c>
// Onboard sram
#include <..\lib\23k256.c>

// same functions
#include <..\lib\snail_functions.c>
// For working with string
#include <string.h>

// ESP8266 Wi-Fi library
// Name of stream of UART to ESP
#define WIFI_DATA_STREAM USART1
// libs
#include <wifi_esp.c>
// variables fo working (variable declaration in wifi_esp.c)
extern int esp_status; // number of step (see wifi_esp.c)
extern int esp_write_index; // position in recive buffer
extern int esp_response_flag; // for waiting respons of ESP
extern char ESP8266Buf[ESP8266BUFFER_LENGHT]; // Buffer for ESP answer

// Other functions
#include <functions.c>

// use for incriment esp_response_flag for waiting respons of ESP
#INT_TIMER1
void int_timer1_task(void) {
   esp_response_flag++;
}

// UART interrupts
#int_RDA
void  RDA_isr(void) 
{
   // get a char
   ESP8266Buf[esp_write_index] = fgetc(WIFI_DATA_STREAM);
   esp_write_index++;
   // end of string fo using buffer in string functions
   ESP8266Buf[esp_write_index]='\0';

   // error - buffer overflow
   if (esp_write_index>ESP8266BUFFER_LENGHT-3)
   {
      esp_status = 0;
      printf(usb_cdc_putc, "Command buffer overflow\r\n");
   }
}


// main programm
void main(void)
{
   // Hardware initialization
   HW_init();
   // setup interrupts
   setup_timer1(TMR_INTERNAL|TMR_DIV_BY_256);   
   
   // Command buffer initialization
   init_ESPBuffer();

   while(true)
   {
      // usb for debug
      usb_task();
      
      // wi-fi esp8266
      wifi_task();
      
      /*******************************
      * *****************************
      * User code must be situated here!
      * *****************************
      *******************************/

   }//while(true)
}



