#include "wifiSettingHandler.h"

#include "../../../data/wifi_setting.h"
#include "DNSServer.h"
#include "Preferences.h"
#include "WebServer.h"
#include "WiFi.h"
#include "esp_system.h"
#include "screen_manager.h"
#include "src/libs/qrcode/lv_qrcode.h"
#include "ws2811_control_2.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {

constexpr char WIFI_PREFS_NAMESPACE[] = "wifi";
constexpr char WIFI_SSID_KEY[] = "ssid";
constexpr char WIFI_PASSWORD_KEY[] = "password";
constexpr char WIFI_AP_SSID_KEY[] = "ap_ssid";
constexpr char WIFI_AP_PASSWORD_KEY[] = "ap_password";
constexpr char VEHICLE_WEIGHT_KEY[] = "weight";
constexpr char VEHICLE_TOP_SPEED_KEY[] = "top_speed";
constexpr char VEHICLE_MAX_RPM_KEY[] = "max_rpm";
constexpr char LED_STYLE_KEY[] = "led_style";
constexpr char PORTAL_URL[] = "http://192.168.4.1/";
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 20000;
constexpr uint32_t AP_RENAME_DELAY_MS = 1200;

WebServer s_server(80);
DNSServer s_dnsServer;
lv_obj_t *s_screen = nullptr;
lv_obj_t *s_statusLabel = nullptr;
lv_obj_t *s_apQr = nullptr;
lv_obj_t *s_apInfoLabel = nullptr;
String s_apSsid;
String s_apPassword;
String s_pendingSsid;
String s_pendingPassword;
String s_pendingApSsid;
String s_pendingApPassword;
bool s_serverStarted = false;
bool s_dnsStarted = false;
bool s_routesRegistered = false;
bool s_connecting = false;
bool s_apRenamePending = false;
uint32_t s_connectStartedAt = 0;
uint32_t s_apRenameAt = 0;
float s_vehicleWeightKg = 0.0f;
uint16_t s_vehicleTopSpeed = 0;
uint16_t s_vehicleMaxRpm = 12000;
WS2811Style s_ledStyle = WS2811Style::RPM;

class CaptiveFallbackHandler : public RequestHandler {
public:
    bool canHandle(WebServer &, HTTPMethod, const String &) override
    {
        return true;
    }

    bool handle(WebServer &server, HTTPMethod, const String &) override
    {
        server.sendHeader("Location", PORTAL_URL);
        server.send(302, "text/plain", "Redirecting to Wi-Fi setup");
        return true;
    }
};

CaptiveFallbackHandler s_fallbackHandler;

void SetStatus(const char *text)
{
    if (s_statusLabel != nullptr) {
        lv_label_set_text(s_statusLabel, text);
    }
}

String BuildWifiQrData(const String &ssid, const String &password)
{
    String data;
    data.reserve(96);
    data = "WIFI:T:WPA;S:";
    const String *fields[] = {&ssid, &password};
    for (size_t field = 0; field < 2; ++field) {
        if (field == 1) {
            data += ";P:";
        }
        for (size_t i = 0; i < fields[field]->length(); ++i) {
            const char ch = (*fields[field])[i];
            if (ch == '\\' || ch == ';' || ch == ',' || ch == ':') {
                data += '\\';
            }
            data += ch;
        }
    }
    data += ";;";
    return data;
}

bool SaveApConfig(const String &ssid, const String &password)
{
    Preferences prefs;
    if (!prefs.begin(WIFI_PREFS_NAMESPACE, false)) {
        Serial.println("[WiFiSetup] Could not open Wi-Fi settings storage");
        return false;
    }

    const size_t bytesWritten = prefs.putString(WIFI_AP_SSID_KEY, ssid);
    const size_t passwordBytesWritten = prefs.putString(WIFI_AP_PASSWORD_KEY, password);
    prefs.end();
    if (bytesWritten != ssid.length() || passwordBytesWritten != password.length()) {
        Serial.println("[WiFiSetup] Could not save setup AP credentials");
        return false;
    }
    return true;
}

bool SaveCredentials(const String &ssid, const String &password)
{
    Preferences prefs;
    if (!prefs.begin(WIFI_PREFS_NAMESPACE, false)) {
        Serial.println("[WiFiSetup] Could not open Wi-Fi settings storage");
        return false;
    }

    const size_t ssidBytes = prefs.putString(WIFI_SSID_KEY, ssid);
    const size_t passwordBytes = prefs.putString(WIFI_PASSWORD_KEY, password);
    prefs.end();
    if (ssidBytes != ssid.length() || passwordBytes != password.length()) {
        Serial.println("[WiFiSetup] Could not save Wi-Fi settings");
        return false;
    }
    return true;
}

