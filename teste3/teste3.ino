#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <PID_v1_bc.h>  // Biblioteca para controle PID
#include <math.h>
#include <max6675.h>
#include <SD.h>         // Biblioteca para comunicação com o cartão SD

LiquidCrystal_I2C lcd(0x27, 20, 4); // Endereço do LCD
File myFile;                        // Arquivo para armazenar dados no SD

#define MAX6675_SO 2
#define MAX6675_CS 3
#define MAX6675_SCK 4
#define RELE_RESISTENCIA 5  // Pino do relé da resistência
#define RELE_VENTILACAO 6   // Pino do relé da ventilação
#define ADCPIN A3           // Pino de leitura do sensor de temperatura (NTC 10k)
#define CS_PIN 10           // Pino de Chip Select (CS) para o SD Card

// Constantes para o NTC 10k
const double SERIES_RESISTOR = 10000.0;
const double BETA_COEFFICIENT = 3950.0;
const double ROOM_TEMP_RESISTANCE = 10000.0;
const double NOMINAL_TEMP = 25.0;

double temperatura;
double tempNTC;
double setPoint_resistencia = 27.5;
double input, output_resistencia, output_ventilacao;
double Kp = 2.0, Ki = 5.0, Kd = 1.0; // Ajustar conforme necessário

unsigned long tempoAnterior = 0;
const long intervaloTela = 10000;     // Intervalo de 10 segundos para atualizar a tela
unsigned long intervaloPID = 200;     // Intervalo para atualizar o PID

// Coeficientes da equação de correção
float intercepto = 3.267;
float slope = 0.938;

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

    // Atualizar leitura e controle PID em intervalos rápidos
    if (currentMillis - tempoAnterior >= intervaloPID) {
        tempoAnterior = currentMillis;

        // Ler temperatura do termopar
        temperatura = leer_termopar();
        
        // Aplicar a equação de correção
        float temperaturaCorrigida = intercepto + slope * temperatura;
        input = temperaturaCorrigida;
       
        // Ler a temperatura do sensor NTC
        tempNTC = lerNTC(ADCPIN);

        // Enviar dados para monitor serial
        Serial.print("Temp. In (termopar corrigido): ");
        Serial.print(input, 2);
        Serial.print(" C, Temp. Out (NTC): ");
        Serial.print(tempNTC, 2);
        Serial.println(" C");

        // Aqui você pode comparar manualmente a leitura do termopar corrigido
        // com o valor observado no termômetro digital externo.

        // Registrar dados no arquivo CSV
        //logToFile(temperaturaCorrigida, tempNTC);
    }

    // Alternar entre telas a cada 10 segundos
    if (currentMillis % intervaloTela < intervaloPID) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp In: ");
        lcd.print(temperatura, 1);

        //lcd.setCursor(0, 1);
        //lcd.print("Temp Out: ");
        //lcd.print(tempNTC, 1);
    }
}

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

double lerNTC(int sensorPin) {
    int adcValue = analogRead(sensorPin);

    if (adcValue == 0) {
        return NAN;
    }

    double resistance = SERIES_RESISTOR * (1023.0 / adcValue - 1.0);
    double steinhart;
    steinhart = resistance / ROOM_TEMP_RESISTANCE;
    steinhart = log(steinhart);
    steinhart /= BETA_COEFFICIENT;
    steinhart += 1.0 / (NOMINAL_TEMP + 273.15);
    steinhart = 1.0 / steinhart;
    steinhart -= 273.15;
    return steinhart;
}
