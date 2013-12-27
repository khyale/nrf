/**
 * \file nordic.h
 * \author Khyale
 * \date 26/12/2013
 * \version 1.0
 * 
 * \brief Arquivo de definição do chip nRF24L01+
 * 
 * */
 
#ifndef NORDIC_H
#define NORDIC_H

/* macro */
#define BIT(pos) ((uint8_t) (1<<( (uint8_t) (pos) )))

/* Register map */
#define CONFIG      0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05
#define RF_SETUP    0x06
#define STATUS      0x07
#define OBSERVE_TX  0x08
#define RPD         0x09
#define RX_ADDR_P0  0x0A
#define RX_ADDR_P1  0x0B
#define RX_ADDR_P2  0x0C
#define RX_ADDR_P3  0x0D
#define RX_ADDR_P4  0x0E
#define RX_ADDR_P5  0x0F
#define TX_ADDR     0x10
#define RX_PW_P0    0x11
#define RX_PW_P1    0x12
#define RX_PW_P2    0x13
#define RX_PW_P3    0x14
#define RX_PW_P4    0x15
#define RX_PW_P5    0x16
#define FIFO_STATUS 0x17
#define DYNPD       0x1C
#define FEATURE     0x1D

/* SPI Commands  */
#define R_REGISTER          0x00
#define W_REGISTER          0x20
#define R_RX_PAYLOAD        0x61
#define W_TX_PAYLOAD        0xA0
#define FLUSH_TX            0xE1
#define FLUSH_RX            0xE2
#define REUSE_TX_PL         0xE3
#define R_RX_PL_WID         0x60
#define W_ACK_PAYLOAD       0xA8 
#define W_TX_PAYLOAD_NOACK  0xB0
#define NOP                 0xFF

/* CONFIG Register bit definitions */
#define MASK_RX_DR    BIT(6)
#define MASK_TX_DS    BIT(5)   
#define MASK_MAX_RT   BIT(4)  
#define EN_CRC        BIT(3)
#define CRCO          BIT(2)
#define PWR_UP        BIT(1)
#define PRIM_RX       BIT(0)

/* EN_AA register bit definitions */
#define ENAA_P5  BIT(5)
#define ENAA_P4  BIT(4)
#define ENAA_P3  BIT(3)
#define ENAA_P2  BIT(2)
#define ENAA_P1  BIT(1)
#define ENAA_P0  BIT(0)

/* EN_RXADDR  register bit definitions */
#define ERX_P5  BIT(5)
#define ERX_P4  BIT(4)
#define ERX_P3  BIT(3)
#define ERX_P2  BIT(2)
#define ERX_P1  BIT(1)
#define ERX_P0  BIT(0)

/* SETUP_AW register */
#define AW 0x02
                    
/* SETUP_RETR register */
#define ARD 0xF0
#define ARC 0x0F

/* RF_SETUP register */
#define CONT_WAVE   BIT(7)
#define RF_DR_LOW   BIT(5)
#define PLL_LOCK    BIT(4)
#define RF_DR_HIGH  BIT(3)
#define RF_PWR      0x06
                            
/* STATUS register */
#define RX_DR       BIT(6)
#define TX_DS       BIT(5)
#define MAX_RT      BIT(4)
#define RX_P_NO     0x0E
#define TX_FULL     BIT(0)

/* OBSERVE_TX */
#define PLOS_CNT    0xF0
#define ARC_CNT     0x0F

/* RPD register */
#define RPD_MASK    BIT(0)

/* register FIFO_STATUS */
#define TX_REUSE        BIT(6)
#define TX_FIFO_FULL    BIT(5)
#define TX_EMPTY        BIT(4)
#define RX_FULL         BIT(1)
#define RX_EMPTY        BIT(0)
                                
/* DYNPD register */
#define DPL_P5  BIT(5)
#define DPL_P4  BIT(4)
#define DPL_P3  BIT(3)
#define DPL_P2  BIT(2)
#define DPL_P1  BIT(1)
#define DPL_P0  BIT(0)

/* FEATURE register */
#define EN_DPL      BIT(2)
#define EN_ACK_PAY  BIT(1)
#define EN_DYN_ACK  BIT(0)

#endif
