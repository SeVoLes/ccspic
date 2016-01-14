/**************************************
* Настройки PIC24FJ64GB004
* Snail Universe Board rev 1.0 
* USB, I2C, SPI, UART, Digital I/O
* ADC I/O, Leds
**************************************/
// Стандартный заголовоный файл
#include <24FJ64GB004.h>
// Настроки таймеров и тактирования
#FUSES NOWDT                    //No Watch Dog Timer 
#FUSES NOJTAG                   //JTAG disabled 
#FUSES NOPROTECT                //Code not protected from reading 
#FUSES NOWRT                    //Program memory not write protected 
#FUSES NODEBUG                  //No Debug mode for ICD 
#FUSES HS                      //External clock with CLKOUT 
#FUSES PR_PLL                       //Primary Oscillator 
#FUSES PLLDIV5
#FUSES SOSC_IO  // set SOSC as digital Inputs 

#use delay(crystal=20M,clock=32M)

// Библиотеки для VCP порта
// Поверх USB
#include <pic24_usb.h>
#include <usb_cdc.h>
#include <usb_desc_cdc_snail.h>

// UART конфигурация
#pin_select U1TX = PIN_C9
#pin_select U1RX = PIN_B7
// 9600 57600 115200
#use rs232(baud=115200,UART1,BITS=8,STOP=1,stream=USART1)

// SPI конфигурация
#pin_select SDI1 = PIN_C8
#pin_select SDO1 = PIN_C7
#pin_select SCK1OUT = PIN_C6
#use SPI(FORCE_HW, SPI1, BITS=16)
// Чип-селекты
#define SPI_CS_FLASH_1 PIN_C3 // Флеш 1
#define SPI_CS_FLASH_2 PIN_C4 // Флеш 2
#define SPI_CS_SRAM PIN_C5    // Фрам
// Выбор чипа
#define SPI_SELECT(x) output_low(x)
#define SPI_DESELECT(x) output_high(x)
// SPI modes
#define SPI_MODE_0 (SPI_L_TO_H | SPI_XMIT_L_TO_H) 
#define SPI_MODE_1 (SPI_L_TO_H) 
#define SPI_MODE_2 (SPI_H_TO_L) 
#define SPI_MODE_3 (SPI_H_TO_L | SPI_XMIT_L_TO_H)

// I2C
#define SDA1 PIN_B9
#define SCL1 PIN_B8
#use i2c(MASTER, FORCE_HW, FAST, sda=SDA1, scl=SCL1)
#define I2C_RTC_ADDRESS 0xD0 // Адрес часов на шине I2C

// Настройка LED'ов
#define LED_PWR PIN_A9
#define LED_1 PIN_A8
#define LED_2 PIN_B4
#define LED_3 PIN_A4

#define LED_ON(x) output_high(x)
#define LED_OFF(x) output_low(x)

// Цифровые входы-выходы
#define D_1 PIN_A7
#define D_2 PIN_B14
#define D_3 PIN_B15
#define D_4 PIN_A0
#define D_5 PIN_A1

// Аналоговые входы-выходы
#define A_1 PIN_B2
#define A_2 PIN_B3
#define A_3 PIN_C0
#define A_4 PIN_C1
#define A_5 PIN_C2

// Вектор ошибок
int init_errors[5];
// часы
#define RTC_ERROR 0
// 1 - фрам
#define SRAM_ERROR 1
// 2 - флеш 1
#define FLASH_1_ERROR 2
// 3 - флеш 2
#define FLASH_2_ERROR 3
//

