import sys
import csv
import random
import binascii

from Crypto.Protocol.KDF import HKDF
from Crypto.Hash import HMAC, SHA256
from Crypto.Cipher import AES

from random import randint

SHARED_KEY = b"cleftresecurisee"
HMAC_KEY = b"asfpsenvdsmaoxnsjdkflezr"

def generate_key(keyType, salt=None):
    ### Key generation method
    #
    #
    # Args :    keyType ; Type of the key to be generated => 1 is SALT, 2 is salted key
    #           salt    ; Optionnal ; The SATL for the generation of the salted key
    # Returns : key     ; The result of the key gen. Can be SALT or salted key
    # Raises :  K001    ; Salted keye generation but no SALT was given
    #           K002    ; Key type does not correspond to an actual key

    key = b'' ### PLACEHOLDER
    if(keyType == 1):   ### Key type is random public key ( is SALT )
        hexString = [b'0', b'1', b'2', b'3', b'4', b'5', b'6', b'7', b'8', b'9', b'a', b'b', b'c', b'd', b'e', b'f']
        for k in range(31):
            key = key + hexString[random.randint(0, 15)]
    elif(keyType == 2): ### Key type is salted key
        if(salt == None):
            raise RuntimeError(f"K001 : La génération d'une clef salée nécessite un sel")
        key = HKDF(master=SHARED_KEY, key_len=16, salt=salt, hashmod=SHA256, num_keys=1)
    else:
        raise RuntimeError(f"K002 : Type de clef ({keyType}) invalide")
    return key

def CT(PlainText, saltedKey):
    ### Cipher text generation method
    #
    #
    # Args :    PlainText   ; Data to encrypt
    #           saltedKey   ; The encryption key
    # Returns : ciphertext  ; The result of the encryption. Contains the nonce, the CT and the HMAC tag
    # Raises :  None

    cipher = AES.new(saltedKey, AES.MODE_CTR)

    ciphertext = cipher.encrypt(PlainText)
    tag = HMAC.new(HMAC_KEY, cipher.nonce + ciphertext, digestmod=SHA256).digest()

    print(f"Nonce size : {len(cipher.nonce)}\nCT size : {len(ciphertext)}\nTag size : {len(tag)}")

    return cipher.nonce + ciphertext + tag

def PT(ciphertext, saltedKey, nonce, tag):
    ### Plain text generation method
    #
    #
    # Args :    ciphertext  ; Data to decrypt
    #           saltedKey   ; The encryption key
    #           nonce       ; The nonce used for encryption
    #           tag         ; The HMAC tag to validate the CT
    # Returns : PlainText   ; The result of the decryption.
    # Raises :  None

    HMAC.new(HMAC_KEY, nonce + ciphertext, digestmod=SHA256).verify(tag)

    PlainText = AES.new(saltedKey, AES.MODE_CTR, nonce=nonce).decrypt(ciphertext)

    return PlainText

def main():
    ######################################################### PHASE 1 : ENVOI DU SEL #####################################################################

    key = generate_key(1)

    ######################################################### PHASE 2 : CREATION DE LA CLEF SALEE ########################################################
    saltedKey = generate_key(2, key)
    data = b"SRAM PUF"

    print(f"Onigiri PT : {data}")

    ciphered = CT(data, saltedKey)
    
    print(f"Cipher text ({len(ciphered)} bytes) : \n {ciphered}")

    with open("ciphertext.txt", "wb") as f:
        f.write(ciphered)
        print("Written cipher text to ciphertext.txt")
    
    with open("ciphertext.txt", "rb") as f:
        cipheredRead = f.read()

    plain = PT(cipheredRead[8:-32], saltedKey, cipheredRead[:8], cipheredRead[-32:])

    print(f"Plain text ({len(plain)} bytes) : {plain}")

main()