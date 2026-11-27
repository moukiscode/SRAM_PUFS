import chipwhisperer as cw
import subprocess
import sys
import csv
import random

##########################################################################################################################################
subprocess.run(["make", "PLATFORM=CW308_STM32F3"], check=True)
##########################################################################################################################################
scope = cw.scope()
target = cw.target(scope)
scope.default_setup()

##########################################################################################################################################
prog = cw.programmers.STM32FProgrammer
cw.program_target(scope, prog, "puf_sram-CW308_STM32F3.hex")

##########################################################################################################################################

## TODO : Écrivez ici le programme pour récupérer les données.
## Choisissez la quantité de données que vous souhaitez extraire de la SRAM

scope.io.target_pwr = True

print("Plain Text Authentification")
print("Manual or automatic mode (manual/auto)?")
cmd = input(">>> ")

if(cmd == "m" or cmd == "manual"):
    print("Card number to test for")
    keyNumber = input(">>> ")
    cardFilePath = f"../computedData/key_{keyNumber}.csv"
    data = csv.reader(open(cardFilePath, "r"))
    key = []
    for row in data:
        for col in row:
            key.append(col)
    query = []
    for k in range(16):
        byteNumber = random.randint(0, 8)
        bitNumber = random.randint(0, 8)
        while(key[(byteNumber*8)+bitNumber] == -1 or (byteNumber, bitNumber) in query):
            byteNumber = random.randint(0, 8)
            bitNumber = random.randint(0, 8)
        query.append((byteNumber, bitNumber))
    for k in range(len(query)):
        query[k] = (query[k][0]<<4) + query[k][1]
    queryBytes = bytes(query)
    print(f"Query ({len(queryBytes)} bytes) : {queryBytes}")
    target.simpleserial_write('a', queryBytes)
    reponse = target.simpleserial_read('w', 2, timeout = 500)
    print(f"Target response : {response}")
elif(cmd == "a" or cmd == "auto"):
    print(f"Automatic mode not yet implemented")
else:
    print(f"'{cmd}' is not a command")


#target.simpleserial_write('a', octets_num)


##########################################################################################################################################
try:
    scope.dis()
except Exception:
    pass

print("Déconnexion réussie.")
