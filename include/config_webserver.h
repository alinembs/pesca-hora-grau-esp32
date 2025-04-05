#include <WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "SPIFFS.h"
#include "salvamento_de_dados.h"
#include "sensor_temperatura.h"

#define servername "phg" // Define the name to server...
#define LED_ESP32_ON 26
#define LED_MONITORAMENTO_ON 25
const char *ssid = "PESCA_HORA_GRAU_PROJETO";
const char *password = "0123456789";
WebServer server(80);
String nome_peixe = "sem_nome";
int hora_grau = 0;
float porcentagem = 0.0;
float tempo_restante_horas = 0.0;
String start = "false";
int minuto_grau = 0;
String finalizado = "false";
// Possiblitar baixar arquivos que estão no SD para o Navegador
void SD_file_download(fs::FS &fs, String filename)
{
  File download = fs.open("/" + filename);
  if (download)
  {
    server.sendHeader("Content-Type", "text/text");
    server.sendHeader("Content-Disposition", "attachment; filename=" + filename);
    server.sendHeader("Connection", "close");
    server.streamFile(download, "application/octet-stream");
    download.close();
  }
  else
  {
    server.send(404, "text/plain", "Arquivo não encontrado");
  }
}

// Manipulador para páginas não encontradas
void handleNotFound()
{
  server.send(404, "text/plain", "Página não encontrada");
}
void initSPIFFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("An error has occurred while mounting SPIFFS");
    Erros++;
  }
  else
  {
    Serial.println("SPIFFS mounted successfully");
  }
}
//////////////////////////////CONTROLE DE ERROS  /////////////////////////////////
void ErrosNaInicializacao()
{

  if (Erros > 0)
  {
    while (true)
    {
      delay(500);
      digitalWrite(LED_ESP32_ON, HIGH); // Liga o LED
      delay(500);
      digitalWrite(LED_ESP32_ON, LOW); // Liga o LED
    }
  }
  else
  {
    digitalWrite(LED_ESP32_ON, HIGH);
  }
}
// Pegar arquivos do SD
bool loadFromSD(fs::FS &fs, String path, String dataType)
{
  Serial.print("Requested page -> ");
  Serial.println(path);
  if (fs.exists(path))
  {
    File dataFile = fs.open(path, "r");
    if (!dataFile)
    {
      handleNotFound();
      return false;
    }

    if (server.streamFile(dataFile, dataType) != dataFile.size())
    {
      Serial.println("Sent less data than expected!");
    }
    else
    {
      Serial.println("Page served!");
    }

    dataFile.close();
  }
  else
  {
    handleNotFound();
    return false;
  }
  return true;
}
// Pegar dados do SPIFFS
bool loadFromSPIFFS(String path, String dataType)
{
  Serial.print("Requested page -> ");
  Serial.println(path);
  if (SPIFFS.exists(path))
  {
    File dataFile = SPIFFS.open(path, "r");
    if (!dataFile)
    {
      handleNotFound();
      return false;
    }

    if (server.streamFile(dataFile, dataType) != dataFile.size())
    {
      Serial.println("Sent less data than expected!");
    }
    else
    {
      Serial.println("Page served!");
    }

    dataFile.close();
  }
  else
  {
    handleNotFound();
    return false;
  }
  return true;
}
// Iniciar Wifi no Modo Acess Point
void init_Wifi_AP()
{
  Serial.println("Configuring access point...");
  delay(1);
  if (!WiFi.softAP(ssid, password))
  {
    log_e("Soft AP creation failed.");
    while (1)
      ;
  }
  // Set your preferred server name, if you use "" the address would be http:///
  if (!MDNS.begin(servername))
  {
    Serial.println(F("Error setting up MDNS responder!"));
    ESP.restart();
  }

  Serial.println("MDNS started");

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}
// Rota da Tela Inicial
void handleRoot()
{
  loadFromSPIFFS("/index.html", "text/html");
}
void mandardados()
{
  // loadFromSPIFFS("/dados.csv", "text/csv");
  loadFromSD(SD, "/dados.csv", "text/csv");
}
// Rota de Vazao de água no momento
void temperatura_agua()
{
  server.send_P(200, "text/plain", readDSTemperatureC().c_str());
}
void SD_dir()
{
  // Action acording to post, dowload or delete, by MC 2022
  if (server.args() > 0) // Arguments were received, ignored if there are not arguments
  {
    Serial.println(server.arg(0));

    String Order = server.arg(0);
    Serial.println(Order);

    if (Order.indexOf("download_") >= 0)
    {
      Order.remove(0, 9);
      // SD_file_download(SPIFFS, Order);
      SD_file_download(SD, Order);
      Serial.println(Order);
    }
  }
}
void tipo_peixe()
{
  String message = "";

  if (server.arg("tipo") == "")
  { // Parameter not found

    message = "Argument not found";
  }
  else
  { // Parameter found

    message += server.arg("tipo"); // Gets the value of the query parameter
    if (nome_peixe == "sem_nome")
    {
      nome_peixe = message;
    }
    else
    {
      server.send(400, "text / plain", "Erro"); // Returns the HTTP response
    }
  }

  server.send(200, "text / plain", "OK"); // Returns the HTTP response
}
void definicao()
{
  String message = "";

  if (server.arg("valor") == "")
  { // Parameter not found

    message = "Argument not found";
  }
  else
  { // Parameter found

    message += server.arg("valor"); // Gets the value of the query parameter
    if (!message.toInt())
    {
      server.send(400, "text / plain", "Erro"); // Returns the HTTP response
    }
    if (hora_grau == 0 && message.toInt() > 0)
    {
      hora_grau = message.toInt();
      minuto_grau = 60 * hora_grau;
    }
    else
    {
      server.send(400, "text / plain", "Erro"); // Returns the HTTP response
    }
  }

  server.send(200, "text / plain", "OK"); // Returns the HTTP response
}
void getporcentagem()
{
  server.send_P(200, "text/plain", String(porcentagem).c_str());
}
void getFinalizado()
{
  server.send_P(200, "text/plain", finalizado.c_str());
}
void getFaltaQuanto()
{
  server.send_P(200, "text/plain", String(tempo_restante_horas).c_str());
}
void gethoragrau()
{
  server.send_P(200, "text/plain", String(hora_grau).c_str());
}
void getespecie()
{
  server.send_P(200, "text/plain", nome_peixe.c_str());
}
void getstart()
{
  server.send_P(200, "text/plain", start.c_str());
}
void getTemp()
{
  server.send_P(200, "text/plain", readDSTemperatureC().c_str());
}
void jquery()
{
  // loadFromSD(SD, "/", "text/js");
  loadFromSPIFFS("/jquery-3.7.1.min.js", "text/js");
}
void iniciar_monitoramento()
{
  if (hora_grau == 0 || nome_peixe == "sem_nome")
  {
    server.send(400, "text / plain", "Erro"); // Returns the HTTP response
  }
  else
  {
    start = "true";
    digitalWrite(LED_MONITORAMENTO_ON, HIGH);
    server.send(200, "text / plain", "OK"); // Returns the HTTP response
  }
}
void parar()
{
  start = "false";
  digitalWrite(LED_MONITORAMENTO_ON, LOW);
  
  server.send(200, "text / plain", "OK"); // Returns the HTTP response
}
// subir o server
void init_Server()
{

  server.on("/", HTTP_GET, handleRoot);
  server.on("/temp", HTTP_GET, temperatura_agua);
  server.on("/tipopeixe", HTTP_GET, tipo_peixe);
  server.on("/definicao_hora_grau", HTTP_GET, definicao);
  server.on("/porcentagem", HTTP_GET, getporcentagem);
  server.on("/faltaquanto", HTTP_GET, getFaltaQuanto);
  server.on("/temperatura_atual", HTTP_GET, getTemp);
  server.on("/dados", HTTP_GET, mandardados);
  server.on("/jquery-3.7.1.min.js", HTTP_GET, jquery);
  server.on("/hora_grau", HTTP_GET, gethoragrau);
  server.on("/especie", HTTP_GET, getespecie);
  server.on("/start", HTTP_GET, getstart);
  server.on("/stop", HTTP_GET, parar);
  server.on("/iniciar", HTTP_GET, iniciar_monitoramento);
  server.on("/terminou", HTTP_GET, getFinalizado);
  server.on("/", HTTP_POST, SD_dir);

  server.onNotFound(handleNotFound);
  // Inicia o servidor
  server.begin();
  Serial.println("Server started");
  // digitalWrite(LED_ESP32_ON, HIGH);
}