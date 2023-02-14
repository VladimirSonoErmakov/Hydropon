#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <DHT.h>
#define countof(a) (sizeof(a) / sizeof(a[0]))
//#define DHTPIN 3 // Пин на который повесить DHT датчик

//DHT dht(DHTPIN, DHT22); //Инициация DHT
ThreeWire myWire(7,6,8); // пины - DAT, CLK, RST
RtcDS1302<ThreeWire> Rtc(myWire);

boolean AllreadyWateredToday =  true; // true Флажок, поливали ли сегодня
boolean PumpOn =  false; // false  Флажок, включить ли насос
boolean LightOn =  false; // false  Флажок, включить ли свет - ДОПИСАТЬ 
boolean HeatOn =  false; // false  Флажок, включить ли обогрев - ДОПИСАТЬ 
boolean VentilationOn =  false; // false  Флажок, включить ли венлитятор продувки - ДОПИСАТЬ 

uint8_t FloodTime = 5;  // время (час суток) когда поднять флаг полива
uint8_t LastHour = 0 ; // контроль времени прошлого цикла - изначально 0 часов
uint8_t PresentHour = 0; // Текущее время - изначально 0 часов


void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];
    snprintf_P(datestring, 
        countof(datestring),
        PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
        dt.Month(),
        dt.Day(),
        dt.Year(),
        dt.Hour(),
        dt.Minute(),
        dt.Second() );
    Serial.print(datestring);
}


boolean TimeIsNow ()
{
    boolean result; 
    if (PresentHour == FloodTime)
    {
        result=false;
        return result;
    } 
    else 
    {  
        result=true;
        return result;
    }
      
}


void setup() 
{
    pinMode(5, OUTPUT);
    Serial.begin(9600);
    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    Rtc.Begin();
    dht.begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

    
    // Проверяем данные в плате часов, при необходимости обновляем до времени компиляции скетча
    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }


    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }


    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }


    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
}

void loop() 
{

// Считываем текущее время в переменную
RtcDateTime now = Rtc.GetDateTime();  
PresentHour = now.Hour();

//Считываем влажность и температуру


// выдаем в консоль статусы
Serial.print("Pump: ");
Serial.print(PumpOn); 
Serial.print(" Water today: ");  
Serial.print(AllreadyWateredToday); 
Serial.print(" Hour: ");
Serial.print(PresentHour);
Serial.print(" UpperTankLevel: "); 
Serial.print(digitalRead(4));
Serial.println();




// Включаем насос в зависимости от флага на включение
if (PumpOn) 
{
    digitalWrite(5, HIGH); 
} 
else 
{
    digitalWrite(5, LOW); 
}

// Опускаем флаг работы насоса как только замкнет датчик уровня в верхнем баке и ставим метку что сегодня уже поливали. 
if (digitalRead(4)) 
{
    PumpOn =  false; AllreadyWateredToday = true;
} 
 
//проверяем врема (который час) и поливали ли сегодня - если не поливали и время как в FloodTime, поднимаем флаг работы насоса
if (AllreadyWateredToday == false && TimeIsNow == true )  
{
    PumpOn=true;
}

// Проверяем нужно ли опустить флаг сегодняшней поливки (наслал ли новый день?)
if (LastHour > PresentHour) 
{
    AllreadyWateredToday =  false;
} 

delay(2000); // 1000 = 1 секунда

//Оюнуляем счетчик часов - начинаем новый день. 
LastHour = PresentHour;

}