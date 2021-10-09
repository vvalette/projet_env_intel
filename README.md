# Projet Environnement Intelligent et Communicant 
## Réalisé par Valentin VALETTE et Ayke Che

### les objectifs du projet

L'objectif du pojet consiste à maquetter un système domotique. 
Celui-ci doit contenir au minimum un routeur Wi-fi (réseau local de la maison),
un capteur de température/humidité (Xiami), une carte ESP32, une carte STM32 et un servo moteur.
La carte ESP32 doit récupérer les données du capteur de température/humidité via Wi-fi puis les transmettre sur un serveur MQTT.
La carte STM32 doit ensuite récupérer ces données via Ethernet et agir en conséquence sur la commande du servo moteur qui simule l'ouverture ou la fermeture du volet de ventilation.

### Réalisation du projet

La totalité des éléments imposés est présente et fonctionnelle dans notre projet. 
De plus, nous avons aussi ajouté l'utilisation de l'ESP32 avec l'affichage d'un message au démarage de celui-ci ainsi que l'affichage de la température, l'humidité et le pourcentage de batterie du capteur.

#### La carte ESP32
##### Pré-requis
Afin de configurer la carte ESP32, nous utilisons l'IDE Visuale Studio Code avec l'extension PlateformeIO. 
De plus, afin que la carte soit reconnue il est nécessaire d'intaller le Driver "CP210x_Windows_Drivers" (Téléchargeable via la lien suivant : https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

Le fichier d'initialisation doit être le suivant : 
```C++
[env:ttgo-lora32-v1]
platform = espressif32
board = ttgo-lora32-v1
framework = arduino
board_build.partitions=huge_app.csv
lib_deps = 
	bodmer/TFT_eSPI@^2.3.70
	nkolban/ESP32 BLE Arduino@^1.0.1
	knolleary/PubSubClient@^2.8
```
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

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setup()
{
  // Initialise le wifi
  setup_wifi();

}
```

On peut vérifier que la carte est bien connectée en réalisant un ping vers l'adresse IP de cette dernière ou en vérifiant sur notre point d'accès que la carte est bien connectée.
Pour connaître l'IP de notre carte il nous suffit de récupérer celle ci via la variable `WiFi.localIP()`

##### Récupération des données du capteur (utilisation du Bluetooth Low Energy [BLE])
Afin d'utiliser le BLE il faut installer la librairie `ESP32 BLE Arduino`. Pour cela il suffit de se rendre dans la page d'accueil de PlateformeIO, puis dans l'onglet `Librairies` et rechercher notre librairies et l'ajouter au projet. De plus, un grand nombre de script d'exemple sont fournis avec toutes les libraires !

Tout d'abord il faut initialiser la connexion BLE dans la fonction setup() à l'aide des lignes suivantes :
```C++
BLEDevice::init("");
pBLEScan = BLEDevice::getScan(); //create new scan
pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
pBLEScan->setActiveScan(true);
pBLEScan->setInterval(100);
pBLEScan->setWindow(99); // less or equal setInterval value
```

Puis il faut créer une class `MyAdvertisedDeviceCallbacks` (cf le code) qui permet de récupérer les données une fois la connexion BLE établie. On peut retrouver cette fontion facilement dans les exemples fournis avec la librairie.

Pour finir on créer une class `BLEResult` qui nous permet de "ranger" proprement nos résulstat et d'y accéder facilement par la suite:
```C++
class BLEResult
{
public:
  double temperature = -200.0f;
  double humidity = -1.0f;
  int16_t battery_level = -1;
};

