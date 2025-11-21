// ---------- DEFINIÇÃO DE PINOS ----------
const int buzzerPin   = 7;   // Piezo
const int ldrPin      = A0;  // LDR no pino analógico A0
const int ledStatus   = 3;   // LED de status
const int ledAcaoPin    = 13;  // Led indicando a ação do servo
const int servoPin    = 11;   // Servo

const int ledVerde    = 8;
const int ledAmarelo  = 9;
const int ledVermelho = 10;

// ---------- VARIÁVEIS GLOBAIS ----------
int ldrValue = 0;
// int valorLuminosidade;
// Limites ORIGINAIS — não alterados
const int LIM_MUITO_BAIXA_MAX = 15;
const int LIM_BAIXA_MAX       = 30;
const int LIM_MEDIA_MAX       = 50;
const int LIM_IDEAL_MAX       = 70;
const int LIM_MUITO_ALTA_MIN  = 90;

// Controle de modos
bool alarmEnabled   = true;

// ---------- PROTÓTIPOS ----------
int lerSensorEAtualizarAtuadores();
void piscaLedStatus();
void apagaLeds();
void verificaAcaoMotor();

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);

  pinMode(ledStatus,   OUTPUT);
  pinMode(buzzerPin,   OUTPUT);
  pinMode(ledVerde,    OUTPUT);
  pinMode(ledAmarelo,  OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAcaoPin,   OUTPUT);


  apagaLeds();
  digitalWrite(ledStatus, LOW);
}

// ---------- LOOP PRINCIPAL ----------
void loop() {
  // 1) Executa monitoramento 
  int valorLuminosidade = lerSensorEAtualizarAtuadores();
  
  Serial.println(valorLuminosidade);

  verificaAcaoMotor();

  piscaLedStatus();
  
  delay(1000);
}

void verificaAcaoMotor() {
if (Serial.available() > 0) {
    char comando = Serial.read();
    if (comando == 'A') {
      digitalWrite(ledAcaoPin, HIGH);
    } 
    else if (comando == 'F') {
      digitalWrite(ledAcaoPin, LOW);
    }
  }
}

void piscaLedStatus() {
  digitalWrite(ledStatus, HIGH);
  delay(8);
  digitalWrite(ledStatus, LOW);
  delay(8);
}

// ---------- LÓGICA DE LUMINOSIDADE ----------
int lerSensorEAtualizarAtuadores() {
  ldrValue = analogRead(ldrPin);
  int valorLuminosidade = map(ldrValue , 0, 1023, 0, 100);

  apagaLeds();
  noTone(buzzerPin);

  // Muito baixa
  if (valorLuminosidade <= LIM_MUITO_BAIXA_MAX) {
    digitalWrite(ledVermelho, HIGH);
    if (alarmEnabled) tone(buzzerPin, 2000);
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
  // Alta
  else if (valorLuminosidade <= LIM_MUITO_ALTA_MIN) {
    digitalWrite(ledAmarelo, HIGH);
  }
  // Muito alta
  else if (valorLuminosidade >= LIM_MUITO_ALTA_MIN) {
    digitalWrite(ledVermelho, HIGH);
    if (alarmEnabled) tone(buzzerPin, 4000);
  }

  return valorLuminosidade;
}

// ---------- APAGA LEDS ----------
void apagaLeds() {
  digitalWrite(ledVerde,    LOW);
  digitalWrite(ledAmarelo,  LOW);
  digitalWrite(ledVermelho, LOW);
  digitalWrite(ledStatus,   LOW);
}

