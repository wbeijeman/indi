#if 0
    INDI Ujari Driver - Encoder
    Copyright (C) 2014 Jasem Mutlaq
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#endif

#include <aiousb.h>

#include "encoder.h"
#include "ujari.h"

using namespace AIOUSB;

// 12bit encoder for BEI H25
#define MAX_ENCODER_COUNT   4096

#define ENCODER_GROUP "Encoders"

extern int DBG_SCOPE_STATUS;
extern int DBG_COMM;
extern int DBG_MOUNT;

Encoder::Encoder(encoderType type, Ujari* scope)
{
        // Initially, not connected
        connection_status = -1;

        setType(type);
        telescope = scope;

        debug = false;
        simulation = false;
        verbose    = true;

        startupEncoderValue = -1;

}

Encoder::~Encoder()
{
}

Encoder::encoderType Encoder::getType() const
{
    return type;
}

void Encoder::setType(const encoderType &value)
{
    type = value;

    switch (type)
    {
        case RA_ENCODER:
        type_name = std::string("RA Encoder");
        SLAVE_ADDRESS = 1;
        break;

        case DEC_ENCODER:
        type_name = std::string("DEC Encoder");
        SLAVE_ADDRESS = 2;
        break;

        case DOME_ENCODER:
        type_name = std::string("Dome Encoder");
        SLAVE_ADDRESS = 3;
        break;

    }

}
unsigned long Encoder::getEncoderValue()
{
    return static_cast<unsigned long>(encoderValueN[0].value);
}

void Encoder::setEncoderValue(unsigned long value)
{
    encoderValueN[0].value = value;
}

/****************************************************************
**
**
*****************************************************************/
bool Encoder::initProperties()
{
    IUFillNumber(&encoderSettingsN[EN_HOME_POSITION], "HOME_POSITION", "Home Position", "%g", 0, 10000000, 1000, 400000);
    IUFillNumber(&encoderSettingsN[EN_HOME_OFFSET], "HOME_OFFSET", "Home Offset", "%g", 0, 10000000, 1000, 0);
    IUFillNumber(&encoderSettingsN[EN_TOTAL], "TOTAL_COUNT", "Total", "%g", 0, 10000000, 1000, 800000);

    IUFillNumber(&encoderValueN[0], "ENCODER_RAW_VALUE", "Value", "%g", 0, 10000000, 1000, 400000);

    switch (type)
    {
        case RA_ENCODER:
         IUFillNumberVector(&encoderSettingsNP, encoderSettingsN, 3, telescope->getDeviceName(), "RA_SETTINGS", "RA Settings", ENCODER_GROUP, IP_RW, 0, IPS_IDLE);
         IUFillNumberVector(&encoderValueNP, encoderValueN, 1, telescope->getDeviceName(), "RA_VALUES", "RA", ENCODER_GROUP, IP_RO, 0, IPS_IDLE);
        break;

        case DEC_ENCODER:
        IUFillNumberVector(&encoderSettingsNP, encoderSettingsN, 3, telescope->getDeviceName(), "DEC_SETTINGS", "DEC Settings", ENCODER_GROUP, IP_RW, 0, IPS_IDLE);
        IUFillNumberVector(&encoderValueNP, encoderValueN, 1, telescope->getDeviceName(), "DEC_VALUES", "DEC", ENCODER_GROUP, IP_RO, 0, IPS_IDLE);
        break;

        case DOME_ENCODER:
        IUFillNumberVector(&encoderSettingsNP, encoderSettingsN, 3, telescope->getDeviceName(), "DOME_SETTINGS", "Dome Settings", ENCODER_GROUP, IP_RW, 0, IPS_IDLE);
        IUFillNumberVector(&encoderValueNP, encoderValueN, 1, telescope->getDeviceName(), "DOME_VALUES", "Dome", ENCODER_GROUP, IP_RO, 0, IPS_IDLE);
        break;

    }

    return true;

}

/****************************************************************
**
**
*****************************************************************/
bool Encoder::connect()
{
    if (simulation)
    {
        DEBUGFDEVICE(telescope->getDeviceName(),INDI::Logger::DBG_SESSION, "%s: Simulating connecting to DIO-48 USB.", type_name.c_str());
        encoderSettingsN[EN_HOME_POSITION].value = 400000;
        encoderSettingsN[EN_HOME_OFFSET].value = 0;
        encoderSettingsN[EN_TOTAL].value = 800000;
        encoderValueN[0].value = 400000;
        connection_status = 0;
        return true;
    }


    unsigned long result = AIOUSB_Init();
    if( result != AIOUSB_SUCCESS )
    {
        DEBUGFDEVICE(telescope->getDeviceName(),INDI::Logger::DBG_ERROR, "%s encoder: Can't initialize AIOUSB USB device.", type_name.c_str());
        return false;
    }

    //AIOUSB_ListDevices();

    // All input
    result = DIO_Configure(diOnly, AIOUSB_FALSE, 0, 0);

    if( result != AIOUSB_SUCCESS )
    {
        DEBUGFDEVICE(telescope->getDeviceName(),INDI::Logger::DBG_ERROR, "%s encoder: failed to configure all ports as input.", type_name.c_str());
        return false;
    }

    connection_status = 0;
    return true;
}

/****************************************************************
**
**
*****************************************************************/
void Encoder::disconnect()
{
    connection_status = -1;

    if (simulation)
        return;



}

