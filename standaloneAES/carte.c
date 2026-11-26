#include <stdint.h>
#include <stdio.h>
#include "aes.c"
#include "hkdf.c"

#define CTR 1
#define SALTED_KEY_SIZE 16
#define NONCE_SIZE 12
#define IV_SIZE 16

#define CT_FILE_SIZE 64
#define CT_SIZE 16

uint8_t NONCE[12]; /* nonce prédéfini = salt */
static const uint8_t INFO[] = "secu mat";
static const uint8_t SHARED_KEY[] = "cleftresecurisee";

uint8_t salted_key[SALTED_KEY_SIZE];
uint8_t iv[IV_SIZE];
uint8_t okm[48];
hkdf_ctx ctx;

static uint8_t salted_key_generation(/*uint8_t *salt , uint8_t saltlen*/){
    if (hkdf_ctx_init(&ctx, NONCE, sizeof(NONCE), INFO, sizeof(INFO)-1) != HKDF_OK ||
        hkdf_ctx_derive(&ctx, SHARED_KEY, sizeof(SHARED_KEY)-1, okm, sizeof(okm)) != HKDF_OK) {
        fprintf(stderr, "dérivation échouée\n"); 
        return 1;
    }

    printf("Derivated AES key : ");
    for(int i = 0;i < sizeof(okm); i++){
        if(i < 16){
            printf("%2x ", okm[i]);
            salted_key[i] = okm[i];
        }
        else if(i < 48){
        }
    }
    printf("\n");
    return 0;
}

static uint8_t decipher(uint8_t *data , uint8_t dlen, const uint8_t *iv){
    struct AES_ctx AESctx;

    AES_init_ctx_iv(&AESctx, salted_key, iv);
    AES_CTR_xcrypt_buffer(&AESctx, data, dlen);
    return 0;
}

int main(void)
{
    FILE *fptr;
    char fileContents[CT_FILE_SIZE]; // 104 bytes for actual PUF
    //char salt[16];
    //char saltK[16];
    uint8_t PTCT[CT_SIZE];
    hkdf_ctx ctx;

    (void)ctx;

    fptr = fopen("PT_transLine.bin", "rb");
    fread(fileContents, sizeof(uint8_t), CT_FILE_SIZE, fptr);
    fclose(fptr);

    printf("Received NONCE : ");
    for(int i = 0; i < NONCE_SIZE; i++){
        printf("%2x ", fileContents[i] & 0xff);
        NONCE[i] = fileContents[i] & 0xff;
    }
    printf("\n");

    salted_key_generation();

    fptr = fopen("AES_cheat.bin", "rb");
    fread(fileContents, sizeof(uint8_t), CT_FILE_SIZE, fptr);
    fclose(fptr);

    printf("Expected AES key :  ");
    for(int i = 0; i < SALTED_KEY_SIZE; i++){
        printf("%2x ", fileContents[i] & 0xff);
        //salted_key[i] = fileContents[i] & 0xff;      // AES CHEAT
    }
    printf("\n");
    
    fptr = fopen("ciphertext.bin", "rb");
    fread(fileContents, sizeof(uint8_t), CT_FILE_SIZE, fptr);
    fclose(fptr);

    for(int i = 0; i < CT_FILE_SIZE; i++){
        if(i < IV_SIZE){
            iv[i] = fileContents[i];
        }
        else if(i < CT_SIZE + IV_SIZE){
            PTCT[i-IV_SIZE] = fileContents[i];
        }
    }

    decipher(PTCT, sizeof(PTCT), iv);
    //decipher(ct, ctl, iv);

    printf("Deciphered PT : ");
    for(int i = 0; i < sizeof(PTCT); i++){
        printf("%2x ", PTCT[i]);
    }
    printf("\n");

    fwrite(PTCT, 1, sizeof(PTCT), stdout);
    fputc('\n', stdout);

    /*
    fptr = fopen("ciphertext.bin", "r");
    fgets(fileContents, 64, fptr);
    fclose(fptr); 
    
    for(int i = 0; i<64; i++){
        printf("%x ", fileContents[i] & 0xff);
    }
    printf("\n");

    fptr = fopen("PT_transLine.bin", "r");
    fgets(salt, 16, fptr);
    fclose(fptr);

    printf("Fetched SALT : ");
    for(int i = 0; i<sizeof(salt); i++){
        printf("%x ", salt[i] & 0xff);
    }
    printf("\n");

    for(int i = 0; i < sizeof(fileContents); i++){
        if(i < IV_SIZE){
            iv[i] = fileContents[i];
        }
        else if(i < 24){
            PTCT[i-IV_SIZE] = fileContents[i];
        }
    }

    printf("IV from transmission : ");
    for(int i = 0; i<IV_SIZE; i++){
        printf("%x ", iv[i] & 0xff);
        ctx.nonce[i] = NONCE[i];
    }
    printf("\n");
    ctx.nonce_len = sizeof(NONCE);

    salted_key_generation((uint8_t *)salt, sizeof(salt));
i < 24
    fptr = fopen("AES_cheat.bin", "r");
    fgets(saltK, 16, fptr);
    fclose(fptr);

    printf("Python AES key : ");
    for(int i = 0; i<sizeof(saltK); i++){
        printf("%x ", saltK[i] & 0xff);
        salted_key[i] = saltK[i];  
    }
    printf("\n");

    printf("Generated AES key : ");
    for(int i = 0; i<sizeof(salted_key); i++){
        printf("%x ", salted_key[i] & 0xff);
    }
    printf("\n");



    printf("Received CT : ");
    for(int i = 0; i<sizeof(PTCT); i++){
        printf("%x ", PTCT[i] & 0xff);
    }
    printf("\n");

    decipher(PTCT, sizeof(PTCT));

    printf("Deciphered PT : ");
    for(int i = 0; i<sizeof(PTCT); i++){
        printf("%x ", PTCT[i] & 0xff);
    }
    printf("\n");
    */

    return 0;
}