/**
 * \file spidrv.h
 * \author Khyale
 * \version 1.0
 * \date 24/12/2013
 * 
 * \brief arquivo de definições do driver SPI
 * */

#ifndef SPIDRV_H
#define SPIDRV_H

#include<stdint.h>
/** 
 * \brief Configura e inicia interface SPI
 * 
 * \param [in] spi_device Pino associado ao Serial Select (SS) do dispositivo escravo.
 * */
void spi_begin(int spi_device);

/**
 * Envia/Recebe dados da inteface SPI
 * 
 * Utilize esta função para ler ou escrever em dispositivo através da intervace SPI.
 * 
 * @param[in] spi_device Pinagem atribuida ao chip select do dispositivo escravo.
 * @param[in,out] *data Ponteiro de leitura e escrida dos dados. Os dados são sobrescritos.
 * @param[in] length Tamanho do buffer.
 * 
 * */
void spi_transfer(int spi_device, uint8_t *data, uint8_t length);
#endif
