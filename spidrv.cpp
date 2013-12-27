/**
 * \file spidrv.cpp
 * \author Khyale
 * \version 1.0
 * \date 24/12/2013
 * 
 * \brief código-fonte do driver SPI
 * */

#include<Arduino.h>
#include <SPI.h>
#include "spidrv.h"

void spi_begin(int spi_device){
    pinMode(SCK, OUTPUT);
    pinMode(MOSI, OUTPUT);
    pinMode(MISO, INPUT);
    pinMode(spi_device,OUTPUT);
    digitalWrite(spi_device,HIGH);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV2); // 8MHz
    SPI.begin();	
}

void spi_transfer(int spi_device, uint8_t *data, uint8_t length){
    digitalWrite(spi_device, LOW);
    while(length-->0)
        *data++ = SPI.transfer(*data);
    digitalWrite(spi_device, HIGH);
}
