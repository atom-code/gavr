//  функция для парсинга массива
// String part01 = getValue(TXT,';',0);
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;
  
  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
      found++;
      strIndex[0] = strIndex[1]+1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void GSM_POWER(){
  digitalWrite(pin_power_key, LOW); // ВЫКЛЮЧАЕМ МОДУЛЬ
  digitalWrite(pin_power_key, HIGH); // ВКЛЮЧАЕМ МОДУЛЬ
  ERROR_STEP = ERROR_START;
  GSM_STATUS.Stop();  // СБРАСЫВАЕМ МОНИТОР ГОТОВНОСТИ И ПЕРЕЗАПУСКА МОДУЛЯ
  GSM_STATUS.Start(); // ЗАПУСКАЕМ МОНИТОР ГОТОВНОСТИ И ПЕРЕЗАПУСКА МОДУЛЯ
  CONTROL.Stop();  // СБРАСЫВАЕМ АВТО-ОТПРАВКУ
  CONTROL.Start(); // ЗАПУСКАЕМ АВТО-ОТПРАВКУ
}

void GSM_SAVE(String val){
  #ifdef DEBUG
    HardSerial.println();
    HardSerial.print(F("%%%% SAVE DATA : "));  HardSerial.println(val);
    HardSerial.println();
  #endif
}


void MODEM_STARTUP(String val){
  if(val.indexOf("MODEM:STARTUP") > -1) { // СЛОВИЛИ СООБЩЕНИЕ О СТАРТЕ МОДУЛЯ В РАБОЧЕМ РЕЖИМЕ ПРОГРАММЫ - ИДЕМ НА ОЖИДАНИЕ +PBREADY 
      GLOB_TIME_OUT = GTO;

      if(STEP_MAIN> 100 && STEP_MAIN< 140 ) { // ЕСЛИ СЛОВИЛИ В ПРОЦЕССЕ ОТПРАВКИ ТО НАМ НУЖНО СОХРАНИТЬ ДАННЫЕ
        GSM_SAVE(GET_BUFER); // СОХРАНЯЕМ ДАННЫЕ
      }
      GLOB_STATUS  = 0;
      STEP_MAIN = 17;
  }
}

void GSM_RESET(){

  #ifdef DEBUG
    HardSerial.println("# RESET");
    HardSerial.println();
  #endif
  
  digitalWrite(pin_power_key, LOW); // ВЫКЛЮЧАЕМ МОДУЛЬ
  
  CMD.Set(2000000UL); // ПЕРИОД ОЖИДАНИЯ 2000 ms
  START_SEND     = false; // Сброс флага регистрации ( СНОВА ОТПРАВИМ ДАННЫЕ ПРИ СТАРТЕ ПОДПРОГРАММЫ )
  GLOB_STATUS    = 0;
  START_TIME_OUT = false;
  START_SEND_DATA= false;
  GLOB_TIME_OUT  = 15; // ЖДЕМ УЖЕ 15 циклов если не сработало по умолчанию на GTO
  STEP_MAIN      = 0;
  
}

/* УПАКОВКА ПАКЕТА И ОТПРАВКА */
//void GSM_TRANSPORT(String reguest){
void GSM_TRANSPORT(){
  STEP_MAIN= 100;
}

void TIMEOUT(){
   if(GLOB_TIME_OUT>0){
      #ifdef DEBUG
        HardSerial.print(F("TIME OUT >"));
        HardSerial.println(GLOB_TIME_OUT);
      #endif
      GLOB_TIME_OUT--;
    } else {
      #ifdef DEBUG
        HardSerial.print(F("TIME OUT >"));
        HardSerial.println(GLOB_TIME_OUT);
      #endif
      GSM_RESET();
    }
}

void TIMEOUT_POWER(){
   if(GLOB_TIME_OUT>0){
      #ifdef DEBUG
        HardSerial.print(F("WAIT POWER > "));
        HardSerial.println(GLOB_TIME_OUT);
      #endif
      GLOB_TIME_OUT--;
    } else {
      #ifdef DEBUG
        HardSerial.print(F("WAIT POWER > "));
        HardSerial.println(GLOB_TIME_OUT);
      #endif
      GSM_RESET();
    }
}

