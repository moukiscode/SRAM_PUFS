#include <stdint.h>
#include <stdio.h>
#include "aes.c"
#include "hkdf.c"

#define SHARED_KEY      "cleftresecurisee"
#define SALTED_KEY_SIZE 16

uint8_t salted_key[SALTED_KEY_SIZE];

static uint8_t salted_key_generation(uint8_t *data , uint8_t dlen){
    
    return 0;
}

static uint8_t decipher(uint8_t *data , uint8_t dlen){
    struct AES_ctx ctx;
    uint8_t iv[16]  = {};

    AES_init_ctx_iv(&ctx, salted_key, iv);
    AES_ctx_set_iv(&ctx, iv);

    AES_CTR_xcrypt_buffer(&ctx, data, dlen);
    return 0;
}

int main(void)
{
    FILE *fptr;
    char fileContents[48]; // 104 bytes for actual PUF

    fptr = fopen("ciphertext.txt", "r");
    fgets(fileContents, 48, fptr);
    fclose(fptr); 
    
    for(int i = 0; i<48; i++){
        printf("%x ", fileContents[i] & 0xff);
    }
    printf("\n");

    //salted_key_generation();

    return 0;
}