// Définir le GPIO de sortie PWM pour le contrôle du ventilateur
const int fanPWMPin = 5; // Utilisez un GPIO disponible sur votre ESP32-C3U, par exemple GPIO5

// Paramètres PWM
const int freq = 25000;      // Fréquence PWM en Hz (25kHz est une bonne valeur pour les ventilateurs, évite le bruit audible)
const int ledChannel = 0;   // Canal LEDC (0-15) - L'ESP32 a plusieurs canaux LEDC
const int resolution = 8;   // Résolution PWM (8 bits pour 0-255, 10 bits pour 0-1023, etc.)

const int RXD2 = 20; // Exemple de broche RX (GPIO4)
const int TXD2 = 21; // Exemple de broche TX (GPIO5)

int vitesse = 0;
int new_vitesse = 0;

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

  Serial.println("LED Channel configuré et attaché au pin.");
}

void loop() {
  if (Serial.available()) {
    // Si des données sont disponibles sur Serial1, les lire.
    // readStringUntil('\n') lit jusqu'à rencontrer un retour chariot ou un timeout.
    // String strvitesse = Serial.readStringUntil('\n');
    new_vitesse = Serial.parseInt();
    if (new_vitesse != 0) vitesse = new_vitesse; 
  }
  // Exemples de contrôle de vitesse

  // Vitesse minimale (arrêt ou très lent selon le ventilateur)
  Serial.print("Vitesse :");
  Serial.println(vitesse);
  ledcWriteChannel(ledChannel, vitesse); // Utilisation de ledcWriteChannel pour définir le cycle de service
  delay(500);

  // Vitesse moyenne (environ 50%)
  // Serial.println("Vitesse : Moyenne (127)");
  // ledcWriteChannel(ledChannel, 127); // Cycle de service de 127 (environ 50% pour 8 bits)
  // delay(5000);

  // // Vitesse maximale (100%)
  // Serial.println("Vitesse : Maximale (255)");
  // ledcWriteChannel(ledChannel, 255); // Cycle de service de 255 (100% pour 8 bits)
  // delay(5000);

  // // Cycle de vitesse de 0 à 100% et retour
  // Serial.println("Cycle de vitesse...");
  // for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
  //   ledcWriteChannel(ledChannel, dutyCycle);
  //   delay(20); // Ajustez pour un balayage plus ou moins rapide
  // }
  // for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {
  //   ledcWriteChannel(ledChannel, dutyCycle);
  //   delay(20); // Ajustez pour un balayage plus ou moins rapide
  // }
  // delay(2000);
}