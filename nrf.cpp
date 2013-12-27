/**
 * \file nrf.cpp
 * \author Khyale
 * \version 1.0
 * 
 * \brief código-fonte da classe 'nrf'
 * */

#include "nrf.h"

/**
 * \brief Construtor da classe
 * 
 * \param[in] ce Pino do Arduino atribuído ao 'chip enable' (CE)
 * \param[in] csn Pino do Arduino atribuído ao 'chip select' (CSN)
 * 
 * \warning Após a classe ser instanciada, o dispositivo é colocado no modo 'POWER_DOWN'. 
 */
nrf::nrf(uint8_t ce, uint8_t csn){
    nrf::_ce = ce;
	nrf::_csn = csn;
	
	pinMode(_ce, OUTPUT);   // setup ce pin
    nrf::chip_disable();    // chip no modo 'POWER_DOWN'
                            // após o 'power on reset'
        
    spi_begin(_csn);    // configura e inicia a interface SPI
    
    nrf::spi_write_register(CONFIG,EN_CRC); // bits PWR_UP=0 e PRIM_RX=0
    _current_mode = NRF_POWER_DOWN;
    _last_mode = _current_mode;
    
    delay(100);  // assegura atraso no 'power on reset'
    
    nrf::flush_rx_fifo();   // limpa buffer de recepção
    nrf::flush_tx_fifo();   // limpa buffer de transmissão
    nrf::clear_all_int_flags(); // limpa flags de interrupção
}

/**
 * \brief Habilita o dispositivo
 * 
 * Utilize esta função colocar o pino CE em '1'. 
 */
void nrf::chip_enable(void){
	digitalWrite(_ce,HIGH);
}

/**
 * \brief Desabilita o dispositivo
 * 
 * Utilize esta função para colocar o pino CE em '0'.
 * 
 */
void nrf::chip_disable(void){
	digitalWrite(_ce,LOW);
}

/**
 * \brief Escreve um valor no registrador
 * 
 * Utilize esta função escrever determinado valor no registrador do
 * dispositivo.
 * 
 * \param[in] register_addr Endereço do registrador 
 * \param[in] data Valor
 * \return Estado do dispositivo (conteúdo do registrador STATUS)
 * 
 */
uint8_t nrf::spi_write_register(uint8_t register_addr, uint8_t data){
	uint8_t buff[2];
	buff[0] = W_REGISTER | (register_addr & 0x1F);
	buff[1]=data; 
    spi_transfer(_csn, buff, sizeof(buff));
    return buff[0]; //retorna estado
}

/**
 * \brief Escreve um bloco de dados no registrador
 * 
 * Utilize esta função escrever um vários bytes no registrador
 * 
 * \param[in] register_addr Endereço do registrador 
 * \param[in] *buff Ponteiro para o bloco de dados
 * \param[in] length Tamanho do bloco de dados
 * 
 * \return Estado do dispositivo (conteúdo do registrador STATUS)
 *
 */
uint8_t nrf::spi_write_multibyte_register(uint8_t register_addr, uint8_t *buff, uint8_t length){
	uint8_t tmp[length+1];
	tmp[0]= W_REGISTER | (register_addr & 0x1F);
	for(int i=0;i<length;i++)
        tmp[i+1]=*buff++; 
    spi_transfer(_csn, tmp, sizeof(tmp));
    return tmp[0];
}  

/**
 * \brief Lê o conteúdo de um registrador
 * 
 * Utilize esta função ler um byte do registrador.
 * 
 * \param[in] register_addr Endereço do registrador
 * \param[in,out] *data Ponteiro para o valor lido
 * 
 * \return Estado do dispositivo (conteúdo do registrador STATUS)
 * 
 */  
uint8_t nrf::spi_read_register(uint8_t register_addr, uint8_t *data){
	uint8_t buff[2];
	buff[0] = R_REGISTER | (register_addr & 0x1F);
	spi_transfer(_csn, buff, sizeof(buff));
    *data = buff[1];
    return buff[0];
}

/**
 * \brief Lê um bloco de dados do registrador
 * 
 * Utilize esta função ler vários bytes do registrador
 *
 * \param[in] register_addr Endereço do registrador 
 * \param[in,out] *buff Ponteiro para o bloco de dados
 * \param[in] length Tamanho do bloco de dados
 * 
 * \return Estado do dispositivo (conteúdo do registrador STATUS)
 *
 * 
 */  
