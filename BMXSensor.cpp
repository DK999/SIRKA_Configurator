#include "BMXSensor.hpp"

#define PREAMBLE_ONE 0
#define PREAMBLE_TWO 1
#define LENGTH 2
#define ADDRESS 3
#define COMMAND 4
#define PARAMETER 5
#define CRCL 6
#define CRCH 7

BMXSensor::BMXSensor():
    m_found_devices(0),m_crc_int(0),serialPort(ioService)
{};

uint16_t BMXSensor::found_devices()
{
    return m_found_devices;
}

uint8_t* BMXSensor::found_addresses()
{
    return m_found_addresses;
}


void BMXSensor::OpenPortBoost()
{
    serialPort.open("\\\\.\\COM2");

    serialPort.set_option(serial_port::baud_rate(1000000));
    serialPort.set_option(serial_port::parity(serial_port::parity::none));
    serialPort.set_option(serial_port::character_size(serial_port::character_size(8)));
    serialPort.set_option(serial_port::stop_bits(serial_port::stop_bits::one));
    serialPort.set_option(serial_port::flow_control(serial_port::flow_control::none));
}

void BMXSensor::ClosePortBoost()
{
    serialPort.close();
}

//void BMXSensor::OpenPortTO()
//{
//    serialto.("\\\\.\\COM2",1000000);
//    serialto.setTimeout(posix_time::seconds(50));
//}
//
//void BMXSensor::ClosePortTO()
//{
//    serialto.close();
//}

void BMXSensor::flash_device()
{   OpenPortBoost();
    uint32_t  end_of_file = 0;
    uint32_t oversize = 0;
    uint32_t  current_position = 0;
    uint16_t nr_of_packages = 0;
    int nr=1;
    char *file_buffer;
    ifstream file;
    file.open("SIRKA_EFM32TG110_DRA.bin",ios::in | ios::binary | ios::ate);
    if(file.is_open())
    {
        end_of_file = file.tellg();
        file.seekg(0,ios::beg);
        file_buffer = new char [end_of_file];
        file.read(file_buffer,end_of_file);
        m_crc_package = 0;
        for(int i = 0; i < end_of_file; i++)
        {
            m_crc_package = m_check.CreateCRC(m_crc_package,file_buffer[i]);
        }
        m_choose_flash[0]=0xAA;
        m_choose_flash[1]=0xAA;
        m_choose_flash[2]=0x05;
        m_choose_flash[3]=0x7F;
        m_choose_flash[4]=0x00;

        serialPort.write_some(buffer(m_choose_flash));
        nr_of_packages = end_of_file / 8;
        if ( end_of_file % 8 != 0)
            nr_of_packages++;
        m_send_pack_size[0]=0xAA;
        m_send_pack_size[1]=0xAA;
        m_send_pack_size[2]=0x06;
        m_send_pack_size[3]=0x7F;
        m_send_pack_size[4]=(uint8_t)nr_of_packages;
        m_send_pack_size[5]=(uint8_t)(nr_of_packages>>8);

        serialPort.write_some(buffer(m_send_pack_size));
        serialPort.read_some(buffer(m_buffer));

        for ( int k = 0; k < nr_of_packages ; k++)
        {
            for( int i = 0; i < 8; i++)
            {   m_command[0]=0xAA;
                m_command[1]=0xAA;
                m_command[2]=0x0E;
                m_command[3]=0x7F;
                m_command[i+4]=file_buffer[current_position++];
                if ( current_position == end_of_file )
                {
                    for ( oversize = end_of_file % 8; oversize != 0 ; oversize--)
                    {
                        m_command[12-oversize]=0x01;
                    }
                    break;
                }
            }
            m_crc_int = 0;
            for(int crc_count = 0; crc_count < 12 ; crc_count++)
            {
                m_crc_int = m_check.CreateCRC(m_crc_int,m_command[crc_count]);
            }
            m_command[12] = (uint8_t)m_crc_int;
            m_command[13] = (uint8_t)(m_crc_int >> 8 );

//            serial.SendByteArray(2,m_command,sizeof(m_command));
            serialPort.write_some(buffer(m_command));
            //Sleep(100);
            serialPort.read_some(buffer(m_buffer));
//            while(!serial.ReadByteArray(2,m_buffer,sizeof(m_buffer)));
            if(m_buffer[4]==0x01)
                cout<<"Writing Package "<<nr++<<" of "<<nr_of_packages<<" succeded!"<<endl;
            else
            {
                cout<<"CRC FAIL"<<endl;
                current_position-=8;
                k--;
            }
            }
            m_command[0]=0xAA;
            m_command[1]=0xAA;
            m_command[2]=0x0E;
            m_command[3]=0x7F;
            m_command[4]=(uint8_t)m_crc_package;
            m_command[5]=(uint8_t)(m_crc_package >> 8 );
            m_command[6]=(uint8_t)(end_of_file);
            m_command[7]=(uint8_t)(end_of_file >> 8 );
            m_command[8]=(uint8_t)(end_of_file >> 16 );
            m_command[9]=(uint8_t)(end_of_file >> 24 );
            m_command[10]=0xFF;
            m_command[11]=0xFF;
            m_crc_int = 0;
            for(int crc_count = 0; crc_count < 12 ; crc_count++)
            {
                m_crc_int = m_check.CreateCRC(m_crc_int,m_command[crc_count]);
            }
            m_command[12] = (uint8_t)m_crc_int;
            m_command[13] = (uint8_t)(m_crc_int >> 8 );
            serialPort.write_some(buffer(m_command));
    }
    else
    {
        cout<<"No Firmware found!"<<endl;
    }
    file.close();
    ClosePortBoost();
}

