import csv
import random

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
elif(cmd == "a" or cmd == "auto"):
    print(f"Automatic mode not yet implemented")
else:
    print(f"'{cmd}' is not a command")