#include <Arduino.h>
#include "config_webserver.h"

long currentMillis = 0;
long previousMillis = 0;
int intervalo = 1;
int contador = 0;
float temp_contator = 0.0;
float temperaturas[5] = {0}; // Vetor para armazenar as últimas 5 temperaturas
int index_temp = 0;          // Índice do vetor de temperaturas
int num_leituras = 0;        // Número de leituras feitas (máximo 5)

// Função para calcular a média das últimas 5 temperaturas
float calcularMediaTemp()
{
  float soma = 0;
  int leituras_validas = min(num_leituras, 5); // Caso ainda não tenha 5 leituras
  for (int i = 0; i < leituras_validas; i++)
  {
    soma += temperaturas[i];
  }
  return soma / leituras_validas;
}
void monitoramento()
{
  if (hora_grau > 0 && nome_peixe != "sem_nome" && start == "true")
  {
    // Verifica se passaram 1 minuto
    if (millis() - previousMillis >= intervalo * 60 * 1000)
    {
      previousMillis = millis();
      String temperatura_agora = readDSTemperatureC();
      Serial.println(temperatura_agora);
      temp_contator += temperatura_agora.toFloat();
      // Armazenar a temperatura atual no vetor de temperaturas
      temperaturas[index_temp] = temperatura_agora.toFloat();
      index_temp = (index_temp + 1) % 5; // Avançar o índice, voltando para 0 após 5 leituras
      if (num_leituras < 5)
      {
        num_leituras++;
      }

      // Calcular a média das últimas 5 temperaturas
      float temp_media = calcularMediaTemp();
      porcentagem = (temp_contator * 100) / minuto_grau;
      contador++;
      String dataString = String(contador) + ";" + temperatura_agora + ";" + nome_peixe;
      Serial.println(dataString);
      appendFile(SD, "/dados.csv", dataString.c_str());
      // Calcular o tempo restante com base na média da temperatura
      float tempo_restante_minutos = (minuto_grau - temp_contator) / temp_media;
      tempo_restante_horas = tempo_restante_minutos / 60;
      finalizado = "false";
      if (porcentagem >= 100)
      {
        start = "false";
        digitalWrite(LED_MONITORAMENTO_ON, LOW);
        finalizado = "true";
        temp_contator = 0.0;
        contador = 0;
        hora_grau = 0;
        nome_peixe = "sem_nome";
        minuto_grau = 0;
        porcentagem = 0.0;
        tempo_restante_horas = 0.0;

        // apagar_dados_do_arquivo(SD,"/dados.cvs");
      }
    }
  }
}
void setup()
{

  Serial.begin(115200);
  sensors.begin();
  initSDCard();
  writeFile(SD, "/dados.csv", "tempo;temperatura;especie");
  pinMode(LED_ESP32_ON, OUTPUT);
  pinMode(LED_MONITORAMENTO_ON, OUTPUT);
  initSPIFFS();
  init_Wifi_AP();
  init_Server();
  ErrosNaInicializacao();
  // createDir(SD,"/teste");
}

void loop()
{
  server.handleClient();
  monitoramento();
  delay(10);
}
