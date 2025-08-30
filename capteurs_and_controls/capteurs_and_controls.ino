#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_SGP30.h>
#include <Wire.h>

// Définir le GPIO de sortie PWM pour le contrôle du ventilateur
const int fanPWMPin = 5; // Utilisez un GPIO disponible sur votre ESP32-C3U, par exemple GPIO5
const int fanTachPin = 4; // GPIO pour la broche tachymétrique (RPM) du ventilateur
const int relaiFan = 3;

volatile unsigned long lastPulseTime = 0; // Temps de la dernière impulsion en micros
volatile unsigned long pulseInterval = 0; // Intervalle entre les deux dernières impulsions en micros
volatile bool newPulseDetected = false;   // Drapeau pour signaler une nouvelle impulsion

// --- Constantes pour le calcul du RPM ---
// Pour la plupart des ventilateurs 4 broches, il y a 2 impulsions par tour.
// Si on déclenche sur CHANGE (front montant et descendant), on a 4 détections par tour.
// Si on déclenche sur FALLING (front descendant), on a 2 détections par tour.
// Choisissons FALLING pour simplifier le calcul et avoir la période d'une impulsion complète.
const float pulsesPerRevolution = 2.0; // Vérifiez la datasheet de votre ventilateur, c'est généralement 2.

// Paramètres PWM
const int freq = 25000;      // Fréquence PWM en Hz (25kHz est une bonne valeur pour les ventilateurs, évite le bruit audible)
const int ledChannel = 0;   // Canal LEDC (0-15) - L'ESP32 a plusieurs canaux LEDC
const int resolution = 8;   // Résolution PWM (8 bits pour 0-255, 10 bits pour 0-1023, etc.)

// --- Paramètres du capteur DHT22 ---
const int DHTPIN = 6;      // Broche GPIO à laquelle le capteur DHT22 est connecté
#define DHTTYPE DHT22      // Spécifier le type de capteur : DHT11, DHT21, DHT22
DHT dht(DHTPIN, DHTTYPE);
float hygro = 0;
float temp = 0;

// --- Variables pour la lecture DHT ---
unsigned long lastDHTReadTime = 0;
const unsigned long dhtReadInterval = 5000;

// --- Paramètres COM Serie entre Arduibo Mega et Esp32
const int RXD2 = 20;
const int TXD2 = 21;

int vitesse = 0;
int new_vitesse = 0;
float rpm = 0;

//SGP30
const int I2C_SDA_PIN = 8;
const int I2C_SCL_PIN = 7;
uint16_t tvoc = 0;
uint16_t co2 = 0;
Adafruit_SGP30 sgp;

void IRAM_ATTR detectPulse() {
  unsigned long currentTime = micros();
  if (lastPulseTime != 0) { // Pour éviter un calcul avec le premier déclenchement
    pulseInterval = currentTime - lastPulseTime;
    newPulseDetected = true;
  }
  lastPulseTime = currentTime;
}

void setup() {
  // Serial.begin(9600);
  Serial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Démarrage du contrôle du ventilateur PWM...");

  // Configurer et attacher le canal LEDC au GPIO spécifié
  // Utilisation de ledcAttachChannel() qui regroupe la configuration et l'attachement
  if (!ledcAttachChannel(fanPWMPin, freq, resolution, ledChannel)) {
    Serial.println("Erreur: Impossible d'attacher le pin au canal LEDC.");
    while(1); // Arrêter l'exécution si l'attachement échoue
  }
  pinMode(relaiFan, OUTPUT);
  digitalWrite(relaiFan, LOW);
  pinMode(fanTachPin, INPUT_PULLUP); // Activer la résistance de PULLUP interne
  attachInterrupt(digitalPinToInterrupt(fanTachPin), detectPulse, FALLING);

  Serial.println("LED Channel configuré et attaché au pin.");
  ledcWriteChannel(ledChannel, vitesse);

  dht.begin();
  Serial.println("DHT init");

  Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.begin();
  if (!sgp.begin()) {
    Serial.println("Erreur: Capteur SGP30 non trouvé ou initialisation échouée. Vérifiez le câblage I2C !");
    while (1); // Bloquer l'exécution si le capteur n'est pas trouvé
  }
  Serial.println("Capteur SGP30 initialisé.");
}

