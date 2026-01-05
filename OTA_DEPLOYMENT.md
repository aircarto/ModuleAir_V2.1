# Guide de Déploiement OTA - ModuleAir

## 1. Compiler le Firmware

```bash
cd /Users/noagiannone/Documents/ModuleAir-Next-Gen

# Build pour la version française
~/.platformio/penv/bin/pio run -e esp32dev_fr

# Build pour la version anglaise
~/.platformio/penv/bin/pio run -e esp32dev_en
```

Le fichier compilé se trouve dans :
```
.pio/build/esp32dev_fr/firmware.bin
```

---

## 2. Mettre à jour la Version

**Avant de compiler**, modifier dans `moduleair.cpp` (lignes 4-5) :

```cpp
#define SOFTWARE_VERSION_STR "ModuleAirV2-V1-012026"
#define SOFTWARE_VERSION_STR_SHORT "V1-012026"
```

Format recommandé : `V{major}-{MMYYYY}` (ex: V1-012026 = version 1, janvier 2026)

---

## 3. Calculer le MD5

```bash
md5 .pio/build/esp32dev_fr/firmware.bin
```

Résultat exemple :
```
MD5 (.pio/build/esp32dev_fr/firmware.bin) = abc123def456...
```

---

## 4. Structure du version.json

Créer/mettre à jour le fichier sur le serveur :

**URL** : `https://aircarto.fr/ota/version.json`

```json
{
    "version": "V1-012026",
    "url": "firmware.bin",
    "md5": "abc123def456789..."
}
```

| Champ | Description |
|-------|-------------|
| `version` | Doit correspondre à `SOFTWARE_VERSION_STR_SHORT` du code |
| `url` | Chemin relatif ou URL absolue du firmware |
| `md5` | Hash MD5 du fichier `.bin` (optionnel mais recommandé) |

---

## 5. Uploader sur le Serveur

1. Uploader `firmware.bin` vers `https://aircarto.fr/ota/firmware.bin`
2. Uploader/mettre à jour `version.json` vers `https://aircarto.fr/ota/version.json`

---

## 6. Vérification

Tester que les URLs sont accessibles :
```bash
curl -I https://aircarto.fr/ota/version.json
curl -I https://aircarto.fr/ota/firmware.bin
```

---

## Commande Complète (Copier-Coller)

```bash
# 1. Compiler
~/.platformio/penv/bin/pio run -e esp32dev_fr

# 2. Obtenir le MD5
md5 .pio/build/esp32dev_fr/firmware.bin

# 3. Afficher le chemin du fichier à uploader
echo "Fichier à uploader: $(pwd)/.pio/build/esp32dev_fr/firmware.bin"
```
