# E-ink Display Pinout

## 1.54" E-ink Display (GxEPD2_154_D67)

### Display Specifications
- **Model**: GDEH0154D67
- **Resolution**: 200x200 pixels
- **Colors**: Black & White (monochrome)
- **Controller**: SSD1681
- **Interface**: 4-wire SPI

### ESP32 Pin Configuration

| Display Pin | ESP32 GPIO | Function | Notes |
|------------|------------|----------|-------|
| **VCC** | 3.3V | Power | 3.3V only! Do NOT use 5V |
| **GND** | GND | Ground | Common ground |
| **SDA** | GPIO 23 | SPI Data (MOSI) | Serial data input |
| **SCL** | GPIO 18 | SPI Clock (SCK) | Serial clock |
| **CS** | GPIO 5 | Chip Select | Active low |
| **D/C** | GPIO 16 | Data/Command | Low=Command, High=Data |
| **RES** | GPIO 17 | Reset | Active low, with pull-up |
| **BUSY** | GPIO 4 | Busy Signal | High when display is updating |

### SPI Bus Configuration
- **SPI Bus**: VSPI (default ESP32 SPI)
- **SPI Mode**: Mode 0 (CPOL=0, CPHA=0)
- **SPI Speed**: Up to 4 MHz (default in GxEPD2)
- **Bit Order**: MSB First

### Pin Functions Explained

#### Power Pins
- **VCC (3.3V)**: Powers the display logic and e-ink panel
  - ⚠️ **CRITICAL**: E-ink displays require 3.3V. Using 5V will damage the display!
  - Current draw: ~50mA during refresh, <1mA sleep
  
- **GND**: Connect to ESP32 ground

#### SPI Communication Pins
- **SDA (MOSI)**: Serial data from ESP32 to display
  - Sends pixel data and commands
  - Connected to ESP32 hardware SPI MOSI (GPIO 23)
  - Despite the I2C-like name, this is SPI data!
  
- **SCL (SCK)**: Serial clock signal
  - Synchronizes data transmission
  - Connected to ESP32 hardware SPI SCK (GPIO 18)
  - Despite the I2C-like name, this is SPI clock!
  
- **CS (Chip Select)**: Selects the display on SPI bus
  - Active LOW (pulled low to enable display)
  - Allows multiple SPI devices on same bus

#### Control Pins
- **D/C (Data/Command)**: Distinguishes between data and commands
  - LOW = Command mode (sending instructions)
  - HIGH = Data mode (sending pixel data)
  
- **RES (Reset)**: Hardware reset pin
  - Active LOW (pulled low to reset display)
  - Has internal pull-up resistor
  - Used during initialization
  
- **BUSY**: Status output from display
  - HIGH = Display is busy (updating/refreshing)
  - LOW = Display is ready for new commands
  - Must wait for LOW before sending new data

### Wiring Diagram

```
ESP32 DevKit          1.54" E-ink Display
┌──────────┐          ┌──────────────┐
│          │          │              │
│      3.3V├──────────┤VCC           │
│      GND ├──────────┤GND           │
│  GPIO 23 ├──────────┤SDA           │
│  GPIO 18 ├──────────┤SCL           │
│   GPIO 5 ├──────────┤CS            │
│  GPIO 16 ├──────────┤D/C           │
│  GPIO 17 ├──────────┤RES           │
│   GPIO 4 ├──────────┤BUSY          │
│          │          │              │
└──────────┘          └──────────────┘

Note: SDA/SCL are SPI pins, not I2C!
      SDA = MOSI (Master Out Slave In)
      SCL = SCK (Serial Clock)
```

### Connection Notes

1. **Use Short Wires**: Keep SPI wires as short as possible (ideally <15cm) to minimize signal noise
2. **Common Ground**: Ensure ESP32 and display share a solid ground connection
3. **Power Quality**: Use decoupling capacitor (10µF) near display VCC if using long power wires
4. **Pin Conflicts**: These pins don't conflict with:
   - Built-in LED (GPIO 2)
   - UART0 (GPIO 1/3 for USB programming)
   - I2C default pins (GPIO 21/22)

### Compatible Displays

This pin configuration works with these WeAct Studio e-ink displays:

| Size | Resolution | Model | Color | Controller |
|------|------------|-------|-------|------------|
| 1.54" | 200×200 | GDEH0154D67 | B/W | SSD1681 |
| 2.13" | 122×250 | DEPG0213BN | B/W | SSD1680 |
| 2.13" | 122×250 | GDEY0213Z98 | B/W/R | SSD1680 |
| 2.9" | 128×296 | DEPG0290BS | B/W | SSD1680 |
| 3.7" | 240×416 | GDEY037T03 | B/W | UC8253 |
| 4.2" | 400×300 | GDEY042T81 | B/W | SSD1683 |

Simply change the display constructor in `app.ino` to match your display model.

### Troubleshooting

**Display doesn't respond:**
- Check VCC is 3.3V (NOT 5V!)
- Verify all connections are secure
- Check CS, DC, RST, BUSY pins are correct
- Ensure SPI wires are not too long

**Partial updates not working:**
- Some displays don't support partial refresh
- Check display datasheet for capabilities
- Try full refresh only (see code comments)

**Ghosting/artifacts:**
- Increase full refresh frequency
- Reduce `MAX_PARTIAL_UPDATES` in code
- Ensure proper GND connection

**Display stays blank:**
- Check BUSY pin connection
- Verify display ribbon cable is fully inserted
- Try longer reset pulse in `display.init()`

### Code Reference

The pin configuration is defined in `/mnt/c/dev/ideal-robot/src/app/app.ino`:

```cpp
// ESP32 Pin Configuration
#define EINK_CS    5   // Chip Select
#define EINK_DC    16  // Data/Command
#define EINK_RST   17  // Reset
#define EINK_BUSY  4   // Busy
// SPI: SCK=18, MOSI=23 (default ESP32 SPI pins)
```

To change pins, modify these defines and rebuild the project.

### Power Consumption

| State | Current | Duration |
|-------|---------|----------|
| Deep Sleep | <10 µA | Between updates |
| Idle | ~1 mA | Ready state |
| Partial Refresh | ~40 mA | 0.5-2 seconds |
| Full Refresh | ~50 mA | 2-5 seconds |

With 5-second update intervals and partial refresh, average current is ~5mA.

### Additional Resources

- [GxEPD2 Library Documentation](https://github.com/ZinggJM/GxEPD2)
- [WeAct Studio E-paper Module](https://github.com/WeActStudio/WeActStudio.EpaperModule)
- [ESP32 SPI Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html)
- [LVGL Display Driver Guide](https://docs.lvgl.io/master/details/main-modules/display/index.html)
