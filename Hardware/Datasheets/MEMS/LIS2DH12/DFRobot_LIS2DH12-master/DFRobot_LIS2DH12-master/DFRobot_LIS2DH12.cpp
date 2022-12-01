/*!
 * @file DFRobot_LIS2DH12.cpp
 * @brief DFRobot's Read LIS2DH12 data
 * @n This example is in order to achieve the serial port to receive LIS2DH12 back to the data
 *
 * @copyright	[DFRobot](http://www.dfrobot.com), 2016
 * @copyright	GNU Lesser General Public License
 * @author [Wuxiao](xiao.wu@dfrobot.com)
 * @version  V1.0
 * @date  2016-10-13
 * @https://github.com/DFRobot/DFRobot_LIS2DH12
 */

#include <DFRobot_LIS2DH12.h>
#include <Wire.h>

uint8_t DFRobot_LIS2DH12::sensorAddress = 0x18; //	0x18

int8_t DFRobot_LIS2DH12::init(uint8_t range)
{
    int8_t ret = 0;
    
    setRange(range);
    Wire.beginTransmission(sensorAddress);
    ret = Wire.endTransmission();
    if(ret != 0){
       ret = -1;
    }else{
        uint8_t ctrl_reg_values[] = {0x2F, 0x00, 0x00, range, 0x00, 0x00};
        ret = (int8_t)writeReg(0xA0, ctrl_reg_values, sizeof(ctrl_reg_values));
    }
    return ret;
}

void DFRobot_LIS2DH12::readXYZ(int16_t &x, int16_t &y, int16_t &z) //read x, y, z data
{
    uint8_t sensorData[6];
    readReg(0xA8, sensorData, 6);
    x = ((int8_t)sensorData[1])*256+sensorData[0]; //return values
    y = ((int8_t)sensorData[3])*256+sensorData[2];
    z = ((int8_t)sensorData[5])*256+sensorData[4];
}

void DFRobot_LIS2DH12::mgScale(int16_t &x, int16_t &y, int16_t &z)
{
    x = (int32_t)x*1000/(1024*mgScaleVel); //transform data to millig, for 2g scale axis*1000/(1024*16),
    y = (int32_t)y*1000/(1024*mgScaleVel); //for 4g scale axis*1000/(1024*8),
    z = (int32_t)z*1000/(1024*mgScaleVel); //for 8g scale axis*1000/(1024*4)
}

uint8_t DFRobot_LIS2DH12::readReg(uint8_t regAddress)
{
    uint8_t regValue;
    Wire.beginTransmission(sensorAddress);
    Wire.write(regAddress);
    Wire.endTransmission();
    Wire.requestFrom(sensorAddress, (uint8_t)1);
    regValue = Wire.read();
    return regValue;
}

void DFRobot_LIS2DH12::readReg(uint8_t regAddress, uint8_t *regValue, uint8_t quanity, bool autoIncrement)
{   
    regAddress = 0x80 | regAddress;
    if(autoIncrement){
        Wire.beginTransmission(sensorAddress);
        Wire.write(regAddress);
        Wire.endTransmission();
        Wire.requestFrom(sensorAddress, quanity);
        for(uint8_t i=0; i < quanity; i++)
            regValue[i] = Wire.read();
    }else{
        for(uint8_t i = 0; i < quanity; i++){
            Wire.beginTransmission(sensorAddress);
            Wire.write(regAddress+i);
            Wire.endTransmission();
            Wire.requestFrom(sensorAddress,(uint8_t)1);
            regValue[i] = Wire.read();
        }
    }
    
}

uint8_t DFRobot_LIS2DH12::writeReg(uint8_t regAddress, uint8_t regValue)
{
    Wire.beginTransmission(sensorAddress);
    Wire.write(regAddress);
    Wire.write(regValue);
    return Wire.endTransmission(true);
}

uint8_t DFRobot_LIS2DH12::writeReg(uint8_t regAddress, uint8_t *regValue, size_t quanity, bool autoIncrement)
{   
    Wire.beginTransmission(sensorAddress);
    if(autoIncrement) {
        Wire.write(regAddress);
        Wire.write(regValue, quanity);
    }
    else {
        for(size_t i = 0; i < quanity; i++){
            Wire.write(regAddress+i);
            Wire.write(regValue[i]);
            if( i<(quanity-1) ){
                Wire.endTransmission(false);
                Wire.beginTransmission(sensorAddress);
            }
        }
    }
    return Wire.endTransmission(true);
}

void DFRobot_LIS2DH12::setRange(uint8_t range)
{
    switch(range)
    {
    case LIS2DH12_RANGE_2GA:
        mgScaleVel = 16;
        break;

    case LIS2DH12_RANGE_4GA:
        mgScaleVel = 8;
        break;

    case LIS2DH12_RANGE_8GA:
        mgScaleVel = 4;
        break;

    case LIS2DH12_RANGE_16GA:
        mgScaleVel = 2;
        break;

    default:
        mgScaleVel = 16;
        break;
    }
}
