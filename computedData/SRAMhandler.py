###
#
#
# Args :    
#
# Returns :
#
# Raises :

import KeyFootprint as footprint
import SRAManalyser as analyser
import csv

class SRAMhandler():

    def __init__(self):
        self.keys = []

    def keyExtraction(self, fileList):
        ### Extracts the identification keys from the given files
        #
        #
        #
        # Args :    self    ;
        #           fileList; Input files for the key extraction
        #
        # Returns : keys    ; The list of computed keys, in the same order as the input files
        #
        # Raises :  None

        if(len(fileList) == 0):
            print("SH0001 : No file given. Cannot extract key")
        else:
            analyse = analyser.SRAManalyser()
            fprint = footprint.KeyFootprint()

            analyse.addFile(fileList)
            analysedData = analyse.computedData(analyse.stackRawData())

            fprint.setCandidateBits(analysedData)

            for file in fileList:
                self.keys.append(fprint.computeKey(file))

        return self.keys

    def keyStoring(self):
        ### Stores the keys ini their own csv file
        #
        #
        # Args :    self    ;
        #
        # Returns : status  ; 0 if storage was successful, 1 otherwise
        #
        # Raises :  None
        
        keyNumber = 1
        for key in self.keys:
            with open(f"key_{keyNumber}.csv", "w") as keyFile:
                writer = csv.writer(keyFile)
                writer.writerows([key])
                print(f">>> Store : Wrote 'key_{keyNumber}.csv'")
                keyNumber = keyNumber + 1

