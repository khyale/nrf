#include "nrf.h"
#include<SPI.h>

/* rf module pins */
const int cePin=9;
const int csnPin=10;

/* device addresses */
const uint8_t ptx_addr[5]={12,48,68,99,14};
const uint8_t prx_addr[5]={17,11,22,134,192};

void setup(){
  Serial.begin(9600);
  Serial.print("<< Teste de dispositivo no modo PTX >>\n\n");
}

void loop(){
  nrf rfmodule(cePin,csnPin);
  
  /* configura modulo rf 
  *
  * - RF: canal 25, 0dBm, 2Mbps
  * 
  * - Tamanho do endereco: 5bytes
  *
  * - Pipes de recepcao:
  * PIPE0: endereco de PTX
  * PIPE1: endereco de PRX
  *
  * - Payload dinamico
  * 
  * - Auto Ack habilitado. Retransmisso de 3 pacotes
  *
  */ 
  rfmodule.set_rf_datarate(NRF_2MBPS);  //taxa 2Mbps
  rfmodule.set_rf_channel(25);  //canal 25
  rfmodule.set_rf_power(NRF_0DBM);  //potencia 0dBm
  rfmodule.set_address_width(NRF_AW_5BYTES);  //tamanho 5 bytes
  rfmodule.enable_rx_pipe(NRF_PIPE0,true);  //pipe0 habilitado com auto-ack
  rfmodule.enable_rx_pipe(NRF_PIPE1,true);  //pipe1 habilitado com auto-ack
  rfmodule.set_dynamic_payload(NRF_PIPE0,true);  //pipe0 com payload dinamico
  rfmodule.set_dynamic_payload(NRF_PIPE1,true);  //pipe1 com payload dinamico
  rfmodule.set_rx_address(NRF_PIPE0,(uint8_t*)prx_addr,5);
  rfmodule.set_rx_address(NRF_PIPE1,(uint8_t*)ptx_addr,5);
  rfmodule.set_tx_address((uint8_t*)prx_addr,5); //retransmissao de 3 pacotes, timeout de 500us
  
  char message1[]="Hello, world!";
  char message2[]="I am alive!";
  while(true){
    rfmodule.write_tx_payload((uint8_t*)message1,sizeof(message1));
    rfmodule.write_tx_payload((uint8_t*)message2,sizeof(message2));
    rfmodule.set_mode(NRF_TX_MODE);
    if(rfmodule.wait_packet_sent()){
      Serial.print("Mensagens enviadas com sucesso!\n");
    }else{
      Serial.print("Falha no envio. Tente novamente.\n");
    }
    rfmodule.set_mode(NRF_STANDBY);
    delay(1000);
  }
}
