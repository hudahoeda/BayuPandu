# BayuPandu: An Open Source Paragliding Flight Computer

BayuPandu is a cost-effective, FAI-compliant, and DIY-friendly integrated flight computer for paragliding. The primary focus is on creating a device that is not only functional but also affordable, maintainable, and replicable, with a total build cost under 1-2 million Rupiah, using components easily sourced in Indonesia.

## Key Features

*   **FAI-Compliant Design:** Built as a single, integrated instrument to meet FAI requirements for competition scoring.
*   **High-Precision Variometer:** Utilizes the MS5611 sensor for sensitive and accurate vertical speed measurement.
*   **GPS & IGC Logging:** Features a U-blox GPS module for tracking and logs flights in the IGC format to a microSD card.
*   **Sunlight-Readable Display:** Supports either a color transflective LCD for fast refresh rates or a low-power E-Paper display for maximum battery life.
*   **Powerful Core:** Based on the versatile and powerful ESP32 microcontroller.
*   **Open Source:** All firmware, hardware schematics, and case designs are open source to encourage community collaboration.
*   **3D-Printable Enclosure:** Comes with design files for a robust, 3D-printable case.

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