uint8_t nrf::spi_read_multibyte_register(uint8_t register_addr, uint8_t *buff, uint8_t length){
    uint8_t data[length+1];
    data[0]=R_REGISTER | (register_addr & 0x1F);
    spi_transfer(_csn, data, sizeof(data));
    for(int i=0;i<length;i++)
        buff[i]=data[i+1];
    return data[0];
}

/**
 * \brief Configura o canal de RF
 * 
 * Utilize esta função para configurar o canal de RF. O chip pode operar
 * nos canais 0 a 125 (126 canais).
 * 
 * @param[in] rf_channel Canal de RF
 *   
 */  
void nrf::set_rf_channel(uint8_t rf_channel){
	nrf::spi_write_register(RF_CH, rf_channel & 0x7F);
}
    
/**
 * \brief Retorna o canal de RF
 * 
 * Utilize esta função para obter o canal de RF atual.
 * 
 * \return  Canal de RF (valor entre 0 e 125)
 *   
 */  
uint8_t nrf::get_rf_channel(void){
	uint8_t channel;
	nrf::spi_read_register(RF_CH, &channel);
	return channel;
}

/**
 * \brief Configura a potência do sinal de RF
 * 
 * 
 * Utilize esta função para configurar a potência de saída de RF. 
 * 
 * @param[in] power Potência do sinal de RF
 * 
 * 
 * Utilize os seguintes valores:
 * \li \c NRF_18DBM para -18dBm
 * \li \c NRF_12DBM para -12dBm
 * \li \c NRF_6DBM para -6dBm
 * \li \c NRF_0DBM para 0dBm
 *   
 */  
void nrf::set_rf_power(nrf_power_t power){
	uint8_t lastValue;
	nrf::spi_read_register(RF_SETUP, &lastValue);
	nrf::spi_write_register(RF_SETUP, (lastValue & ~RF_PWR) | ( ( (uint8_t) power ) << 1) );
}

/**
 * \brief Retorna a potência atual do sinal de RF
 * 
 * Utilize esta função para obter a potência de saída de RF atualmente configurada. 
 * 
 * \return Potência do sinal de RF
 * 
 * \retval 0 (-18dBm)
 * \retval 1 (-12dBm) 
 * \retval 2 (-6dBm) 
 * \retval 3 (0dBm)
 *  
 */  
uint8_t nrf::get_rf_power(void){
	uint8_t reg;
	nrf::spi_read_register(RF_SETUP,&reg);
	return (reg & RF_PWR) >> 1;
}

/**
 * \brief Configura a taxa de dados (bps)
 * 
 * Utilize esta função para configurar a taxa de dados. 
 * 
 * @param[in] speed Taxa de dados (bps)
 * 
 * Utilize os seguintes valores:
 * \li \c NRF_250KBPS para 250kbps
 * \li \c NRF_1MBPS para 1Mbps
 * \li \c NRF_2MBPS para 2Mbps
 * 
 */  
void nrf::set_rf_datarate(nrf_datarate_t speed){
    uint8_t lastValue;
    nrf::spi_read_register(RF_SETUP,&lastValue);
    switch(speed){
        case NRF_250KBPS:
        nrf::spi_write_register(RF_SETUP, (lastValue & ~RF_DR_HIGH) | RF_DR_LOW);
        break;
        case NRF_1MBPS:
        nrf::spi_write_register(RF_SETUP, lastValue & ~(RF_DR_HIGH | RF_DR_LOW));
        break;
        case NRF_2MBPS:
        nrf::spi_write_register(RF_SETUP, (lastValue & ~RF_DR_LOW) | RF_DR_HIGH);
    }
}

/**
 * \brief Retorna a taxa de dados (bps) atual
 * 
 * 
 * Utilize esta função para obter a taxa de dados atualmente configurada.
 * 
 * \return Taxa de dados atual.
 * 
 * \retval 0 (250kbps)
 * \retval 1 (1Mbps)
 * \retval 2 (2Mbps)
 * \retval 3 (reservado)
 */  
uint8_t nrf::get_rf_datarate(void){
    uint8_t reg;
    nrf::spi_read_register(RF_SETUP, &reg);
    uint8_t tmp = ((reg & RF_DR_LOW)>>5) | ((reg & RF_DR_HIGH)>>2);
    switch(tmp){
        case 0:
        return 0x01; //1Mbps
        case 1:
        return 0x00; //250kbps
        case 2:
        return 0x02; //2Mbps
        default:
        return 0x03; //Reserved
    }
}

