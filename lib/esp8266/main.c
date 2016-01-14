/**************************************
* Snail Universe Board rev 1.0
* Тестовая программа
**************************************/
// Описание и настрока чипа PIC24
#include <desc_pic24fj64gb004.h>
// Библиотека для работы с внешними часами
#include <..\LIB\m41t81.c>
// Библиотека для работы с внешней FLASH памятью
#include <..\LIB\at45db0081.c>
// Библиотека для работы с внешний SRAM памятью
#include <..\LIB\23k256.c>
// Функции необходимые для работы
#include <..\LIB\snail_functions.c>
// Работа со строками
#include <string.h>
// библмотека для Wi-Fi модуля
#define WIFI_DATA_STREAM USART1

#include <wifi_esp.c>
extern int esp_status;
extern int esp_write_index;
extern int esp_response_flag;
extern char ESP8266Buf[ESP8266BUFFER_LENGHT];

// Другие функции
#include <functions.c>

#INT_TIMER1 // 260 ms - 1 /32 MHz *2 *0xFFFF (счетчик) * div
void int_timer1_task(void) {
   esp_response_flag++;
}

#int_RDA
void  RDA_isr(void) 
{
   ESP8266Buf[esp_write_index] = fgetc(WIFI_DATA_STREAM);
   esp_write_index++;
   ESP8266Buf[esp_write_index]='\0';

   if (esp_write_index>ESP8266BUFFER_LENGHT-3)
   {
      esp_status = 0;
      printf(usb_cdc_putc, "Over Buff\r\n");
   }
}


// Основная программа
void main()
{
   // Инициализирум железо
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
      
      // есть ли что то в USB буфере
      if (usb_cdc_kbhit())
      {
         // Эхо
         char input_cdc_usb = usb_cdc_getc();
         usb_cdc_echo(input_cdc_usb);

      }//if (usb_cdc_kbhit())

   }//while(true)
}



