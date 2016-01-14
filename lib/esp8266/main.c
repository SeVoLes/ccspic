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
extern int esp_response_flag; // 
extern char ESP8266Buf[ESP8266BUFFER_LENGHT]; // Buffer for ESP answer

// Other functions
#include <functions.c>

// use for incriment esp_response_flag for waiting respons of ESP
#INT_TIMER1
void int_timer1_task(void) {
   esp_response_flag++;
}

// UART inrerrupts
#int_RDA
void  RDA_isr(void) 
{
   // get a char
   ESP8266Buf[esp_write_index] = fgetc(WIFI_DATA_STREAM);
   esp_write_index++;
   // end of string fo using buffer in string functions
   ESP8266Buf[esp_write_index]='\0';

   if (esp_write_index>ESP8266BUFFER_LENGHT-3)
   {
      esp_status = 0;
      printf(usb_cdc_putc, "Over Buff\r\n");
   }
}


// Îñíîâíàÿ ïðîãðàììà
void main()
{
   // Èíèöèàëèçèðóì æåëåçî
   HW_init();
   
   setup_timer1(TMR_INTERNAL|TMR_DIV_BY_256);   // setup interrupts
   //enable_interrupts(INT_TIMER1);
   //enable_interrupts(int_rda);

   init_ESPBuffer();

   while(true)
   {
      // usb
      usb_task();
      
      // wi-fi esp8266
      wifi_task();
      
      // åñòü ëè ÷òî òî â USB áóôåðå
      if (usb_cdc_kbhit())
      {
         // Ýõî
         char input_cdc_usb = usb_cdc_getc();
         usb_cdc_echo(input_cdc_usb);

      }//if (usb_cdc_kbhit())

   }//while(true)
}



