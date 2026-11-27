###
#
#
# Args :    
#
# Returns :
#
# Raises :

import SRAMhandler as sram
from pathlib import Path

fileList = []
keylist = []
handler = sram.SRAMhandler()

def fileAdd():
    global fileList
    print("Relative path to raw datasets. q to exit")
    fpString = ""
    while(fpString != "q"):
        fpString = input(">>> filepath : ")
        if(fpString == "auto"):
            normalisedFP = "../rawData/carte_"
            fpWd = 1
            k = 1
            while(fpWd == 1):
                testedPath = f"../rawData/carte_{k}.csv"
                fp = Path(testedPath)
                if(fp.exists()):
                    fileList.append(testedPath)
                    print(f">>> Filepath crawler : {testedPath}")
                    k = k + 1
                else:
                    fpWd = 0
            print(f">>> Filepath crawler : automatic fetching complete, quitting")
            fpString = "q"
        else:
            fp = Path(fpString)
            if(fp.exists()):
                fileList.append(fpString)
            elif(fpString != "q"):
                print(f"FilePathError : '{fpString}' does not exist")
            
def dataExtract():
    global fileList
    global keylist
    print(f"Extracting key from {len(fileList)} files")
    keylist = handler.keyExtraction(fileList)

def dataStore():
    global fileList
    global keylist
    handler.keyStoring()

def main():
    print("SRAM data controller")
    alive = 1
    while(alive == 1):
        cmd = input(">>> ")
        match cmd:
            case "newFile":
                fileAdd()
            case "extract":
                dataExtract()
            case "store":
                dataStore()
            case "quit":
                alive = 0
            case _:
                print(f"CommandError : Could not recognise '{cmd}'")

main()

