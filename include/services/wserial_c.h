#ifndef __WSERIAL_H
#define __WSERIAL_H

#include <Arduino.h>

#define BAUD_RATE 115200UL
class WSerial_c
{
  typedef void (*CallbackFunction)(String str);
protected: 
  //uint16_t server_port = 0;
  void update(void);
  void start(unsigned long baudrate = BAUD_RATE);
  CallbackFunction on_connect = NULL;

public:
  WSerial_c() {};
  void onConnect(CallbackFunction f);
  template <typename T>
  void plot(const char *varName, T x, T y, const char *unit = NULL);
  template <typename T>
  void plot(const char *varName, T y, const char *unit = NULL);
  template <typename T>
  void print(const T &data);
  template <typename T>
  void print(const T &data, int base);
  void println();
  template <typename T>
  void println(const T &data);
  template <typename T>
  void println(const T &data, int base);

  
  friend inline void startWSerial(WSerial_c *ws, unsigned long baudrate = BAUD_RATE);
  friend inline void updateWSerial(WSerial_c *ws); 
};

inline void startWSerial(WSerial_c *ws, unsigned long baudrate){ws->start(baudrate);}
void WSerial_c::start(unsigned long baudrate)
{
  Serial.begin(baudrate);
}

void WSerial_c::onConnect(CallbackFunction f) {
  on_connect = f;
}  

inline void updateWSerial(WSerial_c *ws) {ws->update();}
void WSerial_c::update(void)
{
  if (Serial.available() && on_input != NULL)
  {
    on_input(Serial.readStringUntil('\n'));
  }
}

template <typename T>
void WSerial_c::plot(const char *varName, T y, const char *unit)
{
  plot(varName, (T)millis(), y, unit);
}
template <typename T>
void WSerial_c::plot(const char *varName, T x, T y, const char *unit)
{
  print(">"); // Inicio de envio de dados para um gráfico.
  print(varName);
  print(":");
  print(x);
  print(":");
  print(y);
  if (unit != NULL)
  {
    print("§"); // Unidade na sequência
    print(unit);
  }
  println("|g"); // Modo Grafico
}

template <typename T>
void WSerial_c::print(const T &data)
{
    Serial.print(data);  
}

template <typename T>
void WSerial_c::print(const T &data, int base)
{
    Serial.print(data, base);  
}

template <typename T>
void WSerial_c::println(const T &data)
{
    Serial.println(data);  
}

template <typename T>
void WSerial_c::println(const T &data, int base)
{
    Serial.println(data, base);  
}

void WSerial_c::println()
{
    Serial.println();
}
#endif