BLEResult result;
```
Il nous reste plus qu'à récupérer une des données (l'humidité par exemple) avec le code suivant : `resultat.humidity`

##### Envoi des données vers un server MQTT (Publication sur un Topic)
Afin d'utiliser l'écran de l'ESP32 nous devons ajouter la librairie `PubSubClient`.
Tout d'abord il faut initiliser l'adresse IP ou le nom de domaine de notre server MQTT :
```C++
const char* mqtt_server = "broker.mqttdashboard.com";
```
Dans notre projet nous utilisons un server MQTT en ligne accecile au lien suivant https://broker.mqttdashboard.com

Puis il faut initialiser la conexion au server dans le `void setup()`:
```C++
client.setServer(mqtt_server, 1883);
```
Puis nous pouvons publier un donnée sur un topic voulu grâce à la fonction suivante:
```C++
client.publish("sensor_data/tem", temp);
```
Ici nous publions notre température sur le topic `sensor_data/temp`

Attention, il est important que la valeur de temps soit une string. Pour cela nous devons changer le type de notre température qui est initilement un Float. Pour cela on utilise les ligne suivantes:
```C++
char temp[10]
sprintf(temp, "%.2f", result.temperature); 
```
Ici notre variable `result.temperature` est notre température mesurée et c'est donc un Float. La fonction sprintf nous permet donc de la mettre dans notre variable temp qui est un tableau de charatères et donc une string.

##### Utilisation de l'écran du ESP32
Afin d'utiliser l'écran de l'ESP32 nous devons ajouter la librairie `TFT_eSPI`. Tout d'abord il est important de modifier le fichier `ILI9341_Defines.h` (..PlatformIO\Projects\projet_env_intel\.pio\libdeps\ttgo-lora32-v1\TFT_eSPI\TFT_Drivers) et notamment les lignes : 
```C++
#define TFT_WIDTH  135
#define TFT_HEIGHT 240
```
Cette modification permet de paramétrer la dimension de l'écran. En effet, avec la configuration de base l'écran est mal configuré et on a une zone non utilisable.

Puis, il est nécessaire de modifier le fichier `User_Setup_Select.h` (PlatformIO\Projects\projet_env_intel\.pio\libdeps\ttgo-lora32-v1\TFT_eSPI). Ainsi, il faut décommenter la ligne 53 :
```C++ 
#include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
``` 
Cette action permet de slectionner le type de carte et le type d'écran de notre ESP32. 

Une fois tout cela réalisé on peut utiliser notre écran.

Pour cela il suffit d'instancier une object de la class `TFT_eSPI()`:
```C++
TFT_eSPI tft = TFT_eSPI(); 
```
Puis on peut écrire ce que l'on veut sur l'écran à l'aide du code suivant par exemple :
```C++
tft.begin(); // allumer l'écran
tft.setRotation(1); // définir le sens de l'écran
tft.setTextSize(2); // définir la taille du texte
tft.fillScreen(TFT_BLACK); //Reset l'écran (efface tout)
tft.setCursor(60, 80); // définir le curseur et donc l'endroit où l'on va écrire sur l'écran (en mode vertical : (largeur,hauteur))
tft.println("Project by"); //écrire sur l'écran une string
tft.setCursor(75, 115);
tft.println("Valentin");
tft.setCursor(75, 150);
tft.println("and Aykel");
```

#### La carte STM32 
##### Pré-requis
Pour pouvoir utiliser la carte stm32746g-discovery il faut utiliser l'IDE STM32CubeIDE qui permet de simplifier la mise en place des codes en important les configurations nécessaires.

##### Connexion IP
Pour pouvoir récupérer les informations du capteur la carte doit pouvoir se connecter à internet. Pour cela, nous somme partis d'un code exemple qui permet de récupérer une addresse IP via le DHCP et ainsi de communiquer sur un réseau via un câble Ethernet. 

##### Serveur MQTT
Après avoir récupéré une adresse IP le but est de se connecter au brocker MQTT pour pouvoir ensuite subscribe aux topics sur lesquelles les données sont transmises.

Pour nous connecter au serveur nous devons instencier un client via la library MQTT de Lwip:
```C++
mqtt_client_t *client = mqtt_client_new();
```

Après avoir créé un object client nous pouvons l'utilisé pour nous connecter.
Mais avant de pouvoir nous connecter il faut d'abord saisir les informations client pour cela on utilise la structure `mqtt_connect_client_info_t` :
```C++
struct mqtt_connect_client_info_t ci;
memset(&ci, 0, sizeof(ci));
ci.client_id = "ident";
```
Les données minimums pour se connecter est l'identifiant. Il ne manque plus que l'adresse IP du serveur :
```C++
  ip_addr_t adr;
  IP4_ADDR(&adr,192,168,1,122);
