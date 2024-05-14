// Librerías 

// Definiciones

// Instancias

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
}

// Bucle ----------------------------------------------------------------------
void loop() {
  task_done();
  delay(2000);
}

// ------------------------------- Funciones ------------------------------- //
// Funciones de control -------------------------------------------------------

// Funciones Generales --------------------------------------------------------
// Enciende el LED integrado cuando se completa una tarea
void task_done(){
  digitalWrite(LED_BUILTIN,HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN,LOW);
}