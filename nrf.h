/**
 * \file nrf.h
 * \author Khyale
 * \version 1.0
 * 
 * \brief Arquivo de definições da classe
 * */

/** \mainpage 
 * 
\verbatim
Uma biblioteca Arduino para o chip nordic nRF24L01+
Copyright (C) 2013  Khyale S. Nascimento
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
\endverbatim
 *
 * \section intro_sec Introdução
 * Esta biblioteca define a Classe 'nrf', que implementa as principais funções
 * para configuração e controle do chip nordic nRF24L01+. Essa biblioteca foi testada
 * ambiente do Arduino Uno e pode ser estendida para outros ambientes como por
 * exemplo o raspberryPi.
 * 
 * \section chip_sec Chip nordic nRF24L01+
 * 
 * O chip nRF24L01+ é uma transceptor digital que pode operar na faixa de 2,4GHz 
 * com taxas de até 2Mbps e potência de saída de até 0dBm. Ele opera em 3,3V e seu
 * consumo é extremamente baixo, sendo ideal para aplicações de baixa potência. Além disso,
 * é de baixo custo e já existem módulos de RF baseados neste chip que custam U$S4,00.
 * O próprio fabricante disponibiliza nos application notes do chip o projeto de placa 
 * com antena PCB que pode ser utilizado para o desenvolvimento de diversas aplicações.
 * 
 *  
 * \subsection modos_subsec Modos de operação
 * O chip pode operar basicamente em quatro modos 'power down', 'stand-by',
 * 'tx mode' e 'rx mode'. Ao ligar o chip, o dispositivo entra no modo 'power down' após um
 * atraso de 100ms. Fazendo PWR_UP=1, o oscilador do sistema é ativado e ele entra no modo 'standby'.
 * 
 * 
 * \image html modos_operacao.jpg
 *
 * 
 * Para entrar no modo 'rx', devemos colocar PWR_UP=1, CE=1 e PRIM_RX=1 no registrador CONFIG.  A escrita nos registros de configuração
 * são permitidas apenas se o dispositivo estiver no modo 'standby' ou 'power-down'. No modo 'rx'
 * os pacotes recebidos são armazenados no buffer de RX, de tamanho 32 bytes (na realidade se trata
 * de uma FIFO de três níveis). As verificações do endereço e CRC dos pacotes são feitas automaticamente
 * e são transparentes para o usuário.
 *  
 * Para entrar no modo 'tx', devemos colocar PWR_UP=1, CE=1 e PRIM_RX=0 no registrador CONFIG. Nesse
 * modo o dispositivo monta os pacotes a partir do payload escrito no buffer de TX, de tamanho 32 bytes (o FIFO de TX
 * também é de três níveis). Após o buffer ficar vazio, o sistema automaticamente faz o dispositivo
 * entrar num estado intermediário ('standbyII'), a fim de economizar energia.
 * 
 * Antes de utilizar o dispositivos, nós devemos configurar os endereços de recepção ou pipes
 * (o dispositivo pode receber simultaneamente pacotes de até 6 fontes distintas), os parâmetros de RF (canal,
 * taxa, potência), habilitar ou não a função de 'Auto-Ack', o tamanho e tipo do payload (estático ou dinâmico),
 * algoritmo CRC, tamanho do endereço dos dispositivos, etc.
 * 
 * \subsection envio_subsec Envio de Pacotes
 * 
 * A seguir, descrevemos os passos necessários para enviar um pacote (dispositivo em PTX):
 * \li Escreva o payload no buffer de TX. O número de bytes escritos será considerado como o tamanho do payload enviado.
 * \li Configure o endereço de destino. Caso, a função auto-ack esteja habilitada,
 * utilize esse mesmo endereço no PIPE0.
 * \li Faça PWR_UP=1, PRIM_RX=0 e habilite o chip (CE=1). A transmissão do pacote tem início
 * \li Verifique o flag TX_DS para determinar se o pacote foi enviado. Se o flag MAX_RETR for setado, ele precisará ser resetado, caso contrário, a transmissão de pacotes subsequentes será bloqueada. 
 * transmissão 
 * \li Coloque o PTX em 'stand-by' 
 * 
 * \subsection receb_subsec Recebimento de Pacotes
 * 
 * \section classe_sec Classe nrf
 * 
 * A classe foi implementada para auxiliar no desenvolvimento de aplicações 
 * do Arduino que utilizem o chip da Nordic nRF24L01+. Existem métodos de configuração
 * dos registradores, verificação de estados, mudança nos modos de operação. Este arquivo
 * html traz a documentação da Classe 'nrf' e um exemplo de utilização, que foi testado
 * no ambiente Arduino Uno.
 * 
 *  
 **/

