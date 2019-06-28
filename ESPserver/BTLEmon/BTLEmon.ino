
#include <WiFi.h>
#include "BLEDevice.h"
#include <U8g2lib.h>

#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <stdarg.h>
#include <assert.h>


#define DATA_HOST "data.cheswick.com"
#define DATA_PORT 8334
#define PASSWORD  "wickwick"

#define PRODUCTION  YES

#ifdef PRODUCTION

#define POLL_SLEEP_T      (60*60)
#define MAX_SLEEP_CHUNK   (30*60)    // hiberating for 60 minutes doesn't work for some reason
#define BT_SCAN_TIME      60 

#else

#define POLL_SLEEP_T      (5*60)
#define MAX_SLEEP_CHUNK   (2*60)
#define BT_SCAN_TIME      20 

#endif

// saved across suspensions:
RTC_NOINIT_ATTR String localBTAddr;
RTC_NOINIT_ATTR String MyID;   // last 2 digits of the local BT address
RTC_NOINIT_ATTR int entryColumns = 1;
RTC_NOINIT_ATTR int secondsToSleep = 0;
RTC_NOINIT_ATTR int lastSleepChunk = 0;

RTC_NOINIT_ATTR int BTCount = 0;

#define SCREEN_ROWS (u8g2.getDisplayHeight()/u8g2.getMaxCharHeight())
#define SCREEN_COLS (u8g2.getDisplayWidth()/u8g2.getMaxCharWidth())


void erreurFatale(int n) {
  while (1) {
    for (int i=0; i<n; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(300);
    }
    delay(1000);
  }
}

void initLED(void) {
  pinMode(LED_BUILTIN, OUTPUT);
}

const char* wl_status_to_string(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
  }
}

U8G2_SSD1306_128X64_NONAME_F_HW_I2C   u8g2(U8G2_R0, 16, 15, 4);

enum FontSize {
  TinyFont,
  SmallFont,
  MediumFont,
  LargeFont,
};

void initScreen(void) {
  u8g2.begin();
  u8g2.clear();
  
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void screenLayout(int fs, int columns) {
  assert(columns > 0);
  entryColumns = columns;
  u8g2.clearBuffer();
  
  switch (fs) {
    case TinyFont:
      u8g2.setFont(u8g2_font_4x6_mf); // real noseprint, 32x9.5
      break;
    case SmallFont:
      u8g2.setFont(u8g2_font_5x7_mf); // 25x7.  definitely readable
      break;
    case MediumFont:
//        u8g2.setFont(u8g2_font_ncenB08_tf);
        u8g2.setFont(u8g2_font_5x8_mf);   // 25x8.  nice
      break;
    case LargeFont:
//        u8g2.setFont(u8g2_font_ncenB10_tf);
        u8g2.setFont(u8g2_font_6x10_mf);  // 21x5.4
  }
  u8g2.setFontPosTop();
}

// Screen entry positions.  All entries exclude the top line, which is the header

#define ENTRIES_IN_COL  (SCREEN_ROWS - 1)
#define MAX_ENTRIES     (ENTRIES_IN_COL*entryColumns)
#define ENTRY_W         (SCREEN_COLS/entryColumns)
#define ENTRY_X(e)
#define ENTRY_Y(e)

#define HEADER  (-1)
void displayEntry(int e, char *fmt, ...) {
  assert(MAX_ENTRIES > 0);
  assert(entryColumns > 0 && entryColumns < 6);
  va_list ap;
  char buf[100];

  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  int screen = 0;
  if (e == HEADER) {
    u8g2.drawStr(0, 0, buf);
  } else {
    screen = e / MAX_ENTRIES;
    e = e % MAX_ENTRIES;
    int c = e / ENTRIES_IN_COL;
    int r = e % ENTRIES_IN_COL + 1; // top row is the header
    int x = c*(ENTRY_W + 1)*u8g2.getMaxCharWidth();
    int y = r*u8g2.getMaxCharHeight();
    assert(x >= 0 && x < u8g2.getDisplayWidth());
    assert(x + strlen(buf)*u8g2.getMaxCharWidth() < u8g2.getDisplayWidth());
    assert(y >= 0 && y < u8g2.getDisplayHeight());
    Serial.print("@");
    Serial.print(x);
    Serial.print(",");
    Serial.print(y);
    Serial.print("  '");
    Serial.print(buf);
    Serial.println("'");
    
    u8g2.drawStr(x, y, buf);

  // if we have too many entries, show what screen we are on in the
  // upper right corner

    if (screen) {
      snprintf(buf, sizeof(buf), "%+2d", screen);
      int x = u8g2.getDisplayWidth() - 2*u8g2.getMaxCharWidth();
      u8g2.drawStr(x, 0, buf);
    }
  }
  u8g2.sendBuffer();
}

int connectToWiFi(const char *SSID, const char *pw) {
  if (WiFi.status() == WL_CONNECTED)
    return 1;
    
  WiFi.begin(SSID, PASSWORD);             // Connect to the network
//Serial.print("Connecting to '");
//Serial.print(WiFi.SSID(i).c_str()); Serial.println("' ...");

  for (int j = 0; WiFi.status() != WL_CONNECTED && j < 7; j++) { // Wait for the Wi-Fi to connect
    delay(1000);
  }
  return WiFi.status() == WL_CONNECTED;
}


void gotoSleep(void) {
  assert(secondsToSleep > 0);
  lastSleepChunk = secondsToSleep;
  if (lastSleepChunk > MAX_SLEEP_CHUNK) {
    lastSleepChunk = MAX_SLEEP_CHUNK;
  }
  Serial.print("Sleeping for ");
  Serial.print(lastSleepChunk);
  Serial.print(" of ");
  Serial.print(secondsToSleep);
  Serial.println("  --------------------------");
  
  u8g2.setPowerSave(1);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  // Needed to keep the RTC_NOINIT_ATTR variables sane.  I wonder how much power...
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON); 
  WiFi.mode( WIFI_MODE_NULL );
  btStop();
  esp_sleep_enable_timer_wakeup((uint64_t)lastSleepChunk*1000000);  // sleep time is in microseconds
  esp_deep_sleep_start();
  // NOTREACHED
}

