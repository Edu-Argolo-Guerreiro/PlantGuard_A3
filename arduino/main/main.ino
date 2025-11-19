// ---------- DEFINIÇÃO DE PINOS ----------
const int buzzerPin   = 7;   // Piezo
const int ldrPin      = A0;  // LDR no pino analógico A0
const int ledStatus   = 3;   // LED de status
const int servoPin    = 13;  // Servo motor

const int ledVerde    = 8;
const int ledAmarelo  = 9;
const int ledVermelho = 10;

// ---------- VARIÁVEIS GLOBAIS ----------
int ldrValue = 0;
int valorLuminosidade;
// Limites ORIGINAIS — não alterados
const int LIM_MUITO_BAIXA_MAX = 10;
const int LIM_BAIXA_MAX       = 30;
const int LIM_MEDIA_MAX       = 50;
const int LIM_IDEAL_MAX       = 70;
const int LIM_MUITO_ALTA_MIN  = 90;

// Controle de modos
bool monitorEnabled = true;
bool alarmEnabled   = true;

// ---------- PROTÓTIPOS ----------
void lerSensorEAtualizarAtuadores();
void apagaLeds();
void processSerialCommands();
void responderStatus();

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);

  pinMode(ledStatus,   OUTPUT);
  pinMode(buzzerPin,   OUTPUT);
  pinMode(ledVerde,    OUTPUT);
  pinMode(ledAmarelo,  OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(servoPin,   OUTPUT);


  apagaLeds();
  digitalWrite(ledStatus, LOW);
}

// ---------- LOOP PRINCIPAL ----------
void loop() {
  // 1) Executa monitoramento 

  if (monitorEnabled) {
    lerSensorEAtualizarAtuadores();
  } else {
    apagaLeds();
    noTone(buzzerPin);
  }
  Serial.println(valorLuminosidade);

  // 2) 
  if (Serial.available() > 0) {
    char comando = Serial.read(); // Lê o dado que chegou
    // Serial.println("entra no laço");
    // Serial.println(comando);
    if (comando == 'A') {
      digitalWrite(servoPin, HIGH);
      // Serial.println("Janela aberta");
    } 
    else if (comando == 'F') {
      digitalWrite(servoPin, LOW);
      // Serial.println("Janela fechada");
    }
  }

  // digitalWrite(servoPin, HIGH);

  // 3) Imprime o valor de luminosidade para leitura da ponte
  // Serial.println(valorLuminosidade);

  //teste
  // digitalWrite(servoPin, 90);
  // digitalWrite(servoPin, 0);


  // Pisca LED de status (indica que o Arduino está vivo)
  digitalWrite(ledStatus, HIGH);
  delay(8);
  digitalWrite(ledStatus, LOW);

  delay(500);
}

// ---------- LÓGICA DE LUMINOSIDADE ----------
void lerSensorEAtualizarAtuadores() {
  ldrValue = analogRead(ldrPin);
  valorLuminosidade = map(ldrValue , 0, 1023, 0, 100);


  // Fazer o print desse valor valorLuminosidade na porta serial

  apagaLeds();
  noTone(buzzerPin);

  // Muito baixa
  if (valorLuminosidade <= LIM_MUITO_BAIXA_MAX) {
    digitalWrite(ledVermelho, HIGH);
    if (alarmEnabled) tone(buzzerPin, 1000);
  }
  // Baixa
  else if (valorLuminosidade <= LIM_BAIXA_MAX) {
    digitalWrite(ledVermelho, HIGH);
  }
  // Média
  else if (valorLuminosidade <= LIM_MEDIA_MAX) {
    digitalWrite(ledAmarelo, HIGH);
  }
  // Ideal
  else if (valorLuminosidade <= LIM_IDEAL_MAX) {
    digitalWrite(ledVerde, HIGH);
  }
  // Muito alta
  else if (valorLuminosidade >= LIM_MUITO_ALTA_MIN) {
    digitalWrite(ledVermelho, HIGH);
    if (alarmEnabled) tone(buzzerPin, 1500);
  }
}

// ---------- APAGA LEDS ----------
void apagaLeds() {
  digitalWrite(ledVerde,    LOW);
  digitalWrite(ledAmarelo,  LOW);
  digitalWrite(ledVermelho, LOW);
  digitalWrite(ledStatus,   LOW);
}