#ifndef NRF_H
#define NRF_H

#include<Arduino.h>
#include<stdint.h>
#include<SPI.h>
#include "spidrv.h"
#include "nordic.h"

typedef enum{
    NRF_18DBM = 0,
    NRF_12DBM,
    NRF_6DBM,
    NRF_0DBM
}nrf_power_t;

typedef enum{
    NRF_250KBPS,
    NRF_1MBPS,
    NRF_2MBPS
}nrf_datarate_t;

typedef enum{
    NRF_AW_3BYTES = 1,
    NRF_AW_4BYTES,
    NRF_AW_5BYTES
}nrf_address_width_t;

typedef enum{
    NRF_CRC_1BYTE,
    NRF_CRC_2BYTES
}nrf_crc_mode_t;

typedef enum {
    NRF_PIPE0=0,
    NRF_PIPE1,
    NRF_PIPE2,
    NRF_PIPE3,
    NRF_PIPE4,
    NRF_PIPE5,
}nrf_address_t;

typedef enum {
    NRF_MAX_RT,     
    NRF_TX_DS,
    NRF_RX_DR
}nrf_int_source_t;

typedef enum{
        NRF_POWER_DOWN,
        NRF_STANDBY,
        NRF_TX_MODE,
        NRF_RX_MODE
}nrf_operation_mode_t;

/**
 * \brief Classe nrf
 * 
 * Esta classe possui métodos de configuração controle para o chip
 * nordic nRF24L01+. Foi implementada e testada no Arduino Uno. 
 * 
 * */
class nrf{
    
public:
	nrf(uint8_t ce=9, uint8_t csn=10);
    void set_rf_channel(uint8_t rf_channel);
    uint8_t get_rf_channel(void);
    void set_rf_power(nrf_power_t power);
    uint8_t get_rf_power(void);
    void set_rf_datarate(nrf_datarate_t speed);
    uint8_t get_rf_datarate(void);
    void set_address_width(nrf_address_width_t width);
    uint8_t get_address_width(void);
    void enable_rx_pipe(nrf_address_t pipe, bool auto_ack);
    void disable_rx_pipe(nrf_address_t pipe);
    void set_rx_address(nrf_address_t pipe, uint8_t *addr, uint8_t length);
    void set_static_payload_width(nrf_address_t pipe, uint8_t width);
	uint8_t get_static_payload_width(nrf_address_t pipe);
	void set_tx_address(uint8_t *addr, uint8_t length);
	void set_retr_param(uint8_t retr_count, uint8_t retr_delay);
	uint8_t get_retr_param(void);
	void set_crc_mode(nrf_crc_mode_t crc_mode);
	uint8_t get_crc_mode(void);
    bool available(void);
    void wait_available(void);
    bool wait_available_timeout(const unsigned long timeout);
    bool write_tx_payload(uint8_t *buff, uint8_t length, bool auto_ack=true);
    bool read_received_payload(uint8_t *buff, uint8_t *length);
    bool wait_packet_sent(void);
    void set_irq_pin(uint8_t irq = 2);
    void set_int_source(nrf_int_source_t int_source, bool enable);
    void set_dynamic_payload(nrf_address_t pipe, boolean dyn_pl);
    void clear_all_int_flags(void);
    void clear_int_flag(nrf_int_source_t int_source);
    uint8_t get_received_payload_width(void);
    uint8_t get_data_source(void);
    void set_mode(nrf_operation_mode_t mode); 
    nrf_operation_mode_t get_current_mode();
    void retrieve_last_mode();
    
    //debug
    void print_registers(void);
    void print_buffer(uint8_t *buff, uint8_t length);
    
    
    
    /* criar funcoes */
    //send_single_packet(destino,buffer,len,ack);
    
    
private:
    uint8_t _ce; //pino de CE
	uint8_t _csn; //pino de CSN
    uint8_t _irq; //pino de IRQ
    nrf_operation_mode_t _last_mode,_current_mode;
    uint8_t spi_write_register(uint8_t register_addr, uint8_t data);
	uint8_t spi_read_register(uint8_t register_addr, uint8_t *data);
    uint8_t spi_write_multibyte_register(uint8_t register_addr, uint8_t *addr, uint8_t length);
    uint8_t spi_read_multibyte_register(uint8_t register_addr, uint8_t *buff, uint8_t length);
    uint8_t get_fifo_status(void);
    uint8_t get_status(void);
    void set_power_up(bool pwr_up);
    void set_primary_rx(bool prim_rx);
    void flush_rx_fifo(void);
	void flush_tx_fifo(void);
    void chip_enable(void);
	void chip_disable(void);
    
};

#endif