void HandleRoot()
{
    s_server.send_P(200, PSTR("text/html; charset=utf-8"), wifimanager_html);
}

void HandleApNameSave()
{
    const String requestedSsid = s_server.hasArg("apSsid") ? s_server.arg("apSsid") : "";
    const String requestedPassword =
        s_server.hasArg("apPassword") ? s_server.arg("apPassword") : "";
    const String newSsid = requestedSsid.isEmpty() ? s_apSsid : requestedSsid;
    const String newPassword = requestedPassword.isEmpty() ? s_apPassword : requestedPassword;

    if (newSsid.isEmpty() || newSsid.length() > 32) {
        s_server.send(400, "text/plain; charset=utf-8",
                      "Setup AP name must be between 1 and 32 bytes.");
        return;
    }
    if (newPassword.length() < 8 || newPassword.length() > 63) {
        s_server.send(400, "text/plain; charset=utf-8",
                      "Setup AP password must be between 8 and 63 characters.");
        return;
    }
    if (newSsid == s_apSsid && newPassword == s_apPassword) {
        s_server.send(200, "text/plain; charset=utf-8",
                      "Setup AP credentials unchanged.");
        return;
    }
    if (s_apRenamePending) {
        s_server.send(409, "text/plain; charset=utf-8",
                      "A setup AP name change is already in progress.");
        return;
    }
    if (!SaveApConfig(newSsid, newPassword)) {
        s_server.send(500, "text/plain; charset=utf-8", "Could not save setup AP credentials.");
        return;
    }

    s_pendingApSsid = newSsid;
    s_pendingApPassword = newPassword;
    s_apRenameAt = millis() + AP_RENAME_DELAY_MS;
    s_apRenamePending = true;
    s_server.send(200, "text/plain; charset=utf-8",
                  "Setup AP name and password changed. Reconnect using the new credentials.");
}

void BeginStationConnection(const String &ssid, const String &password)
{
    const bool keepAccessPoint = (WiFi.getMode() & WIFI_AP) != 0;
    WiFi.setAutoReconnect(false);
    WiFi.mode(keepAccessPoint ? WIFI_AP : WIFI_OFF);
    WiFi.mode(keepAccessPoint ? WIFI_AP_STA : WIFI_STA);

    const wl_status_t status = WiFi.begin(ssid.c_str(), password.c_str());
    if (status == WL_CONNECT_FAILED) {
        Serial.println("[WiFiSetup] WiFi.begin failed to start the connection");
        s_connecting = false;
        s_pendingSsid = "";
        s_pendingPassword = "";
        SetStatus("Could not start Wi-Fi connection. Check the network and try again.");
        return;
    }

    s_connecting = true;
    s_connectStartedAt = millis();
}

void HandleSave()
{
    if (!s_server.hasArg("weight") || !s_server.hasArg("topSpeed") ||
        !s_server.hasArg("maxRpm")) {
        s_server.send(400, "text/plain; charset=utf-8",
                      "Weight, top speed, and max RPM are required.");
        return;
    }

    const String weightText = s_server.arg("weight");
    const String topSpeedText = s_server.arg("topSpeed");
    const String maxRpmText = s_server.arg("maxRpm");
    char *end = nullptr;
    const float weightKg = strtof(weightText.c_str(), &end);
    if (end == weightText.c_str() || *end != '\0' || !isfinite(weightKg) ||
        weightKg < 0.0f || weightKg > 5000.0f) {
        s_server.send(400, "text/plain; charset=utf-8",
                      "Weight must be a number from 0 to 5000 kg.");
        return;
    }

    end = nullptr;
    const unsigned long topSpeed = strtoul(topSpeedText.c_str(), &end, 10);
    if (end == topSpeedText.c_str() || *end != '\0' || topSpeed > 500) {
        s_server.send(400, "text/plain; charset=utf-8",
                      "Top speed must be an integer from 0 to 500 km/h.");
        return;
    }

    end = nullptr;
    const unsigned long maxRpm = strtoul(maxRpmText.c_str(), &end, 10);
    if (end == maxRpmText.c_str() || *end != '\0' || maxRpm < 1000 || maxRpm > 30000) {
        s_server.send(400, "text/plain; charset=utf-8",
                      "Max RPM must be an integer from 1000 to 30000.");
        return;
    }

    Preferences prefs;
    if (!prefs.begin(WIFI_PREFS_NAMESPACE, false)) {
        Serial.println("[WiFiSetup] Could not open settings storage");
        s_server.send(500, "text/plain; charset=utf-8", "Could not open settings storage.");
        return;
    }
    const size_t weightBytes = prefs.putFloat(VEHICLE_WEIGHT_KEY, weightKg);
    const size_t topSpeedBytes = prefs.putUInt(VEHICLE_TOP_SPEED_KEY, topSpeed);
    const size_t maxRpmBytes = prefs.putUInt(VEHICLE_MAX_RPM_KEY, maxRpm);
    prefs.end();
    if (weightBytes != sizeof(float) || topSpeedBytes != sizeof(uint32_t) ||
        maxRpmBytes != sizeof(uint32_t)) {
        Serial.println("[WiFiSetup] Could not save vehicle data settings");
        s_server.send(500, "text/plain; charset=utf-8", "Could not save settings.");
        return;
    }

    s_vehicleWeightKg = weightKg;
    s_vehicleTopSpeed = static_cast<uint16_t>(topSpeed);
    s_vehicleMaxRpm = static_cast<uint16_t>(maxRpm);
    s_server.send(200, "text/plain; charset=utf-8", "Vehicle data settings saved.");
}