/**
 * \brief Configura o tamanho do endereço do dispositivo
 * 
 * 
 * \param[in] width Tamanho do endereço
 * 
* Utilize os seguintes valores:
 * \li \c NRF_AW_3BYTES para 3 bytes de endereço
 * \li \c NRF_AW_4BYTES para 4 bytes de endereço
 * \li \c NRF_AW_5BYTES para 5 bytes de endereço
 * 
 * \warning Certifique-se de que o tamanho do endereço no transmissor
 * e receptor sejam os mesmos.
 * 
 */  
void nrf::set_address_width(nrf_address_width_t width){
    nrf::spi_write_register(SETUP_AW, (uint8_t)width );
}

/**
 * \brief Retorna o tamanho do endereço atual.
 * 
 * \return Tamanho do endereço
 * 
 * \retval 3 (para endereço de 3 bytes)
 * \retval 4 (para endereço de 4 bytes)
 * \retval 5 (para endereço de 5 bytes)
 */
uint8_t nrf::get_address_width(void){
    uint8_t reg;
    nrf::spi_read_register(SETUP_AW, &reg);
    return reg+2;
}

/**
 * \brief Habilita pipe
 * 
 * Utilize esta função para habilitar um pipe de recepção. O chip possui 6 pipes
 * que podem ser utilizados para receber pacotes de até 6 dispositivos distintos, numa arquitetura
 * conhecida como Multiceiver (consulte datasheet).
 * 
 * \param[in] pipe Pipe de recepção
 * \param[in] auto_ack Habilita ou não a função 'auto acknoledgment'. Se auto_ack
 * for 'true', após o pacote ser recebido, o PRX enviará um pacote ACK de volta para o PTX.
 * 
 * \warning Esta função configura os bits dos registradores EN_RXADDR e EN_AA de acordo
 * com o 'pipe' informado.   
 */
void nrf::enable_rx_pipe(nrf_address_t pipe, bool auto_ack){
    uint8_t reg;
    nrf::spi_read_register(EN_RXADDR, &reg);
    nrf::spi_write_register(EN_RXADDR, reg | BIT((uint8_t)pipe));
    
    nrf::spi_read_register(EN_AA, &reg);
    if(auto_ack){
        nrf::spi_write_register(EN_AA, reg | BIT((uint8_t)pipe));
    }else{
        nrf::spi_write_register(EN_AA, reg & ~BIT((uint8_t)pipe) );
    }
}

/**
 * \brief Desabilita pipe
 * 
 * Utilize esta função para desabiltar um pipe de recepção.
 * 
 * 
 * \param [in] pipe Pipe de recepção.
 */
void nrf::disable_rx_pipe(nrf_address_t pipe){
    uint8_t reg;
    nrf::spi_read_register(EN_RXADDR, &reg);
    nrf::spi_write_register(EN_RXADDR, reg & ~BIT((uint8_t)pipe));
    nrf::spi_read_register(EN_AA, &reg);
    nrf::spi_write_register(EN_AA, reg & ~BIT((uint8_t)pipe) );
}

/**
 * \brief Configura endereços de recepção
 * 
 * 
 * \param[in] pipe Pipe de recepção
 * \param[in] *addr Ponteiro para buffer de endereço (byte menos significativo primeiro).
 * \param[in] length Tamanho do endereço
 * 
 * \warning 
 * A variável 'length' deve assumir o mesmo valor definido pela função \ref set_address_width .
 * 
 * \warning Os bytes mais significativos (MSB) dos pipes 2 a 5 são idênticos ao do pipe 1. Os endereços diferem
 * em apenas um byte (o byte menos significativo ou LSB).
 */
void nrf::set_rx_address(nrf_address_t pipe, uint8_t *addr, uint8_t length){
    switch(pipe){
        case NRF_PIPE0:
        case NRF_PIPE1:
        nrf::spi_write_multibyte_register( RX_ADDR_P0 + (uint8_t) pipe, addr, length);
        break;
        case NRF_PIPE2:
        case NRF_PIPE3:
        case NRF_PIPE4:
        case NRF_PIPE5:
            nrf::spi_write_register(RX_ADDR_P0 + (uint8_t) pipe, *addr);
    }
}

/**
 * \brief Configura o tamanho do payload para recepção
 * 
 *  No caso do payload ser estático, utilize essa função para definir o tamanho do payload
 * de dados para recepção. 
 * 
 * \param[in] pipe Pipe de recepção
 * \param[in] width Tamanho do payload de recepção (valor de 1 a 32)
 * 
 */
void nrf::set_static_payload_width(nrf_address_t pipe, uint8_t width){
    nrf::spi_write_register(RX_PW_P0 + (uint8_t) pipe, width & 0x3F);
}

