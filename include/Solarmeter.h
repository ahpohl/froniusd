#ifndef Solarmeter_h
#define Solarmeter_h

#include "SolarmeterConfig.h"
#include "SolarmeterMqtt.h"
#include <FroniusInverter.h>

class Solarmeter {
  static const std::set<std::string> ValidKeys;

public:
  Solarmeter(void);
  ~Solarmeter(void);
  bool Setup(const std::string &config);
  bool Receive(void);
  bool Publish(void);
  std::string GetErrorMessage(void) const;
  unsigned char GetLogLevel(void) const;

private:
  FroniusInverter *Inverter;
  FroniusInverter::StateEvt_t StateEvt;
  FroniusInverter::StateCode_t StateCode;
  SolarmeterMqtt *Mqtt;
  SolarmeterConfig *Cfg;
  std::stringstream Payload;
  std::string Config;
  std::string ErrorMessage;
  unsigned char Log;

  template <typename T> T StringTo(const std::string &str) const;
  void SetLogLevel(void);

  struct InvData_t {
    double AcEnergy;   // AC lifetime energy [kWh]
    double AcCurrent;  // AC current [A]
    double AcVoltage;  // AC voltage [V]
    double AcPowerW;   // AC power [W]
    double AcPowerVa;  // AC apparent power [VA]
    double AcPowerVar; // AC reactive power [VAr]
    double AcPf;       // AC power factor
    double AcFreq;     // AC frequency [Hz]
    double AcEff;      // AC conversion efficiency [%]
    double DcVoltage1; // DC voltage string 1 [V]
    double DcCurrent1; // DC current string 1 [A]
    double DcPower1;   // DC power string 1 [W]
    double DcEnergy1;  // DC energy string 1 [kWh]
    double DcVoltage2; // DC voltage string 2 [V]
    double DcCurrent2; // DC current string 2 [A]
    double DcPower2;   // DC power string 2 [W]
    double DcEnergy2;  // DC energy string 2 [kWh]
    double PaymentKwh; // Payment per kWh
  } InvData;

  enum class LogLevel : unsigned char {
    CONFIG = 0x01,
    JSON = 0x02,
    MQTT = 0x04,
    MODBUS = 0x08
  };
};

#endif
