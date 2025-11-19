# Implémentation de la Sélection d'Écrans de Polluants

## Vue d'ensemble

Ce document décrit l'implémentation d'un système de sélection d'écrans de polluants pour le ModuleAir, permettant aux utilisateurs de choisir via l'interface web quels écrans de données doivent être affichés sur l'OLED et la matrice RGB.

## Modifications apportées

### 1. Variables de configuration (moduleair.cpp)

Les variables de sélection d'écrans ont été ajoutées au namespace `cfg::` (lignes 196-211) :

```cpp
// Screen selection for pollutants display
bool screen_pm01 = true;
bool screen_pm25 = true;
bool screen_pm10 = true;
bool screen_co2 = true;
bool screen_cov = true;
bool screen_temp = true;
bool screen_humi = true;
bool screen_press = false;

bool screen_atmo_pm10 = true;
bool screen_atmo_pm25 = true;
bool screen_atmo_index = true;
bool screen_atmo_o3 = true;
bool screen_atmo_no2 = true;
bool screen_atmo_so2 = true;
```

**Avantage** : Ces variables font partie du système de configuration persistante et sont automatiquement sauvegardées dans SPIFFS.

### 2. Configuration système (moduleair-cfg.h)

Ajout des nouvelles entrées de configuration :
- Enum `ConfigShapeId` : Ajout de Config_screen_* pour chaque écran
- Clés PROGMEM : CFG_KEY_SCREEN_* pour chaque variable
- Tableau `configShape` : Liaison des variables avec leurs clés de configuration

**Résultat** : Les paramètres d'écrans sont maintenant gérés par le système de configuration existant.

### 3. Interface web (moduleair.cpp)

#### Nouvel onglet "Écrans"
- Activation de l'onglet 6 (précédemment commenté)
- Ajout du panneau 6 avec sections :
  - **Capteurs intérieurs** : PM1, PM2.5, PM10, CO2, COV, Température, Humidité, Pression
  - **Prévisions AtmoSud** : Indice, PM10, PM2.5, O3, NO2, SO2

#### Code ajouté (lignes 2250-2273)
```cpp
page_content = tmpl(FPSTR(WEB_DIV_PANEL), String(6));
page_content += FPSTR(WEB_LF_B);
page_content += F("Sélection des écrans de polluants");
page_content += FPSTR(WEB_B_BR_BR);

page_content += F("<h3>Capteurs intérieurs</h3>");
add_form_checkbox(Config_screen_pm01, FPSTR(INTL_SCREEN_PM01));
// ... autres checkboxes
```

### 4. Labels internationalisés (intl_fr.h)

Ajout de 13 nouvelles constantes (lignes 44-58) :
- INTL_SCREEN_PM01, INTL_SCREEN_PM25, INTL_SCREEN_PM10
- INTL_SCREEN_CO2, INTL_SCREEN_COV
- INTL_SCREEN_TEMP, INTL_SCREEN_HUMI, INTL_SCREEN_PRESS
- INTL_SCREEN_ATMO_INDEX, INTL_SCREEN_ATMO_PM10, etc.

### 5. Fonction d'affichage matrice (moduleair.cpp)

Mise à jour de `display_values_matrix()` (lignes 5339-5405) :
- Remplacement de toutes les références `cfg_screen_*` par `cfg::screen_*`
- Respect des nouvelles variables de configuration
- Compatibilité avec les écrans NébuleAir

**Exemple** :
```cpp
if (cfg::npm_read && cfg::display_measure)
{
    if (cfg::screen_pm10)
        screens[screen_count++] = 3; //PM10
    if (cfg::screen_pm25)
        screens[screen_count++] = 4; //PM2.5
    if (cfg::screen_pm01)
        screens[screen_count++] = 5; //PM1
}
```

### 6. Nettoyage (configuration.h)

Suppression des anciennes variables `cfg_screen_*` et ajout d'une note explicative pointant vers :
- Le namespace `cfg::` dans moduleair.cpp
- L'onglet "Écrans" de l'interface web

### 7. Corrections bonus (intl_fr.h)

Correction d'une erreur de syntaxe existante (ligne 108) :
- Avant : `"Le fichier config.json n\'a pas pu être effacé."`
- Après : `"Le fichier config.json n'a pas pu être effacé."`

## Fonctionnement

### Cycle de vie de la configuration

1. **Démarrage** : Les valeurs par défaut sont définies dans `cfg::` namespace
2. **Lecture SPIFFS** : Si un fichier config.json existe, les valeurs sauvegardées sont chargées
3. **Interface web** : L'utilisateur peut modifier les paramètres via l'onglet "Écrans"
4. **Sauvegarde** : Les modifications sont sauvegardées dans SPIFFS au redémarrage
5. **Application** : Les fonctions d'affichage utilisent les valeurs de `cfg::screen_*` pour filtrer les écrans

### Boucle d'affichage

```
Pour chaque type de capteur activé :
    Si cfg::screen_[type] == true :
        Ajouter l'écran à la rotation

Rotation automatique des écrans ajoutés
```

## Avantages de cette implémentation

✅ **Persistance** : Les paramètres sont sauvegardés en SPIFFS
✅ **Interface intuitive** : Onglet dédié dans la page de configuration
✅ **Rétrocompatibilité** : Les anciennes configurations continuent de fonctionner
✅ **Modularité** : Facile d'ajouter de nouveaux types d'écrans
✅ **Cohérence** : Utilise le système de configuration existant
✅ **Pas de cassure** : Aucune modification des fonctionnalités existantes

## Utilisation

1. Accéder à l'interface web du ModuleAir (http://192.168.4.1 ou l'IP locale)
2. Cliquer sur "Configuration"
3. Sélectionner l'onglet "Écrans"
4. Cocher/décocher les écrans désirés :
   - **Capteurs intérieurs** : PM1, PM2.5, PM10, CO2, COV, T, H, P
   - **Prévisions AtmoSud** : Indice, PM10, PM2.5, O3, NO2, SO2
5. Cliquer sur "Sauvegarder et redémarrer"
6. Le système redémarre avec la nouvelle configuration

## Fichiers modifiés

- `moduleair.cpp` : Ajout variables cfg::, onglet web, mise à jour display_values_matrix
- `moduleair-cfg.h` : Ajout entries de configuration
- `intl_fr.h` : Ajout labels, correction erreur syntaxe
- `configuration.h` : Suppression anciennes variables, ajout note

## Tests recommandés

1. ✅ Vérifier que la compilation réussit sans erreurs
2. ✅ Tester l'affichage de l'onglet "Écrans" dans l'interface web
3. ✅ Modifier quelques paramètres et redémarrer
4. ✅ Vérifier que la configuration est persistante après redémarrage
5. ✅ Tester que seuls les écrans cochés apparaissent sur la matrice/OLED
6. ✅ Vérifier la rotation automatique des écrans sélectionnés

## Notes importantes

- Les écrans de base (Air intérieur, Wifi info, Device info, Logos) sont toujours affichés
- La désactivation d'un écran ne désactive pas le capteur correspondant
- Les valeurs par défaut privilégient l'affichage de tous les polluants importants
- L'écran "Pression" est désactivé par défaut pour éviter la surcharge d'information
