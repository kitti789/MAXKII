#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool currentPage = false;

void displayPage();

void setup() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
  } else {
    Serial.println("SSD1306 initialized");
  }
}

void loop() {
  delay(5000);
  currentPage = !currentPage; // Toggle the page
  displayPage();
}

void displayPage() {
  // Clear the display
  display.clearDisplay();

  // Set the font size
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Draw text based on the current page
  if (!currentPage) {
    display.setCursor(22, 13);
    display.println("Current");
  } else {
    // Add your custom content here for the second page
    display.setCursor(35, 40); // x, y
    display.println("Your Content");
  }

  // Display the content
  display.display();
}