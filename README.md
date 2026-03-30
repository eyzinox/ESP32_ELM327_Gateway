# ESP32 ELM327 Gateway

A Bluetooth Low Energy (BLE) to CAN bus gateway running on ESP32, emulating an ELM327 OBD-II adapter. This project allows you to read vehicle diagnostic data via BLE/Bluetooth from your smartphone.

## Features

- **BLE Connectivity**: Connect from any Android or iOS device via Bluetooth
- **ELM327 Emulation**: Compatible with standard OBD-II diagnostic apps
- **CAN Bus Interface**: Direct communication with vehicle CAN networks at 500 kbps
- **AT Command Support**: Full AT command set compatible with ELM327 devices
- **OBD-II Mode 01**: Read and query vehicle parameters (PIDs)
- **Fragmented Response Handling**: Automatic message fragmentation for Android compatibility

## Hardware Requirements

- **ESP32 Development Board**
- **MCP2515 CAN Controller** with SPI interface
- **8 MHz Crystal Oscillator** (for MCP2515)
- **Vehicle OBD-II Connector** or CAN network

### Pinout

| Component | ESP32 Pin |
|-----------|-----------|
| MCP2515 CS | GPIO 5 |
| SPI CLK | GPIO 18 |
| SPI MOSI | GPIO 23 |
| SPI MISO | GPIO 19 |

## Software Setup

### Prerequisites

- PlatformIO (or Arduino IDE with required libraries)
- Libraries:
  - NimBLE or BLE (built-in)
  - MCP2515 CAN controller library
  - Arduino framework for ESP32

### Installation

1. Clone this repository
2. Open in PlatformIO or Arduino IDE
3. Install required dependencies
4. Select ESP32 as target board
5. Build and upload to your ESP32

```bash
pio run --target upload
```

## Usage

### Connecting from Mobile Device

1. Power up the ESP32
2. Open your OBD-II diagnostic app
3. Scan for Bluetooth device named **"OBD_Logger"**
4. Connect to the device
5. Start reading vehicle diagnostics

### Supported Commands

#### AT Commands (ELM327 compatible)
- `ATZ` - Reset device
- `ATI` - Device identity
- `ATE0` - Echo off
- `ATH1` - Headers on
- `ATRV` - Read battery voltage
- `ATDP` - Describe protocol
- Additional AT commands as per ELM327 specification

#### OBD-II Commands (Mode 01)
- `0100` - Show supported PIDs
- `010C` - Engine RPM
- `010D` - Vehicle speed
- And more standard OBD-II PIDs

## Architecture

### Library Structure

```
lib/
├── com_AT/          # AT command processing
│   ├── com_AT.h
│   └── com_AT.cpp
└── com_OBD/         # OBD-II protocol handling
    ├── com_OBD.h
    └── com_OBD.cpp
```

### Communication Flow

```
Smartphone (BLE) 
    ↓↑
ESP32 (NimBLE)
    ↓↑
AT/OBD Processors
    ↓↑
MCP2515 (CAN)
    ↓↑
Vehicle CAN Bus
```

## Known Limitations

- Android has a 20-byte MTU limit for BLE notifications; responses are automatically fragmented
- CAN speed fixed at 500 kbps (configurable)
- Implements subset of ELM327 command set
- Single device connection at a time

## Future Enhancements

- [ ] Support for multiple OBD-II modes (03/04/09/22)
- [ ] Desktop app dashboard for monitoring

## Contributing

Contributions are welcome! Please submit pull requests with improvements or bug fixes.

## License

MIT License - See LICENSE file for details

## References

- [ELM327 Command Set](https://www.elmelectronics.com/)
- [OBD-II Protocol](https://en.wikipedia.org/wiki/On-board_diagnostics)
- [CAN Bus Standard](https://en.wikipedia.org/wiki/CAN_bus)
- [NimBLE Library](https://github.com/h2zero/NimBLE-Arduino)

## Author

eyzinox (Mathieu)
Created for vehicle diagnostics and telemetry projects.

---

**⚠️ Disclaimer**: This project is for educational and diagnostic purposes. Always ensure compliance with vehicle regulations when modifying or accessing vehicle systems.
