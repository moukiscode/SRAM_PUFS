#include "hal.h"
#include "simpleserial.h"
#include <stdint.h>

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

#define TAILLE_ZONE  512
#define TAILLE_BLOC  32
#define NB_BLOCS     (TAILLE_ZONE / TAILLE_BLOC)
#define NOINIT __attribute__((section(".noinit")))

NOINIT uint8_t zone[TAILLE_ZONE];

static uint8_t read_memory(uint8_t *data , uint8_t dlen)
{
    uint8_t bloc = data[0];

    if (bloc >= NB_BLOCS) {
        return 0x01;
    }

    simpleserial_put('w', TAILLE_BLOC, zone + (bloc * TAILLE_BLOC));

    return 0x00;
}

int main(void)
{
    platform_init();
    init_uart();
    trigger_setup();
    simpleserial_init();

    simpleserial_addcmd('r', 1 ,read_memory);

    while (1) {
        simpleserial_get();
    }
}