void BMXSensor::change_address()
{   serial.OpenPort(2,1000000);
    uint8_t Change[8] = {0xAA,0xAA,0x08,0x01,0x13,0x01,0x00,0x00};
    int old,new_address;

    cout<<"Old Address:";
    cin>>old;
    cout<<"New Address:";
    cin>>new_address;
    Change[3] = (uint8_t)old;
    Change[5] = (uint8_t)new_address;
    m_crc_int = 0;
    for(int crc_count = 0; crc_count < 6 ; crc_count++)
    {
        m_crc_int = m_check.CreateCRC(m_crc_int,Change[crc_count]);
    }
    Change[CRCL] = (uint8_t)m_crc_int;
    Change[CRCH] = (uint8_t)(m_crc_int >> 8 );
    serial.SendByteArray(2,Change,sizeof(Change));
    serial.ClosePort(2);
}

void BMXSensor::lookup_addresses()
{   serial.OpenPort(2,1000000);
    m_command[PREAMBLE_ONE] = 0xAA;             // PREAMBLE
    m_command[PREAMBLE_TWO] = 0xAA;             // PREAMBLE
    m_command[ADDRESS] = 0x00;                  // set Address to 00
    m_command[LENGTH] = 8;                      // set FRAME LENGTH
    m_command[COMMAND] = 0x14;                  // Choose "send_hello" command
    m_command[PARAMETER] = 0x00;                // No Parameter needed
    int nr = 0;
    m_found_devices = 0;

         for ( int i = 0; i < 127; i++)          // m_check for 20 Sensors
        {
            m_crc_int = 0;
            for(int crc_count = 0; crc_count < 6 ; crc_count++)
            {
                m_crc_int = m_check.CreateCRC(m_crc_int,m_command[crc_count]);
            }
            m_command[CRCL] = (uint8_t)m_crc_int;
            m_command[CRCH] = (uint8_t)(m_crc_int >> 8 );

             serial.SendByteArray(2,m_command,sizeof(m_command));    // Send Hello Byte
             if(serial.ReadByteArray(2,m_buffer,sizeof(m_buffer)))   // Read if answer available
                {   /* If Device responds, increase number of devices and note address */
                    m_found_devices++;                            // If answer, increase nr of devices
                    m_found_addresses[nr++] = (int)m_command[ADDRESS];           // save address to devicenumber
                }
                m_command[ADDRESS]++;                                       // increase address
        }
    serial.ClosePort(2);
}
