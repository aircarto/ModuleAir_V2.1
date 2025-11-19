# NébuleAir External Sensor Integration

## 🌟 Vue d'ensemble

Cette fonctionnalité permet d'afficher les données d'un capteur NébuleAir externe en récupérant les mesures depuis l'API AirCarto. Les données de particules fines (PM1, PM2.5, PM10) du capteur externe s'affichent automatiquement sur la matrice LED après les mesures de l'air intérieur.

## 📋 Fonctionnalités

- ✅ Récupération automatique des données toutes les 2 minutes
- ✅ Affichage d'un écran résumé "Air Extérieur"
- ✅ Affichage détaillé de PM10, PM2.5 et PM1
- ✅ Codes couleur selon les seuils de qualité de l'air
- ✅ Activation/désactivation via l'interface web
- ✅ Gestion intelligente : si pas de données, pas d'affichage

## ⚙️ Configuration

### Via l'interface web

1. Accédez à l'interface web du ModuleAir (http://IP_DU_MODULEAIR)
2. Allez dans **Configuration** > **Autres paramètres**
3. Section **"NébuleAir External Sensor"** :
   - ☑️ Cochez **"Enable NébuleAir"**
   - 📝 Entrez le **Device ID** du capteur (ex: `nebuleair-181`)
4. Sauvegardez la configuration

### Manuellement (ext_def.h)

Si vous préférez configurer directement dans le code :

```cpp
// Dans ext_def.h ou configuration.h
#define NEBULEAIR_READ 1
#define NEBULEAIR_ID "nebuleair-181"
```

## 🔄 Flux de fonctionnement

### 1. Récupération des données

```
Toutes les 2 minutes (120 secondes) :
  └─> Appel API: https://api.aircarto.fr/capteurs/dataNebuleAir
      └─> Paramètres:
          • capteurID: votre device ID
          • start: -2m (2 minutes avant)
          • end: now()
          • freq: 2m
          • format: JSON
      └─> Réponse:
          {
            "PM1": 8.25,
            "PM25": 10.65,
            "PM10": 15.05,
            "TEMP": 15.87,
            "HUM": 49.23
          }
```

### 2. Stockage des données

Les valeurs sont stockées dans des variables globales :
- `last_value_NebuleAir_PM1`
- `last_value_NebuleAir_PM25`
- `last_value_NebuleAir_PM10`
- `last_value_NebuleAir_TEMP`
- `last_value_NebuleAir_HUM`

### 3. Affichage sur la matrice LED

```
Ordre d'affichage :
1. Air Intérieur (capteurs locaux)
2. → NOUVEAU : Air Extérieur NébuleAir (écran résumé)
3. → NOUVEAU : PM10 NébuleAir
4. → NOUVEAU : PM2.5 NébuleAir
5. → NOUVEAU : PM1 NébuleAir
6. Mesures locales (CO2, Température, etc.)
7. Prévisions AtmoSud (si activé)
8. Informations système
```

## 🎨 Écrans ajoutés

### Écran 23 : Air Extérieur (Résumé)
Affiche l'image `exterieur_nebuleair` (déjà présente dans `aircarto-logo-common.h`)

### Écran 24 : PM10 NébuleAir
- **Valeur** : PM10 en µg/m³
- **Seuils** :
  - 🟢 0-15 : Bon
  - 🟡 15-30 : Moyen
  - 🟠 30-75 : Dégradé
  - 🔴 >75 : Mauvais

### Écran 25 : PM2.5 NébuleAir
- **Valeur** : PM2.5 en µg/m³
- **Seuils** :
  - 🟢 0-10 : Bon
  - 🟡 10-20 : Moyen
  - 🟠 20-50 : Dégradé
  - 🔴 >50 : Mauvais

### Écran 26 : PM1 NébuleAir
- **Valeur** : PM1 en µg/m³
- **Seuils** :
  - 🟢 0-10 : Bon
  - 🟡 10-20 : Moyen
  - 🟠 20-50 : Dégradé
  - 🔴 >50 : Mauvais

## 🔧 Fichiers modifiés

