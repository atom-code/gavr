#define DEBUG                 // VIEW DEBUG
#define ERROR_START 3         // ERROR_STEP
#define GTO 7                 // GLOB_TIME_OUT
#define SS  3                 // STEP_SEND 
#define HardSerial Serial     // UART

#include "SmartDelay.h"       // TIMER
#include <OneWire.h>          //
#include <SoftwareSerial.h>   // UART

// TX - 3 (SoftwareSerial RX) , RX - 2 (SoftwareSerial TX)
SoftwareSerial mySerial(2, 3); 

// ТАЙМЕРЫ
SmartDelay LED13(100000UL);         //  100ms - МИГАЕМ
SmartDelay GSM_STATUS(120000000UL); //  120000ms - ОПРОС СТАТУСА МОДУЛЯ = 2мин
SmartDelay CMD(1000000UL);          //  1000ms - ДЛЯ ПРОЦЕССОВ GSM
SmartDelay CONTROL(20000000UL);       //  20000ms - ОТПРАВКА ДАННЫХ С ПЕРИОДОМ 20 СЕКУНД


int tmp; // буфер HardSerial

const int ledPin        =  13;      // СВЕТОДИОД РАБОТЫ ( СЛЕДИМ ЗА ТОРМОЗАМИ )
int     ledState        = LOW;             

byte    pin_power_key   = 4;        // POWER KEY PIN ( ПИТАНИЕ МОДУЛЯ ВКЛЮЧАЕМ ЧЕРЕЗ МОСФЕТ IRF3205( GND МОДУЛЯ НА СТОК MOSFET )
int     sta_power_key   = HIGH;     // POWER KEY PIN ( ПРИ ВКЛЮЧЕНИИ ПОДАЕМ ЛОГИЧЕСКУЮ ЕДЕНИЦУ ) - В RESET ВЫПОЛНЯЕТСЯ СБРОС МОДУЛЯ ПУТЕМ ОТКЛЮЧЕНИЯ 
boolean START_TIME_OUT  = false;    // ФЛАГ ТАЙМАУТА
byte    GLOB_TIME_OUT   = GTO;      // СЧЕТИЧИК ТАЙМАУТОВ
byte    GLOB_STATUS     = 0;        // ГЛОБАЛЬНЫЙ СТАТУС GSM ( 0 - ЗАПУСК , 1 - ПОДКЛЮЧЕН К ИНЕТУ, 2- НЕТ ПОДКЛЮЧЕНИЯ )
byte    STEP_MAIN       = 0;        // ШАГ ПРОЦЕССА GSM
byte    STEP_SEND       = SS;       // ПОВТОРОВ ОТПРАВКИ

String  (SEND_BUFER);               // БУФЕР ЗАПРОСА 
String  (GET_BUFER);                // БУФУР ПАКЕТА
String  (RESPONE_GLOBE);            //
boolean STATUS_SEND     = false;    // СТАТУС ОТПРАВКИ - БУДЕМ МЕНЯТЬ ИСПОЛЬЗОВАТЬ ФЛАГ ДЛЯ ЗАПИСИ В SD-CARD
boolean START_SEND      = true;    // ФЛАГ ПЕРВОЙ ОТПРАВКИ ( РЕГИСТРАЦИЯ НА СЕРВЕРЕ )
boolean START_SEND_DATA = false;    // ФЛАГ ОТПРАВКИ ДАННЫХ
byte    ERROR_STEP  = ERROR_START;  // КОЛИЧЕСТВО ДОПУСТИМОЙ НЕУДАЧИ
    
// ФЛАГ ПРИВЫШЕНИЯ ПО ОДНОМУ ИЗ ПОКАЗАТЕЛЕЙ ( ЕСЛИ ФЛАГ ПОДНЯТ ) ТО БЛОКИРУЕМ ЗАПИСЬ В ПЕРЕМЕННЫЕ И ФОРМИРЕУМ ПАКЕТ НА ОТПРАВКУ
byte ALERT_STAT = false; 
boolean ALERT_SEND = false; // ФЛАГ ОТПРАВЛЕНИЯ И ЗАПИСИ

