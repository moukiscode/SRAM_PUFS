import sys
import csv
import random
import binascii
import os

from Crypto.Protocol.KDF import HKDF
from Crypto.Hash import HMAC, SHA256
from Crypto.Cipher import AES

from random import randint

SHARED_KEY = b"cleftresecurisee"
HMAC_KEY = b"asfpsenvdsmaoxnsjdkflezr"
INFO = b"secu mat"
NONCE = bytes([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12])

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
        key = os.urandom(16)
    elif(keyType == 2): ### Key type is salted key
        if(salt == None):
            raise RuntimeError(f"K001 : La génération d'une clef salée nécessite un sel")
        okm = HKDF(master=SHARED_KEY, key_len=48, salt=NONCE, hashmod=SHA256, num_keys=1, context=INFO)
        key = okm[:16]
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
    iv = os.urandom(16)
    cipher = AES.new(saltedKey, AES.MODE_CTR, initial_value=int.from_bytes(iv, "big"), nonce=b"")

    ciphertext = cipher.encrypt(PlainText)
    tag = HMAC.new(HMAC_KEY, iv + ciphertext, digestmod=SHA256).digest()

    print(f"Nonce size : {len(cipher.nonce)}\nCT size : {len(ciphertext)}\nTag size : {len(tag)}")

    return iv + ciphertext + tag

def PT(ciphertext, saltedKey, iv, tag):
    ### Plain text generation method
    #
    #
    # Args :    ciphertext  ; Data to decrypt
    #           saltedKey   ; The encryption key
    #           nonce       ; The nonce used for encryption
    #           tag         ; The HMAC tag to validate the CT
    # Returns : PlainText   ; The result of the decryption.
    # Raises :  None

    HMAC.new(HMAC_KEY, iv + ciphertext, digestmod=SHA256).verify(tag)

    PlainText = AES.new(saltedKey, AES.MODE_CTR, nonce=b"", initial_value=iv).decrypt(ciphertext)

    return PlainText

def main():
    ######################################################### PHASE 1 : ENVOI DU SEL #####################################################################

    key = generate_key(1)
    
    print(f"NONCE : {NONCE}")

    with open("PT_transLine.bin", "wb") as f:
        f.write(NONCE)
        f.close()

    ######################################################### PHASE 2 : CREATION DE LA CLEF SALEE ########################################################
    saltedKey = generate_key(2, key)
    print(f"AES key : {saltedKey}")
    with open("AES_cheat.bin", "wb") as f:
        f.write(saltedKey)
        f.close()

    data = b"\x41\x42\x43\x44\x45\x46\x47\x48\x49\x50\x51\x52\x53\x54\x55\x56"

    print(f"Onigiri PT : {data}")

    ciphered = CT(data, saltedKey)
    
    print(f"Cipher text ({len(ciphered)} bytes) : \n {ciphered}")

    with open("ciphertext.bin", "wb") as f:
        f.write(ciphered)
        print("Written cipher text to ciphertext.bin")
    "N/A"#
    with open("ciphertext.bin", "rb") as f:
        cipheredRead = f.read()

    plain = "N/A"#PT(cipheredRead[8:-32], saltedKey, cipheredRead[:8], cipheredRead[-32:])

    print(f"Plain text ({len(plain)} bytes) : {plain}")

main()