### Configuration
- **defines.h** : Ajout de `LEN_NEBULEAIR_ID` et `SAMPLETIME_NEBULEAIR_MS`
- **moduleair-cfg.h** : Ajout des entrées de configuration
- **moduleair.cpp** : Variables cfg::nebuleair_read, cfg::nebuleair_id

### Code principal
- **moduleair.cpp** :
  - `fetchSensorNebuleAir()` : Nouvelle fonction de récupération API (ligne ~4728)
  - `loop()` : Intégration de l'appel toutes les 2 minutes (ligne ~7215)
  - `setup()` : Initialisation du timer (ligne ~7077)
  - `display_values_matrix()` : Ajout des 4 nouveaux écrans (lignes 5267, 6024-6122)

### Interface web
- **moduleair.cpp** : Section NébuleAir dans la config web (ligne ~2098)

## 🐛 Débogage

### Activer les logs
Dans l'interface web, définissez **Debug Level = 5** pour voir tous les logs.

Logs à surveiller :
```
Fetching NébuleAir data...
API URL: https://api.aircarto.fr/capteurs/dataNebuleAir?...
HTTP GET failed, code: XXX
NébuleAir PM1: XX.XX
NébuleAir PM2.5: XX.XX
NébuleAir PM10: XX.XX
```

### Problèmes courants

#### ❌ Pas d'affichage des écrans NébuleAir
**Causes possibles** :
1. **NébuleAir non activé** → Vérifiez la config web
2. **Device ID incorrect** → Vérifiez l'orthographe
3. **Pas de WiFi** → Les données nécessitent une connexion
4. **API inaccessible** → Vérifiez la connectivité internet
5. **Pas de données récentes** → Le capteur externe est peut-être hors ligne

**Solution** : Activez Debug Level 5 et surveillez les logs série

#### ❌ Erreur HTTP
- **Code 404** : Device ID incorrect
- **Code 500** : Problème serveur API
- **Code -1** : Pas de connexion WiFi

## 🚀 Extension future

### Ajouter d'autres paramètres

Si vous voulez afficher d'autres données (température, humidité, gaz), ajoutez des écrans supplémentaires :

```cpp
// Dans display_values_matrix(), ajouter :
case 27: // Température NébuleAir
    if (last_value_NebuleAir_TEMP != -128.0) {
        // Code d'affichage similaire aux écrans PM
    }
    break;
```

### Ajouter une icône personnalisée

L'icône "nuage/extérieur" n'est pas encore définie. Pour l'ajouter :

1. Créez une image 7x9 pixels en RGB565
2. Ajoutez-la dans `aircarto-logo-common.h` :
```cpp
uint16_t static cloudExterieur[] = {
    // Vos pixels RGB565...
};
```
3. Décommentez dans `moduleair.cpp` ligne 6045 :
```cpp
drawImage(55, 0, 7, 9, cloudExterieur);
```

## 📊 Performance

- **Mémoire RAM** : ~50 bytes pour les variables
- **Appels API** : 1 appel toutes les 2 minutes
- **Temps d'exécution** : ~500ms par requête
- **Impact sur l'affichage** : +4 écrans dans la rotation

## 🔒 Sécurité

- ✅ Connexion HTTPS à l'API
- ⚠️ Validation du certificat désactivée (`client.setInsecure()`)
  - Pour plus de sécurité, ajoutez le certificat racine d'AirCarto

## 📝 Notes de version

### Version 1.0 (2025-01-15)
- ✨ Première implémentation
- ✅ Récupération des données PM1, PM2.5, PM10
- ✅ Affichage sur matrice LED
- ✅ Interface web de configuration

## 🤝 Contribution

Pour améliorer cette fonctionnalité :
1. Ajoutez des tests de la récupération API
2. Implémentez un cache local en cas de perte de connexion
3. Ajoutez des statistiques (min/max/moyenne)
4. Créez des graphiques historiques

## 📞 Support

Pour toute question ou bug :
- GitHub Issues : https://github.com/aircarto/ModuleAir_V2/issues
- Documentation : https://docs.aircarto.fr

---

**Développé avec ❤️ pour AirCarto**