/**
 * \brief Retorna o tamanho do payload
 * 
 *  No caso do payload ser estático, utilize essa função para definir obter o tamanho do payload
 * de dados para recepção. 
 * 
 * @param[in] pipe Pipe de recepção
 * 
 * @return uint8_t Tamanho do payload para o pipe informado
 */
uint8_t nrf::get_static_payload_width(nrf_address_t pipe){
    uint8_t reg;
    nrf::spi_read_register(RX_PW_P0 + (uint8_t) pipe, &reg);
    return reg;
}

/**
 * \brief Configura o endereço de transmissão
 * 
 * Utilize essa função para definir o endereço do dispositivo de destino. 
 * 
 * 
 * @param[in] *addr Ponteiro para buffer do endereço
 * @param[in] length Tamanho do endereço
 * 
 * \warning A variável 'length' deve assumir o mesmo valor definido pela função \ref set_address_width .
 */
void nrf::set_tx_address(uint8_t *addr, uint8_t length){
    nrf::spi_write_multibyte_register(TX_ADDR, addr, length);
}

/**
 * \brief Configura os parâmetros de retransmissão.
 * 
 * Se a função Auto Ack estiver habilidata, esta função define 
 * os parâmatros de retransmissão para o caso de falha na recepção dos pacotes ACK.
 * 
 * \param[in] retr_count Número de tentativas de retransmissão. Pode assumir os valores
 * 0 (função de retransmissão desabilitada) a 15.
 * \param[in] retr_delay Tempo em que o PTX deve aguadar até o reenvio. Pode assumir
 * os valores de 0 a 15. O atraso é definido como 250*(1+retr_delay) (em us)
 * 
 */
void nrf::set_retr_param(uint8_t retr_count, uint8_t retr_delay){
    nrf::spi_write_register(SETUP_RETR, ((retr_delay & 0x0F)<<4) | (retr_count & 0x0F) );
}

/**
 * \brief Retorna os parâmetros de retransmissão
 * 
 * \return Retorna os parâmetro de retransmissão. 0s primeiros 4 bits mais significativos
 * representam o atraso e os 4 últimos o número de tentativas de retransmissão. 
 * 
 */
uint8_t nrf::get_retr_param(void){
    uint8_t reg;
    nrf::spi_read_register(SETUP_RETR, &reg);
    return reg;
}

/**
 * \brief Seleciona o argoritmo de cálculo do CRC
 * 
 * \param[in] crc_mode Algoritmo CRC utilizado.
 * 
 * Pode assumir os seguintes valores:
 * \li \c NRF_CRC_1BYTE (CRC de 1 byte)
 * \li \c NRF_CRC_2BYTES (CRC de 2 bytes)
 * 
 */
void nrf::set_crc_mode(nrf_crc_mode_t crc_mode){
    uint8_t lastValue;
    nrf::spi_read_register(CONFIG, &lastValue);
    switch(crc_mode){
        case NRF_CRC_1BYTE:
        nrf::spi_write_register(CONFIG, (lastValue & ~CRCO) | EN_CRC);
        break;
        case NRF_CRC_2BYTES:
        nrf::spi_write_register(CONFIG, lastValue | CRCO | EN_CRC);
    }
}

/**
 * \brief Retorna o argoritmo de cálculo do CRC
 * 
 * \return Algoritmo de cálculo do CRC.
 * \retval 1 (para CRC de 1 byte)
 * \retval 2 (para CRC de 2 bytes)
 */
uint8_t nrf::get_crc_mode(void){
    uint8_t reg;
    nrf::spi_read_register(CONFIG, &reg);
    if(reg & EN_CRC){
        if(reg & CRCO){
            return 0x02;
        }else{
            return 0x01;
        }
    }else{
        return 0x00;
    }
}

/**
 * \brief Descarrega buffer de recepção
 * 
 */
void nrf::flush_rx_fifo(void){
    uint8_t data = FLUSH_RX;
    spi_transfer(_csn, &data, sizeof(data));
}

/**
 * \brief Descarrega buffer de transmissão
 * 
 */
void nrf::flush_tx_fifo(void){
    uint8_t data = FLUSH_TX;
    spi_transfer(_csn, &data, sizeof(data));
}

/**
 * \brief Retorna o estado do dispositivo.
 * 
 * \return Estado do dispositivo.
 * 
 */
uint8_t nrf::get_status(void){
    uint8_t reg=NOP;
    spi_transfer(_csn, &reg, sizeof(reg));
    return reg;
}

