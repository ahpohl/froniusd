#include <iostream>
#include <iomanip>
#include <cstring>
#include <charconv>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <vector>
#include <FroniusInverter.h>
#include "Solarmeter.h"

const std::set<std::string> Solarmeter::ValidKeys {"log_level", "mqtt_broker", "mqtt_password",
	"mqtt_port", "mqtt_topic", "mqtt_user", "mqtt_tls_cafile", "mqtt_tls_capath", "payment_kwh",
	"serial_device"};

Solarmeter::Solarmeter(void) : Log(0)
{
	Inverter = new FroniusInverter();
	Mqtt = new SolarmeterMqtt();
	Cfg = new SolarmeterConfig();
}

Solarmeter::~Solarmeter(void)
{
	if (Mqtt->GetConnectStatus()) {
		Mqtt->PublishMessage("offline", Cfg->GetValue("mqtt_topic") + "/status", 1, true);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	if (Mqtt) { delete Mqtt; }
	if (Inverter) { delete Inverter; }
	if (Cfg) { delete Cfg; };
}

bool Solarmeter::Setup(const std::string &config)
{
	if (!Cfg->Begin(config)) {
		ErrorMessage = Cfg->GetErrorMessage();
		return false;
	}
	if (!Cfg->ValidateKeys(Solarmeter::ValidKeys)) {
		ErrorMessage = Cfg->GetErrorMessage();
		return false;
	}
	this->SetLogLevel();
	if (Log & static_cast<unsigned char>(LogLevel::MODBUS)) {
		Inverter->SetModbusDebug(true);
	}
	Mqtt->SetLogLevel(Log);
	if (Log & static_cast<unsigned char>(LogLevel::CONFIG)) {
		Cfg->ShowConfig();
	}
	if (!(Cfg->KeyExists("payment_kwh"))) {
		ErrorMessage = Cfg->GetErrorMessage();
		return false;
	}
	if (!(Cfg->KeyExists("serial_device"))) {
		ErrorMessage = Cfg->GetErrorMessage();
		return false;
	}
	if (!Inverter->ConnectModbusRtu(Cfg->GetValue("serial_device"))) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetSiteEnergyTotal(InvData.AcEnergy)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	std::cout << "Current meter reading: " << InvData.AcEnergy << " kWh" << std::endl;

	if (!Mqtt->Begin()) {
		ErrorMessage = Mqtt->GetErrorMessage();
		return false;
	}
	if (!(Cfg->KeyExists("mqtt_topic"))) {
		ErrorMessage = Cfg->GetErrorMessage();
		return false;
	}
	if ((Cfg->KeyExists("mqtt_user") && Cfg->KeyExists("mqtt_password"))) {
		if (!Mqtt->SetUserPassAuth(Cfg->GetValue("mqtt_user"), Cfg->GetValue("mqtt_password"))) {
			ErrorMessage = Mqtt->GetErrorMessage();
			return false;
		}
	}
	if (Cfg->KeyExists("mqtt_tls_cafile") || Cfg->KeyExists("mqtt_tls_capath")) {
		if (!Mqtt->SetTlsConnection(Cfg->GetValue("mqtt_tls_cafile"), Cfg->GetValue("mqtt_tls_capath"))) {
			ErrorMessage = Mqtt->GetErrorMessage();
			return false;
		}
	}
	if (!(Cfg->KeyExists("mqtt_broker")) || !(Cfg->KeyExists("mqtt_port")) ) {
		ErrorMessage = Cfg->GetErrorMessage();
		return false;
	}
	if (!Mqtt->SetLastWillTestament("offline", Cfg->GetValue("mqtt_topic") + "/status", 1, true)) {
		ErrorMessage = Mqtt->GetErrorMessage();
		return false;
	}
	if (!Mqtt->Connect(Cfg->GetValue("mqtt_broker"), StringTo<double>(Cfg->GetValue("mqtt_port")), 60)) {
		ErrorMessage = Mqtt->GetErrorMessage();
		return false;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	return true;
}

bool Solarmeter::Receive(void)
{
	if (!Inverter->GetAcEnergyLifetime(InvData.AcEnergy)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetAcCurrent(InvData.AcCurrent)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetAcVoltage(InvData.AcVoltage)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetAcPower(InvData.AcPowerW)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetAcPowerApparent(InvData.AcPowerVa)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetAcPowerReactive(InvData.AcPowerVar)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetAcPowerFactor(InvData.AcPf)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetAcFrequency(InvData.AcFreq)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetDcVoltage(InvData.DcVoltage1, 1)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetDcCurrent(InvData.DcCurrent1, 1)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetDcPower(InvData.DcPower1, 1)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetDcEnergyLifetime(InvData.DcEnergy1, 1)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetDcVoltage(InvData.DcVoltage2, 2)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetDcCurrent(InvData.DcCurrent2, 2)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetDcPower(InvData.DcPower2, 2)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	if (!Inverter->GetDcEnergyLifetime(InvData.DcEnergy2, 2)) {
		ErrorMessage = Inverter->GetErrorMessage();
		return false;
	}
	try {
		float denominator = InvData.DcPower1 + InvData.DcPower2;
		if (denominator == 0) {
			throw std::runtime_error("Math error: Attempted to divide by Zero");
		}
		InvData.Efficiency = InvData.AcPowerW / denominator * 100.0f;
	}
	catch (std::runtime_error& e) {
		ErrorMessage = e.what();
		return false;
	}
	return true;
}

bool Solarmeter::Publish(void)
{
	unsigned long long now = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	std::ios::fmtflags old_settings = Payload.flags();
	Payload.str(std::string());
	Payload.setf(std::ios::fixed, std::ios::floatfield);
	Payload << std::setprecision(2);

	Payload << "{"
			<< "\"time\":"         << now                << ","
			<< "\"ac_energy\":"    << InvData.AcEnergy   << ","
			<< "\"ac_current\":"   << InvData.AcCurrent  << ","
			<< "\"ac_voltage\":"   << InvData.AcVoltage  << ","
			<< "\"ac_power_w\":"   << InvData.AcPowerW   << ","
			<< "\"ac_power_va\":"  << InvData.AcPowerVa  << ","
			<< "\"ac_power_var\":" << InvData.AcPowerVar << ","
			<< "\"ac_pf\":"        << InvData.AcPf       << ","
			<< "\"ac_freq\":"      << InvData.AcFreq     << ","
			<< "\"dc_voltage_1\":" << InvData.DcVoltage1 << ","
			<< "\"dc_current_1\":" << InvData.DcCurrent1 << ","
			<< "\"dc_power_1\":"   << InvData.DcPower1   << ","
			<< "\"dc_energy_1\":"  << InvData.DcEnergy1  << ","
			<< "\"dc_voltage_2\":" << InvData.DcVoltage2 << ","
			<< "\"dc_current_2\":" << InvData.DcCurrent2 << ","
			<< "\"dc_power_2\":"   << InvData.DcPower2   << ","
			<< "\"dc_energy_2\":"  << InvData.DcEnergy2  << ","
			<< "\"efficiency\":"   << InvData.Efficiency << ","
			<< "\"payment\":"      << Cfg->GetValue("payment_kwh")
			<< "}";

	/*
  std::ostringstream oss;
  oss << "[{"
    << "\"global_state\":\"" << State.GlobalState << "\"" << ","
    << "\"inverter_state\":\"" << State.InverterState << "\"" << ","
    << "\"ch1_state\":\"" << State.Channel1State << "\"" << ","
    << "\"ch2_state\":\"" << State.Channel2State << "\"" << ","
    << "\"alarm_state\":\"" << State.AlarmState << "\"" << "}]";
	 */

	if (Mqtt->GetNotifyOnlineFlag()) {
		std::cout << "Solarmeter is online." << std::endl;
		if (!Mqtt->PublishMessage("online", Cfg->GetValue("mqtt_topic") + "/status", 1, true)) {
			ErrorMessage = Mqtt->GetErrorMessage();
			return false;
		}
	}

	/*
  static ABBAurora::State previous_state;
  if ( Mqtt->GetNotifyOnlineFlag() || 
       (!((previous_state.GlobalState == State.GlobalState) &&
       (previous_state.InverterState == State.InverterState) &&
       (previous_state.Channel1State == State.Channel1State) &&
       (previous_state.Channel2State == State.Channel2State) &&
       (previous_state.AlarmState == State.AlarmState))) )
  {
    if (!(Mqtt->PublishMessage(oss.str(), Cfg->GetValue("mqtt_topic") + "/state", 0, false)))
    {
      ErrorMessage = Mqtt->GetErrorMessage();
      return false;
    }
    previous_state = State;
  }
	 */
	if (Mqtt->GetConnectStatus())
	{
		if (!(Mqtt->PublishMessage(Payload.str(), Cfg->GetValue("mqtt_topic") + "/live", 0, false)))
		{
			ErrorMessage = Mqtt->GetErrorMessage();
			return false;
		}
	}
	Mqtt->SetNotifyOnlineFlag(false);

	if (Log & static_cast<unsigned char>(LogLevel::JSON)) {
		std::cout << Payload.str() << std::endl;
		//std::cout << oss.str() << std::endl;
	}
	Payload.flags(old_settings);

	return true;
}

std::string Solarmeter::GetErrorMessage(void) const
{
	return ErrorMessage;
}

void Solarmeter::SetLogLevel(void)
{
	if (Cfg->KeyExists("log_level")) {
		std::string line = Cfg->GetValue("log_level");
		std::istringstream iss(line);
		std::string token;
		std::vector<std::string> log_level;

		while(std::getline(iss, token, ',')) {
			log_level.push_back(token);
		}
		for (auto it = log_level.cbegin(); it != log_level.cend(); ++it) {
			if (!(*it).compare("config")) {
				Log |= static_cast<unsigned char>(LogLevel::CONFIG);
			} else if (!(*it).compare("json")) {
				Log |= static_cast<unsigned char>(LogLevel::JSON);
			} else if (!(*it).compare("mosquitto")) {
				Log |= static_cast<unsigned char>(LogLevel::MQTT);
			} else if (!(*it).compare("modbus")) {
				Log |= static_cast<unsigned char>(LogLevel::MODBUS);
			}
		}
	} else {
		Log = 0;
	}
}

unsigned char Solarmeter::GetLogLevel(void) const
{
	return Log;
}

template <typename T>
T Solarmeter::StringTo(const std::string &str) const
{
	T value;
	std::istringstream iss(str);
	iss >> value;
	if (iss.fail()) {
		return T();
	}
	return value;
}
