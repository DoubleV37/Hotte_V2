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
float value_rpm = 0;

float value_temp = 0;
float value_hygro = 0;

uint16_t value_tvoc = 0;
uint16_t value_co2 = 0;

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
    Serial1.begin(9600);
    Serial.begin(115200);

    tft.reset();
    ID = tft.readID();
    tft.begin(ID);
    tft.setRotation(Orientation);
    tft.fillScreen(BLACK);

    BOXSIZE = tft.width() / 6;
    tft.fillScreen(BLACK);

    drawButtons();
    displayValue();

    delay(1000);
}

void loop()
{
    if (Serial1.available()) {
    // Si des données sont disponibles sur Serial1, les lire.
    // readStringUntil('\n') lit jusqu'à rencontrer un retour chariot ou un timeout.
    String messageESP32 = Serial1.readStringUntil('\n');

    decode_recv_msg(messageESP32);
    }
    uint16_t xpos, ypos;  //screen coordinates
    tp = ts.getPoint();   //tp.x, tp.y are ADC values

    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE) {
        xpos = map(tp.y, TS_TOP, TS_BOT, 0, tft.width());
        ypos = map(tp.x, TS_RT, TS_LEFT, 0, tft.height());

        if (ypos < BOXSIZE) {
            if (xpos < BOXSIZE * 2) {
                value_speed += 10;
                if (value_speed >= 100) { 
                    value_speed = 100;
                } else {
                    Serial1.println(value_speed*255/100);
                }
                tft.drawRect(0, 0, BOXSIZE * 2, BOXSIZE, YELLOW);
            } else if (xpos < BOXSIZE * 4) {
                value_speed -= 10;
                if (value_speed <= 0) {
                    value_speed = 0;
                    Serial1.println(-2);
                } else {
                    Serial1.println(value_speed*255/100);
                }
                tft.drawRect(BOXSIZE * 2, 0, BOXSIZE * 2, BOXSIZE, YELLOW);
            } else if (xpos < BOXSIZE * 6) {
                value_speed = -1;
                tft.drawRect(BOXSIZE * 4, 0, BOXSIZE * 2, BOXSIZE, YELLOW);
                Serial1.println(-1);
            }
        }
        update_vitesse();
    }
    tft.drawRect(0, 0, BOXSIZE * 2, BOXSIZE, WHITE);
    tft.drawRect(BOXSIZE * 2, 0, BOXSIZE * 2, BOXSIZE, WHITE);
    tft.drawRect(BOXSIZE * 4, 0, BOXSIZE * 2, BOXSIZE, WHITE);
}