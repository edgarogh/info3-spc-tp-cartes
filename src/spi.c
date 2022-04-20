// Copyright © 2022 Edgar Onghena <dev@edgar.bzh>
// Ce fichier est sous license APACHE-2.0
// http://www.apache.org/licenses/LICENSE-2.0

// Spec:
// GPIO: https://polytech.gricad-pages.univ-grenoble-alpes.fr/info/info3/spc/doc/en.DM00135183.pdf#%5B%7B%22num%22%3A286%2C%22gen%22%3A0%7D%2C%7B%22name%22%3A%22XYZ%22%7D%2C67%2C585%2Cnull%5D
//  SPI: https://polytech.gricad-pages.univ-grenoble-alpes.fr/info/info3/spc/doc/en.DM00135183.pdf#%5B%7B%22num%22%3A1426%2C%22gen%22%3A0%7D%2C%7B%22name%22%3A%22XYZ%22%7D%2C67%2C754%2Cnull%5D

#include "sys/devices.h"
#include "spi.h"

// On décide d'utiliser SPI2 mais il existe 3 autres interfaces SPI possibles
#define SPI SPI2_I2S2
#define GPIO GPIOB

#define WRITE_REG(field, bitCount, value) field = ((value<<bitCount*12U) | (value<<bitCount*15U) | (value<<bitCount*13U))
#define WRITE_GPIO(field, bitCount, value) WRITE_REG(GPIO.field, bitCount, value)

void spiBegin() {
    // Horloges
    RCC.APB1ENR |= (1<<14);
    RCC.AHB1ENR |= (1<<1);

    // Pins (même config pour les 3 pins : B12, B13, B15)
    WRITE_GPIO(MODER, 2, 0b10);
    GPIO.AFRH |= ((0b0101<<(4*4)) | (0b0101<<(4*7)) | (0b0101<<(4*5)));
    WRITE_GPIO(PUPDR, 2, 0b01);
    WRITE_GPIO(OSPEEDR, 2, 0b10);

    // SPI hardware
    SPI.CR1 = 0b0000100000001100;
    //            │ │     │││└Bit 2: Master selection = master
    //            │ │     └┴┴Bits 5-3: Baud rate = expérimentalement défini au plus rapide qui semble marcher
    //            │ └Bit 11: Data frame format = 16 bits
    //            └Bit 13: Output enable in bidirectional mode

    // Activation du SSOE (pour contrôler le slave-select manuellement avec `spiSetSs`)
    // Bit 2 SSOE: SS output enable
    SPI.CR2 |= (1 << 2);
}

/// Renvoie `true` si un symbole est encore dans le registre de données, d'après le registre de status
__really_inline__ bool spiBusy() {
    // Bit 7 BSY: Busy flag
    return (SPI.SR & (1 << 7)) != 0;
}

/// Active ou désactive le contrôle de l'esclave SPI en définissant l'état du "slave select"
void spiSetSs(bool high) {
    if (high) {
        __set_bit(SPI.CR1, 6);
    } else {
        // Il ne faut pas clore la transaction avant la fin de la communication
        while (spiBusy());

        __clr_bit(SPI.CR1, 6);
    }
}

/// Renvoie `true` si un symbole est encore dans le registre de données, d'après le registre de status
__really_inline__ bool spiTransmitting() {
    // Bit 1 TXE: Transmit buffer empty
    return (SPI.SR & (1 << 1)) == 0;
}

/// Transmet deux octets d'un coup. Requiert que le SPI soit configuré en "data frame format" = 16bits
void spiTransfer16(uint8_t octet0, uint8_t octet1) {
    uint16_t to_send = octet1;
    to_send |= octet0 << 8;

    while (spiTransmitting());

    SPI.DR = to_send;
}
