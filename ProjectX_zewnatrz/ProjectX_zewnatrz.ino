// CONNECTIONS:
// DS1302 CLK/SCLK --> 5
// DS1302 DAT/IO --> 4
// DS1302 RST/CE --> 2
// DS1302 VCC --> 3.3v - 5v
// DS1302 GND --> GND
#include <ThreeWire.h>  
#include <RtcDS1302.h>

ThreeWire myWire(5,6,7); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

int8_t power = -1;

uint16_t wschod[53] = {458, 455, 450, 443, 434, 424, 412, 398, 385,
                        370, 355, 340, 325, 310, 295, 281, 267,
                        254, 243, 233, 224, 217, 213, 211, 211, 213,
                        218, 224, 233, 240, 250, 260, 270, 281, 291,
                        301, 312, 322, 333, 343, 354, 365, 377, 388,
                        400, 411, 422, 433, 442, 449, 455, 458, 458 };
                        
uint16_t zachod[53] = {937, 945, 955, 967, 978, 990, 1002, 1014, 1026, 
                       1038, 1049, 1060, 1071, 1082, 1093, 1104, 1115,
                       1156, 1166, 1176, 1185, 1193, 1200, 1205, 1207, 1208,
                       1206, 1202, 1195, 1187, 1177, 1165, 1152, 1139, 1124, 
                       1110, 1094, 1079, 1064, 1018, 1004, 989, 976, 963,
                       952, 942, 932, 928, 924, 923, 925, 928, 928 };

void setup () 
{
    Serial.begin(57600);

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();
//    Rtc.SetDateTime(compiled);
    

    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
//        Rtc.SetDateTime(compiled);
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
        //Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    pinMode(9, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

}

void loop () 
{
    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);

    if (!now.IsValid())
    {
        // Common Causes:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
        digitalWrite(LED_BUILTIN, LOW);    // time is not ok

    }
    else 
    {
        digitalWrite(LED_BUILTIN, HIGH);   // time is ok)
        uint8_t week =  getWeekNumber(now);
        uint16_t timeOfDay = getHourProperFormat(now);
        uint16_t morningBarrier = 240;  //4
        uint16_t eveningBarrier = 1380; //23
        setBarriers(now, &morningBarrier, &eveningBarrier);
//        printDateTime(now);
        Serial.println();
        //MAIN LOGIC
        int8_t newState = 0;
        // --------C------T-------W------------
        if (wschod[week - 1] > timeOfDay || zachod[week - 1] < timeOfDay ) 
        {
            newState = 1;  
        }
        
        //----Z----T----------C
        printLogic(week, timeOfDay, wschod[week - 1], zachod[week - 1], newState);
        
        if (newState != power)
        {
          //change output state
          digitalWrite(9, newState);
          power = newState;
          Serial.println("Switched output!");
          
        }
    }

    delay(2000); // ten seconds
}

#define countof(a) (sizeof(a) / sizeof(a[0]))


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

uint8_t getWeekNumber(const RtcDateTime& dt)
{
  uint8_t days[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  uint16_t day = 0;
  for (uint8_t mo = 1; mo<=dt.Month(); mo++)
  {
    if (mo != dt.Month())
    {
      day = day + days[mo-1];
    }
    else 
    {
      day = day + dt.Day();
    }
  }
  uint8_t week = day/7;
  week = week + 1;
  char datestring[20];
  return week;
}

uint16_t getHourProperFormat(const RtcDateTime& dt)
{
  return dt.Hour()*60 + dt.Minute();
}

void printLogic(uint8_t week, uint16_t timeNow, uint16_t wschod, uint16_t zachod, int8_t  state)
{
     char datestring[60];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("Res: %d, week=%d, time=%d, wsch=%d, zach=%d\n"),
            state,
            week,
            timeNow,
            wschod,
            zachod);
    Serial.print(datestring);
}

void setBarriers(const RtcDateTime& dt, uint16_t* mor, uint16_t* ev)
{
  uint8_t mo = dt.Month();
  if (mo >= 4 && mo <= 10)
  {
    *mor = 180;
    *ev = 1320;
  }
  else //winter time - ok for rtc
  {
     *mor = 240;  //4
     *ev = 1380; //23
  }
  
}
