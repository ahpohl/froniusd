# Froniusd

Although Fronius provides a nice web application in the cloud and a mobile application which shows graphs and a history of the produced energy, I still would like to fully own my data and be able to store the data without dependence on a third party service. Hence the motivation to utilize the Modbus data export function of my Fronius inverter to store the data locally in TimescaleDB and Grafana for visualisation.

The Froniud daemon connects to the inverter either with Modbus TCP (network) or Modbus RTU (serial cable) and forwards the data as a formatted JSON string to a MQTT broker. My personal software stack consists of the Mosquitto MQTT broker, which in turn is connected to PostgreSQL with TimescaleDB extension via a NodeRED flow. The complete setup is described in detail at the [Smartmeter](https://github.com/ahpohl/smartmeter/wiki) wiki, but all the necessay files are provided here in the resources section.

## Build instructions

Froniusd depends on [libsunspec](https://github.com/ahpohl/libsunspec), a library which provides the necessary low level methods to access the inverter. Packages for [froniusd](https://aur.archlinux.org/packages/froniusd) and [libsunspec](https://aur.archlinux.org/packages/libsunspec) are published in the AUR. If you need to compile Froniusd directly from source, a Makefile for (cross-)compilation is provided:

```
make CROSS_COMPILE=aarch64-unknown-linux-gnu
make install
```

## Configuration

The Modbus data export needs to be enabled in the web interface. Here you have to choose between tcp and rtu mode. In case of rtu, a serial RS485-to-USB converter is required to connect the inverter to the host running Froniusd.

![Screenshot of the Fronius web interface modbus configuration section](resources/images/fronius_primo_modbus_tcp.png)

Froniusd is configured through a config file. Here you need to set the IP address of the inverter or the serial device, the broker and the topic, and the payment for each produced kWh at a minimum. A fully commented example is provided in the resources section. 

```
modbus_tcp 192.168.6.53
# modbus_rtu /dev/ttyUSB0
mqtt_topic solarmeter
mqtt_broker localhost
payment_kwh 0.3914
```

The daemon can be started with:

```
$ froniusd --config froniusd_example.conf

```

## Json output

Example JSON string published by Froniusd in the `solarmeter/live` topic:

```json
{"time":1729597414501,"ac_energy":6813.517,"ac_current":3.330,"ac_voltage":234.400,"ac_power_w":781.000,"ac_power_va":781.016,"ac_power_var":-7.000,"ac_pf":99.996,"ac_freq":50.010,"ac_eff":94.565,"dc_voltage_1":292.900,"dc_current_1":1.360,"dc_power_1":399.300,"dc_energy_1":3349.270,"dc_voltage_2":294.200,"dc_current_2":1.440,"dc_power_2":426.590,"dc_energy_2":3398.550,"payment":0.3914}

```

## Docker

There is a docker-compose provided, which lets you build and run froniusd completely inside a docker container.

## Visualization

Grafana is used show the current and historic energy production on a dashborad, which is provided in the resource section. The energy trend calculation requires a model of the yearly energy production specific to the actual photovoltaic system. The model folder in the resource section contains a spreadsheet with raw data, a gnuplot fit and the final sql file with my current model.

![Screenshot of the Grafana dashboard showing production data](resources/images/grafana_dashboard.png)
