# Froniusd

The froniud daemon is built upon the [libsunspec](https://ahpohl.github.io/libsunspec/) library to communicate with the Fronius inverter. The daemon reads the energy production from the inverter, outputs a JSON formatted string and sends it to a MQTT broker on the network. From there, the data is forwarded into a time series database for permanent data storage and for visualization. The complete software stack consists of the following components, which need to be installed and configured separately:
- Froniusd with RS485 dongle for data readout
- Mosquitto MQTT broker
- Node-RED (MQTT client, PostgreSQL and optional email alerts)
- PostgreSQL with TimescaleDB and pg_cron extensions
- Grafana for visualization

The software stack is light weight in terms of necessary resources and runs on any SBC such as an Odroid C2/C4 or Raspberry Pi 3/4. Step-by-step [instructions](https://github.com/ahpohl/smartmeter/wiki) can be found at the [Smartmeter](https://github.com/ahpohl/smartmeter) sister project wiki pages. The Aurora converter needs to be connected to the host USB with a [RS485-to-USB adapter](https://www.waveshare.com/usb-to-rs232-485-ttl.htm) like the one from Waveshare. 

## Changelog

All notable changes and releases are documented in the [CHANGELOG](CHANGELOG.md).

## License

This project is licensed under the MIT license - see the [LICENSE](LICENSE) file for details
