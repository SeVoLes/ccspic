char Network_User_Pass[] = "\"Voloh\\'s network\",\"volohkrut\"";
char site_url[] = "192.168.13.42";
char url_host[] = "192.168.13.31";
char page[] = "/snail_stat/index.php";
char data[] = "time=2015-11-10&data=Preved";

// Переменная статуса работы с модулем
#define ESP8266BUFFER_LENGHT 256
int esp_status = 0;
int esp_write_index = 0;
int esp_response_flag = 0;
char ESP8266Buf[ESP8266BUFFER_LENGHT];


// Отправка GET запроса
void wifi_send_GET_request(*ptr_host,*ptr_page,*ptr_data)
{
   fprintf(WIFI_DATA_STREAM,"GET %s?%s HTTP/1.1\r\n",ptr_page,ptr_data);//16
   fprintf(WIFI_DATA_STREAM,"Host: %s\r\n",ptr_host);//8
   fputs("Accept: */*\r\n",WIFI_DATA_STREAM);//13
   fputs("Content-Type: text/html\r\n",WIFI_DATA_STREAM);//25
   fputs("Content-Length: 0\r\n\r\n\r\n",WIFI_DATA_STREAM);//23
   // total len minus data, host, page - 85
}

// Подсчет длины GET сообщения
int wifi_send_data_GET_len(*ptr_host,*ptr_page,*ptr_data)
{
   int host_len = strlen(ptr_host);
   int page_len = strlen(ptr_page);
   int data_len = strlen(ptr_data);
   
   return host_len + page_len + data_len + 85;
}

//
void init_ESPBuffer(void)
{
   int i;
   ESP8266Buf[0]='\0';
   for(i=1;i<ESP8266BUFFER_LENGHT-2;i++)
   {
      ESP8266Buf[i] = 0;
   }
   esp_write_index = 0;
   ESP8266Buf[ESP8266BUFFER_LENGHT-1]='\0';
}
// очистка буфера приема
void Clear_ESPBuffer(void)
{
   ESP8266Buf[0]='\0';
   esp_write_index=0;
}
// Вклчение таймаута на прием
void wifi_wait_on(void)
{
   Clear_ESPBuffer();
   set_timer1(0);//reset the timer.
   enable_interrupts(INT_TIMER1);
   enable_interrupts(int_rda);
   esp_response_flag=0;
}
// Выключение таймаута
void wifi_wait_off(void)
{
   disable_interrupts(INT_TIMER1);
   disable_interrupts(int_rda);
   esp_response_flag=0;
}

