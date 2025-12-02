# 🧠 Frisquet Connect (ESP32 SX1262)

Emulation complète du module **Frisquet Connect** sur base **Heltec ESP32 WiFi LoRa 32 (SX1262)**.  
Ce projet permet à un ESP32 de dialoguer directement avec votre chaudière Frisquet,  
de récupérer les informations en temps réel et de les exposer à **Home Assistant via MQTT**.

---

## 🚀 Fonctionnalités principales

### 🔧 Emulation Frisquet Connect
- Récupération des informations chaudière :
  - Température **ECS (eau chaude sanitaire)**
  - Température **corps de chauffe (CDC)**
  - **Consommation gaz** ECS et chauffage (veille)
- Gestion des **zones 1, 2 et 3** :
  - Température **départ eau**
  - Température **consigne**
  - **Configuration des consignes** pour chaque mode :
    - Réduit
    - Hors gel
    - Confort
  - **Sélection du mode actif** (Auto, Confort, Réduit, Hors Gel)

### 🌡️ Emulation de la sonde extérieure
Deux modes possibles :
- Lecture **réelle** via une sonde **DS18B20**
- Lecture **virtuelle** via **MQTT**, permettant d’utiliser la température issue de la météo via un capteur HA.

### 🧩 Intégration Home Assistant (MQTT Discovery)
- Découverte automatique de tous les capteurs et entités :
  - Capteurs de température, de consommation, d’état de zones
  - Sélecteurs de modes
  - Commandes de consigne et interrupteurs (Boost, associations, etc.)
- Entités disponibles immédiatement dans Home Assistant

### 🌐 Portail Web intégré
- Configuration du **WiFi** et du **MQTT**
- Visualisation des **logs**
- Informations système et réseau
- Envoi de trame radio personnalisée (debug)

### 🔁 Mise à jour OTA
- Mise à jour du firmware directement via WiFi

---

## 🧰 Matériel nécessaire

| Composant | Description | Remarques |
|------------|-------------|-----------|
| 🧠 Heltec ESP32 WiFi LoRa 32 (SX1262) | Carte principale | Doit posséder le module **SX1262** |
| 🌡️ DS18B20 (optionnel) | Sonde de température | Connectée sur GPIO **33** par défaut |
| 🔩 Résistance 4,7 kΩ | Pull-up pour DS18B20 | Entre **VCC** et **DATA** |
| ☕ Café | Indispensable | Pour le développeur 😁 |

---

## ⚙️ Configuration et installation

### 1️⃣ Préparation du firmware

Avant le flash :
- Ouvrir le fichier **`DS18B20.h`**  
  et vérifier / modifier les options selon vos besoins :

| Option | Description |
|---------|-------------|
| `PIN_DS18B20` | GPIO utilisé (par défaut 33) |

---

### 2️⃣ Flash du firmware

- Connecter la carte **Heltec ESP32** via **USB**  
- Compiler et téléverser avec **PlatformIO** ou **Arduino IDE**
- Au premier démarrage, le module crée un **point d’accès WiFi**

---

### 3️⃣ Configuration via le portail web

1. Se connecter au WiFi créé : `HeltecFrisquet-Setup`, mot de passe `frisquetconfig`
2. Ouvrir un navigateur sur `192.168.4.1`
3. Renseigner :
   - Vos **informations WiFi**
   - Vos **informations MQTT**
   - Vos **Les modules à utiliser**
4. Sauvegarder → le module redémarre automatiquement

---

### 4️⃣ Association avec la chaudière

#### 🔹 Module Connect
1. Sur la chaudière : **lancer l’association Connect**  
2. Sur le portail ou Home Assistant : activer le bouton **“Associer Connect”**
3. Une fois reconnu, la chaudière commencera à envoyer les données vers le module

#### 🔹 Sonde extérieure
1. Sur la chaudière : **lancer l’association Sonde Extérieure**
2. Sur le portail ou Home Assistant : activer le bouton **“Associer Sonde Extérieure”**

Si une **DS18B20** est branchée, la température sera lue localement.
Sinon, envoyez la température via MQTT (ex. depuis un capteur météo HA), en publiant sur le topic **“"frisquet/sondeExterieure/temperatureExterieure/set"”**. (si Base Topic est toujours par défault à "frisquet" )

---

## 🔄 Fonctionnement général

1. Le module se connecte au **WiFi** et au **broker MQTT**
2. Il **écoute** les trames radio LoRa de la chaudière (mode SX1262)
3. Il **publie** les mesures et états via MQTT
4. Home Assistant les découvre automatiquement via **MQTT Discovery**
5. Les commandes (modes, consignes, associations) envoyées depuis HA  
   sont traduites en trames radio vers la chaudière

---

## 🔧 Dépannage

| Problème | Cause possible | Solution |
|-----------|----------------|----------|
| Pas de données reçues sur Connect | Changer de consigne sur chacun des satellites de zone pour initialiser le Connect une première fois.

---

## 🧑‍💻 Crédits

Projet développé pour émuler le **Frisquet Connect** avec compatibilité Home Assistant.  
- Firmware basé sur **Arduino / PlatformIO**
- Utilise **RadioLib** (SX1262)
- Intégration **MQTT + Auto-discovery HA**

---

## ☕ Licence

Projet open-source à but expérimental.  
Utilisation à vos risques et périls — aucune affiliation avec Frisquet.  
Mais bon, si ça marche, vous pouvez toujours m’offrir un café ☕ 😉
[!["Buy Me A Coffee"](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://www.buymeacoffee.com/freedomnx)

---

**Auteur :** FreedomNX  
**Année :** 2025  
**Plateforme :** ESP32 (Heltec WiFi LoRa 32, SX1262)  
**Compatibilité :** Home Assistant, MQTT, Frisquet Chaudière série Eco Radio Visio  
**Version :** 2.0.0as