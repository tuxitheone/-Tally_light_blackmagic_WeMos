/*****************
  Tally light ESP32 for Blackmagic ATEM switcher

  Version 2.0

  A wireless (WiFi) tally light for Blackmagic Design
  ATEM video switchers, based on the M5StickC ESP32 development
  board and the Arduino IDE.

  For more information, see:
  https://oneguyoneblog.com/2020/06/13/tally-light-esp32-for-blackmagic-atem-switcher/

  Based on the work of Kasper Skårhøj:
  https://github.com/kasperskaarhoj/SKAARHOJ-Open-Engineering

******************/

#include <ESP8266WiFi.h>
#include <SkaarhojPgmspace.h>
#include <ATEMbase.h>
#include <ATEMstd.h>

IPAddress switcherIp(192, 168, 0, 101);       // IP address of the ATEM switcher
ATEMstd AtemSwitcher;

// WiFi parameters
#define WLAN_SSID       "atem"
#define WLAN_PASS       "tuxiatem"

char newHostname[12];

// LED PIN DEFINE
#define LED_BUILTIN  D4
#define ledPin1 D1
#define ledPin2 D2

//CAMARA DEFINE
#define ABIT0        D5       // Bit 0 of ID
#define ABIT1        D6       // Bit 1 of ID

int8_t cameraNumber = 4;

int PreviewTallyPrevious = 1;
int ProgramTallyPrevious = 1;

void setup() {

  Serial.begin(9600);
  WiFi.mode(WIFI_STA);

  // Set up address pins
  pinMode( ABIT0, INPUT_PULLUP );    // Has a value of 1
  pinMode( ABIT1, INPUT_PULLUP );    // Has a value of 2


  cameraNumber = 0;
  if (digitalRead( ABIT0 )) cameraNumber = 1;
  if (digitalRead( ABIT1 )) cameraNumber += 2;
  cameraNumber +=  1;

  sprintf(&newHostname[0], "CamTally_%d\n", cameraNumber);

  pinMode(ledPin1, OUTPUT);  // LED: 1 is on Program (Tally)
  pinMode(ledPin2, OUTPUT); // LED: 2 is on Preview (Tally)
  pinMode(LED_BUILTIN, OUTPUT); // LED: Status online


  Serial.println(); Serial.println(); // Connect to WiFi access point.
  delay(10);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);

  WiFi.hostname(newHostname); //Set new hostname
  Serial.printf("New hostname: %s\n", WiFi.hostname().c_str()); //Get Current Hostname

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  Serial.println(F("WiFi connected"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());

  digitalWrite(LED_BUILTIN, LOW); // ON
  digitalWrite(ledPin1, HIGH); // off
  digitalWrite(ledPin2, HIGH); // off

  // Initialize a connection to the switcher:
  AtemSwitcher.begin(switcherIp);
  AtemSwitcher.serialOutput(0x80);
  AtemSwitcher.connect();
}

void loop() {

  // Check for packets, respond to them etc. Keeping the connection alive!
  AtemSwitcher.runLoop();

  int ProgramTally = AtemSwitcher.getProgramTally(cameraNumber);
  int PreviewTally = AtemSwitcher.getPreviewTally(cameraNumber);

  if ((ProgramTallyPrevious != ProgramTally) || (PreviewTallyPrevious != PreviewTally)) { // changed?

    if ((ProgramTally && !PreviewTally) || (ProgramTally && PreviewTally) ) { // only program, or program AND preview
      digitalWrite(ledPin1, HIGH);
      digitalWrite(ledPin2, LOW);
    } else if (PreviewTally && !ProgramTally) { // only preview
      digitalWrite(ledPin2, LOW);
    } else if (!PreviewTally || !ProgramTally) { // neither
      digitalWrite(ledPin1, LOW);
      digitalWrite(ledPin2, HIGH);
    }

  }

  ProgramTallyPrevious = ProgramTally;
  PreviewTallyPrevious = PreviewTally;
}

void drawLabel(unsigned long int screenColor, unsigned long int labelColor, bool ledValue) {
  digitalWrite(ledPin1, ledValue);
  digitalWrite(ledPin2, ledValue);
}
