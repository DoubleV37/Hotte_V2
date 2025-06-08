#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;       // hard-wired for UNO shields anyway.
#include <TouchScreen.h>

char *name = "Please Calibrate.";  //edit name of shield
const int XP=6,XM=A2,YP=A1,YM=7; //240x320 ID=0x9341
const int TS_LEFT=175,TS_RT=920,TS_TOP=950,TS_BOT=175;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;

#define MINPRESSURE 200
#define MAXPRESSURE 1000

int16_t BOXSIZE;
int16_t PENRADIUS = 1;
uint16_t ID;
uint8_t Orientation = 1;    //PAYSAGE

int value_speed = 0;
int value_rpm = 0;

float value_temp = 0;
float value_hygro = 0;

float value_tvoc = 0;
float value_pm = 0;

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

void setup(void)
{
    uint16_t tmp;

    tft.reset();
    ID = tft.readID();
    tft.begin(ID);
    Serial.begin(9600);
    tft.setRotation(Orientation);
    tft.fillScreen(BLACK);

    BOXSIZE = tft.width() / 6;
    tft.fillScreen(BLACK);

    drawButtons();
    displayValue();

    tft.drawRect(0, 0, BOXSIZE, BOXSIZE, WHITE);
    delay(1000);
}

void loop()
{
    uint16_t xpos, ypos;  //screen coordinates
    tp = ts.getPoint();   //tp.x, tp.y are ADC values

    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE) {
        xpos = map(tp.y, TS_TOP, TS_BOT, 0, tft.width());
        ypos = map(tp.x, TS_RT, TS_LEFT, 0, tft.height());

        // are we in top color box area ?
        if (ypos < BOXSIZE) {               //draw white border on selected color box
            if (xpos < BOXSIZE) {
                value_speed += 10;
                tft.drawRect(0, 0, BOXSIZE, BOXSIZE, YELLOW);
            } else if (xpos < BOXSIZE * 2) {
                value_speed -= 10;
                if (value_speed <= 0) value_speed = 0;
                tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, YELLOW);
            } else if (xpos < BOXSIZE * 3) {
                value_speed = 100;
                tft.drawRect(BOXSIZE * 2, 0, BOXSIZE * 2, BOXSIZE, YELLOW);
            }
        }
        displayValue();
    }
    tft.drawRect(0, 0, BOXSIZE, BOXSIZE, WHITE);
    tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, WHITE);
    tft.drawRect(BOXSIZE * 2, 0, BOXSIZE * 2, BOXSIZE, WHITE);
}

void drawButtons() {
  tft.drawRect(0, 0, BOXSIZE, BOXSIZE, WHITE);
  tft.setCursor(0 + 20, 15);
  tft.setTextColor(WHITE); tft.setTextSize(3);
  tft.print("+");

  tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, WHITE);
  tft.setCursor(BOXSIZE + 20, 15);
  tft.setTextColor(WHITE); tft.setTextSize(3);
  tft.print("-");

  tft.drawRect(BOXSIZE * 2, 0, BOXSIZE * 2, BOXSIZE, WHITE);
  tft.setCursor(BOXSIZE * 2 + 20, 15);
  tft.setTextColor(WHITE); tft.setTextSize(3);
  tft.print("Auto");
}

void displayValue() {
  tft.fillRect(5, 60, 180, 180, BLACK); // Efface ancienne valeur
  tft.setCursor(5, 60);
  tft.setTextColor(YELLOW); tft.setTextSize(2);
  tft.print("Reglage: ");
  tft.print(value_speed);
  tft.print("%");

  tft.setCursor(5, 80);
  tft.setTextColor(YELLOW); tft.setTextSize(2);
  tft.print("Vitesse: ");
  tft.println(value_rpm);

  tft.setCursor(5, 110);
  tft.setTextColor(YELLOW); tft.setTextSize(2);
  tft.print("Temp: ");
  tft.print(value_temp);
  tft.print(" C");
  tft.setCursor(5, 130);
  tft.setTextColor(YELLOW); tft.setTextSize(2);
  tft.print("Hygro: ");
  tft.print(value_hygro);
  tft.print("%");

  tft.setCursor(5, 160);
  tft.setTextColor(YELLOW); tft.setTextSize(2);
  tft.print("TVoc: ");
  tft.println(value_tvoc);
  tft.setCursor(5, 180);
  tft.setTextColor(YELLOW); tft.setTextSize(2);
  tft.print("PM: ");
  tft.println(value_pm);
}