/* ЭТИ ДАННЫЕ НАДО ПОЛУЧАТЬ С ДАТЧИКОВ, EEPROM И SD-CARD */
String data_s = "00.00.00";         // СЮДА ПОТОМ ВВЕДЕМ ЗНАЧЕНИЕ С RTC МОДУЛЯ
String time_s = "00:00";            // СЮДА ПОТОМ ВВЕДЕМ ЗНАЧЕНИЕ С RTC МОДУЛЯ
boolean RTC_STATUS = false;         // ФЛАГ ПОЛУЧЕННОЙ ДАТЫ И ВРЕМЕНИ
boolean RTC_PROG   = false;         // ДЛЯ ЗАДАНИЯ RTC В ПАМЯТЬ ТЕКУЩЕГО ВРЕМЕНИ И ДАТЫ ( ЕСЛИ ВЫДАННАЯ ДАТА МОДУЛЕМ RTC ОТЛИЧАЕТСЯ ОТ ВРЕМЕНИ СЕРВЕРА ТО ЗАПИСЫВАЕМ НОВУЮ )

String id_device = "0001";          // ID УСТРОЙСТВА КАСТОМНО
String phone = "79126025587";       // НОМЕР ТЕЛЕФОНА ( SD CARD )
String apn = "internet.mts.ru";     // точка подключения ( SD CARD )
String status_s = "N";              // СТАТУСЫ КОНТРОЛЯ

String balance_sms = "100";                   // ПОЛУЧИТЬ ПО СМС
String server_ip_port = "81.177.139.105,80";  // сервер подключения ( SD CARD )
String server_domain  = "atomcode.ru";        // домен подключения ( SD CARD )
String server_url = "/x.php?p=";              // адрес запроса ( SD CARD )
String FirmWareVersion = "G0001";             // ВЕРСИЯ ПРОШИВКИ КАСТОМНО

int balance = 0;    // ПОЛУЧИТЬ ПО СМС
byte max_amp = 100; // МАКСИМАЛЬНЫЙ ТОК  ( SD CARD )
byte norm_amp = 5;  // НОРМАЛЬНЫЙ ТОК НИЖЕ 5 АМПЕР ( SD CARD )
byte max_u = 13;    // МАКСИМАЛЬНОЕ НАПРЯЖЕНИЕ ( SD CARD )
byte min_u = 10;    // МИНИМАЛЬНОЕ НАПРЯЖЕНИЕ ( SD CARD )
byte max_t = 60;    // МАКСИМАЛЬНАЯ ТЕМПЕРАТУРА ( SD CARD )
char amp_s[5];      // ТОК
char u1_s[5];       // НАПРЯЖЕНИЕ АККУМУЛЯТОРА №1
char u2_s[5];       // НАПРЯЖЕНИЕ АККУМУЛЯТОРА №2
char t1_s[5];       // ТЕМПЕРАТУРА АККУМУЛЯТОРА №1
char t2_s[5];       // ТЕМПЕРАТУРА АККУМУЛЯТОРА №2

void setup() {
  // put your setup code here, to run once:
  mySerial.begin(9600);
  HardSerial.begin(9600);
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  pinMode(10, OUTPUT);
  

  pinMode(pin_power_key, OUTPUT);
}

