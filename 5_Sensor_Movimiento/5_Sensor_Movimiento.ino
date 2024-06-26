// Librerías
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <WiFi.h>
#include <DHT.h>

// Definiciones
// Pantalla OLED
#define SCREEN_WIDTH   128   // Ancho pantalla OLED, en pixeles
#define SCREEN_HEIGHT  64    // Alto pantalla OLED, en pixeles
#define OLED_RESET     -1    // Pin reset # (o -1 si comparte el pin de reset del MCU)
#define SCREEN_ADDRESS 0x3C  // Mirar Datasheet para la dirección; 0x3D for 128x64, 0x3C for 128x32

// Reloj
#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     -18000         // offset = GTM*3600. EJ: -5*3600
#define UTC_OFFSET_DST 0

// Sensores
#define DHT_PIN    15 // Pin del sensor DHT11
#define SR501_PIN  23 // Pin del sensor hc-sr501
#define TRANSISTOR 19 // Pin controlador del sensor SR-501

// Luces
#define GRADAS 27 // Pin para la luz de gradas

// Instancias
Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // Pantalla OLED
hw_timer_t *tempo = NULL;                                               // Temporizador
DHT dht(DHT_PIN, DHT11);                                                // Sesor DHT11
struct tm reloj;                                                        // Reloj de tiempo real (RTC)

// Constantes
// Constantes Wi-Fi
const char* ssid = "tu usuario";      // SSID
const char* pass = "tu contraseña";   // Contraseña

// Variables
volatile bool estado = false; // Estado de interrupciones
char *estado_SR501 = "ON";    // Estado del sensor SR501
float temp = 0, hum = 0;      // Temperatura y humedad
int i;                        // Contador Multipropósito

// ----------------------- Funciones de Interrupción ----------------------- //
// Encender luz de gradas -----------------------------------------------------
void IRAM_ATTR gradas_on(){
  digitalWrite(GRADAS, LOW);
  timerStart(tempo);
  estado = true;
}

// Apagar luz de gradas -------------------------------------------------------
void IRAM_ATTR gradas_off(){
  digitalWrite(GRADAS, HIGH);
  timerStop(tempo);
  estado = false;
}

// --------------------------- Cuerpo del código --------------------------- //
// Configuración --------------------------------------------------------------
void setup() {
  // Configuración Serial
  Serial.begin(115200);

  // Configuración de puertos
  int salidas[] = {LED_BUILTIN, GRADAS, TRANSISTOR};
  for (i=0; i<sizeof(salidas)/sizeof(salidas[0]); i++){
    pinMode(salidas[i], OUTPUT);
    digitalWrite(salidas[i], HIGH);
  }
  digitalWrite(TRANSISTOR,LOW);
  task_done();
  // Entradas
  pinMode(SR501_PIN, PULLDOWN);

  // Configuración pantalla OLED
  if(!OLED.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) Serial.println("SSD1306 asignación fallida");
  OLED.clearDisplay();
  OLED.setTextSize(1);
  OLED.setTextColor(SSD1306_WHITE);
  OLED.cp437(true);

  escribir("Hola, JEISON", 28, 0);
  escribir("Bienvenido a", 28, 8);
  escribir("tu SmartRoom", 28, 16);
  task_done();

  // Configuración Wi-Fi
  conectar_WiFi();

  // Configuración Fecha y Hora
  OLED.clearDisplay();
  escribir("Configurando Reloj",10, 0);
  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  task_done();

  // Configuración DHT11
  escribir("Configurando DHT11",10,8);
  dht.begin();    
  task_done();

  // Configuración SR-501
  escribir("Configurando SR501", 10, 16);
  while (!getLocalTime(&reloj)) delay(1);
  int aux = reloj.tm_hour;                // Hora, formato: 24h
  if(aux>5 && aux<19){
    digitalWrite(TRANSISTOR, LOW);
    estado_SR501 = "OFF";    
  }

  //Configurar Temporizador
  escribir("Conf. Temporizador", 10, 24);
  //f = 80e6/50e3 = 1600 Hz; T = 1/1600 s = 625 us
  tempo = timerBegin(0, 50000, false);  //(Canal, divisor, hacia abajo) Div. Max: 65535
  timerWrite(tempo, 15*1600);
  timerStop(tempo);
  task_done();
    
  //Configuración interrupciones
  escribir("Conf. Interrupciones", 7, 32);
  attachInterrupt(SR501_PIN, gradas_on, RISING);  // (Pin, funcion, LOW a HIGH)
  timerAttachInterrupt(tempo, gradas_off, true);  // (temporizador, funcion, edge)
  
  //Evento temporizador. EJ: Para 1s -> veces = 1/625u = 1600
  timerAlarmWrite(tempo, 0, true);          // (temporizador, veces, resetear)
  timerAlarmEnable(tempo);
  task_done();

  delay(1000);
}