bool ParseLedStyle(const String &value, WS2811Style &style)
{
    if (value == "FLASH") style = WS2811Style::FLASH;
    else if (value == "STROBE") style = WS2811Style::STROBE;
    else if (value == "FADE") style = WS2811Style::FADE;
    else if (value == "SMOOTH") style = WS2811Style::SMOOTH;
    else if (value == "CUSTOM") style = WS2811Style::CUSTOM;
    else if (value == "RPM") style = WS2811Style::RPM;
    else if (value == "OFF") style = WS2811Style::OFF;
    else return false;
    return true;
}

void HandleLedStyle()
{
    WS2811Style style;
    if (!s_server.hasArg("style") || !ParseLedStyle(s_server.arg("style"), style)) {
        s_server.send(400, "text/plain; charset=utf-8", "Unknown LED style.");
        return;
    }

    Preferences prefs;
    if (!prefs.begin(WIFI_PREFS_NAMESPACE, false)) {
        Serial.println("[WiFiSetup] Could not open settings storage");
        s_server.send(500, "text/plain; charset=utf-8", "Could not open settings storage.");
        return;
    }
    const size_t bytesWritten = prefs.putUChar(LED_STYLE_KEY, static_cast<uint8_t>(style));
    prefs.end();
    if (bytesWritten != sizeof(uint8_t)) {
        Serial.println("[WiFiSetup] Could not save LED style");
        s_server.send(500, "text/plain; charset=utf-8", "Could not save LED style.");
        return;
    }

    s_ledStyle = style;
    WS2811_SetStyle(style);
    s_server.send(200, "text/plain; charset=utf-8", "LED style updated.");
}

void StartPortal()
{
    WiFi.mode(WIFI_AP_STA);

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    s_apSsid = "displayMotocycle-" + mac.substring(mac.length() - 6);

    char password[9];
    snprintf(password, sizeof(password), "%08lX", static_cast<unsigned long>(esp_random()));
    s_apPassword = password;

    const IPAddress address(192, 168, 4, 1);
    const IPAddress mask(255, 255, 255, 0);
    Preferences prefs;
    if (prefs.begin(WIFI_PREFS_NAMESPACE, false)) {
        const String savedApSsid = prefs.getString(WIFI_AP_SSID_KEY, "");
        const String savedApPassword = prefs.getString(WIFI_AP_PASSWORD_KEY, "");
        prefs.end();
        if (!savedApSsid.isEmpty()) {
            s_apSsid = savedApSsid;
        }
        if (savedApPassword.length() >= 8 && savedApPassword.length() <= 63) {
            s_apPassword = savedApPassword;
        }
    } else {
        Serial.println("[WiFiSetup] Could not read setup AP name; using generated name");
    }

    if (!WiFi.softAPConfig(address, address, mask) ||
        !WiFi.softAP(s_apSsid.c_str(), s_apPassword.c_str())) {
        SetStatus("Could not start Wi-Fi setup access point.");
        Serial.println("[WiFiSetup] Failed to start setup access point");
        return;
    }

    if (!s_routesRegistered) {
        s_server.on("/", HTTP_GET, HandleRoot);
        s_server.on("/ap-name", HTTP_POST, HandleApNameSave);
        s_server.on("/save", HTTP_POST, HandleSave);
        s_server.on("/led-style", HTTP_POST, HandleLedStyle);
        s_server.addHandler(&s_fallbackHandler);
        s_routesRegistered = true;
    }
    s_server.begin();
    s_serverStarted = true;
    s_dnsStarted = s_dnsServer.start(53, "*", address);
    if (!s_dnsStarted) {
        Serial.println("[WiFiSetup] Could not start captive portal DNS server");
        SetStatus("Scan QR to connect, then open http://192.168.4.1/");
    } else {
        SetStatus("Scan QR to connect. Setup page should open automatically.");
    }
    Serial.printf("[WiFiSetup] Portal started at %s\n", PORTAL_URL);
}

