#include "hal.h"
#include "simpleserial.h"
#include <stdint.h>
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------
// PARAMETRES DE LA LECTURE PAR BLOCS
// ---------------------------------------------------------------
// TAILLE_BLOC doit etre IDENTIQUE a TAILLE_BLOC dans le programme
// Python. Elle doit diviser TAILLE_ZONE (512 / 32 = 16 blocs).
// Le protocole simpleserial accepte jusqu'a ~190 octets par reponse,
// donc 32 octets ne pose aucun probleme.
#define TAILLE_ZONE     512                          // nombre d'octets de SRAM analyses
#define TAILLE_BLOC     32                           // nombre d'octets envoyes par reponse
#define NB_BLOCS        (TAILLE_ZONE / TAILLE_BLOC)  // = 16 blocs
// Section ".noinit" : le demarrage du programme ne met PAS ces octets a zero.
// On retrouve donc dans "zone" la valeur de SRAM obtenue juste apres
// la mise sous tension (c'est ce qu'on veut mesurer : l'empreinte PUF).
#define NOINIT __attribute__((section(".noinit")))

NOINIT uint8_t zone[TAILLE_ZONE];
uint8_t salted_key[SALTED_KEY_SIZE]


// ---------------------------------------------------------------
// Commande 'r' : le PC envoie 1 octet = NUMERO DE BLOC (et non plus
// un numero d'octet). On renvoie les TAILLE_BLOC octets du bloc.
//   bloc 0 -> octets   0 a  31
//   bloc 1 -> octets  32 a  63
//   ...
//   bloc 15 -> octets 480 a 511
// ---------------------------------------------------------------
static uint8_t authentificate(uint8_t *data , uint8_t dlen)
{
    uint16_t puf_state = 0;
    for(int i = 0; i < dlen; i++){
        if((data[i] & 0xf0 < 0x40) || (data[i] & 0x0f < 0x40)){
            puf_state |= zone[((data[i] & 0xf0) * 8) + (data[i] & 0x0f)] << i;
        }
    }

    // Envoie TAILLE_BLOC octets au PC, en partant du debut du bloc.
    // 'w' = commande de reponse, que Python lit avec simpleserial_read('w', ...)
    simpleserial_put('w', 2, puf_state);

    return 0x00;   // 0x00 = tout s'est bien passe
}

////////////////////////////////////////////////////////////////////
int main(void)
{
    if(sodium_init() < 0){
        while(1);
    }
    platform_init();
    init_uart();
    trigger_setup();
    simpleserial_init();

    // Declare la commande 'r' : elle attend 1 octet de donnees (le numero de bloc)
    simpleserial_addcmd('a', 64 ,authentificate);

    // Boucle infinie : on attend les commandes envoyees par le PC
    while (1) {
        simpleserial_get();
    }
}