void startSleepFor(uint sleepSeconds) {
  assert(sleepSeconds > 0);
  secondsToSleep = sleepSeconds;
  gotoSleep();
}

// If the full sleep time isn't finished, go back and do some more
void processWakeup(void) {
  Serial.println(localBTAddr);
  if (secondsToSleep == 0) {  // first startup, not sleeping
    Serial.println("Starting...");
    return;
  }
  Serial.print("Wake up: ");
  Serial.print(lastSleepChunk);
  Serial.print(" of ");
  Serial.println(secondsToSleep);
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED) {  // startup: we weren't sleeping
    Serial.println("Startup?!");
    return;
  }
  
  if (wakeup_reason != ESP_SLEEP_WAKEUP_TIMER) {
    Serial.print("Unexpected wake up:  ");
    switch(wakeup_reason) {
      case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
      case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
      case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
      case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
 //     case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
      default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
    }
    gotoSleep();  // beats me, try again
  }

Serial.print("slept ");
Serial.print(lastSleepChunk);
  secondsToSleep -= lastSleepChunk;
Serial.print(", remaining: ");
Serial.println(secondsToSleep);
  if (secondsToSleep > 0) {
    gotoSleep();
    //NOTREACHED
  }
  secondsToSleep = 0;
  Serial.println("Fully awake");
}

int scanForWiFi(String id, char *pw) {
  int i;
  char *ssid = 0;

  int nWiFi = WiFi.scanNetworks();
  if (nWiFi == 0) {
    Serial.println("No WiFi networks found");
      return 0;
  }

  displayEntry(HEADER, "%s  %d networks", id.c_str(), nWiFi);

  for (i=0; i<nWiFi; i++) {
    String dSSID = WiFi.SSID(i).substring(0,ENTRY_W - 2 - 4); // status chars and RSSI
    const char *ssid = dSSID.c_str();
    
    displayEntry(i, " %s%*s%4d",  
      WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? " " : "/", 
      -(ENTRY_W - 2 - 4),
      ssid,
      WiFi.RSSI(i));
  }

  for (i=0;  WiFi.status() != WL_CONNECTED && i<nWiFi; i++) {   // find a network we can connect to, and do so
    displayEntry(i, "?");
    if (connectToWiFi(WiFi.SSID(i).c_str(), PASSWORD)) {
      displayEntry(i, ">");
      return 1;
    } else {
      displayEntry(i, "x");
      continue;
    }
  }
  return 0;
}

void initBT(void) {
  BLEDevice::init("");
  localBTAddr = BLEDevice::getAddress().toString().c_str();
  localBTAddr.toUpperCase();
  MyID = localBTAddr.substring(strlen("a4:cf:12:03:c2:"));  // last two chars
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9); // made no difference
}

unsigned long BTScanStart = 0;
std::stringstream results;
std::string sep = "\t";

/**
 * @brief Create a string representation of this device.
 * @return A string representation of this device.
 */