```

Pour lancer la connexion on utilise `Mqtt_connect`:
```C++
  err_t err;
  err = mqtt_client_connect(client,&adr,1883, mqtt_connection_cb, 0, &ci);
```
Si la connexion ce déroule sans acros on renvoie cette connection dans la fonction mqtt_connection_cb. C'est dans cette fonction que nous subscribon au différent topic: 
```C++
err_t err;
mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);
err = mqtt_subscribe(client, "sensor_data/hum", 0, mqtt_sub_request_cb, arg);
```
Comme nous pouvons le voir un callback est mis en place lors de la reception des données du topic dans un premier lieux la fonction mqtt_incoming_publish_cb permet d'analyser quelle topic publie des données en cas de souscription a plusieurs topics: 
```C++
if(strcmp(topic, "sensor_data/hum") == 0) {
    inpub_id = 0;
  } else if(strcmp(topic, "sensor_data/bat") == 0) {
    
    inpub_id = 1;
  } else if(strcmp(topic, "sensor_data/tem") == 0) {
    
    inpub_id = 2;
  }
  ```
  Dans notre exemple un valeur inpu_id est modifier en fonction du nom du topic. Après avoir mis a jour la valeur d'id la fonction mqtt_incoming_data_cb prned le relais. En effet elle permet l'affichage des différentes valeur qui sont publier:
  ```C++
   if(flags & MQTT_DATA_FLAG_LAST) {
    /* Last fragment of payload received (or whole part if payload fits receive buffer
       See MQTT_VAR_HEADER_BUFFER_LEN)  */
	  //LCD_UsrLog ("Temp: %s\n", MQTT_DATA_FLAG_LAST);

	  if(inpub_id == 0){
		  LCD_UsrLog ("humidity: %s\n", (const char *)data);
	  }
	  if(inpub_id == 1){
		  LCD_UsrLog ("batterye_power: %s\n", (const char *)data);
	  	  }
	  if(inpub_id == 2){
		  LCD_UsrLog ("temperature: %s\n", (const char *)data);
	  	  }
  }
```

Ici le `LCD_UsrLog` permet d'afficher les datas en fonction de l'inpub_id.

##### Servo Moteur
Après avoir reçu l'humidité de la pièce sur la carte Il faut que la bouche VMC puise etre ouvrte ou fermer en fonction de la valeur d'humidité.
Pour ce faire nous avons configuré les PIN D10 en TIMER PWM pour avoir un rapport cyclique entre 1/20 et 2/20.
Pour pouvoir lancer le timer voici la commande:
```C++
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
```
Affin de modifier la valeur de manière proportionnel à l'humidité nous créons une valeur affinée :
```C++
int value_Prepared = 16 * hum + 800;
```
Que nous passons ensuite dans une fonction qui modifie le rapport cyclique :
```C++
void change_motor(int value){
HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
TIM_OC_InitTypeDef  sConfig;
sConfig.OCMode = TIM_OCMODE_PWM1;
sConfig.Pulse = value;
sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
sConfig.OCNPolarity = TIM_OCNPOLARITY_HIGH;
sConfig.OCFastMode = TIM_OCFAST_DISABLE;
sConfig.OCIdleState = TIM_OCIDLESTATE_RESET;
sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfig, TIM_CHANNEL_1) != HAL_OK)
{
  Error_Handler();
}
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}
```
Ici on remarque que avant de changer la valeurs de Pulse on stop le timer via
```C++
HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
```
Grâce à cela nous avons bien notre servo moteur qui réagit en fonction de la valeur de l'humidité. Cependant, par manque de temps, nous n'avons pas pu rassembler les codes de récupération du server MQTT et ce code de pilotage du moteur... 

### Vidéo

Vous pouvez retrouver une vidéo de démo de notre projet via le lien suivant : https://www.youtube.com/watch?v=gv1G64o1bH0&ab_channel=AykelCheniour