void StopPortal()
{
    const bool keepStation = WiFi.status() == WL_CONNECTED || s_connecting;
    if (s_serverStarted) {
        s_server.stop();
        s_serverStarted = false;
    }
    if (s_dnsStarted) {
        s_dnsServer.stop();
        s_dnsStarted = false;
    }
    s_apRenamePending = false;
    s_pendingApSsid = "";
    s_pendingApPassword = "";
    WiFi.softAPdisconnect(false);
    WiFi.mode(keepStation ? WIFI_STA : WIFI_OFF);
}

} // namespace

void WifiSettingHandler_ConnectSavedNetwork(void)
{
    Preferences prefs;
    if (!prefs.begin(WIFI_PREFS_NAMESPACE, false)) {
        Serial.println("[WiFiSetup] Could not open Wi-Fi settings storage");
        return;
    }

    const String ssid = prefs.getString(WIFI_SSID_KEY, "");
    const String password = prefs.getString(WIFI_PASSWORD_KEY, "");
    s_vehicleWeightKg = prefs.getFloat(VEHICLE_WEIGHT_KEY, 0.0f);
    s_vehicleTopSpeed = static_cast<uint16_t>(prefs.getUInt(VEHICLE_TOP_SPEED_KEY, 0));
    const uint32_t savedMaxRpm = prefs.getUInt(VEHICLE_MAX_RPM_KEY, 12000);
    if (savedMaxRpm >= 1000 && savedMaxRpm <= 30000) {
        s_vehicleMaxRpm = static_cast<uint16_t>(savedMaxRpm);
    }
    const uint8_t savedStyle =
        prefs.getUChar(LED_STYLE_KEY, static_cast<uint8_t>(WS2811Style::RPM));
    prefs.end();

    if (savedStyle <= static_cast<uint8_t>(WS2811Style::RPM)) {
        s_ledStyle = static_cast<WS2811Style>(savedStyle);
        WS2811_SetStyle(s_ledStyle);
    }

    if (!ssid.isEmpty()) {
        BeginStationConnection(ssid, password);
        if (s_connecting) {
            Serial.println("[WiFiSetup] Connecting once to the saved Wi-Fi network");
        }
    }
}