void PREV_CMD(int S){
   if(GLOB_TIME_OUT>0){
      #ifdef DEBUG
        HardSerial.println(F("#"));
        HardSerial.println();
      #endif
      GLOB_TIME_OUT--;
    } else {
      #ifdef DEBUG
        HardSerial.println(F("#"));
        HardSerial.println();
      #endif
      STEP_MAIN = S;
    }
}

void ERROR_CMD(){
    if (ERROR_STEP == ERROR_START ){
       #ifdef DEBUG
        HardSerial.println(F("COUNT ERROR"));
        HardSerial.println();
      #endif
    } else if(ERROR_STEP > 0){
      #ifdef DEBUG
        HardSerial.print(F("@ - "));
        HardSerial.println(ERROR_STEP);
        HardSerial.println();
      #endif
      ERROR_STEP--;
    } else {
      #ifdef DEBUG
        HardSerial.println(F("COUNT RESET"));
        HardSerial.println();
      #endif
      GSM_RESET();
    }  
}

void GSM_CMD(byte _STEP_MAIN){
  
  if(_STEP_MAIN==0){
    if(START_TIME_OUT==false){
      GSM_POWER(); // ВКЛЮЧАЕМ МОДУЛЬ 
      START_TIME_OUT=true; // ПОДНИМАЕМ ФЛАГ ВКЛЮЧЕННОГО МОДУЛЯ
    } else {
      TIMEOUT_POWER(); // ТАЙМАУТ ПОСЛЕ ЗАПУСКА
    }
  }
  if(_STEP_MAIN==1){
    GLOB_STATUS = 0;
    
    #ifdef DEBUG
      HardSerial.print(F("Turn on AOH "));
    #endif
    mySerial.println("AT+CLIP=1"); //включить АОН
    STEP_MAIN = 2;
  }
  if(_STEP_MAIN==2){
    #ifdef DEBUG
      HardSerial.print(F("Text format sms "));
    #endif
    mySerial.println("AT+CMGF=1"); // текстовый формат SMS
    STEP_MAIN = 3;
  }
  if(_STEP_MAIN==3){
    #ifdef DEBUG
      HardSerial.print(F("Mode GSM "));
    #endif
    mySerial.println("AT+CSCS=\"GSM\""); // кодировка текста - GSM
    STEP_MAIN = 4;
  }
  if(_STEP_MAIN==4){
    #ifdef DEBUG
      HardSerial.print(F("SMS to terminal "));
    #endif
    mySerial.println("AT+CNMI=2,2,0,0,0"); // вывод смс в консоль
    STEP_MAIN = 5;
  }
  if(_STEP_MAIN==5){
    #ifdef DEBUG
      HardSerial.print(F("GET BALANCE >"));
    #endif
    mySerial.println("ATD#100#;"); // вывод смс в консоль
    STEP_MAIN = 6;
    CMD.Set(2500000UL); // ПЕРИОД ОЖИДАНИЯ 2500 ms
  }
  if(_STEP_MAIN==6){
    // В ЭВЕНТЕ ЖДЕМ РЕЗУЛЬТАТ БАЛАНСА
  }
  
  if(_STEP_MAIN==7){ // ПРИ ПОЛУЧЕНИИ БАЛАНСА ПОЛУЧИЛИ ОШИБКУ - ПЕРЕЗАУСКАЕМ МОДУЛЬ
    if(ERROR_STEP > 0){
      #ifdef DEBUG
        HardSerial.print(F("WAIT: "));
        HardSerial.println(ERROR_STEP);
      #endif
      ERROR_STEP--;
    } else {
      GSM_RESET();
    }
  }
  
  if(_STEP_MAIN==8){ // ПОДКЛЮЧАЕМСЯ К ИНЕТУ 1
    #ifdef DEBUG
      HardSerial.print(F("SEND ( AT+XISP=0 ) "));
    #endif
    mySerial.println("AT+XISP=0"); // ОЖИДАЕНИЕ ОТВЕТА В ШАГЕ №9
    STEP_MAIN = 9;
    GLOB_TIME_OUT = GTO;
  }
  if(_STEP_MAIN==9){
    // В ЭВЕНТЕ ЖДЕМ РЕЗУЛЬТАТ -  ЗДЕСЬ ЖЕ ЗАПУСКАЕМ TIMEOUT 
    TIMEOUT(); // ТАЙМАУТ ЕСЛИ НЕ ДОЖДАЛИСЬ 
  }

  if(_STEP_MAIN==10){ // ПОДКЛЮЧАЕМСЯ К ИНЕТУ 2
    #ifdef DEBUG
      HardSerial.print(F("SEND ( AT+CGDCONT=1 ) "));
    #endif
    mySerial.println("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\""); // ОЖИДАЕНИЕ ОТВЕТА В ШАГЕ №11
    STEP_MAIN = 11;
    GLOB_TIME_OUT = GTO;
  }
  if(_STEP_MAIN==11){
    // В ЭВЕНТЕ ЖДЕМ РЕЗУЛЬТАТ
    TIMEOUT(); // ТАЙМАУТ ЕСЛИ НЕ ДОЖДАЛИСЬ 
  }

  if(_STEP_MAIN==12){ // ПОДКЛЮЧАЕМСЯ К ИНЕТУ 3
    #ifdef DEBUG
      HardSerial.print(F("SEND ( AT+XIIC=1 ) "));
    #endif
    mySerial.println("AT+XIIC=1"); // ЗАПРОС НА ПОЛУЧЕНИЕ IP
    STEP_MAIN = 13;
    GLOB_TIME_OUT = GTO;
  }
  if(_STEP_MAIN==13){
    // В ЭВЕНТЕ ЖДЕМ РЕЗУЛЬТАТ
    TIMEOUT(); // ТАЙМАУТ ЕСЛИ НЕ ДОЖДАЛИСЬ 
  }

  if(_STEP_MAIN==14){ // ПОДКЛЮЧАЕМСЯ К ИНЕТУ 3
    #ifdef DEBUG
      HardSerial.print(F("SEND ( AT+XIIC? ) "));
    #endif
    mySerial.println("AT+XIIC?"); // ПОДКЛЮЧАЕМСЯ К СЕРВЕРУ ПО IP + PORT
    STEP_MAIN = 15;
    GLOB_TIME_OUT = GTO;
  }
  if(_STEP_MAIN==15){
    // В ЭВЕНТЕ ЖДЕМ РЕЗУЛЬТАТ
    TIMEOUT(); // ТАЙМАУТ ЕСЛИ НЕ ДОЖДАЛИСЬ 
  }
  
  if(_STEP_MAIN==16){ 
    /* 
    ПОСЛЕ УСЕШНГО СТАРТА ОСТАНАВЛИВАЕМСЯ НА ЭТОМ ШАГЕ, ТАК ЖЕ ПРИ УСПЕШНОЙ ОТПРАВКЕ ВОЗВРАЩАЕМСЯ НА ЭТОТ ШАГ 
    В EVENT - ОТСЛЕЖИВАЕМ 
    МОДУЛЬ ГОТОВ К РАБОТЕ  
    ЗДЕСЬ МОЖЕМ ЕЩЕ ЧТО-ТО ДЕЛАТЬ 
    */
  }

  if(_STEP_MAIN==17){ // СЛОВИЛИ MODEM:STARTUP В РЕЖИМЕ 1 - ЖДЕМ +PBREADY В EVENT
    TIMEOUT(); // ТАЙМАУТ ЕСЛИ НЕ ДОЖДАЛИСЬ 
  }

  if(_STEP_MAIN==100){ // ПОДКЛЮЧАЕМСЯ К ИНЕТУ №1
    #ifdef DEBUG
      HardSerial.print(F("SEND ( AT+TCPSETUP ) "));
    #endif
    mySerial.println("AT+TCPSETUP=1,"+server_ip_port); // ПОДКЛЮЧАЕМСЯ К СЕРВЕРУ ПО IP + PORT

    CMD.Set(2000000UL); // ПЕРИОД ОЖИДАНИЯ 1000 ms
    
    GLOB_TIME_OUT = 6;
    ERROR_STEP = ERROR_START;
    STEP_MAIN = 101;
  }
  if(_STEP_MAIN==101){
    // В ЭВЕНТЕ ЖДЕМ РЕЗУЛЬТАТ
    TIMEOUT(); // ТАЙМАУТ ЕСЛИ НЕ ДОЖДАЛИСЬ 
  }

  if(_STEP_MAIN==110){ // ПОДКЛЮЧАЕМСЯ К ИНЕТУ №2
    #ifdef DEBUG
      HardSerial.print(F("SEND ( AT+TCPSEND ) "));
    #endif
    mySerial.println("AT+TCPSEND=1," + String(SEND_BUFER.length(), DEC)); // Отправляем размер пакета модулю
    STEP_MAIN = 111;
    GLOB_TIME_OUT = 6;
  }
  if(_STEP_MAIN==111){
    // В ЭВЕНТЕ ЖДЕМ РЕЗУЛЬТАТ
    TIMEOUT(); // ТАЙМАУТ ЕСЛИ НЕ ДОЖДАЛИСЬ 
  }

  if(_STEP_MAIN==120){ // ПОДКЛЮЧАЕМСЯ К ИНЕТУ №3
    #ifdef DEBUG
      HardSerial.print(F("SEND ( +TCPRECV ) "));
    #endif
    mySerial.println(SEND_BUFER + 0x0d); // Отправляем пакет из буфера

    CMD.Set(5000000UL); // ПЕРИОД ОЖИДАНИЯ 5000 ms
    
    STEP_MAIN = 121;
    GLOB_TIME_OUT = 6; // ЖДЕМ 8 ТАЙМАУТОВ
    
  }
  if(_STEP_MAIN==121){
    // В ЭВЕНТЕ ЖДЕМ РЕЗУЛЬТАТ
     TIMEOUT(); // ТАЙМАУТ ЕСЛИ НЕ ДОЖДАЛИСЬ 
  }

  if(_STEP_MAIN==130){ // ПОДКЛЮЧАЕМСЯ К ИНЕТУ №4
    #ifdef DEBUG
      HardSerial.print(F("SEND ( AT+TCPCLOSE ) "));
    #endif
    mySerial.println("AT+TCPCLOSE=1"); // Отправляем пакет из буфера
    
    CMD.Set(2000000UL); // ПЕРИОД ОЖИДАНИЯ 1000 ms
    
    STEP_MAIN = 131;
    GLOB_TIME_OUT = 6;
  }
  if(_STEP_MAIN==131){
    // В ЭВЕНТЕ ЖДЕМ РЕЗУЛЬТАТ
    TIMEOUT(); // ТАЙМАУТ ЕСЛИ НЕ ДОЖДАЛИСЬ 
  }

  if(_STEP_MAIN==140){
    // ОТПРАВКА ВЫПОЛНЕНА УСПЕШНО
    #ifdef DEBUG
      HardSerial.println(F("# SEND MESSAGE"));
    #endif
    STEP_MAIN = 16;
  }
}


