#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <math.h>
#include <max6675.h>
#include <SD.h>  // Biblioteca para comunicação com o cartão SD

// Configuração do LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Endereço do LCD
File myFile;  // Arquivo para armazenar dados no SD

// Pinos para o MAX6675
#define MAX6675_SO 2
#define MAX6675_CS 3
#define MAX6675_SCK 4
#define RELE_RESISTENCIA 5  // Pino do relé da resistência

// Pino para o sensor LM35
const int pinLM35 = A0;  // Pino analógico para o LM35
#define CS_PIN 10  // Pino de Chip Select (CS) para o SD Card

double temperatura;
double setPoint_resistencia = 30;
double input;
const double histerese = 2.0;  // Faixa de histerese de +- 2°C
float intercepto = 3.267;
float slope = 0.938;
unsigned long tempoAnterior = 0;
const long intervaloTela = 10000;  // Intervalo de 10 segundos para atualizar a tela
unsigned long intervaloPID = 200;   // Intervalo para atualizar

void setup() {
    lcd.init();  // Inicia o LCD
    lcd.backlight();  // Liga o backlight do LCD
    lcd.setCursor(0, 0);
    lcd.print("Inicializando...");
    
    analogReference(DEFAULT);
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    pinMode(RELE_RESISTENCIA, OUTPUT); 
    digitalWrite(RELE_RESISTENCIA, LOW); // Relé desligado inicialmente

    // Inicializar o cartão SD
    if (!SD.begin(CS_PIN)) {
        Serial.println("Falha ao inicializar o cartão SD");
        lcd.setCursor(0, 0);
        lcd.print("Falha no SD!");
        while (1);  // Fica em loop se não conseguir inicializar
    }

    myFile = SD.open("datalog.csv", FILE_WRITE);
    if (myFile) {
        myFile.println("Tempo,Temp In (C),Temp Amb (C)");  // Cabeçalho
        myFile.close();
        Serial.println("Arquivo CSV criado com sucesso");
    } else {
        Serial.println("Erro ao criar arquivo CSV");
    }

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

    // Atualizar leitura a cada intervaloPID
    if (currentMillis - tempoAnterior >= intervaloPID) {
        tempoAnterior = currentMillis;

        // Ler temperatura do termopar e aplicar a equação de calibração
        temperatura = leer_termopar();
        float temperaturaCorrigida = (slope * temperatura) + intercepto;
        input = temperaturaCorrigida - 3;

        bool releStatus = false;

        // Controle de histerese para o relé da resistência
        if (input >= setPoint_resistencia - histerese) {
            digitalWrite(RELE_RESISTENCIA, HIGH);  // Liga a resistência
            releStatus = true;
        } else if (input <= setPoint_resistencia + histerese) {
            digitalWrite(RELE_RESISTENCIA, LOW);  // Desliga a resistência
            releStatus = true;
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

        // Registrar dados no arquivo CSV
        logToFile(input, temperaturaAmbiente);
    }

    // Alternar entre telas a cada 10 segundos
    if (currentMillis % intervaloTela < intervaloPID) {
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
    int leituraLM35 = analogRead(pinLM35);
    float temperatura = (leituraLM35 * (5.0 / 1023.0)) * 100.0;
    return temperatura;
}

// Função para gravar os dados no arquivo CSV
void logToFile(double tempIn, double tempOut) {
    myFile = SD.open("datalog.csv", FILE_WRITE);
    if (myFile) {
        unsigned long currentTime = millis() / 1000;  // Tempo em segundos
        myFile.print(currentTime);
        myFile.print(",");
        myFile.print(tempIn, 2);
        myFile.print(",");
        myFile.println(tempOut, 2);
        myFile.close();
        Serial.println("Dados gravados no arquivo CSV");
    } else {
        Serial.println("Erro ao abrir o arquivo CSV para escrita");
    }
}
