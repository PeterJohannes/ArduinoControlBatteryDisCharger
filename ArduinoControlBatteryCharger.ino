/*
Campingwagen Steuerung für die Akkus.
Schaltschema:

Falls D+ = 12 V: Dauerstrom auf PIN 85-86 ein (Batterien werden durch Lichtmaschine geladen)
Sonst:
Nach ca. 1 Std: Dauerstrom auf PIN 85-86 ein
1 Sek. später: Schaltimpuls auf PIN 2-5: Kontakt 4-3 offen, Kontakt 6-3 geschlossen
1 Sek. Später: Dauerstrom auf PIN 85-86 aus
Nach ca. 1 Std.: Dauerstrom auf PIN 85-86 ein
1 Sek. später: Schaltimpuls auf PIN 1-5: Kontakt 6-3 offen, Kontakt 4-3 geschlossen
1 Sek. Später: Dauerstrom auf PIN 85-86 aus
Nach ca. 1 Std.: Dauerstrom auf PIN 85-86
Zusätzlich: jeweils ein Taster, der manuell Schaltsignal auf PIN 2-5 bzw. PIN 1-5 legt.
Wenn Taster betätigt wird, keine stündliche Umschaltung mehr.
Umschaltung startet wieder, wenn Zustand D+=12 V eingetreten ist oder Spannungsversorgung für Steuerlogig Null war.
 */

// Hardware Interface
// Analog In
#define PIN_ANALOG_VOLTAGE_AKKU_1        A5
#define PIN_ANALOG_VOLTAGE_AKKU_2        A4
#define PIN_ANALOG_VOLTAGE_POWER_IN      A3
//#define PIN_ANALOG_VOLTAGE_POWER_OUT     A3

// Digital In
//#define INTERRUPT_INPUT_TASTER_AKKU_1              1
//#define INTERRUPT_INPUT_TASTER_AKKU_2              1
//#define INTERRUPT_INPUT_TASTER_TEST                1
//#define INTERRUPT_INPUT_TASTER_CONNECTED           1

// Digital Out
#define PIN_OUTPUT_BISTABILE_ON       7
#define PIN_OUTPUT_BISTABILE_OFF      6
#define PIN_OUTPUT_AKKU_CONNECTED     5
#define PIN_OUTPUT_TAKT               4
//
//int  AkkuNumber;      // 0 = Akku_1, 1 = Akku_2
//int  AkkuTime;        //Runtime for the Akku
//
//#define  SIZE_OUTPUT_TIMMING_ARRAY  5
//int      OutputTimingArray  [SIZE_OUTPUT_TIMMING_ARRAY];

//#define START_TIME_FOR_AKKU       100  //
//#define SET_TIME_RELAY_BISTABILE   2   //
//#define SET_TIME_RELAY_CONNECTED   5   //

//#define WAITTIME  20000
//
//volatile  int actCounter;
//volatile  int oldCounter;

#define OUTPUT_BISTABILE_ON           0x01
#define OUTPUT_BISTABILE_OFF          0x02
#define OUTPUT_AKKU_CONNECTED         0x04

//#define StateForButton_1    0
//#define StateForButton_2    0
//#define StateForButton_3    0

#define ENDLOS      9999

int  actualState;
int  actualTimer;

int VoltageAkku_1;
int VoltageAkku_2;
int VoltagePowerIn;

bool TaktLed;

struct Vorgabe
{
  int State;
  int Time;
  int Output;
};

Vorgabe  Ablauf[] = {
  // state, sec,  active Outputs
  { 0,       15*60,  0                     | 0                    | 0                   },
  { 1,       2,  OUTPUT_AKKU_CONNECTED | 0                    | 0                   },
  { 2,       2,  OUTPUT_AKKU_CONNECTED | 0                    | OUTPUT_BISTABILE_ON },
  { 3,       2,  OUTPUT_AKKU_CONNECTED | 0                    | 0                   },
  { 4,       15*60,  0                     | 0                    | 0                   },
  { 5,       2,  OUTPUT_AKKU_CONNECTED | 0                    | 0                   },
  { 6,       2,  OUTPUT_AKKU_CONNECTED | OUTPUT_BISTABILE_OFF | 0                   },
  { 7,       2,  OUTPUT_AKKU_CONNECTED | 0                    | 0                   },

  // Sondermodus mit unendlich langer Dauer
  { 8,       1,  0                     | 0                    | 0                   },
  { 9,       2,  OUTPUT_AKKU_CONNECTED | 0                    | 0                   },
  {10,       2,  OUTPUT_AKKU_CONNECTED | 0                    | OUTPUT_BISTABILE_ON },
  {11,       2,  OUTPUT_AKKU_CONNECTED | 0                    | 0                   },
  {12,  ENDLOS,  0                     | 0                    | 0                   },

  // Sondermodus mit unendlich langer Dauer
  {13,       1,  0                     | 0                    | 0                   },
  {14,       2,  OUTPUT_AKKU_CONNECTED | 0                    | 0                   },
  {15,       2,  OUTPUT_AKKU_CONNECTED | OUTPUT_BISTABILE_OFF | 0                   },
  {16,       2,  OUTPUT_AKKU_CONNECTED | 0                    | 0                   },
  {17,  ENDLOS,  0                     | 0                    | 0                   },

  // Powermode
  {18,  ENDLOS,  OUTPUT_AKKU_CONNECTED | 0                    | 0                   },

  { 0, 0, 0}
};

void showState()
{
  Serial.print("actualState = ");
  Serial.print(actualState);
  Serial.print(", actualTimer = ");
  Serial.println(actualTimer);
}

