<p align="center">
    <strong>
        <span style="font-size:2em;">BayuPandu: An Open Source Paragliding Flight Computer</span>
    </strong>
</p>

BayuPandu is a cost-effective, FAI-compliant, and DIY-friendly integrated flight computer for paragliding. This project represents a complete ground-up engineering solution using readily available components, ensuring long-term maintainability while avoiding reliance on discontinued consumer electronics. The primary focus is on creating a device that is not only functional but also affordable, maintainable, and replicable, with a total build cost under 1-2 million Rupiah, using components easily sourced in Indonesia.

## Project Philosophy

This project embraces the integrated instrument architecture, combining the screen, processor, sensors, and power supply into a single, robust 3D-printed enclosure. This self-contained approach is essential for meeting FAI requirements where all components of a flight recorder must be housed in a single unit for official competition scoring. By focusing on purpose-built design rather than modifying existing devices, BayuPandu delivers a flight computer tailored specifically to paraglider pilot needs.

## Key Features

### Core Functionality
*   **FAI-Compliant Design:** Built as a single, integrated instrument to meet FAI requirements for competition scoring
*   **High-Precision Variometer:** Utilizes the MS5611 sensor capable of detecting altitude changes as small as 10cm for sensitive thermal detection
*   **Advanced Data Filtering:** Implements Kalman filter algorithms to provide clean, responsive vertical speed readings by fusing barometric and accelerometer data
*   **GPS & IGC Logging:** Features a U-blox GPS module for tracking and logs flights in the standardized IGC format to microSD card

### Display Technology
*   **Sunlight-Readable Display Options:** 
    *   **Transflective LCD:** Color display with fast refresh rates, uses ambient light for excellent sunlight readability
    *   **E-Paper Alternative:** Ultra-low power consumption with superior readability that increases in bright conditions
*   **Intelligent UI:** Displays critical flight data including graphical variometer, altitude (barometric and GPS), ground speed, glide ratio, and battery status

### Hardware Platform
*   **Powerful ESP32 Core:** Dual-core 240MHz processor with integrated Wi-Fi and Bluetooth
*   **Comprehensive Sensor Suite:** 
    *   MS5611 barometric pressure sensor for high-precision altitude measurement
    *   U-blox NEO-6M GPS module with external antenna for reliable positioning
    *   MPU-9250 IMU for TE compensation, G-force monitoring, and artificial horizon capability
*   **Reliable Power System:** 18650 Li-ion battery with TP4056 charging module and protection circuit

### Build Quality & Accessibility
*   **3D-Printable Enclosure:** Custom-designed case optimized for flight conditions with proper barometer venting
*   **Indonesian Market Optimized:** All components readily available from local marketplaces like Tokopedia and Shopee
*   **Open Source Everything:** Firmware, hardware schematics, and case designs are fully open source

## Project Structure

The repository is organized into the following main directories:

*   `./firmware/`: Contains all the PlatformIO source code for the ESP32 microcontroller.
*   `./hardware/`: Contains hardware design files, such as schematics and PCB layouts.
*   `./case/`: Contains all 3D-printable files (STL and source CAD) for the enclosure.
*   `./BOM.md`: A detailed Bill of Materials with sourcing guides for Indonesian marketplaces.

## Getting Started

The first step is to set up your ESP32 Development Environment and upload the initial firmware.

## Bill of Materials

All components required for this project are listed in the [Bill of Materials (BOM.md)](BOM.md) file. The list is optimized for sourcing from local Indonesian marketplaces like Tokopedia and Shopee.

## Contributing

Contributions are welcome! Whether it's improving the firmware, refining the hardware design, or enhancing the documentation, your input is valuable. Please feel free to open an issue or submit a pull request.

## License

The project is released under a mix of open-source licenses.
*   **Firmware:** MIT License
*   **Hardware & Case Design:** Creative Commons Attribution-ShareAlike (CC BY-SA)

Please see the [LICENSE](LICENSE) file for more details.

## Getting Started

### Quick Build Overview
1. **Source Components:** All parts available through Tokopedia/Shopee (see BOM.md)
2. **Assembly:** Solder components to perfboard following provided schematic
3. **3D Print:** Case files optimized for PETG/ASA/ABS materials
4. **Program:** Upload firmware using Arduino IDE
5. **Calibrate:** Test sensors and perform initial flight validation

### Prerequisites
*   Basic soldering skills and electronics tools
*   3D printer access
*   Arduino IDE setup
*   Total budget: Under Rp 1-2 million

## Indonesian Sourcing & Cost

**Total Budget:** Under Rp 1-2 million
**Primary Sources:** Tokopedia and Shopee with local shipping

**Component Availability:**
*   **Readily Available (>95%):** ESP32, GPS modules, charging circuits, batteries, SD card modules
*   **Generally Available (70-95%):** MS5611 sensors, IMU modules
*   **Specialized (50-70%):** Transflective displays, e-paper modules

**Cost Breakdown:**
*   Core components: ~Rp 400,000-600,000
*   Display options: ~Rp 300,000-500,000  
*   Assembly materials: ~Rp 50,000-100,000

See [BOM.md](BOM.md) for detailed sourcing information and search terms in Bahasa Indonesia.

## Safety & Usage

**Important:** BayuPandu is designed as a secondary instrument. Always maintain primary backup instruments and never rely solely on DIY equipment for flight safety.

**Pre-Flight:** Verify GPS acquisition, variometer calibration, battery level, and IGC logging functionality.

**Regulatory:** Check local aviation regulations for electronic device usage and competition requirements for IGC compliance.

## Support & Contributing

**Documentation:** Comprehensive build guides, schematics, and troubleshooting in the repository

**Community:** Open-source development encourages contributions in hardware design, firmware development, documentation, and testing

**Contributions Welcome:**
*   Hardware improvements and PCB design
*   Firmware features and optimization  
*   Documentation and tutorials
*   Flight testing and validation

See issues and discussions on GitHub for current development priorities.

## Conclusion

BayuPandu represents more than just a DIY flight computer - it embodies a vision of accessible, transparent, and community-driven aviation technology. By combining modern electronics with proven aeronautical principles, this project delivers professional-grade functionality at a fraction of commercial costs.

The integrated design ensures FAI compliance for competition use, while the open-source approach guarantees long-term supportability and continuous improvement. With components readily available in the Indonesian market and comprehensive documentation in local context, BayuPandu makes advanced flight instrumentation accessible to a broader community of pilots.

Whether you're a recreational pilot seeking reliable flight data, a competitive pilot requiring IGC-compliant logging, or an electronics enthusiast interested in aviation applications, BayuPandu provides a solid foundation for exploration and innovation in paragliding technology.

The future of flight instrumentation lies in open collaboration, shared knowledge, and community-driven development. Join us in building safer, more capable, and more accessible tools for the paragliding community.

**Happy flying, and welcome to the BayuPandu community!**