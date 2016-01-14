/********************************
 *  Librari for pic CCS
 *  ESP8266 - Wi-Fi module
 *  Initialization module in the background
 *  It need timer1 and RDA interrupts for working - see main.c
 *  Contact me: voloh@land.ru
 ********************************/
 
char Network_User_Pass[] = "\"Voloh\\'s network\",\"volohkrut\"";
char site_url[] = "192.168.13.42";
char url_host[] = "192.168.13.31";
char page[] = "/snail_stat/index.php";
char data[] = "time=2015-11-10&data=Preved";

// Ïåðåìåííàÿ ñòàòóñà ðàáîòû ñ ìîäóëåì
#define ESP8266BUFFER_LENGHT 256
int esp_status = 0;
int esp_write_index = 0;
int esp_response_flag = 0;
char ESP8266Buf[ESP8266BUFFER_LENGHT];


// Send GEP request
void wifi_send_GET_request(*ptr_host,*ptr_page,*ptr_data)
{
   fprintf(WIFI_DATA_STREAM,"GET %s?%s HTTP/1.1\r\n",ptr_page,ptr_data);//16
   fprintf(WIFI_DATA_STREAM,"Host: %s\r\n",ptr_host);//8
   fputs("Accept: */*\r\n",WIFI_DATA_STREAM);//13
   fputs("Content-Type: text/html\r\n",WIFI_DATA_STREAM);//25
   fputs("Content-Length: 0\r\n\r\n\r\n",WIFI_DATA_STREAM);//23
   // total len minus data, host, page - 85
   // it need for wifi_send_data_GET_len()
}

// calculation of length of request
int wifi_send_data_GET_len(*ptr_host,*ptr_page,*ptr_data)
{
   int host_len = strlen(ptr_host);
   int page_len = strlen(ptr_page);
   int data_len = strlen(ptr_data);
   
   return host_len + page_len + data_len + 85;
}

// Initialization recive buffer
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
// Clear of buffer
void Clear_ESPBuffer(void)
{
   ESP8266Buf[0]='\0';
   esp_write_index=0;
}

// background waiting mode on
void wifi_wait_on(void)
{
   Clear_ESPBuffer();
   set_timer1(0);//reset the timer.
   enable_interrupts(INT_TIMER1);
   enable_interrupts(int_rda);
   esp_response_flag=0;
}
// background waiting mode off
void wifi_wait_off(void)
{
   disable_interrupts(INT_TIMER1);
   disable_interrupts(int_rda);
   esp_response_flag=0;
}

