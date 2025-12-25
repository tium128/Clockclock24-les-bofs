# Master Test Slaves

Programme de test pour les cartes slaves du ClockClock24. Interface web pour diagnostic et test des moteurs.

## Objectif

Tester individuellement chaque carte slave et ses moteurs avant l'assemblage final, via une interface web accessible sur le réseau WiFi.

## Architecture

### API REST (web_server.cpp)

| Endpoint | Méthode | Description | Paramètres |
|----------|---------|-------------|------------|
| `/api/scan` | GET | Scan I2C, retourne les cartes détectées | - |
| `/api/status` | GET | État actuel (positions, drivers) | - |
| `/api/motor/test` | POST | Test un moteur 360° | `board`, `clock`, `hand`, `direction` |
| `/api/drivers/enable` | POST | Active tous les drivers | - |
| `/api/drivers/disable` | POST | Désactive tous les drivers | - |
| `/api/stop` | POST | Toutes les aiguilles à 6h00 | - |

### Réponses JSON

**GET /api/scan**
```json
{
  "boards": [
    {"address": 1, "found": true},
    {"address": 2, "found": false},
    ...
  ],
  "count": 3
}
```

**GET /api/status**
```json
{
  "drivers_enabled": true,
  "positions": [
    {"board": 1, "clock": 0, "angle_h": 270, "angle_m": 270},
    ...
  ]
}
```

**POST /api/motor/test**
- Paramètres: `board` (1-8), `clock` (0-2), `hand` (H/M), `direction` (CW/CCW)
- Réponse: `{"success": true, "message": "Motor test started"}`

### Interface Web (test.html)

Page séparée accessible à `http://clockclock24-test.local/test.html`

**Sections:**
1. **I2C Scanner** - Bouton scan + affichage des cartes détectées
2. **Test Moteur** - Sélecteurs board/clock/hand + boutons CW/CCW
3. **Drivers** - Boutons Enable/Disable all
4. **Position** - Bouton "All to 6h00"
5. **Logs** - Zone de texte avec historique des commandes

### État interne (main.cpp)

```cpp
// Position assumée au démarrage: toutes les aiguilles à 270° (6h00)
// Le master maintient une copie des positions pour l'affichage
int motor_positions[8][3][2]; // [board][clock][hand] = angle (0-359)
bool drivers_enabled = true;
```

## Fichiers modifiés

| Fichier | Modifications |
|---------|---------------|
| `src/main.cpp` | Simplifié, plus de menu série, init positions à 270° |
| `src/web_server.cpp` | Ajout endpoints API REST |
| `include/web_server.h` | Déclarations nouvelles fonctions |
| `web/test.html` | Nouvelle page interface de test |
| `include/web_page.h` | Ajout test_page_html |

## Build

```bash
cd master-test-slaves
pio run
pio run -t upload
```

Accès: `http://clockclock24-test.local/test` ou `http://192.168.x.x/test`

## Progression

- [x] Copie du projet master
- [x] Documentation architecture (ce fichier)
- [x] API REST dans web_server.cpp
  - [x] GET /api/scan
  - [x] GET /api/status
  - [x] POST /api/motor/test
  - [x] POST /api/drivers/enable
  - [x] POST /api/drivers/disable
  - [x] POST /api/stop
- [x] Page test (embedded dans web_server.cpp)
  - [x] Section I2C Scanner
  - [x] Section Test Moteur
  - [x] Section Drivers
  - [x] Section Logs
- [x] Simplifier main.cpp (web only, no serial menu)

## Notes

- Positions initiales assumées à 270° (6h00) au boot
- Le slave doit être flashé avec le firmware `slave/` standard
- WiFi conservé pour accès web