/****************************************************************
**
**
*****************************************************************/
void Encoder::ISGetProperties()
{

}

/****************************************************************
**
**
*****************************************************************/
bool Encoder::updateProperties(bool connected)
{
    if (connected)
    {
        telescope->defineNumber(&encoderSettingsNP);
        telescope->defineNumber(&encoderValueNP);
    }
    else
    {
        telescope->deleteProperty(encoderSettingsNP.name);
        telescope->deleteProperty(encoderValueNP.name);
    }
}

/****************************************************************
**
**
*****************************************************************/
bool Encoder::ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n)
{
    if (!strcmp(encoderSettingsNP.name, name))
    {
        if (IUUpdateNumber(&encoderSettingsNP, values, names, n) < 0)
            encoderSettingsNP.s = IPS_ALERT;
        else
            encoderSettingsNP.s = IPS_OK;

        IDSetNumber(&encoderSettingsNP, NULL);

        return true;
    }

    return false;

}

/****************************************************************
**
**
*****************************************************************/
bool Encoder::ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n)
{

    return false;
}

/****************************************************************
**
**
*****************************************************************/
unsigned long Encoder::GetEncoder()  throw (UjariError)
{
    return static_cast<unsigned long>(encoderValueN[0].value);

}

/****************************************************************
**
**
*****************************************************************/
unsigned long Encoder::GetEncoderZero()
{
    return static_cast<unsigned long>(startupEncoderValue+encoderSettingsN[EN_HOME_POSITION].value+encoderSettingsN[EN_HOME_OFFSET].value);
}

/****************************************************************
**
**
*****************************************************************/
unsigned long Encoder::GetEncoderTotal()
{
    return static_cast<unsigned long>(encoderSettingsN[EN_TOTAL].value);

}

/****************************************************************
**
**
*****************************************************************/
unsigned long Encoder::GetEncoderHome()
{
    return static_cast<unsigned long>(encoderSettingsN[EN_HOME_POSITION].value);

}

/****************************************************************
**
**
*****************************************************************/
void Encoder::simulateEncoder(double speed, int dir)
{
    // Make fast speed faster
    if (speed > 0.5)
        speed *= 10;
    else if (speed > 0.1)
        speed *= 3;

    int deltaencoder = speed * (speed > 0.1 ? 400 : 200);
    if (dir == 0)
        deltaencoder *= -1;

    encoderValueN[0].value += deltaencoder;
}

/****************************************************************
**
**
*****************************************************************/
bool Encoder::update()
{
    static double lastEncoderValue=0;

    if (simulation == false)
    {
        DIOBuf *buf= NewDIOBuf(0);
        int LSB, MSB, encoderRAW;
        DIO_ReadAll( diOnly, buf );
        DEBUGFDEVICE(telescope->getDeviceName(), DBG_COMM, "%s enocder Binary was: %s", type_name.c_str(), DIOBufToString( buf ) );
        DEBUGFDEVICE(telescope->getDeviceName(), DBG_COMM, "%s enocder Hex was: %s", type_name.c_str(),  DIOBufToHex( buf ) );

        // RA Encoder bits 0-11
        // DE Encoder bits 12-23
        // Dome Encoder bits 24-35? or just 8 bits for dome encoder IIRC?

        // RA 12bit connected to DIO-48 ports 47 (I/O 0) to 25 (I/O 11)
        // DE 12bit connected to DIO-38 ports 23 (I/O 12) to 1 (I/O 23)
        DIO_Read8( diOnly, 0, &LSB  );
        DEBUGFDEVICE(telescope->getDeviceName(), DBG_COMM, "LSB Single data was : hex:%x, int:%d", (int)LSB, (int)LSB );

        DIO_Read8( diOnly, 1, &MSB  );
        DEBUGFDEVICE(telescope->getDeviceName(), DBG_COMM, "MSB Single data was : hex:%x, int:%d", (int)MSB, (int)MSB );

        // 0x0FFF picks up only first 12bits
        encoderRAW = ((MSB << 8) | LSB) & 0x0FFF;

    }

    if (lastEncoderValue != encoderValueN[0].value)
    {
        IDSetNumber(&encoderValueNP, NULL);
        lastEncoderValue = encoderValueN[0].value;
    }

    return true;
}

/****************************************************************
**
**
*****************************************************************/
bool Encoder::saveConfigItems(FILE *fp)
{
    IUSaveConfigNumber(fp, &encoderSettingsNP);
    return true;
}

/****************************************************************
**
**
*****************************************************************/
unsigned long Encoder::getRange(unsigned long startEncoder, unsigned long endEncoder, encoderDirection dir)
{
    switch (dir)
    {
        case EN_CW:
        if (endEncoder > startEncoder)
            return (endEncoder - startEncoder);

        return (endEncoder + MAX_ENCODER_COUNT - startEncoder);

        break;

    case EN_CCW:
        if (startEncoder > endEncoder)
            return (startEncoder - endEncoder);
        else
            return (MAX_ENCODER_COUNT - (startEncoder + endEncoder));
        break;
    }
}

int Encoder::getMin(unsigned long CWEncoder, unsigned long CCWEncoder)
{
    if (CWEncoder < CCWEncoder)
        return ((int) CWEncoder);
    // Sounds redundant, but we need to make sure it'
    else return ((int)CCWEncoder)*-1;
}
