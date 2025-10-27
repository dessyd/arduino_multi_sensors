# Guide de Tests de Validation

Ce document décrit les procédures de test pour valider la refactorisation du code Arduino.

## Tests Préliminaires (Compilation)

### Test 1: Vérification de la compilation
```bash
# Compiler avec Arduino CLI (si disponible)
arduino-cli compile --fqbn arduino:samd:mkrwifi1010 .
```

**Résultat attendu**: Compilation sans erreur ni warning

---

## Tests Matériels

### Test 2: Initialisation des Composants

**Procédure**:
1. Téléverser le sketch sur la carte MKR WiFi 1010
2. Ouvrir le moniteur série (9600 baud)
3. Observer les messages de démarrage

**Résultats attendus**:
```
========================================
Arduino Multi-Sensor System - Refactored
========================================
Board ID: <MAC_ADDRESS>
Connecting to WiFi...
WiFi connected!
SSID: <YOUR_SSID>
IP Address: <IP>
Signal strength (RSSI): <VALUE> dBm
Network initialized
Syncing time from NTP.....
Time synchronized: 2025-10-27
RTC synchronized
SD Card initialized
Found MKR ENV shield
Waiting for Air Quality sensor to initialize (20s)...
Found Air Quality sensor
Detected 2 sensor(s)
Setup completed - entering main loop
========================================
```

**Vérifications**:
- [✓] WiFi connecté
- [✓] IP obtenue
- [✓] RTC synchronisé avec NTP
- [✓] SD card initialisée (si présente)
- [✓] MKR ENV shield détecté
- [✓] Capteur Air Quality détecté

### Test 3: Lecture Non-Bloquante (millis)

**Procédure**:
1. Observer les messages dans le moniteur série
2. Vérifier que les lectures se font à intervalles réguliers (60 secondes)
3. Pendant l'attente, vérifier que la carte est responsive (LED clignotante si implémentée)

**Résultats attendus**:
- Les lectures s'effectuent toutes les 60 secondes
- Pas de blocage entre les lectures
- Messages réguliers : "--- Sensor read cycle completed ---"

### Test 4: Envoi de Données via UDP

**Procédure**:
1. Lancer le script de réception UDP sur un ordinateur (voir section suivante)
2. Observer les paquets reçus
3. Vérifier le format des messages

**Résultats attendus**:
Messages au format statsd:
```
sensor.temperature:22.40|g|#board_id:AABBCCDDEEFF,board_type:mkr1010,sensor_type:mkr_env
sensor.humidity:45.20|g|#board_id:AABBCCDDEEFF,board_type:mkr1010,sensor_type:mkr_env
sensor.pressure:1013.25|g|#board_id:AABBCCDDEEFF,board_type:mkr1010,sensor_type:mkr_env
...
```

### Test 5: Écriture sur Carte SD

**Procédure**:
1. Insérer une carte SD formatée (FAT32)
2. Laisser tourner pendant 5 minutes
3. Retirer la carte SD et vérifier son contenu

**Résultats attendus**:
- Fichier `20251027.log` avec les messages de log
- Fichier `20251027.csv` avec les mesures
- Format CSV: `timestamp,measure_name=value`

### Test 6: Gestion de Reconnexion WiFi

**Procédure**:
1. Laisser le système tourner normalement
2. Désactiver temporairement le WiFi (éteindre le routeur ou bloquer le réseau)
3. Observer les messages d'erreur
4. Réactiver le WiFi
5. Observer la reconnexion automatique

**Résultats attendus**:
```
WiFi disconnected - attempting reconnection
WiFi reconnected
```

### Test 7: Qualité de l'Air - Niveaux de Pollution

**Procédure**:
1. Placer le capteur dans un environnement normal
2. Observer le niveau : "Fresh air."
3. Exposer le capteur à de la fumée ou vapeur (test optionnel)
4. Observer le changement de niveau

**Résultats attendus**:
- Messages de log avec niveaux variables : Fresh air, Low pollution, High pollution, etc.
- Sévérité appropriée dans les logs (SEV_INFO à SEV_CRITICAL)

---

## Tests de Ressources

### Test 8: Utilisation Mémoire

**Procédure**:
1. Après compilation, noter l'utilisation de la RAM
2. Comparer avec l'ancienne version

**Résultats attendus**:
- Utilisation RAM optimisée (pas d'allocation dynamique excessive)
- Pas de fragmentation mémoire après plusieurs heures de fonctionnement

### Test 9: Stabilité Long Terme

**Procédure**:
1. Laisser tourner pendant 24 heures
2. Observer les logs et les données collectées

**Résultats attendus**:
- Aucun crash ou redémarrage
- Données cohérentes sur toute la période
- Pas de fuite mémoire

---

## Tests de Comparaison (Ancien vs Nouveau Code)

### Test 10: Comparaison Fonctionnelle

| Fonctionnalité | Ancien Code | Nouveau Code | Status |
|----------------|-------------|--------------|--------|
| Lecture capteurs | ✓ | ✓ | ✓ |
| Envoi UDP | ✓ | ✓ | ✓ |
| Log SD card | ✓ | ✓ | ✓ |
| Synchronisation NTP | ✓ | ✓ | ✓ |
| Gestion WiFi | Bloquant | Non-bloquant | Amélioré |
| Init SD card | À chaque écriture | Une seule fois | Optimisé |
| UDP Socket | Recréé chaque loop | Maintenu ouvert | Optimisé |
| DNS resolution | À chaque loop | Cachée | Optimisé |

---

## Checklist Finale de Validation

- [ ] Compilation sans erreur
- [ ] Tous les capteurs détectés
- [ ] WiFi connecté et stable
- [ ] Données UDP reçues correctement
- [ ] Fichiers SD créés et formatés correctement
- [ ] Comportement non-bloquant vérifié
- [ ] Reconnexion WiFi fonctionnelle
- [ ] Stabilité sur 24h
- [ ] Consommation mémoire acceptable
- [ ] Code plus lisible et maintenable

---

## Notes de Débogage

Si un test échoue, vérifier:

1. **Compilation échoue**: Vérifier les dépendances des bibliothèques
2. **WiFi ne connecte pas**: Vérifier `arduino_secrets.h`
3. **Capteurs non détectés**: Vérifier les connexions I2C/analogiques
4. **SD card erreur**: Vérifier format FAT32, pin chipSelect
5. **UDP non reçu**: Vérifier pare-feu, adresse serveur Splunk

---

## Script de Test UDP (voir test_udp_receiver.py)

Utiliser le script Python fourni pour capturer et valider les paquets UDP statsd.
