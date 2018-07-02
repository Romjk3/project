#include <FastLED.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <SparkFun_APDS9960.h>

#ifdef __AVR__
#include <avr/power.h>
#endif

#define APDS9960_INT 2
#define NUM_RUN_LEDS      33  
#define NUM_DOWN_LEDS      61 
#define DATA_PIN3 3 
#define DATA_PIN6 6 

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
SparkFun_APDS9960 apds = SparkFun_APDS9960();

CRGB leds_run[NUM_RUN_LEDS]; 
CRGB leds_down[NUM_DOWN_LEDS];
int isr_flag =0;
int sensorPin = A0; // Пин для датчика давления
int Val = 0;  // Давление
char bluetooth = 'r'; // Bluetooth
int state = 'D'; // Мод 
int sense = 200; // Чувстительность
uint8_t bright = 50;
uint8_t pale = 255;
uint8_t color_1 = 0;
uint8_t color_2 = 64;

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code
 
  FastLED.addLeds<NEOPIXEL, DATA_PIN3>(leds_run, NUM_RUN_LEDS); // Инициализация
  FastLED.addLeds<NEOPIXEL, DATA_PIN6>(leds_down, NUM_DOWN_LEDS); // Инициализация
  Serial.begin(9600);
  mlx.begin();
  attachInterrupt(0, interruptRoutine, FALLING);
  apds.init();
  apds.enableGestureSensor();
  SetColour(color_1);  
  }

void SetColour(uint8_t current_color) // Поставить ленты в цвет c1 
{  
  for (int i = 0; i < NUM_RUN_LEDS; i++)
  {
    leds_run[i] = CHSV(current_color, pale, bright);
    FastLED.show();
  }
  for (int i = 0; i < NUM_DOWN_LEDS; i++)
  {
    leds_down[i] = CHSV(current_color, pale, bright);
    FastLED.show();
  }
}

void Run() // Бегущая лента
{
  for(int dot = NUM_RUN_LEDS-1; dot >= 0; dot--) 
  { 
            leds_run[dot] = CHSV( color_1, pale, bright);
            FastLED.show();
            // clear this led for the next time around the loop
            leds_run[dot] = CHSV( color_1, pale, 0);
            delay(15);
  }
  FastLED.show();
  delay(75); // Delay
}

void Signal() //Общение
{   
  bluetooth = Serial.read(); //Получить сообщение
   
  if (bluetooth == 'r')
  {
    color_1 = 0;
    color_2 = 64;
  }

  if (bluetooth == 'c')
  {
    color_1 = 128;
    color_2 = 160;
  }
  
  if (bluetooth == 'g')
  {
    color_1 = 96;
    color_2 = 160;
  }
  
  if (bluetooth == 'D') {state = 'D';}
  if (bluetooth == 'B') {state = 'B';}
  if (bluetooth == 'P') {state = 'P';}
  
  if (bluetooth == 'T') {Serial.print(mlx.readObjectTempC());} //Получить температуру объекта
  if (bluetooth == 't') {Serial.print(mlx.readAmbientTempC());} //Получить температуру датчика
  
  if  (state == 'D') { SetColour(color_1);}
  if  ((state == 'B') || (state == 'P'))
  { 
    for (int i = 0; i < NUM_RUN_LEDS; i++)
    {
      leds_run[i] = CHSV(color_1, pale, 0);
      FastLED.show();
    }
    for (int i = 0; i < NUM_DOWN_LEDS; i++)
    {
      leds_down[i] = CHSV(color_1, pale, 0);
      FastLED.show();
    }
  }
}
void interruptRoutine() 
{
  isr_flag = 1;
}

void handleGesture()
{
  if ( apds.isGestureAvailable())
  {
    switch (apds.readGesture())
    {
      case DIR_UP:
        Serial.println("UP");
        break;
      case DIR_DOWN:
        Serial.println("DOWN");
        break;
      case DIR_LEFT:
        Serial.println("LEFT");
        break;
      case DIR_RIGHT:
        Serial.println("RIGHT");
        break;
      case DIR_NEAR:
        Serial.println("NEAR");
        break;
      case DIR_FAR:
        Serial.println("FAR");
        break;
      default:
        Serial.println("NONE");
    }
  }
}

void loop() {
  if (Serial.available() > 0) {Signal();}
  Val = analogRead(sensorPin);
  if( isr_flag == 1)
  {
    detachInterrupt(0);
    handleGesture();
    isr_flag = 0;
    attachInterrupt(0, interruptRoutine, FALLING);
  }
  
  if (Val < sense)
  {
    if (state == 'B')
    { 
      uint8_t k_up = 0;
      uint8_t Max_1 = 0;
      uint8_t Max_porog = 0;
      Max_1 = analogRead(sensorPin);
      Run();
      
      while ((sense > Val) && (k_up <= Max_porog)) 
      {
        Val = analogRead(sensorPin);   
        if  (Max_1 > Val)
        {
          Max_1 = Val;
        }
        Max_porog = 255 - Max_1;
        k_up += 5;
        for (int i = 0; i < NUM_DOWN_LEDS; i++) //Ленту внизу окрасить 
        {
          leds_down[i] = CHSV( color_1, pale, k_up);
          FastLED.show();
        }
      } 
      while (k_up != 0)
      {   
          k_up-=5;
          for (int i = 0; i < NUM_DOWN_LEDS; i++) 
          {
            leds_down[i] = CHSV( color_1, pale, k_up);
            FastLED.show();
          }
      }      
    } 
    if (state == 'D')
    {
      Run();
      for (int i = 0; i < NUM_DOWN_LEDS; i++)
      {
        leds_down[i] = CHSV( color_2, pale, bright);
        FastLED.show();
      }
    }
  }  
}


