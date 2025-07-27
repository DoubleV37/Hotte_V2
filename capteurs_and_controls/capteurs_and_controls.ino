// Définir le GPIO de sortie PWM pour le contrôle du ventilateur
const int fanPWMPin = 5; // Utilisez un GPIO disponible sur votre ESP32-C3U, par exemple GPIO5
const int fanTachPin = 4; // GPIO pour la broche tachymétrique (RPM) du ventilateur

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

const int RXD2 = 20; // Exemple de broche RX (GPIO4)
const int TXD2 = 21; // Exemple de broche TX (GPIO5)

int vitesse = 0;
int new_vitesse = 0;

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
  // Note: ledcAttachChannel retourne un bool, donc on peut vérifier si l'opération a réussi
  if (!ledcAttachChannel(fanPWMPin, freq, resolution, ledChannel)) {
    Serial.println("Erreur: Impossible d'attacher le pin au canal LEDC.");
    while(1); // Arrêter l'exécution si l'attachement échoue
  }
  pinMode(fanTachPin, INPUT_PULLUP); // Activer la résistance de PULLUP interne
  attachInterrupt(digitalPinToInterrupt(fanTachPin), detectPulse, FALLING);

  Serial.println("LED Channel configuré et attaché au pin.");
}

void loop() {
  if (Serial.available()) {
    new_vitesse = Serial.parseInt();
    if (new_vitesse != 0) vitesse = new_vitesse;
  }

  Serial.print("vitesse:");
  Serial.println(vitesse);
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
      float rpm = (60000000.0 / currentPulseInterval) / pulsesPerRevolution;
      Serial.print("RPM:");
      Serial.println(rpm);
    }
  }

  // Gérer le cas où le ventilateur s'arrête ou tourne trop lentement
  // Si aucune impulsion n'est détectée pendant un certain temps (ex: 2 secondes), assumez RPM = 0
  if (micros() - lastPulseTime > 2000000 && lastPulseTime != 0) { // 2 secondes sans impulsion
    if (pulseInterval != 0) { // Éviter de réafficher 0 RPM si déjà affiché
      Serial.println("RPM:0");
      pulseInterval = 0; // Réinitialiser pour ne pas réafficher continuellement
    }
  } else if (vitesse == 0 && lastPulseTime != 0 && pulseInterval != 0) {
      // Si la vitesse PWM est à 0 et qu'on avait des impulsions, mais plus maintenant
      if (micros() - lastPulseTime > 1000000) { // Attendre 1 seconde après la dernière impulsion
         Serial.println("RPM:0");
         pulseInterval = 0;
      }
  }
}