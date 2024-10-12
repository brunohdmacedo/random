#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <math.h>
#include <max6675.h>

// Configuração do LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Endereço do LCD

// Pinos para o MAX6675
#define MAX6675_SO 2
#define MAX6675_CS 3
#define MAX6675_SCK 4
#define RELE_RESISTENCIA 5  // Pino do relé da resistência

// Pino para o sensor LM35
const int pinLM35 = A0;  // Pino analógico para o LM35

double temperatura;
double setPoint_resistencia = 27.5;
double input;
const double histerese = 4.0;  // Faixa de histerese de +- 4°C
float intercepto = 3.267;
float slope = 0.938;
unsigned long tempoAnterior = 0;
const long intervaloTela = 10000;  // Intervalo de 10 segundos para atualizar a tela

void setup() {
    analogReference(DEFAULT);
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    pinMode(RELE_RESISTENCIA, OUTPUT);

    lcd.setCursor(0, 1);
    lcd.print("Bem Vindo!");
    delay(1000);
    lcd.clear();

    lcd.setCursor(0, 1);
    lcd.print("Projeto Capeta");
    delay(1000);
    lcd.clear();
}

void loop() {
    unsigned long currentMillis = millis();

    // Atualizar leitura
    if (currentMillis - tempoAnterior >= 200) {
        tempoAnterior = currentMillis;

        // Ler temperatura do termopar e aplicar a equação de calibração
        temperatura = leer_termopar();
        float temperaturaCorrigida = (slope * temperatura) + intercepto;
        input = temperaturaCorrigida - 3;

        // Controle de histerese para o relé da resistência
        if (input < setPoint_resistencia - histerese) {
            digitalWrite(RELE_RESISTENCIA, HIGH);  // Liga a resistência
        } else if (input > setPoint_resistencia + histerese) {
            digitalWrite(RELE_RESISTENCIA, LOW);  // Desliga a resistência
        }

        // Leitura da temperatura ambiente com o LM35
        float temperaturaAmbiente = lerTemperaturaLM35();

        // Enviar dados para monitor serial
        Serial.print("Temp. Bruta: ");
        Serial.print(temperatura, 2);
        Serial.print(" C, Temp. Corrigida: ");
        Serial.print(input, 2);
        Serial.print(" C, Temp. Ambiente: ");
        Serial.print(temperaturaAmbiente, 2);
        Serial.println(" C");
    }

    // Alternar entre telas a cada 10 segundos
    if (currentMillis % intervaloTela < 200) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp In: ");
        lcd.print(input, 1);
        lcd.setCursor(0, 1);
        lcd.print("Temp Amb: ");
        lcd.print(lerTemperaturaLM35(), 1);
    }
}

// Função para ler temperatura do termopar (MAX6675)
double leer_termopar() {
    uint16_t v;
    pinMode(MAX6675_CS, OUTPUT);
    pinMode(MAX6675_SO, INPUT);
    pinMode(MAX6675_SCK, OUTPUT);

    digitalWrite(MAX6675_CS, LOW);

    // Usar micros() para medir o tempo ao invés de delay()
    unsigned long startMicros = micros();
    while (micros() - startMicros < 1);  // Esperar 1 microsegundo

    v = shiftIn(MAX6675_SO, MAX6675_SCK, MSBFIRST);
    v <<= 8;
    v |= shiftIn(MAX6675_SO, MAX6675_SCK, MSBFIRST);

    digitalWrite(MAX6675_CS, HIGH);
    if (v & 0x4) {
        return NAN;  // Termopar desconectado
    }

    v >>= 3;
    return v * 0.25;
}

// Função para ler a temperatura do LM35
float lerTemperaturaLM35() {
    // Ler o valor analógico do LM35 (0 a 1023)
    int leituraLM35 = analogRead(pinLM35);

    // Converter a leitura para graus Celsius
    float temperatura = (leituraLM35 * (5.0 / 1023.0)) * 100.0;

    return temperatura;
}
