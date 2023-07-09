#ifndef Solarmeter_h
#define Solarmeter_h
#include <FroniusInverter.h>
#include "SolarmeterMqtt.h"
#include "SolarmeterConfig.h"

class Solarmeter
{
  static const std::set<std::string> ValidKeys;

private:
  FroniusInverter *Inverter;
  SolarmeterMqtt *Mqtt;
  SolarmeterConfig *Cfg;
  //ABBAurora::State State;
  std::stringstream Payload;
  std::string Config;
  std::string ErrorMessage;
  unsigned char Log;

  template <typename T>
  T StringTo(const std::string &str) const;
  void SetLogLevel(void);
 
public:
  Solarmeter(void);
  ~Solarmeter(void);
  bool Setup(const std::string &config);
  bool Receive(void);
  bool Publish(void);
  std::string GetErrorMessage(void) const;
  unsigned char GetLogLevel(void) const;
  
  struct Datagram
  {
    double VoltageP1;          // Voltage pin 1 [V]
    double CurrentP1;          // Current pin 1 [A]
    double PowerP1;            // Power pin 1 [W]
    double VoltageP2;          // Voltage pin 2 [V]
    double CurrentP2;          // Current pin 2 [A]
    double PowerP2;            // Power pin 2 [W]
    double GridVoltage;        // Grid voltage [V]
    double GridCurrent;        // Grid current [A]
    double GridPower;          // Grid power [W]
    double Frequency;          // Grid frequency [Hz]
    double Efficiency;         // AC/DC conversion efficiency [%]
    double TotalEnergy;        // Lifetime total energy [kWh]
    double PaymentKwh;         // Payment per kWh
  } Datagram;

  enum class LogLevel : unsigned char
  {
    CONFIG = 0x01,
    JSON = 0x02,
    MQTT = 0x04,
    MODBUS = 0x08
  };
};

#endif
