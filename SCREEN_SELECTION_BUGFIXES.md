# Corrections des Bugs - Sélection d'Écrans

## Problèmes identifiés et corrigés

### Bug 1 : Aucun écran ne s'affichait avec seulement CO2 sélectionné

**Cause** : La fonction `display_values_oled()` n'utilisait PAS les variables `cfg::screen_*` pour filtrer les écrans. Elle affichait toujours tous les écrans configurés, peu importe la sélection de l'utilisateur.

**Solution** : Ajout de conditions de filtrage dans `display_values_oled()` (lignes 5111-5131) :

```cpp
// Add screens based on sensor availability AND user selection
if (cfg::npm_read && cfg::display_measure && (cfg::screen_pm01 || cfg::screen_pm25 || cfg::screen_pm10))
{
    screens[screen_count++] = 0;
}

if (cfg::mhz16_read && cfg::display_measure && cfg::screen_co2)
{
    screens[screen_count++] = 2;
}
// ... etc
```

**Résultat** : L'OLED respecte maintenant les paramètres de sélection d'écrans.

---

### Bug 2 : Division par zéro si aucun écran sélectionné

**Cause** : Si l'utilisateur décochait tous les écrans, `screen_count` valait 0, causant une division par zéro dans `screens[next_display_count % screen_count]`.

**Solution** : Ajout de vérifications de sécurité (lignes 5155-5160 et 5436-5441) :

```cpp
// Safety check: if no screens selected, show at least device info
if (screen_count == 0)
{
    debug_outln_info(F("No screens selected for OLED, showing device info"));
    screens[screen_count++] = 7; // Device info as fallback
}
```

**Résultat** :
- OLED : Affiche au moins les infos de l'appareil
- Matrice : Affiche au moins l'écran "Air intérieur"

---

### Bug 3 : Rotation trop rapide avec peu d'écrans

**Problème rapporté** : Avec seulement 1-2 écrans sélectionnés, la rotation de 5 secondes était trop courte. L'utilisateur n'avait pas le temps de lire les données.

**Solution** : Implémentation d'un système d'intervalle dynamique (lignes 1168-1193) :

```cpp
static unsigned long getDisplayInterval(unsigned int screen_count)
{
    // Vise ~40 secondes pour une rotation complète
    const unsigned long TARGET_ROTATION_TIME_MS = 40000;
    unsigned long calculated_interval = TARGET_ROTATION_TIME_MS / screen_count;

    // Contraintes : minimum 3s, maximum 15s
    const unsigned long MIN_INTERVAL_MS = 3000;
    const unsigned long MAX_INTERVAL_MS = 15000;

    if (calculated_interval < MIN_INTERVAL_MS)
        return MIN_INTERVAL_MS;
    if (calculated_interval > MAX_INTERVAL_MS)
        return MAX_INTERVAL_MS;

    return calculated_interval;
}
```

**Logique de calcul** :
- **Objectif** : Maintenir environ 40 secondes pour voir tous les écrans une fois
- **1 écran** : 15 secondes (plafonné au maximum)
- **2 écrans** : 15 secondes (40/2 = 20s, plafonné à 15s)
- **3 écrans** : ~13 secondes (40/3 ≈ 13s)
- **5 écrans** : 8 secondes (40/5 = 8s)
- **8 écrans** : 5 secondes (40/8 = 5s)
- **13+ écrans** : 3 secondes (plancher minimum)

**Variables ajoutées** (lignes 1141-1142) :
```cpp
uint8_t oled_screen_count = 0;
uint8_t matrix_screen_count = 0;
```

Ces variables stockent le nombre d'écrans actifs et sont mises à jour à chaque affichage.

**Modification de la boucle principale** (lignes 7468 et 7474) :
```cpp
// Avant :
if ((msSince(last_display_millis_oled) > DISPLAY_UPDATE_INTERVAL_MS) && ...)

// Après :
if ((msSince(last_display_millis_oled) > getDisplayInterval(oled_screen_count)) && ...)
```

**Résultat** : L'intervalle s'adapte automatiquement au nombre d'écrans sélectionnés.

---

## Exemples concrets

### Scénario 1 : Seulement CO2 activé
- **Avant** : Aucun écran ne s'affichait (bug)
- **Après** :
  - OLED montre : CO2, Wifi info, Device info, Lora info (si configurés)
  - Intervalle : ~10 secondes par écran (pour 4 écrans)

### Scénario 2 : Tous les écrans désactivés
- **Avant** : Crash (division par zéro)
- **Après** :
  - OLED : Montre au moins Device Info
  - Matrice : Montre au moins Air intérieur
  - Intervalle : 15 secondes (1 seul écran)

### Scénario 3 : 10 écrans activés
- **Avant** : 5 secondes par écran (50 secondes pour tout voir)
- **Après** : 4 secondes par écran (40 secondes pour tout voir)

### Scénario 4 : 2 écrans activés
- **Avant** : 5 secondes par écran (trop rapide)
- **Après** : 15 secondes par écran (confortable)

---

## Fichiers modifiés

1. **moduleair.cpp**
   - Lignes 1141-1142 : Variables globales `oled_screen_count` et `matrix_screen_count`
   - Lignes 1168-1193 : Fonction `getDisplayInterval()`
   - Lignes 5111-5131 : Filtrage des écrans dans `display_values_oled()`
   - Lignes 5155-5160 : Protection contre screen_count = 0 (OLED)
   - Ligne 5295 : Mise à jour de `oled_screen_count`
   - Lignes 5436-5441 : Protection contre screen_count = 0 (Matrice)
   - Ligne 6271 : Mise à jour de `matrix_screen_count`
   - Lignes 7468 et 7474 : Utilisation de `getDisplayInterval()`

---

## Tests recommandés

1. ✅ Tester avec seulement CO2 activé → doit afficher CO2 + écrans système
2. ✅ Tester avec tous les écrans désactivés → doit afficher écran de secours
3. ✅ Tester avec 1-2 écrans → intervalle doit être long (~13-15s)
4. ✅ Tester avec 10+ écrans → intervalle doit être court (~3-4s)
5. ✅ Vérifier qu'OLED et Matrice peuvent avoir des configurations différentes
6. ✅ Redémarrer et vérifier que la configuration persiste

---

## Avantages de cette correction

✅ **Robustesse** : Plus de crash si aucun écran sélectionné
✅ **Cohérence** : OLED et Matrice respectent tous deux la sélection
✅ **UX améliorée** : Temps d'affichage adapté au nombre d'écrans
✅ **Flexibilité** : 3-15 secondes selon le contexte
✅ **Intelligent** : Vise 40 secondes de rotation totale (temps optimal)

---

## Notes techniques

- Le calcul d'intervalle se fait à chaque appel, donc très léger en ressources
- Les variables `*_screen_count` sont mises à jour après chaque affichage
- Si la configuration change (via web), les compteurs seront mis à jour au prochain cycle d'affichage
- La protection "fallback" garantit qu'il y a toujours au moins 1 écran visible