void loop() {
  
  /* ЭМИТАЦИЯ СБОРА ДАННЫХ*/
  float amp = 10.1; dtostrf(amp, 4, 1, amp_s);
  float u1 = 12.2;  dtostrf(u1, 4, 1, u1_s);
  float u2 = 12.5;  dtostrf(u2, 4, 1, u2_s);
  float t1 = 26.5;  dtostrf(t1, 4, 1, t1_s);
  float t2 = 26.0;  dtostrf(t2, 4, 1, t2_s);

  time_s = millis();

  if(LED13.Now()) {
    digitalWrite(ledPin,ledState);
    ledState=(ledState==LOW?HIGH:LOW);
  }

  
  if(CMD.Now()) {
      GSM_CMD(STEP_MAIN); // ВЫПОЛНЕНИЕ КОМАНД

      GET_BUFER  = "R|" + id_device + ";" + data_s + ";" + String(time_s) + ";" + phone + ";" + apn + ";" + String(balance) + ";" + String(max_amp) + ";" + String(norm_amp) + ";" + String(max_u) + ";" + String(min_u) + ";" + String(max_t) + ";" + FirmWareVersion;
          
      if(START_SEND==false && GLOB_STATUS==1 && STEP_MAIN==16){ // РЕГИСТРАЦИЯ УСТРОЙСТВА
          STATUS_SEND = false; // СТАТУС ОТПРАВКИ
          START_SEND=true;
          
          SEND_BUFER = "GET " + server_url + GET_BUFER + " HTTP/1.1\r\nHost: " + server_domain + "\r\n\r\n";
          
          #ifdef DEBUG
            HardSerial.println();
            HardSerial.println(F("SEND REGISTRATION"));
            HardSerial.println(SEND_BUFER);
          #endif
          
          GSM_TRANSPORT();
          
      } 
  }

  if(GSM_STATUS.Now()){ // ОПРАШИВАЕМ СТАТУС МОДУЛЯ ( ЕСЛИ НЕ ПОДКЛЮЧЕН ПРОБУЕМ ПОДКЛЮЧИТЬ )
    if(GLOB_STATUS==0){ // ЗА 120 секунд модуль не готов - пробуем перезапустить его
      #ifdef DEBUG
        HardSerial.println(F("LONG WAIT STATUS GSM"));
      #endif
      GSM_RESET();
    }
  }

  if(ALERT_STAT==false){ // ЕСЛИ СРАБОТАЛ ALERT - ТО НЕМЕДЛЕННО ОТПРАВЛЯЕМ ПАКЕТ ИЛИ ЗАПИСЫВАЕМ
    if(CONTROL.Now()){ 
      /// ДОБАВИТЬ УСЛОВИЕ - ЕСЛИ ПОКАЗАТЕЛИ ТОКА ПЕРВЫШАЮТ НОРМАЛЬНОЕ ЗНАЧЕНИЕ ТО ОТПРАВЛЯЕМ ДАННЫЕ
      // ЕСЛИ ТОК НИЖЕ НОРМАЛЬНОГО ИЛИ РАВЕН - ТО ОТКЛЮЧАЕМ ОТПРАВКУ И ЗАПУСКАЕМ ЧТЕНИЕ С ФЛЕШКИ И ОТПРАВЛЯЕМ
      // ЕСЛИ ВО ВРЕМЯ ОТПРАВКИ С ФЛЕШКИ ТОК СНОВА ПРЕВЫСЕЛ НОРМАЛЬНЫЙ ТО ЗАПУСКАЕМ ОТПРАВКУ ПО ТАЙМЕРУ CONTROL
      if(START_SEND==true && GLOB_STATUS==1 && STEP_MAIN==16 ){ // МОДУЛЬ ГОТОВ К РАБОТЕ И ЗАРЕГИСТРИРОВАН НА СЕРВЕРЕ
            STATUS_SEND = false; // СТАТУС ОТПРАВКИ
   
            // ПАКЕТ
            GET_BUFER = "A|" + id_device + ";" + data_s + ";" + String(time_s) + ";" + status_s + ";" + String(amp_s) + ";" + String(u1_s) + ";" + String(u2_s) + ";" + String(t1_s) + ";" + String(t2_s);
            SEND_BUFER = "GET " + server_url + GET_BUFER + " HTTP/1.1\r\nHost: " + server_domain + "\r\n\r\n";
  
            #ifdef DEBUG
              HardSerial.println();
              HardSerial.println(F("AUTO SEND"));
              HardSerial.println(SEND_BUFER);
            #endif
            
            GSM_TRANSPORT();
      } else {
            GSM_SAVE(GET_BUFER); // СОХРАНЯЕМ ДАННЫЕ
      }
    }
  } else if(ALERT_STAT==true){ // НЕМЕДЛЕННО ОТПРАВЛЯЕМ ДАННЫЕ
    
      if(ALERT_SEND == false) {
         ALERT_SEND = true;
        
        if(START_SEND==true && GLOB_STATUS==1 && STEP_MAIN==16 ){ // МОДУЛЬ ГОТОВ К РАБОТЕ И ЗАРЕГИСТРИРОВАН НА СЕРВЕРЕ
              STATUS_SEND = false; // СТАТУС ОТПРАВКИ
     
              // ПАКЕТ
              GET_BUFER = "E|" + id_device + ";" + data_s + ";" + String(time_s) + ";" + status_s + ";" + String(amp_s) + ";" + String(u1_s) + ";" + String(u2_s) + ";" + String(t1_s) + ";" + String(t2_s);
              SEND_BUFER = "GET " + server_url + GET_BUFER + " HTTP/1.1\r\nHost: " + server_domain + "\r\n\r\n";
    
              #ifdef DEBUG
                HardSerial.println();
                HardSerial.println(F("AUTO SEND"));
                HardSerial.println(SEND_BUFER);
              #endif
              
              GSM_TRANSPORT();
  
              
        } else {
              GSM_SAVE(GET_BUFER); // СОХРАНЯЕМ ДАННЫЕ
              ALERT_SEND = false;
              ALERT_STAT = false;
        }
      }
  }
  
  
  if(HardSerial.available()>0){
    tmp = HardSerial.read();
    switch(tmp){
      case 49:
        #ifdef DEBUG
          HardSerial.println();
          HardSerial.print(F("# STEP_MAIN  = ")); HardSerial.print(STEP_MAIN);   
          HardSerial.print(F("  GLOB_STATUS = ")); HardSerial.print(GLOB_STATUS);
          HardSerial.print(F("  STATUS_SEND = ")); HardSerial.println(STATUS_SEND);
          HardSerial.println();
        #endif
        break;
      case 50:
        if(GLOB_STATUS==1 && STEP_MAIN==16){ // ЕСЛИ МОДУЛЬ ГОТОВ И В РЕЖИМЕ ОЖИДАНИЯ
          STATUS_SEND = false; // СТАТУС ОТПРАВКИ
      
          // ПАКЕТ
          GET_BUFER = "D|" + id_device + ";" + data_s + ";" + String(time_s) + ";" + status_s + ";" + String(amp_s) + ";" + String(u1_s) + ";" + String(u2_s) + ";" + String(t1_s) + ";" + String(t2_s);
          SEND_BUFER = "GET " + server_url + GET_BUFER + " HTTP/1.1\r\nHost: " + server_domain + "\r\n\r\n";
          
          #ifdef DEBUG
            HardSerial.println();
            HardSerial.println(F("DATA SEND"));
            HardSerial.println(SEND_BUFER);
          #endif
          
          GSM_TRANSPORT();
 
        } else {
          #ifdef DEBUG
            HardSerial.println(F("SAVE TEST"));
          #endif
        }
        break;
      case 51:
        #ifdef DEBUG
          HardSerial.println(F("START SEND AUTO"));
        #endif
        //START_SEND_DATA = true;
        CONTROL.Stop();  // СБРАСЫВАЕМ МОНИТОР ГОТОВНОСТИ И ПЕРЕЗАПУСКА МОДУЛЯ
        CONTROL.Start(); // ЗАПУСКАЕМ МОНИТОР ГОТОВНОСТИ И ПЕРЕЗАПУСКА МОДУЛЯ
        break;
      case 52:
        #ifdef DEBUG
          HardSerial.println(F("STOP SEND AUTO"));
        #endif
        //START_SEND_DATA = false;
        CONTROL.Stop(); 
        break;
    }
  }

  // ОПРОС SERIAL GSM МОДУЛЯ
  if(mySerial.available()) //если модуль что-то послал
  {
    char ch = ' ';
    String val_input = "";
    
    while(mySerial.available()){  
       ch = mySerial.read();
       val_input += char(ch); //собираем принятые символы в строку
       delay(3); // придется оставить ибо хардварный сериал глючит
    } 
    val_input.replace("\n","");  // УДАЛЯЕМ ПЕРЕНОСЫ К ХЕРАМ
    val_input.replace("\r"," "); // УДАЛЯЕМ ПЕРЕНОСЫ К ХЕРАМ

    // КОСТЫЛЬ ВНЕ ШАГОВ ( ИБО НЕ УСЕВАЕМ ОТЛОВИТЬ )
    if (val_input.indexOf("HTTP/1.1 404") > -1){ // ОТЛОВА НЕДОСТУПНОСТИ LINK
      #ifdef DEBUG
        HardSerial.println(F("# ERROR 404 SERVER "));
      #endif
      GSM_SAVE(GET_BUFER); // СОХРАНЯЕМ ДАННЫЕ
      if(ALERT_SEND == true) { ALERT_SEND = false; ALERT_STAT = false; } 
      
      STEP_MAIN=130; // ЗАКРЫВАЕМ СОЕДИНЕНИЕ

    } else if(val_input.indexOf("HTTP/1.1 200") > -1){ // УСПЕШНО ОТПРАВИЛИ
      #ifdef DEBUG
        HardSerial.println(F("# OK SEND DATA TO SERVER"));
      #endif
      
      if(ALERT_SEND == true) { ALERT_SEND = false; ALERT_STAT = false; }
      
      STEP_MAIN=130; // ЗАКРЫВАЕМ СОЕДИНЕНИЕ
    }
    
    GSM_EVENT(STEP_MAIN, val_input); // Обработка событий ответов GSM

    #ifdef DEBUG
      HardSerial.print(F("> "));
      HardSerial.println(val_input);
    #endif
    
  }

  digitalWrite(10, GLOB_STATUS);
  
}
