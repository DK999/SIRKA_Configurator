#include <iostream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <boost\asio.hpp>
#include <boost\system\system_error.hpp>
#include <boost/utility.hpp>
#include "RS232.h"
#include "crc16.hpp"
#include "TimeoutSerial.h"

using namespace std;
using namespace boost::system;
using namespace boost::asio;
using namespace boost;

class BMXSensor
{
public:
    BMXSensor();
    void OpenPortBoost();
    void ClosePortBoost();
    void OpenPortTO();
    void ClosePortTO();
    void lookup_addresses();
    void change_address();
    void flash_device();
    uint16_t found_devices();
    uint8_t* found_addresses();
private:
    uint8_t m_buffer[5];
    uint16_t m_found_devices;
    uint16_t m_crc_int;
    uint16_t m_crc_package;
    uint8_t  m_found_addresses[0xFFFF];
    uint8_t m_command[14];
    uint8_t m_choose_flash[5];
    uint8_t m_send_pack_size[6];

    CRC16 m_check;
    RS232::SerialConnection serial;
    io_service ioService;
    serial_port serialPort;
    TimeoutSerial serialto;
};
