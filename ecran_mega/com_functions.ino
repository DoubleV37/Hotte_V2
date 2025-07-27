void decode_recv_msg(String msg) {
  int separatorIndex = msg.indexOf(':');
  if (separatorIndex == -1) {
    Serial.println("Format invalide. Utilise 'nom:valeur'.");
    return;
  }

  String key = msg.substring(0, separatorIndex);
  String valueStr = msg.substring(separatorIndex + 1);
  float value = valueStr.toFloat();

  key.trim();
  valueStr.trim();

  if (key == "RPM") {
    value_rpm = value;
    update_rpm();
  } else if (key == "hygro") {
    value_hygro = value;
    update_hygro();
  } else if (key == "temp") {
    value_temp = value;
    update_temp();
  }
}