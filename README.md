# Projet Environnement Intelligent et Communicant 
## Réalisé par Valentin VALETTE et Ayke Che

### les objectifs du projet

Le l'objectif du pojet consiste à maquetter un système domotique. 
Celui-ci doit contenir au minimum un routeur Wi-fi (réseau local de la maison),
un capteur de température/humidité (Xiami), une carte ESP32, une carte STM32 et un cerveau moteur.
La carte ESP32 doit récupérer les données du capteur de température/humidité via Wi-fi puis les transmettre sur un serveur MQTT.
La carte STM32 doit ensuite récupérer ces données via Ethernet et agir en conséquence sur la commande du cerveau moteur qui simule l'ouverture ou la fermeture du volet de ventilation.

### Réalisation du projet

La totalité des éléments imposés est présente et fonctionnelle dans notre projet. 
De plus, nous avons aussi ajouté l'utilisation de l'ESP32 avec l'affichage d'un message au démarage de celui-ci ainsi que l'affichage d la température, l'humidité et le pourcentage de batterie du capteur.

#### La carte ESP32
##### Pré-requis
Afin de configurer la carte ESP32, nous utilisons l'IDE Visuale Studio Code avec l'extension PlateformeIO. 
De plus, afin que la carte soit reconnue il est nécessaire d'intaller le Driver "CP210x_Windows_Drivers" (Téléchargeable via la lien suivant : https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

##### La gestion du Wifi-fi
Afin de configurer le wifi nous devons importer la librairie <Wifi.h> :
```C++
#include <WiFi.h>
```
Puis nous devons nous connecter à un point d'accès Wi-fi. Pour cela, on utilise le code suivant : 
```C++
const char* ssid = "B127-EIC";
const char* password = "b127-eic";

WiFiClient espClient;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setup()
{
  // Initialise wifi
  setup_wifi();

}
```

On peut vérifier que la carte est bien connecté en réalisant un ping vers l'adresse IP de cette dernière ou en vérifiant sur notre point d'accès que la carte est bien connectée.
Pour connaître l'IP de notre carte il nous suffit de récupérer celle ci via la variable `WiFi.localIP()`

##### Récupération des données du capteur (utilisation du Bluetooth Low Energy [BLE])
Afin d'utiliser le BLE il faut installer la librairie `ESP32 BLE Arduino`

##### Envoi des données vers un server MQTT (Publication sur un Topic)

##### Utilisation de l'écran du ESP32




Fichier d'itinitialisation `plateformio.ini`
