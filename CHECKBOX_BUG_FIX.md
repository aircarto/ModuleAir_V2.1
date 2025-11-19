# Bug Critique : Checkboxes toujours décochées après sauvegarde

## Description du bug

**Symptôme** : Même si les capteurs étaient activés par défaut dans `ext_def.h` (ex: `MHZ16_READ 1`), quand l'utilisateur allait dans l'interface web et cliquait "Sauvegarder et redémarrer", toutes les checkboxes non explicitement cochées étaient enregistrées comme `false` dans config.json, écrasant les valeurs par défaut.

**Impact** :
- Les capteurs configurés dans `ext_def.h` étaient désactivés après une simple sauvegarde web
- Les utilisateurs devaient re-cocher manuellement tous les capteurs
- Les écrans de polluants ne s'affichaient pas si le capteur correspondant était désactivé

## Cause racine

### Problème dans form_checkbox() (ligne 1881)

```cpp
s = F("<label for='{n}'>"
      "<input type='checkbox' name='{n}' value='1' id='{n}' {c}/>"
      "<input type='hidden' name='{n}' value='0'/>"  // ← BUG ICI
      "{i}</label><br/>");
```

Chaque checkbox génère **deux inputs** avec le même nom :
1. Un `<input type='hidden' value='0'>` (toujours présent)
2. Un `<input type='checkbox' value='1'>` (seulement si coché)

### Problème dans webserver_config_send_body_post() (ligne 2339-2340)

```cpp
case Config_Type_Bool:
    *(c.cfg_val.as_bool) = (server_arg == "1");  // ← BUGUÉ
    break;
```

**Comportement bugué** :

Quand le formulaire est soumis :
- **Checkbox COCHÉE** → Envoie : `mhz16_read=0` ET `mhz16_read=1`
- **Checkbox DÉCOCHÉE** → Envoie : `mhz16_read=0` seulement

`server.arg("mhz16_read")` retourne la **première** valeur trouvée, donc :
- Si cochée : Retourne `"0"` (du hidden) → `server_arg == "1"` est FALSE → Sauvegardé comme `false` ❌
- Si décochée : Retourne `"0"` → `server_arg == "1"` est FALSE → Sauvegardé comme `false` ❌

**Résultat** : Toutes les checkboxes étaient TOUJOURS sauvegardées comme `false` !

## Solution implémentée

### Modification du traitement POST (lignes 2339-2355)

```cpp
case Config_Type_Bool:
    // Handle checkbox with hidden input: if "1" is present anywhere, it's checked
    // The form sends both "0" (hidden) and "1" (checkbox if checked)
    {
        bool is_checked = false;
        // Check all arguments with this name
        for (int i = 0; i < server.args(); i++)
        {
            if (server.argName(i) == s_param && server.arg(i) == "1")
            {
                is_checked = true;
                break;
            }
        }
        *(c.cfg_val.as_bool) = is_checked;
    }
    break;
```

**Logique de la correction** :
- Parcourt TOUS les arguments du formulaire
- Si on trouve un argument avec le bon nom ET la valeur "1", la checkbox est cochée
- Sinon, elle est décochée

**Comportement corrigé** :
- **Checkbox COCHÉE** → Trouve `mhz16_read=1` → Sauvegardé comme `true` ✅
- **Checkbox DÉCOCHÉE** → Ne trouve que `mhz16_read=0` → Sauvegardé comme `false` ✅

## Scénario de test

### Avant la correction

1. `ext_def.h` : `#define MHZ16_READ 1`
2. Démarrage : `cfg::mhz16_read = true` ✅
3. Interface web → Config → Capteurs → MHZ16 est coché (car cfg::mhz16_read = true)
4. Utilisateur clique "Sauvegarder" sans toucher aux checkboxes
5. POST reçoit : `mhz16_read=0` et `mhz16_read=1`
6. `server.arg("mhz16_read")` retourne `"0"` (premier trouvé)
7. Sauvegarde : `"mhz16_read": false` dans config.json ❌
8. Redémarrage : config.json écrase → `cfg::mhz16_read = false` ❌
9. Écran CO2 ne s'affiche plus même si `cfg::screen_co2 = true` ❌

### Après la correction

1. `ext_def.h` : `#define MHZ16_READ 1`
2. Démarrage : `cfg::mhz16_read = true` ✅
3. Interface web → Config → Capteurs → MHZ16 est coché (car cfg::mhz16_read = true)
4. Utilisateur clique "Sauvegarder" sans toucher aux checkboxes
5. POST reçoit : `mhz16_read=0` et `mhz16_read=1`
6. Boucle trouve `mhz16_read=1` → `is_checked = true`
7. Sauvegarde : `"mhz16_read": true` dans config.json ✅
8. Redémarrage : config.json confirme → `cfg::mhz16_read = true` ✅
9. Écran CO2 s'affiche correctement ✅

## Impact de la correction

✅ **Valeurs par défaut respectées** : Les capteurs définis dans `ext_def.h` restent activés
✅ **Cohérence** : Ce qui est affiché (coché) est ce qui est sauvegardé
✅ **UX améliorée** : L'utilisateur n'a pas à tout re-configurer après chaque sauvegarde
✅ **Écrans fonctionnels** : Les écrans de polluants s'affichent si leur capteur est activé

## Fichiers modifiés

- `moduleair.cpp` (lignes 2339-2355) : Traitement POST des checkboxes

## Notes techniques

- Le `<input type='hidden' value='0'>` est une pratique courante en HTML pour s'assurer qu'une valeur est toujours envoyée même si la checkbox n'est pas cochée
- `server.arg(name)` retourne seulement la première valeur si plusieurs existent avec le même nom
- La solution parcourt tous les arguments pour détecter la présence de "1"
- Cette correction s'applique à TOUTES les checkboxes du formulaire de configuration

## Tests recommandés

1. ✅ Configurer `MHZ16_READ 1` dans ext_def.h
2. ✅ Démarrer le système (première fois)
3. ✅ Aller dans Config → Capteurs → Vérifier que MHZ16 est coché
4. ✅ Cliquer "Sauvegarder et redémarrer" sans toucher aux cases
5. ✅ Après redémarrage, vérifier que MHZ16 est toujours coché
6. ✅ Vérifier que l'écran CO2 s'affiche sur OLED/Matrice
7. ✅ Décocher MHZ16, sauvegarder, redémarrer, vérifier qu'il est bien décoché
8. ✅ Re-cocher MHZ16, sauvegarder, redémarrer, vérifier qu'il est bien coché