void wifi_task(void)
{
   static int errors_count;
   // для обработки ответов от модуля
   char temp_string[10];
   
   switch(esp_status)
   {
      case 0: // Исходная точка
      // Добавить сетчик АТ без OK и хард ресет
      // сбросим счетчик ошибок
         errors_count = 0;
         wifi_wait_on();
         // Жив ли модуль? AT
         fputs("AT\r\n",WIFI_DATA_STREAM);
         printf(usb_cdc_putc, "Snail - AT\r\n");
         esp_status = 1;
      break;
        
      case 1: // статус что мы отправили AT
         if (esp_response_flag>2)
         {
            wifi_wait_off();
            strcpy(temp_string,"OK");
            if (strstr(ESP8266Buf,temp_string) != NULL)
            {
               esp_status = 2;
               printf(usb_cdc_putc, "Module - OK\r\n");
            }
            else
            {
               esp_status = 0;
               printf(usb_cdc_putc, "Module - NOT OK\r\n");
               //printf(usb_cdc_putc, "Buffer c - %s;\r\ntmp - %s\r\n",ESP8266Buf,temp_string);
            }//if (strstr(ESP8266Buf,TempStr) != NULL)

         }//if (esp_response_flag>2)   
      break;
         
      case 2: // Получили OK на AT
         wifi_wait_on();
         // Проверка нужного режима работы
         fputs("AT+CWMODE?\r\n",WIFI_DATA_STREAM);
         printf(usb_cdc_putc, "Snail - AT+CWMODE?\r\n");
         esp_status = 3;
      break;
      // Узнаем режим работы модуля 1 - то то нужно
      case 3: // Отправили запрос на режим работы модуля
         if (esp_response_flag>2)
         {
            wifi_wait_off();
            strcpy(temp_string,"+CWMODE:1");
            if (strstr(ESP8266Buf,temp_string) != NULL)
            {
                esp_status = 4;
                printf(usb_cdc_putc, "Module - MODE = 1\r\n");
            }
            else
            {
               printf(usb_cdc_putc, "Buffer c - %s ; tmp - %s\r\n",ESP8266Buf,temp_string);
               fputs("AT+CWMODE=1\r\n",WIFI_DATA_STREAM);
               printf(usb_cdc_putc, "AT+CWMODE=1\r\n");
               fputs("AT+RST\r\n",WIFI_DATA_STREAM);
               printf(usb_cdc_putc, "AT+RST\r\n");
               esp_status = 0;
            }//if (strstr(ESP8266Buf,temp_string) != NULL)
         }//if (esp_response_flag==1)     
        break;
        
        case 4: // Попробуем подключиться к точке доступа
            wifi_wait_on();
            fprintf(WIFI_DATA_STREAM,"AT+CWJAP=%s\r\n",Network_User_Pass);
            printf(usb_cdc_putc, "Snail - AT+CWJAP=%s\r\n",Network_User_Pass);
            esp_status = 5;
        break;
        
        case 5: // Получилось подключитсья?
            if (esp_response_flag>5)
            {
               wifi_wait_off();
               strcpy(temp_string,"OK");
               if (strstr(ESP8266Buf,temp_string) != NULL) // Да, получилось
               {
                  printf(usb_cdc_putc, "Module - connect to %s\r\n", Network_User_Pass);
                  esp_status = 6;
               }
               else
               {
                  strcpy(temp_string,"ERROR");
                  if (strstr(ESP8266Buf,temp_string) != NULL) // ERROR
                  {
                     printf(usb_cdc_putc, "Module - ERROR connect to %s - %s\r\n", Network_User_Pass,ESP8266Buf);
                  }

                  printf(usb_cdc_putc, "Module - NOT connected to %s\r\n", Network_User_Pass);
                  // посчитаем количество попток
                  errors_count++;
                  if (errors_count>10)
                  {
                     esp_status = 0; // Попробуем еще раз
                     errors_count = 0;
                  }
                  else
                  {
                     esp_status = 4; // Попробуем еще раз
                  }// if (errors_count>10)
                  
               }//if (strstr(ESP8266Buf,temp_string) != NULL)

            }//if (esp_response_flag>2)
         break;
         
         case 6: // Отправим команду, что бы узнать IP адрес полученный по DHCP
            wifi_wait_on();
            fputs("AT+CIFSR\r\n",WIFI_DATA_STREAM);
            printf(usb_cdc_putc, "Snail - AT+CIFSR\r\n");
            esp_status = 7;
         break;
         
         case 7: // Узнаем IP адрес
            if (esp_response_flag>5)// Подождем дольше чем обычно
            {
               wifi_wait_off(); 
               strcpy(temp_string,"ERROR");
               if (strstr(ESP8266Buf,temp_string) == NULL)
               {
                   printf(usb_cdc_putc, "Module - IP = %s\r\n",ESP8266Buf);
                   esp_status = 8;
               }
               else
               {
                   printf(usb_cdc_putc, "Module - IP = ERROR - %s\r\n",ESP8266Buf);
                   
                   // посчитаем количество попток
                  errors_count++;
                  if (errors_count>10)
                  {
                     esp_status = 0; // Попробуем еще раз
                     errors_count = 0;
                  }
                  else
                  {
                     esp_status = 6; // Попробуем еще раз
                  }// if (errors_count>10)

               }// if (strstr(ESP8266Buf,temp_string) == NULL)
            }// if (esp_response_flag>5)   
         break;
         
         case 8: //
            wifi_wait_on();
            fprintf(WIFI_DATA_STREAM,"AT+CIPSTART=\"TCP\",\"%s\",80\r\n",url_host);
            printf(usb_cdc_putc, "Snail - AT+CIPSTART=\"TCP\",\"%s\",80\r\n",url_host);
            esp_status = 9;
         break;
         
         case 9: // Подключимся к сайту
            if (esp_response_flag>5)// Подождем дольше чем обычно
            {
               wifi_wait_off(); 
               strcpy(temp_string,"CONNECT");
               if (strstr(ESP8266Buf,temp_string) != NULL)
               {
                   printf(usb_cdc_putc, "Module - TCP connected to %s\r\n",url_host);
                   esp_status = 10;
               }
               else
               {
                   printf(usb_cdc_putc, "Module - NOT TCP connected to %s\r\n",url_host);
                   // посчитаем количество попток
                  errors_count++;
                  if (errors_count>10)
                  {
                     esp_status = 0; // Начнем все с начала
                     errors_count = 0;
                  }
                  else
                  {
                     esp_status = 8; // Попробуем еще раз
                  }// if (errors_count>10)

               }// if (strstr(ESP8266Buf,temp_string) != NULL)
            }// if (esp_response_flag>5)   
         break;
         
         case 10: // отправка данных
            wifi_wait_on();
            //int data_len = strlen(data_get_string);
            int data_len = wifi_send_data_GET_len(url_host,page,data);
            fprintf(WIFI_DATA_STREAM,"AT+CIPSEND=%d\r\n",data_len);
            //fputs("AT+CIPSEND=100\r\n",WIFI_DATA_STREAM);
            printf(usb_cdc_putc, "Snail - AT+CIPSEND=%d\r\n",data_len);
            esp_status = 11;
         break;
         
         case 11:
            if (esp_response_flag>3)
            {
               wifi_wait_off(); 
               strcpy(temp_string,">");
               if (strstr(ESP8266Buf,temp_string) != NULL)
               {
                   //printf(usb_cdc_putc, "%s",data_get_string);
                   wifi_send_GET_request(url_host,page,data);
                   esp_status = 12;
               }
               else
               {
                   printf(usb_cdc_putc, "Module - NOT >\r\n");
                   esp_status = 8;// попробуем еще раз
               }// if (strstr(ESP8266Buf,temp_string) != NULL)
            }// if (esp_response_flag>1)  
         break;
         
         case 12:
            // Ничего не делать
         break;
   }//switch(esp_status)
   
}