void readAnalogValues ()
{
  VoltageAkku_1 = analogRead(PIN_ANALOG_VOLTAGE_AKKU_1);
  VoltageAkku_2 = analogRead(PIN_ANALOG_VOLTAGE_AKKU_2);
  VoltagePowerIn = analogRead(PIN_ANALOG_VOLTAGE_POWER_IN);

  Serial.print("  Voltage Akku 1 :");
  Serial.print(VoltageAkku_1);
  Serial.print(", Akku 2 :");
  Serial.print(VoltageAkku_2);
  Serial.print(", Power IN :");
  Serial.print(VoltagePowerIn);
  Serial.println(" ");

}


void setup() {

  //  float voltage= sensorValue * (5.0 / 1023.0) * 3; //  15 Volt -- R 20K -- A0 -- R 10K --- GND
//  readAnalogValues ();
//  if (VoltageAkku_1 < VoltageAkku_2)
//  {
//    actualState = 0;
//  }
//  else
//  {
//    actualState = 4;
//  }

  actualState = 0;
  actualTimer = 0;
  //  readAnalogValues ();
  TaktLed = false;

  //start serial connection
  Serial.begin(9600);
  //configure pin2 as an input and enable the internal pull-up resistor
  //pinMode(2, INPUT_PULLUP);

  //pinMode(23, INPUT_PULLUP);
  pinMode(PIN_OUTPUT_BISTABILE_ON, OUTPUT);
  pinMode(PIN_OUTPUT_BISTABILE_OFF, OUTPUT);
  pinMode(PIN_OUTPUT_AKKU_CONNECTED, OUTPUT);
  pinMode(PIN_OUTPUT_TAKT, OUTPUT);

}

void SetHardware(int StateNumber)
{
  Serial.print("  Hardware: ");

  if ( Ablauf[StateNumber].Output & OUTPUT_BISTABILE_ON )
  {
    digitalWrite( PIN_OUTPUT_BISTABILE_ON   , HIGH);
    Serial.print("A");
  }
  else
  {
    digitalWrite( PIN_OUTPUT_BISTABILE_ON   , LOW);
    Serial.print("a");
  }

  if ( Ablauf[StateNumber].Output & OUTPUT_BISTABILE_OFF )
  {
    digitalWrite( PIN_OUTPUT_BISTABILE_OFF   , HIGH);
    Serial.print("B");
  }
  else
  {
    digitalWrite( PIN_OUTPUT_BISTABILE_OFF   , LOW);
    Serial.print("b");
  }

  if ( Ablauf[StateNumber].Output & OUTPUT_AKKU_CONNECTED )
  {
    digitalWrite( PIN_OUTPUT_AKKU_CONNECTED   , HIGH);
    Serial.print("C");
  }
  else
  {
    digitalWrite( PIN_OUTPUT_AKKU_CONNECTED   , LOW);
    Serial.print("c");
  }
}


void  SetTimer(int StateNumber)
{
  actualTimer = Ablauf[StateNumber].Time;
}

//void  SetOutput (int Port, int LedState)
//{
//  if ( LedState )
//  {
//    digitalWrite( Port   , HIGH);
//  }
//  else
//  {
//    digitalWrite( Port   , LOW);
//  }
//}


void  ShowActualState(int actualState)
{
  //  SetOutput (PIN_OUTPUT_BIT0, actualState & 0x01);
  //  SetOutput (PIN_OUTPUT_BIT1, actualState & 0x02);
  //  SetOutput (PIN_OUTPUT_BIT2, actualState & 0x04);
  //  SetOutput (PIN_OUTPUT_BIT3, actualState & 0x08);
  //  SetOutput (PIN_OUTPUT_BIT4, actualState & 0x10);
  //  SetOutput (PIN_OUTPUT_BIT5, actualState & 0x20);
  //  SetOutput (PIN_OUTPUT_BIT6, actualState & 0x40);
  //  SetOutput (PIN_OUTPUT_BIT7, actualState & 0x80);
}

int readButton()
{
  return 0;
}

void ShowTaktLED()
{
  if (TaktLed)
  {
    digitalWrite( PIN_OUTPUT_TAKT   , HIGH);
    Serial.println(" - TAKT -");
    TaktLed = false;
  }
  else
  {
    digitalWrite( PIN_OUTPUT_TAKT   , LOW);
    Serial.println(" - takt -");
    TaktLed = true;
  }
}

void loop() {

  bool automatic = true;

  do {
    ShowTaktLED();
    showState();
    readButton();
    delay(1000);
    
    readAnalogValues ();
    
    if ( VoltagePowerIn > 100 )
    {
       automatic = false;
       Serial.println(" AUTOMATIK ist FALSE");
    }
    else
    {
       automatic = true;
       Serial.println(" AUTOMATIK ist TRUE");
    }
  
    
    if ((actualTimer > 0 )
        && (actualTimer != ENDLOS ))
    {
      actualTimer--;
    }
  } while (actualTimer && automatic);

 
  if (automatic == false)
  {
    actualState = 1;  // laden
  }
  else
  {
    if (actualTimer == 0)
    {
      if (actualState == 7)
      {
        //normal loop for state 0 - 7
        actualState = 0;
      }
      else
      {
        actualState++;
      }
    }
  }
  SetHardware(actualState);
  SetTimer(actualState);
  ShowActualState(actualState);

  Serial.println("");
  Serial.print("Wechsel auf neuen State >>> ");
  showState();
}









