// Librerías
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// Definiciones
// Pantalla OLED
#define SCREEN_WIDTH   128   // Ancho pantalla OLED, en pixeles
#define SCREEN_HEIGHT  64    // Alto pantalla OLED, en pixeles
#define OLED_RESET     -1    // Pin reset # (o -1 si comparte el pin de reset del MCU)
#define SCREEN_ADDRESS 0x3C  // Mirar Datasheet para la dirección; 0x3D for 128x64, 0x3C for 128x32

// Instancias
Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // Pantalla OLED

// Constantes

// Variables
int i;  // Contador Multipropósito

// ----------------------- Funciones de Interrupción ----------------------- //

// --------------------------- Cuerpo del código --------------------------- //
// Configuración --------------------------------------------------------------
void setup() {
  // Configuración Serial
  Serial.begin(115200);

  // Configuración de puertos
  int salidas[]  = {LED_BUILTIN};
  for (i=0; i<sizeof(salidas)/sizeof(salidas[0]); i++){
    pinMode(salidas[i], OUTPUT);
    digitalWrite(salidas[i], HIGH);
  }
  task_done();

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


  delay(1000);
}

// Bucle ----------------------------------------------------------------------
void loop() {
  // Mostrar datos
  pantalla_principal();
  delay(250);
}

// ------------------------------- Funciones ------------------------------- //
// Funciones de control -------------------------------------------------------

// Funciones Generales --------------------------------------------------------
// Pantalla principal con la información relevante
void pantalla_principal(){
  // Limpiar pantlla
  OLED.clearDisplay();

  // Mostrar Fecha y hora
  OLED.setCursor(0,0);
  OLED.print("dd/mm/aaaa hh:mm:ssAM");

  // Mostrar Temperatura y humedad
  OLED.setCursor(0,8);
  OLED.printf("Temperatura:00.0%cC\nHumedad:00%c",167,37);

    // Mostrar estado sensor SR501
  OLED.setCursor(0, 24);
  OLED.print("Sensor SR-501:OFF");

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