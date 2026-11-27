###
#
#
# Args :    
#
# Returns :
#
# Raises :

import csv

class KeyFootprint():

    def __init__(self):
        self.candidateBits = -1

    def setCandidateBits(self, candidateBits):
        ###
        #
        # Args :    self            ;
        #           candidateBits   ; List of available bits for footprint generation
        #
        # Returns : status          ; 0 if successful set, 1 otherwise
        #
        # Raises :  None

        self.candidateBits = candidateBits
        return 0
    
    def computeKey(self, rawData):
        ###
        #
        # Args :    self    ;
        #           rawData ; 
        #
        # Returns : key     ; 
        #
        # Raises :  None

        computedKey = []
        data = csv.reader(open(rawData, "r"))
        k = 0
        for row in data:
            for col in row:
                keyData = -1
                if(self.candidateBits[k] == 1):
                    if(float(col) == 1.0):
                        keyData = 1
                    elif(float(col) == 0.0):
                        keyData = 0
                computedKey.append(keyData)
                k = k + 1
        return computedKey