void loop() {
  if (Serial.available()) {
    new_vitesse = Serial.parseInt();
  }
  if (new_vitesse > 0 ) {
    if (vitesse == 0)
      digitalWrite(relaiFan, HIGH);
    vitesse = new_vitesse;
  } else {
    if (new_vitesse == -1) {
      digitalWrite(relaiFan, HIGH);
      autoVitesse();
    } else if (new_vitesse == -2) {
      vitesse = 0;
      digitalWrite(relaiFan, LOW);
    }
  }
  ledcWriteChannel(ledChannel, vitesse); // Utilisation de ledcWriteChannel pour définir le cycle de service
  
  // --- Lecture et affichage du RPM ---
  if (newPulseDetected) {
    // Désactiver temporairement les interruptions pour lire les variables 'volatile' en toute sécurité
    detachInterrupt(digitalPinToInterrupt(fanTachPin));

    unsigned long currentPulseInterval = pulseInterval;
    newPulseDetected = false; // Réinitialiser le drapeau

    // Réactiver les interruptions
    attachInterrupt(digitalPinToInterrupt(fanTachPin), detectPulse, FALLING);

    // Si l'intervalle est très grand (ventilateur arrêté ou très lent), éviter la division par zéro
    if (currentPulseInterval > 0) {
      // RPM = (microsecondes par minute) / (impulsions par tour * microsecondes par impulsion)
      // microsecondes par minute = 60 secondes/minute * 1 000 000 microsecondes/seconde = 60 000 000
      // RPM = (60,000,000 / currentPulseInterval) / pulsesPerRevolution
      rpm = (6000000.0 / currentPulseInterval) / pulsesPerRevolution;
    }
  }

  // Gérer le cas où le ventilateur s'arrête ou tourne trop lentement
  // Si aucune impulsion n'est détectée pendant un certain temps (ex: 2 secondes), assumez RPM = 0
  if (micros() - lastPulseTime > 2000000 && lastPulseTime != 0) { // 2 secondes sans impulsion
    if (pulseInterval != 0) { // Éviter de réafficher 0 RPM si déjà affiché
      rpm = 0;
      pulseInterval = 0; // Réinitialiser pour ne pas réafficher continuellement
    }
  } else if (vitesse == 0 && lastPulseTime != 0 && pulseInterval != 0) {
      // Si la vitesse PWM est à 0 et qu'on avait des impulsions, mais plus maintenant
      if (micros() - lastPulseTime > 1000000) { // Attendre 1 seconde après la dernière impulsion
         rpm = 0;
         pulseInterval = 0;
      }
  }
  readDHT22();
  readTVOC();
  sendCOM();
}

// --- Fonction utilitaire pour calculer l'humidité absolue (requise par SGP30.setHumidity) ---
// input: temperature [°C], relative humidity [%RH]
// output: absolute humidity [mg/m^3]
uint32_t getAbsoluteHumidity(float temperature, float relative_humidity) {
  // Convertir l'humidité relative en fraction (ex: 50% -> 0.5)
  const float rh_fraction = relative_humidity / 100.0f;
  // Calculer la pression de vapeur saturante (en hPa)
  const float svp = 6.112f * exp((17.62f * temperature) / (243.12f + temperature));
  // Calculer la densité de vapeur d'eau (en g/m^3)
  const float vapor_density_gm3 = rh_fraction * svp * (18.015f / 8.314472f) / (273.15f + temperature);
  // Convertir en mg/m^3 (et arrondir à l'entier le plus proche)
  return (uint32_t)(vapor_density_gm3 * 1000.0f);
}

void readTVOC() {
  if (!sgp.IAQmeasure()) {
    Serial.println("Erreur de lecture du SGP30!");
    tvoc = 0; // Ou une valeur d'erreur spécifique
    co2 = 0;
  } else {
    tvoc = sgp.TVOC;
    co2 = sgp.eCO2;
  }

  // Fournir la compensation de l'humidité au SGP30 si le DHT a une bonne lecture
  // C'est important pour la précision des mesures du SGP30.
  if (!isnan(temp) && !isnan(hygro)) {
    uint32_t absolute_humidity = getAbsoluteHumidity(temp, hygro);
    sgp.setHumidity(absolute_humidity);
  }

  // Sauvegarder les baselines (optionnel, mais recommandé pour la précision à long terme)
  // Les baselines aident le capteur à s'adapter à l'environnement.
  // Idéalement, elles devraient être stockées dans la mémoire non volatile (NVRAM/EEPROM)
  // et rechargées au démarrage. Ici, on se contente de les re-calculer.
  // if (millis() - lastBaselineSave >= baselineSaveInterval) {
  //   lastBaselineSave = millis();
  //   Serial.print("Baseline TVOC: 0x"); Serial.print(sgp.getTVOC_baseline(), HEX);
  //   Serial.print(" eCO2: 0x"); Serial.println(sgp.geteCO2_baseline(), HEX);
  //   // Ici, vous devriez sauvegarder ces valeurs dans l'EEPROM de l'ESP32 pour les réutiliser au prochain démarrage.
  // }
}

void sendCOM() {
  Serial.print("RPM:");
  Serial.println(rpm);
  delay(100);
  Serial.print("temp:");
  Serial.println(temp);
  delay(100);
  Serial.print("hygro:");
  Serial.println(hygro);
  delay(100);
  Serial.print("tvoc:");
  Serial.println(tvoc);
  delay(100);
  Serial.print("co2:");
  Serial.println(co2);
  delay(100);
}

void readDHT22() {
  // --- Lecture et affichage du DHT22 ---
  if (millis() - lastDHTReadTime >= dhtReadInterval) {
    lastDHTReadTime = millis();

    // Lire la température en Celsius
    float h = dht.readHumidity();
    float t = dht.readTemperature(); // Par défaut en Celsius

    // Vérifier si la lecture a échoué
    if (isnan(h) || isnan(t)) {
      Serial.println("Erreur de lecture du capteur DHT22 !");
    } else {
      hygro = h;
      temp = t;
    }
  }
}

void autoVitesse() {
  if (co2 > 800 || tvoc > 150) {
    vitesse += 20;
  } else if (co2 < 500 || tvoc < 50) {
    vitesse -= 20;
  } else {
    vitesse = 150;
  }
  if (vitesse > 255)
    vitesse = 255;
  else if (vitesse < 10) 
    vitesse = 10;
}