// Bucle ----------------------------------------------------------------------
void loop() {
  // Control Wifi  
  if(WiFi.status() != WL_CONNECTED) conectar_WiFi();

  // Leer Temperatura y Humedad
  temp = dht.readTemperature();
  hum = dht.readHumidity();

  // Control temporal
  temporal();

  // Mostrar datos
  pantalla_principal();
  delay(250);
}

// ------------------------------- Funciones ------------------------------- //
// Funciones de control -------------------------------------------------------
// Conectar o reconectar WiFi
void conectar_WiFi(){
  i = 1;

  escribir("Conectando Wifi",19,40);
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED and i < OLED.width()){
    OLED.drawLine(0, 48, i, 48, SSD1306_WHITE);
    OLED.display();
    i++;
    delay(500);
  }

  if(WiFi.status() == WL_CONNECTED) escribir("Conectado a: " + String(ssid),0,50);
  else escribir("Wifi no conectado",13,50);    

  task_done();
  delay(1000);
}

// Control cada hora
void temporal(){
  if(reloj.tm_sec == 0 and reloj.tm_min == 0){
    int t = millis();
    switch(reloj.tm_hour){
      case 6:
        digitalWrite(TRANSISTOR, LOW);
        estado_SR501 = "OFF";
      break;
      case 18:
        digitalWrite(TRANSISTOR, HIGH);
        estado_SR501 = "ON";
      break;
      default: break;      
    }

    // ThingSpeak
    task_done();   
    t = millis()-t;
    if(t<1e3) delay(1e3-t); 
  }  
}

// Funciones Generales --------------------------------------------------------
// Pantalla principal con la información relevante
void pantalla_principal(){
  // Limpiar pantlla
  OLED.clearDisplay();

  // Mostrar Fecha y hora
  OLED.setCursor(0,0);
  if (getLocalTime(&reloj)) OLED.println(&reloj, "%d/%m/%Y %I:%M:%S%p");
  else OLED.println("Error en Reloj");

  // Mostrar Temperatura y humedad
  OLED.setCursor(0,8);
  if(!isnan(temp) and !isnan(hum)) OLED.printf("Temperatura:%2.1f%cC\nHumedad:%2.0f%c",temp,167,hum,37);
  else OLED.print("Error en sensor DHT11");

  // Mostrar estado del SR-501 y la interrupciones
  OLED.setCursor(0, 24);
  OLED.print("Sensor SR-501:" + String(estado_SR501));
  
  if(estado){
    OLED.printf(" %2.0f", timerReadSeconds(tempo));
    OLED.fillCircle(124,27,3,SSD1306_WHITE);
  }
  else OLED.drawCircle(124,27,3,SSD1306_WHITE);

  // Mostrar información de luces
  OLED.setCursor(0, 32);
  OLED.print("Luces:0");

  for (int i=0; i<5; i++){
    OLED.drawCircle(i*8+52,35,3,SSD1306_WHITE);
    delay(1);
  }

  // Mostrar Crédito
  OLED.setCursor(43, 56);
  OLED.print("By JEXX");

  // Mostrar todo
  OLED.display();
}

// Escribe un mensaje en la pantalla OLED con las coordenadas enviadas
void escribir(String mensaje, int x, int y){
  OLED.setCursor(x,y);
  OLED.println(mensaje);
  OLED.display();
  delay(10);
}

// Enciende el LED integrado cuando se completa una tarea
void task_done(){
  digitalWrite(LED_BUILTIN,HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN,LOW);
}