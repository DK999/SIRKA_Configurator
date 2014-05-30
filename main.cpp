#include <iostream>
#include <cstdlib>
#include <boost\asio.hpp>
#include <boost\system\system_error.hpp>
#include "RS232.h"
#include "crc16.hpp"
#include "BMXSensor.hpp"
using namespace std;
using namespace boost::system;
using namespace boost::asio;

#define PREAMBLE_ONE 0
#define PREAMBLE_TWO 1
#define LENGTH 2
#define ADDRESS 3
#define COMMAND 4
#define PARAMETER 5
#define CRCL 6
#define CRCH 7

//RS232::SerialConnection serial;
BMXSensor Sirka;
CRC16 m_check;

uint8_t *addresses;
uint8_t buffer[30];
uint16_t found_devices = 0;
uint16_t m_crc_int = 0x00;
uint8_t  found_addresses[0xFFFF];
uint8_t m_command[8];

int main()
{
    char choice;
     while(1)
        {
            Sirka.lookup_addresses();
            cout << "Found Devices : " << Sirka.found_devices() <<endl;
            cout << "Available Addresses:" << endl;
            addresses = Sirka.found_addresses();
            for(int i = 0; i < Sirka.found_devices() ; i++)
                {
                    cout<<(int)addresses[i]<<" "<<endl;
                    if( i % 5 == 0)
                        cout<<endl;
                }

            cout<<"Change Address (C), Flash Firmware (F) or Quit (Q)?"<<endl;
            cin>>choice;

            if(choice == 'c')
                choice = 'C';
            if(choice == 'q')
                choice = 'Q';
            if(choice == 'f')
                choice = 'F';

            switch(choice)
            {
                case 'C':   Sirka.change_address();
                            break;

                case 'F':   Sirka.flash_device();
                            break;
                case 'Q':   return 0;

                default:
                            break;
            }
//            system("cls");
        }

//    }else
//    {
//        std::cout<<"Fehler bei COM PORT Initialisierung"<<std::endl;    // show failure
//        return 1;
//    }



    return 0;
}