/**
 * \brief Limpa todos os flags de interrupção.
 * 
 * Utilize essa função para zerar todos os flags de interrupção do dispositivo.
 * 
 */
void nrf::clear_all_int_flags(void){
    uint8_t lastValue=nrf::get_status();
    nrf::spi_write_register(STATUS, lastValue | (RX_DR|TX_DS|MAX_RT) );
}

/**
 * \brief Verifica se pacote foi recebido
 * 
 * Utilize essa função para checar se há pacote no buffer de recepção.
 * 
 * \return true ou false
 * \retval true Pacote recebido
 * \retval false Não há pacote no buffer de recepção.
 * 
 * \warning Antes de executar essa função, verifique se o dispostivo
 * está no modo de recepção.
 */
bool nrf::available(void){
    uint8_t fifo_status = nrf::get_fifo_status();
    if(!(fifo_status & RX_EMPTY)){
        return true;
    }else{
        return false;
    }
}

/**
 * \brief Retorna o valor do pipe em que houve recepção do último pacote. 
 * 
 * 
 * \return Pipe do último pacote recebido.
 * \retval 0 (PIPE0)
 * \retval 1 (PIPE1)
 * \retval 2 (PIPE2)
 * \retval 3 (PIPE3)
 * \retval 4 (PIPE4)
 * \retval 5 (PIPE5)
 * \retval 6 (Não usado)
 * \retval 7 (Buffer de recepção vazio)
 */ 
uint8_t nrf::get_data_source(void){                           
    uint8_t reg = nrf::get_status();
    return (reg & RX_P_NO) >> 1;
}

/**
 * \brief Lê o payload do pacote recebido
 * 
 * \param [out] *buff Ponteiro para o buffer de dados
 * \param [out] *length Ponteiro para o tamanho do payload recebido.
 * 
 * \return true ou false
 * \retval true Pacote recebido com sucesso
 * \retval false Erro na leitura.
 * 
 */ 
bool nrf::read_received_payload(uint8_t *buff, uint8_t *length){
    if( !(nrf::available()) ){
        buff=NULL;
        return false;
    }
    
    *length = nrf::get_received_payload_width();
    if(*length>32){
        nrf::flush_rx_fifo();
        nrf::clear_int_flag(NRF_RX_DR);
        buff=NULL;
        return false;
    }
    
    uint8_t incoming[*length+1];
    incoming[0]= R_RX_PAYLOAD;
    spi_transfer(_csn,incoming,sizeof(incoming));
    
    for(int i=0;i<*length;i++)
        buff[i]=incoming[i+1];
    
    return true;
}

/**
 * \brief Bloqueia a execução do programa até a chegada de um pacote.
 * 
 * @warning Antes de executar essa função coloque o dispositivo no modo recepção.
 * 
 */ 
void nrf::wait_available(void){
    while( !(nrf::available()) ){
    }
}

/**
 * \brief Bloqueia a executação do programa até a chegada de um pacote ou timeout ocorrer.
 * 
 * \param [in] timeout Timeout em milissegundos.
 * \return true ou false
 * \retval true Pacote recebido
 * \retval false Pacote não recebido (timeout)
 * 
 * @warning Antes de executar essa função coloque o dispositivo no modo recepção.
 */ 
bool nrf::wait_available_timeout(const unsigned long timeout){
    unsigned long currentTime = millis();
    while( (millis() - currentTime) < timeout ){
        if(nrf::available()){
            return true;
        }
    }
    return false;
}

/**
 * \brief Configura o modo de potência do dispositivo.
 * 
 * Utilize esta função para setar ou resetar o bit PWR_UP no registrador CONFIG.
 * 
 * \param [in] pwr_up true ou false.
 */ 
void nrf::set_power_up(bool pwr_up){
    uint8_t lastValue;
    nrf::spi_read_register(CONFIG,&lastValue);
    if(pwr_up){
        nrf::spi_write_register(CONFIG, lastValue | PWR_UP);
    }else{
        nrf::spi_write_register(CONFIG, lastValue & ~PWR_UP);  
    }
}

/**
 * \brief Configura o modo primário do dispositivo.
 * 
 * Utilize esta função para setar ou resetar o bit PRIM_RX no registrador CONFIG.
 * 
 * \param [in] prim_rx true ou false.
 */ 
void nrf::set_primary_rx(bool prim_rx){
    uint8_t lastValue;
    nrf::spi_read_register(CONFIG, &lastValue);
    if(prim_rx){
        nrf::spi_write_register(CONFIG, lastValue | PRIM_RX);
    }else{
        nrf::spi_write_register(CONFIG, lastValue & ~PRIM_RX);
    }
}