void WifiSettingHandler_Init(void)
{
    if (s_screen != nullptr) {
        return;
    }

    s_screen = lv_obj_create(NULL);
    lv_obj_remove_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(s_screen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_screen, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(s_screen);
    lv_label_set_text(title, "WIFI SETTING");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    lv_obj_t *apTitle = lv_label_create(s_screen);
    lv_label_set_text(apTitle, "Scan QR to connect to setup Wi-Fi");
    lv_obj_set_style_text_color(apTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(apTitle, LV_ALIGN_TOP_MID, 0, 55);

    s_apQr = lv_qrcode_create(s_screen);
    lv_qrcode_set_size(s_apQr, 140);
    lv_qrcode_set_dark_color(s_apQr, lv_color_black());
    lv_qrcode_set_light_color(s_apQr, lv_color_white());
    lv_obj_align(s_apQr, LV_ALIGN_TOP_MID, 0, 85);

    s_statusLabel = lv_label_create(s_screen);
    lv_obj_set_style_text_color(s_statusLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(s_statusLabel, 40, 365);

    StartPortal();

    s_apInfoLabel = lv_label_create(s_screen);
    lv_obj_set_style_text_color(s_apInfoLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    if (s_serverStarted) {
        lv_label_set_text_fmt(s_apInfoLabel, "Network: %s\nPassword: %s\nAddress: 192.168.4.1",
                              s_apSsid.c_str(), s_apPassword.c_str());
    } else {
        lv_label_set_text(s_apInfoLabel, "Setup access point unavailable.");
        lv_obj_add_flag(s_apQr, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_align(s_apInfoLabel, LV_ALIGN_TOP_MID, 0, 245);

    lv_obj_t *backLabel = lv_label_create(s_screen);
    lv_label_set_text(backLabel, "SW4: Back to Settings");
    lv_obj_set_style_text_color(backLabel, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
    lv_obj_align(backLabel, LV_ALIGN_BOTTOM_MID, 0, -12);

    if (s_serverStarted) {
        const String wifiQrData = BuildWifiQrData(s_apSsid, s_apPassword);
        if (lv_qrcode_update(s_apQr, wifiQrData.c_str(), wifiQrData.length()) != LV_RESULT_OK) {
            Serial.println("[WiFiSetup] Could not generate setup Wi-Fi QR code");
            SetStatus("Could not generate Wi-Fi QR code.");
        }
    }
}

void WifiSettingHandler_Update(void)
{
    if (s_serverStarted) {
        s_server.handleClient();
    }

    if (s_apRenamePending && static_cast<int32_t>(millis() - s_apRenameAt) >= 0) {
        if (WiFi.softAP(s_pendingApSsid.c_str(), s_pendingApPassword.c_str())) {
            s_apSsid = s_pendingApSsid;
            s_apPassword = s_pendingApPassword;
            s_pendingApSsid = "";
            s_pendingApPassword = "";
            s_apRenamePending = false;

            if (s_apInfoLabel != nullptr) {
                lv_label_set_text_fmt(s_apInfoLabel, "Network: %s\nPassword: %s\nAddress: 192.168.4.1",
                                      s_apSsid.c_str(), s_apPassword.c_str());
            }
            if (s_apQr != nullptr) {
                const String wifiQrData = BuildWifiQrData(s_apSsid, s_apPassword);
                if (lv_qrcode_update(s_apQr, wifiQrData.c_str(), wifiQrData.length()) != LV_RESULT_OK) {
                    Serial.println("[WiFiSetup] Could not update Wi-Fi QR code after AP rename");
                    SetStatus("AP name changed, but could not update the Wi-Fi QR code.");
                    return;
                }
            }
            SetStatus("Setup AP name and password changed. Reconnect using the new credentials.");
            Serial.printf("[WiFiSetup] Setup AP credentials changed; SSID=%s\n", s_apSsid.c_str());
        } else {
            if (!SaveApConfig(s_apSsid, s_apPassword)) {
                Serial.println("[WiFiSetup] Could not restore previous setup AP credentials in storage");
            }
            s_pendingApSsid = "";
            s_pendingApPassword = "";
            s_apRenamePending = false;
            Serial.println("[WiFiSetup] Failed to apply new setup AP credentials");
            SetStatus("Could not change setup AP credentials. Try again.");
        }
    }

    if (!s_connecting) {
        return;
    }
    if (WiFi.status() == WL_CONNECTED) {
        s_connecting = false;
        if (!s_pendingSsid.isEmpty()) {
            if (SaveCredentials(s_pendingSsid, s_pendingPassword)) {
                SetStatus("Connected. Wi-Fi settings saved.");
                Serial.println("[WiFiSetup] Connected and saved Wi-Fi settings");
            } else {
                SetStatus("Connected, but could not save Wi-Fi settings.");
            }
            s_pendingSsid = "";
            s_pendingPassword = "";
        } else {
            Serial.println("[WiFiSetup] Connected to the saved Wi-Fi network");
        }
    } else if (millis() - s_connectStartedAt >= WIFI_CONNECT_TIMEOUT_MS) {
        s_connecting = false;
        const bool apNotFound = WiFi.status() == WL_NO_SSID_AVAIL;
        const bool keepAccessPoint = (WiFi.getMode() & WIFI_AP) != 0;
        WiFi.mode(keepAccessPoint ? WIFI_AP : WIFI_OFF);
        if (apNotFound) {
            SetStatus("Wi-Fi not found. Check the SSID and use a 2.4 GHz network.");
            Serial.println("[WiFiSetup] Wi-Fi network not found; check SSID and 2.4 GHz support");
        } else {
            SetStatus("Wi-Fi connection timed out. Check the SSID, password, and signal.");
            Serial.println("[WiFiSetup] Wi-Fi connection timed out");
        }
        s_pendingSsid = "";
        s_pendingPassword = "";
    }
}

void WifiSettingHandler_Exit(void)
{
    ScreenManager_SwitchTo(SCREEN_SETTING);
    StopPortal();
    if (s_screen != nullptr) {
        lv_obj_del(s_screen);
        s_screen = nullptr;
    }
    s_statusLabel = nullptr;
    s_apQr = nullptr;
    s_apInfoLabel = nullptr;
    s_apSsid = "";
    s_apPassword = "";
}

lv_obj_t* WifiSettingHandler_GetScreen(void)
{
    return s_screen;
}

uint16_t WifiSettingHandler_GetMaxRpm(void)
{
    return s_vehicleMaxRpm;
}