// task for ESP
void wifi_task(void)
{
   // errors count
   static int errors_count;
   // Temp string
   char temp_string[10];
   
   // step by step working
   switch(esp_status)
   {
      case 0: // Start to work
         // Send AT
         errors_count = 0;
         wifi_wait_on();
         // Are you reade ESP? AT
         fputs("AT\r\n",WIFI_DATA_STREAM);
         // debug
         printf(usb_cdc_putc, "Snail - AT\r\n");
         // next step
         esp_status = 1;
      break;
        
      case 1: // Waiting OK
         if (esp_response_flag>2)
         {
            wifi_wait_off();
            strcpy(temp_string,"OK");
            if (strstr(ESP8266Buf,temp_string) != NULL)
            {
               // next step
               esp_status = 2;
               // debug
               printf(usb_cdc_putc, "Module - OK\r\n");
            }
            else
            {
               // to start
               esp_status = 0;
               printf(usb_cdc_putc, "Module - NOT OK\r\n");
            }//if (strstr(ESP8266Buf,TempStr) != NULL)

         }//if (esp_response_flag>2)   
      break;
         
      case 2: // Get mode of ESP
         wifi_wait_on();
         // Send command
         fputs("AT+CWMODE?\r\n",WIFI_DATA_STREAM);
         // debug
         printf(usb_cdc_putc, "Snail - AT+CWMODE?\r\n");
         // next step
         esp_status = 3;
      break;
      
      case 3: // We need mode 1 
         if (esp_response_flag>2)
         {
            wifi_wait_off();
            strcpy(temp_string,"+CWMODE:1");
            if (strstr(ESP8266Buf,temp_string) != NULL)
            {
               // next step
               esp_status = 4;
               // debug
               printf(usb_cdc_putc, "Module - MODE = 1\r\n");
            }
            else
            {
               // change mode to mode 1
               fputs("AT+CWMODE=1\r\n",WIFI_DATA_STREAM);
               // debug
               printf(usb_cdc_putc, "AT+CWMODE=1\r\n");
               // restart module
               fputs("AT+RST\r\n",WIFI_DATA_STREAM);
               // debug
               printf(usb_cdc_putc, "AT+RST\r\n");
               // to start
               esp_status = 0;
            }//if (strstr(ESP8266Buf,temp_string) != NULL)
         }//if (esp_response_flag==1)     
        break;
        
        case 4: // Connect to Wi-Fi router
            wifi_wait_on();
            fprintf(WIFI_DATA_STREAM,"AT+CWJAP=%s\r\n",Network_User_Pass);
            // debug
            printf(usb_cdc_putc, "Snail - AT+CWJAP=%s\r\n",Network_User_Pass);
            // next step
            esp_status = 5;
        break;
        
        case 5: // OK?
            if (esp_response_flag>5)
            {
               wifi_wait_off();
               strcpy(temp_string,"OK");
               if (strstr(ESP8266Buf,temp_string) != NULL) // OK
               {
                 // debug
                 printf(usb_cdc_putc, "Module - connect to %s\r\n", Network_User_Pass);
                 // next step
                 esp_status = 6;
               }
               else
               {
                  strcpy(temp_string,"ERROR");
                  if (strstr(ESP8266Buf,temp_string) != NULL) // ERROR
                  {
                    // debug
                     printf(usb_cdc_putc, "Module - ERROR connect to %s - %s\r\n", Network_User_Pass,ESP8266Buf);
                  }
                  // debug
                  printf(usb_cdc_putc, "Module - NOT connected to %s\r\n", Network_User_Pass);
                  // Try again
                  errors_count++;
                  if (errors_count>10)
                  {
                     esp_status = 0; // to start
                     errors_count = 0;
                  }
                  else
                  {
                     esp_status = 4; // return by 1 step
                  }// if (errors_count>10)
                  
               }//if (strstr(ESP8266Buf,temp_string) != NULL)

            }//if (esp_response_flag>2)
         break;
         
         case 6: // IP by DHCP
            wifi_wait_on();
            fputs("AT+CIFSR\r\n",WIFI_DATA_STREAM);
            // debug
            printf(usb_cdc_putc, "Snail - AT+CIFSR\r\n");
            // next step
            esp_status = 7;
         break;
         
         case 7: // get IP ?
            if (esp_response_flag>5)
            {
               wifi_wait_off(); 
               strcpy(temp_string,"ERROR");
               if (strstr(ESP8266Buf,temp_string) == NULL)// IP is get
               {
                   // debug
                   printf(usb_cdc_putc, "Module - IP = %s\r\n",ESP8266Buf);
                   // next step
                   esp_status = 8;
               }
               else
               {
                   // debug
                   printf(usb_cdc_putc, "Module - IP = ERROR - %s\r\n",ESP8266Buf);
                   
                   // error count
                  errors_count++;
                  if (errors_count>10)
                  {
                     esp_status = 0; // to start
                     errors_count = 0;
                  }
                  else
                  {
                     esp_status = 6; // Try again
                  }// if (errors_count>10)

               }// if (strstr(ESP8266Buf,temp_string) == NULL)
            }// if (esp_response_flag>5)   
         break;
         
         case 8: // connect to host
            wifi_wait_on();
            fprintf(WIFI_DATA_STREAM,"AT+CIPSTART=\"TCP\",\"%s\",80\r\n",url_host);
            // debug
            printf(usb_cdc_putc, "Snail - AT+CIPSTART=\"TCP\",\"%s\",80\r\n",url_host);
            // next step
            esp_status = 9;
         break;
         
         case 9: // Cinnected?
            if (esp_response_flag>5)
            {
               wifi_wait_off(); 
               strcpy(temp_string,"CONNECT");
               if (strstr(ESP8266Buf,temp_string) != NULL) // Connected
               {
                   // debug
                   printf(usb_cdc_putc, "Module - TCP connected to %s\r\n",url_host);
                   // next step
                   esp_status = 10;
               }
               else
               {
                   // Not connected
                   printf(usb_cdc_putc, "Module - NOT TCP connected to %s\r\n",url_host);
                   // try again
                  errors_count++;
                  if (errors_count>10)
                  {
                     esp_status = 0; // to start
                     errors_count = 0;
                  }
                  else
                  {
                     esp_status = 8; // try again
                  }// if (errors_count>10)

               }// if (strstr(ESP8266Buf,temp_string) != NULL)
            }// if (esp_response_flag>5)   
         break;
         
         case 10: // îòïðàâêà äàííûõ
            wifi_wait_on();

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
                   esp_status = 8;// ïîïðîáóåì åùå ðàç
               }// if (strstr(ESP8266Buf,temp_string) != NULL)
            }// if (esp_response_flag>1)  
         break;
         
         case 12:
            // Íè÷åãî íå äåëàòü
         break;
   }//switch(esp_status)
   
}
