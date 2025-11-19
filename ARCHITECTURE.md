# 📚 Architecture ModuleAir - Guide Complet

## 📖 Table des matières

1. [Vue d'ensemble](#vue-densemble)
2. [Structure des fichiers](#structure-des-fichiers)
3. [Flux de fonctionnement](#flux-de-fonctionnement)
4. [Lecture des capteurs](#lecture-des-capteurs)
5. [Envoi des données](#envoi-des-données)
6. [Configuration](#configuration)
7. [Migration MHZ → SensAir S88](#migration-mhz--sensair-s88)

---

## 🎯 Vue d'ensemble

**ModuleAir** est un capteur de qualité de l'air basé sur ESP32 qui :
- Mesure les **particules fines** (PM1, PM2.5, PM10)
- Mesure le **CO2** (SensAir S88 ou MH-Z16)
- Mesure la **température, humidité, pression** (BMX280)
- Mesure les **COV** (CCS811)
- Affiche les données sur **écran OLED** ou **matrice LED**
- Envoie les données via **WiFi** (AirCarto, AtmoSud, sensor.community)
- Envoie les données via **LoRaWAN** (Helium/TTN)

---

## 📁 Structure des fichiers

### **Fichiers principaux**

```
ModuleAir-Next-Gen/
│
├── moduleair.cpp              ⭐ FICHIER PRINCIPAL (cerveau du projet)
├── moduleair-cfg.h            📋 Gestion configuration (lecture/écriture SPIFFS)
├── configuration.h            ⚙️  Configuration logos et écrans
├── ext_def.h                  🔧 Configuration hardware (capteurs activés)
├── defines.h                  📏 Constantes (timing, pins GPIO)
│
├── utils.cpp / utils.h        🛠️  Fonctions utilitaires (envoi HTTP, debug)
├── html-content.h             🌐 Pages web du serveur de configuration
├── intl_fr.h / intl_en.h      🌍 Traductions FR/EN
│
├── bmx280_i2c.cpp/.h          📡 Driver BMX280 (temp/humi/pression)
├── SensAir_S88.cpp/.h         📡 Driver SensAir S88 (CO2) ✅ NOUVEAU
├── ccs811.h                   📡 Driver CCS811 (COV)
│
├── Fonts/                     🔤 Polices pour affichage OLED/Matrix
├── ca-root.h                  🔒 Certificats SSL
└── logos-custom.h             🎨 Logos personnalisés
```

---

## 🔄 Flux de fonctionnement

### **1️⃣ Démarrage (fonction `setup()` - ligne ~6559)**

```cpp
void setup() {
    1. Initialisation Serial (debug)
    2. Lecture configuration depuis SPIFFS
    3. Initialisation WiFi (mode client ou AP)
    4. Initialisation écrans (OLED/Matrix)
    5. Initialisation capteurs :
       - BMX280 (I2C)
       - NPM (Serial1)
       - SensAir S88 (Serial2) ✅
       - CCS811 (I2C)
    6. Démarrage serveur web (port 80)
    7. Initialisation LoRaWAN (si activé)
}
```

### **2️⃣ Boucle principale (fonction `loop()` - ligne ~6777)**

```cpp
void loop() {
    ┌─────────────────────────────────────┐
    │  Toutes les 10 secondes             │
    ├─────────────────────────────────────┤
    │  • Lit SensAir S88 (CO2)            │
    │  • Lit CCS811 (COV)                 │
    │  • Accumule les valeurs dans *_sum  │
    └─────────────────────────────────────┘

    ┌─────────────────────────────────────┐
    │  Toutes les 60 secondes             │
    ├─────────────────────────────────────┤
    │  • Lit NPM (particules)             │
    └─────────────────────────────────────┘

    ┌─────────────────────────────────────┐
    │  Toutes les 5 secondes              │
    ├─────────────────────────────────────┤
    │  • Met à jour l'affichage           │
    │    (OLED ou Matrix)                 │
    └─────────────────────────────────────┘

    ┌─────────────────────────────────────┐
    │  Toutes les 2 minutes (120s)        │
    ├─────────────────────────────────────┤
    │  1. Calcule les moyennes            │
    │  2. Lit BMX280 (T/H/P)              │
    │  3. Crée JSON avec toutes les data  │
    │  4. Envoie vers APIs (WiFi)         │
    │  5. Envoie vers LoRaWAN             │
    │  6. Reset compteurs                 │
    └─────────────────────────────────────┘
}
```

---

## 📡 Lecture des capteurs

### **Architecture générale**

Chaque capteur a :
1. **Une fonction `fetchSensorXXX()`** qui lit les données
2. **Des variables globales** pour accumuler les valeurs
3. **Un compteur** pour faire la moyenne

### **Exemple : SensAir S88 (CO2)**

#### **Déclaration (ligne ~963)**
```cpp
SensAir_S88 s88;                    // Instance du capteur
float last_value_S88 = -1.0;        // Dernière valeur envoyée
uint32_t s88_sum = 0;               // Somme pour moyenne
uint16_t s88_val_count = 0;         // Nombre de mesures
unsigned long starttime_S88;        // Timer
```

#### **Initialisation dans `setup()` (ligne ~6700)**
```cpp
if (cfg::s88_read) {
    serialMHZ.begin(9600, SERIAL_8N1, D16, D17);  // RX=D16, TX=D17
    s88.begin(serialMHZ);
    Debug.println("SensAir S88 initialized");
}
```

#### **Lecture dans `loop()` (ligne ~6826)**
```cpp
if (cfg::s88_read) {
    // Lit toutes les 10 secondes
    if ((msSince(starttime_S88) > 10000 && s88_val_count < 11) || send_now) {
        starttime_S88 = act_milli;
        fetchSensorS88(result_S88);  // ← Appel de la fonction
    }
}
```

#### **Fonction `fetchSensorS88()` (ligne ~4347)**
```cpp
static void fetchSensorS88(String &s) {
    // 1. Lit le capteur
    int value = s88.getCO2();

    // 2. Si lecture OK, accumule
    if (value >= 0) {
        s88_sum += value;           // Ajoute à la somme
        s88_val_count++;            // Compte +1
    }

    // 3. Si c'est l'heure d'envoyer (12 mesures)
    if (send_now && s88_val_count >= 12) {
        // Calcule moyenne
        last_value_S88 = float(s88_sum / s88_val_count);

        // Ajoute au JSON
        add_Value2Json(s, F("MHZ16_CO2"), FPSTR(DBG_TXT_CO2PPM), last_value_S88);

        // Reset
        s88_sum = 0;
        s88_val_count = 0;
    }
}
```

### **Driver SensAir S88 (SensAir_S88.cpp)**

```cpp
int SensAir_S88::getCO2() {
    // 1. Nettoie le buffer série
    while (_serial->available() > 0) {
        _serial->read();
    }

    // 2. Envoie commande Modbus
    // [0xFE, 0x04, 0x00, 0x03, 0x00, 0x01, 0xD5, 0xC5]
    _serial->write(READ_CO2_CMD, CMD_LENGTH);

    // 3. Attend 200ms
    delay(200);

    // 4. Lit la réponse (7 octets)
    if (!readResponse()) return -1;

    // 5. Vérifie validité
    if (!isResponseValid()) return -1;

    // 6. Extrait CO2 (octets 3 et 4)
    int co2 = (_response[3] << 8) | _response[4];

    return co2;  // Retourne CO2 en ppm
}
```

---

## 📤 Envoi des données

### **Format JSON généré**

```json
{
    "software_version": "ModuleAirV2-V1-042022",
    "sensordatavalues": [
        {"value_type": "NPM_P1", "value": "5.2"},
        {"value_type": "NPM_P2", "value": "3.8"},
        {"value_type": "MHZ16_CO2", "value": "582"},  ✅ CLÉ CORRIGÉE
        {"value_type": "BMX280_temperature", "value": "21.5"},
        {"value_type": "BMX280_humidity", "value": "45.2"},
        {"value_type": "BMX280_pressure", "value": "101325"}
    ]
}
```

### **APIs configurables**

| API | Variable config | Fonction |
|-----|----------------|----------|
| **AirCarto** | `cfg::send2custom` | Envoi données temps réel |
| **AtmoSud** | `cfg::send2custom2` | Envoi données + prévisions |
| **sensor.community** | `cfg::send2dusti` | Base de données publique |
| **Madavi** | `cfg::send2madavi` | Archive historique |

### **Fonction d'envoi (ligne ~6048)**

```cpp
static unsigned long sendDataToOptionalApis(const String &data) {
    // 1. AirCarto (host_custom)
    if (cfg::send2custom) {
        String data_4_custom = "{\"moduleairid\": \"" + esp_chipid + "\", ";
        data_4_custom += data_to_send;
        sendData(LoggerCustom, data_4_custom, 0,
                 cfg::host_custom, cfg::url_custom, cfg::ssl_custom);
    }

    // 2. AtmoSud (host_custom2)
    if (cfg::send2custom2) {
        String data_4_custom = "{\"moduleairid\": \"" + esp_chipid + "\", ";
        data_4_custom += data_to_send;
        sendData(LoggerCustom2, data_4_custom, 0,
                 cfg::host_custom2, cfg::url_custom2, cfg::ssl_custom2);
    }

    // 3. CSV série
    if (cfg::send2csv) {
        send_csv(data);
    }
}
```

---

## ⚙️ Configuration

### **Fichier `ext_def.h` - Active/désactive les capteurs**

```cpp
// WiFi et LoRa
#define HAS_WIFI true
#define HAS_LORA true

// Capteurs actifs
#define NPM_READ true           // Particules (NextPM)
#define BMX280_READ true        // Temp/Humi/Pression
#define MHZ16_READ false        // ❌ Ancien CO2
#define S88_READ true           // ✅ Nouveau CO2
#define CCS811_READ false       // COV

// Écrans
#define HAS_SSD1306 true        // OLED 128x64
#define HAS_MATRIX true         // Matrice LED 64x32

// APIs
#define SEND2SENSORCOMMUNITY true
#define SEND2MADAVI false
#define SEND2CUSTOM true        // AirCarto
#define SEND2CUSTOM2 false      // AtmoSud
#define SEND2CSV false

// URLs APIs
#define HOST_CUSTOM "api.aircarto.fr"
#define URL_CUSTOM "/sensors/moduleair"
```

### **Fichier `defines.h` - Timing des mesures**

```cpp
// Intervalles de lecture
#define SAMPLETIME_NPM_MS 60000        // NPM : 1 minute
#define SAMPLETIME_S88_MS 10000        // S88 : 10 secondes ✅
#define SAMPLETIME_CCS811_MS 10000     // CCS811 : 10 secondes
#define SENDING_INTERVALL_MS 120000    // Envoi : 2 minutes

// GPIO Pins
#define D16 16  // RX SensAir S88
#define D17 17  // TX SensAir S88
```

### **Fichier `configuration.h` - Écrans à afficher**

```cpp
// Logos (1 à 6, ou 0 pour désactiver)
const int cfg_logo_moduleair = 1;
const int cfg_logo_aircarto = 2;

// Écrans mesures
bool cfg_screen_pm25 = true;
bool cfg_screen_co2 = true;
bool cfg_screen_temp = true;
bool cfg_screen_humi = true;

// Écrans prévisions AtmoSud
bool cfg_screen_atmo_pm10 = true;
bool cfg_screen_atmo_index = true;
```

### **Configuration via interface web**

1. Connectez-vous au WiFi `moduleair-XXXXXX`
2. Allez sur `http://192.168.4.1`
3. Onglets disponibles :
   - **WiFi Settings** : SSID, mot de passe
   - **LoRa Settings** : AppEUI, DevEUI, AppKey
   - **Sensors** : Activer/désactiver capteurs
   - **APIs** : Configurer URLs, identifiants
   - **More Settings** : Coordonnées GPS, offset température

---

## 🔄 Migration MHZ → SensAir S88

### **Fichiers créés**

| Fichier | Rôle |
|---------|------|
| `SensAir_S88.h` | Header du driver (classe, prototypes) |
| `SensAir_S88.cpp` | Implémentation du driver (Modbus RTU) |

### **Fichiers modifiés**

| Fichier | Ligne | Changement |
|---------|-------|------------|
| **moduleair.cpp** | 54 | ✅ Ajout `#include "SensAir_S88.h"` |
| **moduleair.cpp** | 144 | ✅ Ajout `bool s88_read = S88_READ;` |
| **moduleair.cpp** | 963 | ✅ Déclaration `SensAir_S88 s88;` |
| **moduleair.cpp** | 1076-1078 | ✅ Variables `last_value_S88`, `s88_sum`, etc. |
| **moduleair.cpp** | 4347-4390 | ✅ Fonction `fetchSensorS88()` |
| **moduleair.cpp** | 4376 | ✅ **CORRECTION** : `"MHZ16_CO2"` au lieu de `"S88_CO2"` |
| **moduleair.cpp** | 6826-6833 | ✅ Appel dans `loop()` |
| **ext_def.h** | - | ✅ `#define S88_READ true` |
| **ext_def.h** | - | ✅ `#define MHZ16_READ false` |
| **defines.h** | 53 | ✅ `#define SAMPLETIME_S88_MS 10000` |

### **Pourquoi changer la clé JSON ?**

**Problème** :
```json
{"value_type": "S88_CO2", "value": "582"}  ❌ API ne reconnaît pas
```

**Solution** :
```json
{"value_type": "MHZ16_CO2", "value": "582"}  ✅ Compatible AirCarto
```

L'API AirCarto s'attend à recevoir `"MHZ16_CO2"` car c'était l'ancien format. En gardant la même clé, pas besoin de modifier l'API.

### **Différences MHZ vs SensAir**

| Caractéristique | MH-Z16/Z19C | SensAir S88 |
|----------------|-------------|-------------|
| **Protocole** | Commandes propriétaires | Modbus RTU |
| **Librairie** | MHZ16_uart | SensAir_S88 (custom) |
| **Baudrate** | 9600 | 9600 |
| **Pins** | Serial2 (D16/D17) | Serial2 (D16/D17) |
| **Lecture** | `mhz16.getPPM()` | `s88.getCO2()` |
| **Timing** | 10s | 10s |

---

## 🔍 Variables importantes

### **Variables globales des capteurs**

```cpp
// NPM (Particules)
float last_value_NPM_P0 = -1.0;   // PM1
float last_value_NPM_P1 = -1.0;   // PM10
float last_value_NPM_P2 = -1.0;   // PM2.5

// SensAir S88 (CO2)
float last_value_S88 = -1.0;      // CO2 en ppm
uint32_t s88_sum = 0;             // Somme pour moyenne
uint16_t s88_val_count = 0;       // Nombre de mesures

// BMX280 (Temp/Humi/Pression)
float last_value_BMX280_T = -128.0;  // Température
float last_value_BMX280_P = -1.0;    // Pression
float last_value_BME280_H = -1.0;    // Humidité

// CCS811 (COV)
float last_value_CCS811 = -1.0;   // eCO2 en ppm
```

### **Variables de timing**

```cpp
unsigned long starttime;              // Début du cycle
unsigned long act_milli;              // Temps actuel (ms)
unsigned long starttime_NPM;          // Timer NPM
unsigned long starttime_S88;          // Timer SensAir S88
bool send_now = false;                // Flag envoi données
```

### **Variables de configuration (namespace cfg)**

```cpp
namespace cfg {
    // WiFi
    char wlanssid[35];
    char wlanpwd[65];

    // APIs
    char host_custom[100];         // AirCarto
    char url_custom[100];
    char host_custom2[100];        // AtmoSud

    // Capteurs
    bool npm_read;
    bool bmx280_read;
    bool s88_read;  ✅

    // Écrans
    bool has_ssd1306;
    bool has_matrix;
}
```

---

## 🐛 Debug

### **Niveaux de debug (defines.h)**

```cpp
#define DEBUG_ERROR 1      // Erreurs seulement
#define DEBUG_WARNING 2    // Warnings
#define DEBUG_MIN_INFO 3   // Info minimale
#define DEBUG_MED_INFO 4   // Info moyenne
#define DEBUG_MAX_INFO 5   // Tout afficher
```

### **Activer le debug**

Dans `ext_def.h` :
```cpp
#define DEBUG 5  // Niveau max
```

### **Sortie série**

```
Free Stack at start of setup() is: 8192
WiFi connected, IP: 192.168.1.100
BMX280 initialized (ID: 0x60)
SensAir S88 initialized
State NPM... NPM start...
[S88] CO2: 582 ppm
Sending to aircarto api...
HTTP POST: 200 OK
```

---

## 📊 Résumé du fonctionnement

```
┌───────────────────────────────────────────────────────────────┐
│                     CYCLE DE 2 MINUTES                         │
├───────────────────────────────────────────────────────────────┤
│                                                                │
│  0s    : Début cycle, reset compteurs                         │
│  10s   : Lit S88 #1 → s88_sum += 582                          │
│  20s   : Lit S88 #2 → s88_sum += 584                          │
│  30s   : Lit S88 #3 → s88_sum += 580                          │
│  ...                                                           │
│  60s   : Lit NPM → npm_pm25_sum += 5.2                        │
│  ...                                                           │
│  110s  : Lit S88 #12 → s88_sum += 579                         │
│                                                                │
│  120s  : ENVOI !                                               │
│         ├─ Moyenne CO2 = 582 ppm                              │
│         ├─ Moyenne PM2.5 = 4.8 µg/m³                          │
│         ├─ Lit BMX280 → T=21.5°C, H=45%, P=1013hPa           │
│         ├─ Crée JSON avec toutes les données                  │
│         ├─ POST vers AirCarto                                 │
│         ├─ POST vers AtmoSud                                  │
│         ├─ Envoi LoRaWAN                                       │
│         └─ Reset compteurs, retour à 0s                       │
│                                                                │
└───────────────────────────────────────────────────────────────┘
```

---

## 🚀 Compilation et upload

### **PlatformIO**

```bash
# Compiler pour ESP32 (français)
pio run -e esp32dev_fr

# Upload via USB
pio run -e esp32dev_fr --target upload

# Monitor série
pio device monitor
```

### **Environnements disponibles (platformio.ini)**

```ini
[env:esp32dev_fr]    # Version française
[env:esp32dev_en]    # Version anglaise
```

---

## 📞 Points de contact dans le code

### **Ajouter un nouveau capteur**

1. Créer `MonCapteur.h` et `MonCapteur.cpp`
2. Ajouter `#include "MonCapteur.h"` dans `moduleair.cpp`
3. Déclarer instance : `MonCapteur monCapteur;`
4. Créer `fetchSensorMonCapteur()` (voir ligne 4347 pour exemple)
5. Appeler dans `loop()` (voir ligne 6826)
6. Ajouter `#define MONCAPTEUR_READ true` dans `ext_def.h`
7. Ajouter dans JSON (ligne 6916)

### **Modifier l'intervalle d'envoi**

`ext_def.h` :
```cpp
#define SENDING_INTERVALL_MS 120000  // 2 minutes
// Changer en 300000 pour 5 minutes
```

### **Ajouter une nouvelle API**

1. Ajouter config dans `ext_def.h` :
   ```cpp
   #define SEND2CUSTOM3 true
   #define HOST_CUSTOM3 "mon-api.fr"
   ```
2. Ajouter dans `sendDataToOptionalApis()` (ligne 6048)

---

## 📚 Ressources

- **ESP32 Pinout** : https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
- **LoRaWAN TTN** : https://www.thethingsnetwork.org/
- **sensor.community** : https://sensor.community/
- **PlatformIO** : https://platformio.org/

---

**Créé le** : 2025
**Version** : ModuleAir V2.1
**Auteur** : Documentation générée par Claude Code
