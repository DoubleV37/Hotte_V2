void drawButtons() {
  tft.drawRect(0, 0, BOXSIZE * 2, BOXSIZE, WHITE);
  tft.setCursor(BOXSIZE - 10, 15);
  tft.setTextColor(WHITE); tft.setTextSize(3);
  tft.print("+");

  tft.drawRect(BOXSIZE * 2, 0, BOXSIZE * 2, BOXSIZE, WHITE);
  tft.setCursor(BOXSIZE * 3 - 10, 15);
  tft.setTextColor(WHITE); tft.setTextSize(3);
  tft.print("-");

  tft.drawRect(BOXSIZE * 4, 0, BOXSIZE * 2, BOXSIZE, WHITE);
  tft.setCursor(BOXSIZE * 4 + 20, 15);
  tft.setTextColor(WHITE); tft.setTextSize(3);
  tft.print("Auto");
}

void displayValue() {
  tft.fillRect(BOXSIZE * 2, BOXSIZE, 180, 20, BLACK);
  tft.setCursor(5, 60);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.print("Reglage: ");
  tft.setCursor(BOXSIZE * 2, 60);
  tft.print(value_speed);
  tft.print("%");

  tft.fillRect(BOXSIZE * 2, 80, 180, 20, BLACK);
  tft.setCursor(5, 80);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.print("Vitesse: ");
  tft.setCursor(BOXSIZE * 2, 80);
  tft.println(value_rpm);

  tft.fillRect(BOXSIZE * 2, 110, 180, 20, BLACK);
  tft.setCursor(5, 110);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.print("Temp: ");
  tft.setCursor(BOXSIZE * 2, 110);
  tft.print(value_temp);
  tft.print(" C");
  tft.fillRect(BOXSIZE * 2, 130, 180, 20, BLACK);
  tft.setCursor(5, 130);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.print("Hygro: ");
  tft.setCursor(BOXSIZE * 2, 130);
  tft.print(value_hygro);
  tft.print("%");

  tft.fillRect(BOXSIZE * 2, 160, 180, 20, BLACK);
  tft.setCursor(5, 160);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.print("TVoc: ");
  tft.setCursor(BOXSIZE * 2, 160);
  tft.println(value_tvoc);
  tft.fillRect(BOXSIZE * 2, 180, 180, 20, BLACK);
  tft.setCursor(5, 180);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.print("PM: ");
  tft.setCursor(BOXSIZE * 2, 180);
  tft.println(value_pm);
}

void update_vitesse() {
  tft.fillRect(BOXSIZE * 2, BOXSIZE, 180, 20, BLACK);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.setCursor(BOXSIZE * 2, 60);
  tft.print(value_speed);
  tft.print("%");
}

void update_temp() {
  tft.fillRect(BOXSIZE * 2, 110, 180, 20, BLACK);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.setCursor(BOXSIZE * 2, 110);
  tft.print(value_temp);
  tft.print(" C");
}

void update_rpm() {
  tft.fillRect(BOXSIZE * 2, 80, 180, 20, BLACK);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.setCursor(BOXSIZE * 2, 80);
  tft.println(value_rpm);
}

void update_hygro() {
  tft.fillRect(BOXSIZE * 2, 130, 180, 20, BLACK);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.setCursor(BOXSIZE * 2, 130);
  tft.print(value_hygro);
  tft.print("%");
}

void update_tvoc() {
  tft.fillRect(BOXSIZE * 2, 160, 180, 20, BLACK);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.setCursor(BOXSIZE * 2, 160);
  tft.println(value_tvoc);
}

void update_PM() {
  tft.fillRect(BOXSIZE * 2, 180, 180, 20, BLACK);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.setCursor(BOXSIZE * 2, 180);
  tft.println(value_pm);
}