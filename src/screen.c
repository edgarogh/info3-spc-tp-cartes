// Copyright © 2022 Edgar Onghena <dev@edgar.bzh>
// Ce fichier est sous license APACHE-2.0
// http://www.apache.org/licenses/LICENSE-2.0

#include <stdbool.h>
#include <string.h>

// Taille du buffer d'envoi (4 écrans * 2 octets par écran)
#define BUFFER_SIZE 8

#include "spi.h"
#include "screen.h"
#include "screen_glyphs.h"

uint8_t screen_spi_buffer[BUFFER_SIZE];

/// Écrit le contenu du buffer d'envoi sur la ligne SPI
static void screen_commit(void) {
    // J'ai honte de cette ligne, mais je peux vraiment pas faire autrement
    // sinon les "commits" sont trop proches et ça casse le protocole.
    // Il y a probablement moyen de faire plus propre en lisant un registre
    // de status, mais je ne l'ai toujours pas trouvé.
    for (volatile int i = 0; i < 10; i++);

    spiSetSs(true);
    for (uint8_t i = 0; i < BUFFER_SIZE; i += 2) {
        spiTransfer16(screen_spi_buffer[i], screen_spi_buffer[i+1]);
    }
    spiSetSs(false);
}

static void screen_setRow(uint8_t dev, uint8_t row, uint8_t value) {
    dev *= 2;
    screen_spi_buffer[dev + 0] = row + 1;
    screen_spi_buffer[dev + 1] = value;
}

// Envoie un code de contrôle aux 4 écrans
static void screen_control(uint8_t code, uint8_t arg) {
    for (uint8_t i = 0; i < BUFFER_SIZE; i += 2) {
        screen_spi_buffer[i + 0] = code;
        screen_spi_buffer[i + 1] = arg;
    }

    screen_commit();
}

/// Met en place le SPI et initialise l'écran
void screen_init(void) {
    spiBegin();

    screen_control(15, 0); // Display test disabled
    screen_control(11, 8 - 1); // Scan limit 8
    screen_control(10, 15); // Maximum led intensity
    screen_control(9, 0); // Decode mode disabled
    screen_control(12, 1); // Shutdown disabled
}

union {
    uint32_t i[8];
    uint8_t b[32];
} screen_buffer;

void screen_writeCounter(uint32_t tenths) {
    memset(screen_buffer.b, 0, 32);

    // Dixièmes
    for (uint8_t row = 0; row < 8; row++) {
        screen_buffer.i[row] = ((uint32_t) TENTHS[tenths % 10][row]);
    }

    uint32_t offset = 8;
    uint32_t time_units = tenths / 10;
    bool zero = time_units == 0;

    // Unités
    if (zero) {
        for (uint8_t row = 0; row < 8; row++) {
            screen_buffer.i[row] |= (((uint32_t) INTEGERS[0][row]) << offset);
        }
    } else {
        while (time_units > 0 && offset < 28) {
            uint8_t digit = time_units % 10;
            for (uint8_t row = 0; row < 8; row++) {
                screen_buffer.i[row] |= (((uint32_t) INTEGERS[digit][row]) << offset);
            }
            offset += INTEGER_WIDTHS[digit];
            time_units /= 10;
        }
    }

    for (uint8_t row = 0; row < 8; row++) {
        for (uint8_t dev = 0; dev < 4; dev++) {
            screen_setRow(3 - dev, row, screen_buffer.b[row * 4 + dev]);
        }

        screen_commit();
    }
}

bool screen_isOn = true;
void screen_setOn(bool on) {
    if (on != screen_isOn) {
        screen_control(12, on);
        screen_isOn = on;
    }
}