std::string dumpDevice(BLEAdvertisedDevice dev) {
  std::stringstream ss;

  ss << dev.getAddress().toString() << sep << millis() - BTScanStart << sep;
  if (dev.haveName()) {
    ss << dev.getName();
  }
  ss << sep;
  if (dev.haveManufacturerData()) {
    char *pHex = BLEUtils::buildHexData(nullptr, (uint8_t*)dev.getManufacturerData().data(), dev.getManufacturerData().length());
    ss << pHex;
    free(pHex);
  }
  ss << sep;
  if (dev.haveTXPower()) {
    ss << (int)dev.getTXPower();
  }
  ss << sep;
  if (dev.haveRSSI()) {
    ss << (int)dev.getRSSI();
  }
  return ss.str();
}

void displayBTDevice(BLEAdvertisedDevice dev) {
  uint nameLen = ENTRY_W - 4;
  std::string addr = dev.getAddress().toString();
  String saddr = &addr.c_str()[sizeof("00:11:22:")-1];
  displayEntry(BTCount, "%*s%4d",
      -nameLen,
      saddr.c_str(),
      (int)dev.getRSSI());
}

class BTCallback: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice dev) {
    std::string advDump = dumpDevice(dev);
    results << advDump << "\n";
    Serial.println(advDump.c_str());
    // access to the display in this callback is broken.  There is probably a good, logical
    // reason for this.
    displayBTDevice(dev);
    BTCount++;
  }
};

void setup() {
  unsigned long pollStart = millis();;

  Serial.begin(115200);
  delay(10);

  Serial.println(xPortGetCoreID());
  
  processWakeup();     // only resumes if time to
  
  initLED();
  initBT();
  initScreen();

  screenLayout(LargeFont, 1);   // large list, single column, for SSIDs

  if (WiFi.SSID().length() == 0 || !connectToWiFi(WiFi.SSID().c_str(), PASSWORD)) {
    displayEntry(HEADER, "%s  seeking WiFi-s", MyID.c_str());
    if (!scanForWiFi(MyID, PASSWORD)) {
      Serial.println("No WiFi available");
      erreurFatale(1);
    }
  }
  delay(1000);
  
  // Bluetooth time

  screenLayout(MediumFont, 2);   // The bottom three bytes of BT addresses, and strength
  int maxSSIDW = SCREEN_COLS - 2 - 2 - 2 - 3;
  displayEntry(HEADER, "%2s  %*s  %3d", 
    MyID.c_str(),
    -maxSSIDW,
    WiFi.SSID().substring(0,maxSSIDW).c_str(),
    WiFi.RSSI());

    delay(2000);
  digitalWrite(LED_BUILTIN, HIGH);  // we are collecting data
  Serial.println("BT scan start");
  BTScanStart = millis();
  BTCount = 0;
  displayEntry(1, "Scanning...");

  BLEScan* pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new BTCallback());
  pBLEScan->setActiveScan(true);    //active scan uses more power, but get results faster

  BLEScanResults foundDevices = pBLEScan->start(BT_SCAN_TIME);
  BLEDevice::deinit(true);

  for (int i=0; i<BTCount; i++) {
    BLEAdvertisedDevice dev = foundDevices.getDevice(i);
    displayBTDevice(dev);
  }
  pBLEScan->clearResults();
  Serial.println("BT scan finished");
  delay(7000);
  digitalWrite(LED_BUILTIN, LOW);  // finished collecting data

  displayEntry(HEADER, "Connecting to server...");
  
  WiFiClient client;
  for (int i=0; i<5; i++) {
    if (!client.connect(DATA_HOST, DATA_PORT)) {
      Serial.print("connection failed, (");
      Serial.print(errno);
      Serial.print(")  ");
      Serial.println(strerror(errno));
      displayEntry(1, "Try %d failed: %d", i, errno);
      displayEntry(2, "%s", strerror(errno));
      if (i < 5)
        sleep(15000);
    }
  }
  if (!client.connected()) 
    erreurFatale(2);   // should never return
    
  displayEntry(HEADER, "Connected");

  Serial.println("Connected to host");
  
  client.print("Report\t" + localBTAddr + "\t" + foundDevices.getCount() + " devices\t");
  client.println(WiFi.SSID() + "\t" + WiFi.RSSI());
  client.println(results.str().c_str());
  client.flush();
  Serial.println("Results delivered");
  client.stop();
  displayEntry(HEADER, "Succeeded");
  delay(2000);

  u_long elapsed = (millis() - pollStart)/1000;
  startSleepFor(POLL_SLEEP_T - elapsed);
  erreurFatale(3);   // should never return
}

void loop() {
  // put your main code here, to run repeatedly:

}
