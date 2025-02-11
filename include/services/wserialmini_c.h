#ifndef __WSERIALmini_H
#define __WSERIALmini_H

#include <Arduino.h>

#define BAUD_RATE 115200UL
class WSerialmini_c
{
  typedef void (*CallbackFunction)(String str);
protected: 
  //uint16_t server_port = 0;
  void update(void);
  void start(unsigned long baudrate);
  CallbackFunction on_input = NULL;

public:
  WSerialmini_c() {};
  void onInput(CallbackFunction f);
  template <typename T>
  void plot(const char *varName, uint32_t x, T y, const char *unit = NULL);
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

  
  friend inline void startWSerialmini(WSerialmini_c *ws, unsigned long baudrate);
  friend inline void updateWSerialmini(WSerialmini_c *ws); 
};

inline void startWSerialmini(WSerialmini_c *ws, unsigned long baudrate = BAUD_RATE){ws->start(baudrate);}
void WSerialmini_c::start(unsigned long baudrate = BAUD_RATE)
{
  Serial.begin(baudrate);
}

void WSerialmini_c::onInput(CallbackFunction f) {
  on_input = f;
}  

inline void updateWSerialmini(WSerialmini_c *ws) {ws->update();}
void WSerialmini_c::update(void)
{
  if (Serial.available() && on_input != NULL)
  {
    on_input(Serial.readStringUntil('\n'));
  }
}

template <typename T>
void WSerialmini_c::plot(const char *varName, T y, const char *unit)
{
  plot(varName, millis(), y, unit);
}
template <typename T>
void WSerialmini_c::plot(const char *varName, uint32_t x, T y, const char *unit)
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
void WSerialmini_c::print(const T &data)
{
    Serial.print(data);  
}

template <typename T>
void WSerialmini_c::print(const T &data, int base)
{
    Serial.print(data, base);  
}

template <typename T>
void WSerialmini_c::println(const T &data)
{
    Serial.println(data);  
}

template <typename T>
void WSerialmini_c::println(const T &data, int base)
{
    Serial.println(data, base);  
}

void WSerialmini_c::println()
{
    Serial.println();
}
#endif