/**
 * \brief Limpa flag de interrupção.
 *
 * \param [in] int_source Fonte de interrupação.
 */ 
void nrf::clear_int_flag(nrf_int_source_t int_source){
    uint8_t lastValue = nrf::get_status();
    switch(int_source){
        case NRF_RX_DR:
            nrf::spi_write_register(STATUS, lastValue | RX_DR);
            break;
        case NRF_TX_DS:
            nrf::spi_write_register(STATUS, lastValue | TX_DS);
            break;
        case NRF_MAX_RT:
            nrf::spi_write_register(STATUS, lastValue | MAX_RT);
    }    
}


/**
 * \brief Imprime no terminal serial o conteúdo dos registradores de configuração.
 * 
 * Utilize essa função durante processo de debug
 * 
 */ 
void nrf::print_registers(void){
  uint8_t reg,address_width;
  Serial.print("--- Register's content (in hexa) ---\n");

  nrf::spi_read_register(CONFIG,&reg);
  Serial.print("CONFIG: ");
  Serial.println(reg,HEX);

  nrf::spi_read_register(EN_AA,&reg);  
  Serial.print("EN_AA: ");
  Serial.println(reg,HEX);

  nrf::spi_read_register(EN_RXADDR,&reg);  
  Serial.print("EN_RXADDR: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(SETUP_AW,&reg); 
  address_width=reg+2;
  Serial.print("SETUP_AW: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(SETUP_RETR,&reg);  
  Serial.print("SETUP_RETR: ");
  Serial.println(reg,HEX);  
  
  nrf::spi_read_register(RF_CH,&reg);  
  Serial.print("RF_CH: ");
  Serial.println(reg,HEX); 
  
  nrf::spi_read_register(RF_SETUP,&reg);  
  Serial.print("RF_SETUP: ");
  Serial.println(reg,HEX);
  
  Serial.print("STATUS: ");
  Serial.println(nrf::get_status(),HEX);

  nrf::spi_read_register(OBSERVE_TX,&reg);  
  Serial.print("OBSERVE_TX: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(RPD,&reg);  
  Serial.print("RPD: ");
  Serial.println(reg,HEX);
  
  uint8_t buff[5];
  nrf::spi_read_multibyte_register(RX_ADDR_P0, buff, address_width);
  Serial.print("RX_ADDR_P0 (LSB first): ");
  for(int i=0;i<address_width;i++){
    Serial.print(buff[i],HEX);
    Serial.print(" ");
  }
  Serial.print("\n");
  
  
  nrf::spi_read_multibyte_register(RX_ADDR_P1, buff, address_width);
  Serial.print("RX_ADDR_P1 (LSB first): ");
  for(int i=0;i<address_width;i++){
    Serial.print(buff[i],HEX);
    Serial.print(" ");
  }
  Serial.print("\n");
  
  nrf::spi_read_register(RX_ADDR_P2,&reg);  
  Serial.print("RX_ADDR_P2: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(RX_ADDR_P3,&reg);  
  Serial.print("RX_ADDR_P3: ");
  Serial.println(reg,HEX);

  nrf::spi_read_register(RX_ADDR_P4,&reg);  
  Serial.print("RX_ADDR_P4: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(RX_ADDR_P5,&reg);  
  Serial.print("RX_ADDR_P5: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_multibyte_register(TX_ADDR, buff, address_width);
  Serial.print("TX_ADDR (LSB first): ");
  for(int i=0;i<address_width;i++){
    Serial.print(buff[i],HEX);
    Serial.print(" ");
  }
  Serial.print("\n");

  nrf::spi_read_register(RX_PW_P0,&reg);  
  Serial.print("RX_PW_P0: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(RX_PW_P1,&reg);  
  Serial.print("RX_PW_P1: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(RX_PW_P2,&reg);  
  Serial.print("RX_PW_P2: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(RX_PW_P3,&reg);  
  Serial.print("RX_PW_P3: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(RX_PW_P4,&reg);  
  Serial.print("RX_PW_P4: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(RX_PW_P5,&reg);  
  Serial.print("RX_PW_P5: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(FIFO_STATUS,&reg);  
  Serial.print("FIFO_STATUS: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(DYNPD,&reg);  
  Serial.print("DYNPD: ");
  Serial.println(reg,HEX);
  
  nrf::spi_read_register(FEATURE,&reg);  
  Serial.print("FEATURE: ");
  Serial.println(reg,HEX);
  
}

/**
 * \brief Imprime no terminal serial o conteúdo de um registrador ou buffer de dados.
 * 
 * \param [in] *buff Ponteiro para o buffer de dados lido.
 * \param [in] length tamanho do buffer de dados.
 * 
 */ 
void nrf::print_buffer(uint8_t *buff, uint8_t length){
    for(int i=0;i<length;i++){
        Serial.print(buff[i],HEX);
        Serial.print(" ");
    }
}

/**
 * \brief Retorna o estado dos buffers de recepção e transmissão.
 * 
 * \return Retorna byte de estado dos FIFO's de TX e RX
 * 
 */ 
uint8_t nrf::get_fifo_status(void){
    uint8_t reg;
    nrf::spi_read_register(FIFO_STATUS,&reg);
    return reg;
}

/**
 * \brief Escreve o payload no buffer de transmissão.
 * 
 * \param [in] *buff Ponteiro para o buffer de dados que armazena o payload
 * \param [in] length Tamanho do buffer
 * \param [in] auto_ack true or false. Habilita ou não a função de auto-ack para o pacote a ser enviado.
 *
 * \return true ou false
 * \retval true Dados escritos com sucesso.
 * \retval false Erro durante a escrita. Buffer de TX pode está cheio.
 * 
 */
bool nrf::write_tx_payload(uint8_t *buff, uint8_t length, bool auto_ack){
    
    if(nrf::get_fifo_status() & TX_FIFO_FULL){
        return false; 
    }
    
    uint8_t outcoming[length+1];
    outcoming[0]=(auto_ack)? W_TX_PAYLOAD:W_TX_PAYLOAD_NOACK;
    
    //copy data
    for(int i=0;i<length;i++)
        outcoming[i+1]=buff[i];
    
    // write into tx fifo
    spi_transfer(_csn, outcoming, sizeof(outcoming));
    
    return true;
}

/**
 * \brief Aguarda o envio do pacote.
 * 
 * Utilize essa função para bloquear a execução do programa até o pacote ser enviado.
 * 
 * \return true ou false
 * \retval true Pacote enviado com sucesso.
 * \retval false Erro durante o envio. Dispositivo não está no modo de transmissão ou o número máximo de retransmissões foi atingido.
 * Verifique se o chip está no modo transmissão e se o receptor está ativo e dentro da área de cobertura. 
 */
bool nrf::wait_packet_sent(void){
    uint8_t config;
    nrf::spi_read_register(CONFIG, &config);
    if( (config & PRIM_RX) | !digitalRead(_ce) | !(config & PWR_UP) )  
        return false;
    
    do{
        while( !(nrf::get_status() & (TX_DS|MAX_RT)) ){
        }
    
        if(nrf::get_status() & MAX_RT){
            nrf::clear_int_flag(NRF_MAX_RT);
            nrf::flush_tx_fifo();
            return false;
        }
        nrf::clear_int_flag(NRF_TX_DS);
    }while(!(nrf::get_fifo_status() & TX_EMPTY));
    
    return true;
}

/**
 * \brief Retorna o tamanho do payload recebido.
 * 
 * \return Tamanho do payload recebido em bytes
 * 
 */
uint8_t nrf::get_received_payload_width(void){
    uint8_t buff[2];
    buff[0] = R_RX_PL_WID;
    spi_transfer(_csn, buff, sizeof(buff));
    return buff[1];
}

/**
 * \brief Configura Interrupção externa (IRQ).
 * 
 * Utilize essa função para configurar o pino de entrada para tratar o sinal de interrupção gerada pelo dispositivo (IRQ).
 * 
 * \param[in] irq Pino de IRQ
 */
void nrf::set_irq_pin(uint8_t irq){
    _irq = irq;
    pinMode(_irq,INPUT);
}

/**
 * \brief Configura a fonte de interrupção.
 * 
 * Utilize essa função para setar ou resetar os bits da máscara de interrupção.
 * 
 * \param[in] int_source Fonte de interrupção
 * \param[in] enable true ou false (habilita ou desabilita a fonte de interrupção)
 * 
 * \warning A condição inicial são todas as fontes habilitadas. Ocorrendo 
 * uma interrupção, o bit correspondente é setado no registrador STATUS. Para
 * resetá-lo utilize a função \ref clear_int_flag ou \ref clear_all_int_flags
 */
void nrf::set_int_source(nrf_int_source_t int_source, bool enable){
    uint8_t config;
    nrf::spi_read_register(CONFIG,&config);
    switch(int_source){
        case (NRF_RX_DR):
        if(enable){
            nrf::spi_write_register(CONFIG, config & ~MASK_RX_DR);
        }else{
            nrf::spi_write_register(CONFIG, config | MASK_RX_DR);
        }
        break;
        case (NRF_TX_DS):
        if(enable){
            nrf::spi_write_register(CONFIG, config & ~MASK_TX_DS);
        }else{
            nrf::spi_write_register(CONFIG, config | MASK_TX_DS);
        }
        break;
        case (NRF_MAX_RT):
        if(enable){
            nrf::spi_write_register(CONFIG, config & ~MASK_MAX_RT);
        }else{
            nrf::spi_write_register(CONFIG, config | MASK_MAX_RT);
        }
    }
}

/**
 * \brief Configura o payload dinâmico
 * 
 * Utilize essa função para habilitar ou desabilitar o payload dinâmico. Esta função atua sobre os bits EN_DPL do registrador FEATURE
 * e do registrador DYNPD. 
 * 
 * \warning Um PTX que transmite para um PRX com payload dinâmico habilitado deve ter o bit DPL_P0 no registrador DYNPD setado.
 * */
void nrf::set_dynamic_payload(nrf_address_t pipe, boolean dyn_pl){
    uint8_t last_value;
    nrf::spi_read_register(DYNPD,&last_value);
    switch(pipe){
        case NRF_PIPE0:
        case NRF_PIPE1:
        case NRF_PIPE2:
        case NRF_PIPE3:
        case NRF_PIPE4:
        case NRF_PIPE5:
        
        if(dyn_pl){
            nrf::spi_write_register(DYNPD, last_value | BIT((uint8_t)pipe) );
        }else{
            nrf::spi_write_register(DYNPD, last_value & ~BIT((uint8_t)pipe) );
        }
    }
    
    nrf::spi_read_register(DYNPD,&last_value);
    uint8_t feature;
    nrf::spi_read_register(FEATURE, &feature);
    if(last_value){
        nrf::spi_write_register(FEATURE, feature|EN_DPL);
    }else{
        nrf::spi_write_register(FEATURE, feature & ~EN_DPL);
    }
}

/**
 * \brief Configura o modo de operação do dispositivo
 * 
 * \param [in] mode Modo de operação.
 * 
 * Os modos de operação recomendados são:
 * \li \c NRF_POWER_DOWN. Este modo é o estado inicial e de menor consumo. 
 * \li \c NRF_STANDBY. Este modo é alcançado fazendo-se o bit PWR_UP=1 e CE=0. O consumo é intermediário pois
 * o oscilador já estará ligado.  
 * \li \c NRF_RX_MODE. Modo de recepção. Neste estado é possível receber pacotes.
 * \li \c NRF_TX_MODE. TX mode Modo de transmissão. Neste modo o dispositivo monta os pacotes a partir do FIFO de TX
 * e os envia. Quando o FIFO está vazio, o dispositivo automaticamente vai para um estado de menor consumo.
 * 
 * */
void nrf::set_mode(nrf_operation_mode_t mode){
    _last_mode = _current_mode;
     
    switch(mode){
    
    case NRF_POWER_DOWN:
    nrf::chip_disable();
    nrf::set_power_up(false);
    _current_mode = NRF_POWER_DOWN;
    break;
    
    case NRF_STANDBY:
    nrf::chip_disable();
    nrf::set_power_up(true);
    if(_current_mode == NRF_POWER_DOWN)
        delay(5);
    _current_mode = NRF_STANDBY;
    break;
    
    case NRF_RX_MODE:
    nrf::chip_disable();
    nrf::set_power_up(true);
    nrf::set_primary_rx(true);
    nrf::chip_enable();
    if(_current_mode == NRF_STANDBY)
        delayMicroseconds(150);
    if(_current_mode == NRF_POWER_DOWN)
        delay(5);
    _current_mode = NRF_RX_MODE;
    break;
    
    case NRF_TX_MODE:
    nrf::chip_disable();
    nrf::set_power_up(true);
    nrf::set_primary_rx(false);
    nrf::chip_enable();
    if(_current_mode == NRF_STANDBY)
        delayMicroseconds(150);
    if(_current_mode == NRF_POWER_DOWN)
        delay(5);
    _current_mode = NRF_TX_MODE;
    
    }
}

/**
 * \brief Retorna o modo de operação corrente
 * 
 * */
nrf_operation_mode_t nrf::get_current_mode(){
    return _current_mode;
}

/**
 * \brief Coloca o dispositivo no modo de operação anterior.
 * 
 * */
void nrf::retrieve_last_mode(){
    nrf::set_mode(_last_mode);
}