void GSM_EVENT(byte _STEP_MAIN, String val){
  // ПРИ ПРИСВОЕНИЕ ПЕРЕМЕННОЙ STEP_MAIN ЗНАЧЕНИЕ ВЫПОЛНЯЕТСЯ ГЛОБАЛЬНО
  if(_STEP_MAIN==0){
    if(val.indexOf("+PBREADY") > -1) {  // ЖДЕМ ГОТОВНОСТИ
      //mySerial.println("AT+CCLK?"); //включить АОН
      START_SEND=false;
      STEP_MAIN = 1; //
    }
  }

  if(_STEP_MAIN==6){
      if(val.indexOf("+CUSD") > -1) {
        if(val.indexOf("OK") > -1) {
          if(val.indexOf("Balance") > -1) {
            val = val.substring(val.indexOf("Balance"),val.indexOf("r")); 
            val = val.substring(val.indexOf(":"));
            val = val.substring(1);
            balance = val.toInt();
            if(balance){
              #ifdef DEBUG
                HardSerial.print(F("BALANCE:"));
                HardSerial.println(balance);
              #endif
            }
            STEP_MAIN = 8; // ПЕРЕХОДИМ К ПОДКЛЧЕНИЮ
          }
        } else {
          #ifdef DEBUG
                HardSerial.println(F("ERROR BALANCE:"));
          #endif
          STEP_MAIN = 7; // ТАМ СЧИТАЕМ КОЛИЧЕСТВО ПОПЫТОК
        }
      } else if(val.indexOf("ERROR") > -1) {
          #ifdef DEBUG
                HardSerial.println(F("ERROR BALANCE:"));
          #endif
          STEP_MAIN = 7; // ТАМ СЧИТАЕМ КОЛИЧЕСТВО ПОПЫТОК
      }
  }

  if(_STEP_MAIN==9){
    if(val.indexOf("OK") > -1) {
      #ifdef DEBUG
        HardSerial.println(F("AT+XISP=0 > OK"));
      #endif
      STEP_MAIN = 10;
    }
    else if(val != "" && (val.indexOf("ERROR") > -1)){
      #ifdef DEBUG
        HardSerial.println(F("AT+XISP=0 > NOT"));
      #endif
      GSM_RESET();
    }
  }

  if(_STEP_MAIN==11){
    if(val.indexOf("OK") > -1) {
      #ifdef DEBUG
        HardSerial.println(F("AT+CGDCONT > OK"));
      #endif
      STEP_MAIN = 12;
    }
    else if(val != "" && (val.indexOf("ERROR") > -1)){
      #ifdef DEBUG
        HardSerial.println(F("AT+CGDCONT > NOT"));
      #endif
      GSM_RESET();
    }
  }

  if(_STEP_MAIN==13){
    if(val.indexOf("OK") > -1) {
      #ifdef DEBUG
        HardSerial.println(F("AT+XIIC=1 > OK"));
      #endif
      STEP_MAIN = 14;
    }
    else if(val != "" && (val.indexOf("ERROR") > -1)){
      #ifdef DEBUG
        HardSerial.println(F("AT+XIIC=1 > NOT"));
      #endif
      GSM_RESET();
    }
  }

  if(_STEP_MAIN==15){
    if(val.indexOf("+XIIC:    0, 0.0.0.0") > -1){
      #ifdef DEBUG
        HardSerial.println(F("AT+XIIC=1 > NOT"));
      #endif
      GSM_RESET();
    } else if(val.indexOf("OK") > -1) {
      #ifdef DEBUG
        HardSerial.println(F("AT+XIIC=1 > OK"));
      #endif
      GLOB_STATUS = 1;
      STEP_MAIN = 16;
    }
  }
  if(_STEP_MAIN==16){
    // МОДУЛЬ ГОТОВ 
    // НО ЕСЛИ ПОСТУПИТ ( +PBREADY ) - модуль нужно переподключить с шага №0
    // ОТСЛЕЖИВАЕМ ЕСЛИ ПОСТУПИТЬ MODEM:STARTUP - ТО ( GLOB_STATUS = 0 ) И ЖДЕМ ( +PBREADY ) - ДОБАВИТЬ ТАЙМ-АУТ ДЛЯ СБРОСА НА ШАГЕ №17
    MODEM_STARTUP(val);
  }

  if(_STEP_MAIN==17){ // ЖДЕМ +PBREADY 
    if(val.indexOf("+PBREADY") > -1) { // ПОЛУЧИЛИ +PBREADY - ЗНАЧИТ ЗАПУСКАЕМСЯ С ШАГА №1
      ERROR_STEP = ERROR_START;
      GLOB_TIME_OUT = 5;
      GLOB_STATUS = 0;
      STEP_MAIN = 1; //
    }
  }

  
  if(_STEP_MAIN==100){
    //MODEM_STARTUP(val);
  }
  if(_STEP_MAIN==101){ // ИНЕТ №1
    
    
    if(val.indexOf("+TCPSETUP:1,OK") > -1) {
      STEP_MAIN = 110; // УСПЕШНО ПОДКЛЮЧИЛИСЬ К СЕРВЕРУ
    }
    else if(val.indexOf("+TCPSETUP:1,FAIL") > -1){ 
      // ОШИБКА ПРИ ПОДКЛЮЧЕНИИ К СЕРВЕРУ - ПРОБУЕМ ПЕРЕЗАПУСТИТЬ МОДУЛЬ
      //STEP_MAIN = 100;
      STEP_MAIN=8; // ВОЗВРАЩАЕМСЯ К ПОЛУЧЕНИЮ IP АДРЕСА В СЕТИ
    }

    //MODEM_STARTUP(val);
  }

  if(_STEP_MAIN==110){
    //MODEM_STARTUP(val);
  }
  if(_STEP_MAIN==111){ // ИНЕТ №2
    
    
    if(val.indexOf(">") > -1) {
      STEP_MAIN = 120; // УСПЕШНО ПОДКЛЮЧИЛИСЬ К СЕРВЕРУ
    }
    else { 
      // ОШИБКА ПРИ ПОДКЛЮЧЕНИИ К СЕРВЕРУ - ПРОБУЕМ ПЕРЕЗАПУСТИТЬ МОДУЛЬ
      STEP_MAIN = 110; 
    }
    //MODEM_STARTUP(val);
  }
  
  if(_STEP_MAIN==120){
    //MODEM_STARTUP(val);
  }
  if(_STEP_MAIN==121){ // ИНЕТ №3
    if (val.indexOf("+TCPCLOSE:1,Link Closed") > -1){
        STEP_MAIN = 130; // СОЕДИНЕНИЕ ОТВАЛИЛОСЬ САМО - ПЕРЕХОДИМ КАК ПРИ УСПЕШНОМ
    }
  }
  
  if(_STEP_MAIN==130){
    if (val.indexOf("+TCPCLOSE:Error") > -1){
      STEP_MAIN = 140; 
      STATUS_SEND = true; // ЗДЕСЬ НАДО СДЕЛАТЬ ПОМЕТКУ ЧТО ВЫПОЛНЕНА ОТПРАВКА
    }
    //MODEM_STARTUP(val);
    
  }
  if(_STEP_MAIN==131){ // ИНЕТ №4
    
    if(val.indexOf("OK") > -1) {
      // УСПЕШНО ПОДКЛЮЧИЛИСЬ К СЕРВЕРУ
      STEP_MAIN = 140; 
      STATUS_SEND = true; // ЗДЕСЬ НАДО СДЕЛАТЬ ПОМЕТКУ ЧТО ВЫПОЛНЕНА ОТПРАВКА
    }
    else if(val.indexOf("+TCPCLOSE:Error") > -1) { 
      // ОШИБКА ПРИ ПОДКЛЮЧЕНИИ К СЕРВЕРУ - ПРОБУЕМ ПЕРЕЗАПУСТИТЬ МОДУЛЬ
      STEP_MAIN = 140; 
    }
    
    //MODEM_STARTUP(val);
  }

  if(val.indexOf("+CMT") > -1) {
    String DATE_T   = getValue(val,'"',3); // Получаем дату для синхронизации
    String PHOME_T  = getValue(val,'"',1); // ВХОДЯЩИЙ НОМЕР
    String CMD_T    = getValue(val,'"',4); // СООБЩЕНИЕ
    #ifdef DEBUG
      HardSerial.println(F("###################"));
      HardSerial.print(F("DATE_TIME: "));
      HardSerial.println(DATE_T);
      HardSerial.print(F("PHONE: "));
      HardSerial.println(PHOME_T);
      HardSerial.print(F("CMD: "));
      HardSerial.println(CMD_T);
      HardSerial.println(F("###################"));
    #endif
  }
  
  
}
