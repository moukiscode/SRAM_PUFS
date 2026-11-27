import csv
import sys

###
#
#
# Args :    carte.csv           ;   Raw SRAM data to be analysed
#
# Returns : Stacked raw data    ;   Float matrix    ;   Ranging from 0 to N, 0 is bit always at 0, N is bit always at 1 (across all SRAMs)
#           Computed data       ;   Bool matrix     ;   0 if raw data is 0 or N, 1 otherwise (stable bits filtering)
#
# Raises :  SA001                ;   No csv file to analyse
#           SAW01                ;   Single dataset analysis

class SRAManalyser:

    def __init__(self):
        print("Key footprint generator init...")
        self.FILES = []
        self.stackedRawData = -1
        self.stackedData = -1
        self.MATRIX_SIZE = 64
    
    def addFile(self, newFile):
        ### Adds new file(s) to the dataset
        #
        #
        # Args :    self    ;
        #           newFile ; String or List ; The new file(s) to be added
        #
        # Returns : status  ; Wether the addition was successful. 0 on success, 1 on failure
        #
        # Raises :  None

        status = 1
        if(type(newFile) == type([])):
            for file in newFile:
                self.FILES.append(file)
            status = 0
        elif(type(newFile) == type("")):
            self.FILES.append(newFile)
            status = 0
        return status

    def stackRawData(self):
        ### Stacks and return the raw SRAM data
        #
        #
        # Args :    self            ;
        #
        # Returns : stackedRawData  ; The result of the stacking of individual SRAM data
        #None
        # Raises :  SA001           ; No CSV file given, cannot compute
        #           SAW01           ; Single dataset analysis

        if(len(self.FILES) == 0):
            print("FAIL SA001 : Cannot compute data (no csv file given)")
        else:
            if(len(self.FILES) == 1):
                print("WARN SAW01 : RAM analysis on a single dataset")

            print("Stacked raw data computing")
            print(f"Using {self.FILES}")

            self.stackedRawData = []
            for k in range(self.MATRIX_SIZE*self.MATRIX_SIZE):
                self.stackedRawData.append(-1)

            for f in self.FILES:
                data = csv.reader(open(f, "r"))
                k = 0
                for row in data:
                    for col in row:
                        if (self.stackedRawData[k] == -1):
                            self.stackedRawData[k] = float(col)
                        else:
                            self.stackedRawData[k] = self.stackedRawData[k] + float(col)
                        k = k + 1
        return self.stackedRawData
    
    def computedData(self, givenRawData = -1):
        ### Computes the raw SRAM data and returns a matrix of non constant bit across all given datasets
        #
        #
        # Args :    self        ;
        #
        # Returns : stackedData ; The result of the stacking of individual SRAM data
        #
        # Raises :  SA001       ; No CSV file given, cannot compute
        #           SA002       ; Raw data not stacked, cannot compute
        #           SAW01       ; Single dataset analysis

        if(givenRawData == -1):
            givenRawData = self.stackedRawData

        if(len(self.FILES) == 0):
            print("FAIL SA001 : Cannot compute data (no csv file given)")
            self.stackedData = -1
        elif(givenRawData == -1):
            print("FAIL SA002 : No raw data to compute")
        else:
            if(len(self.FILES) == 1):
                print("WARN SAW01 : RAM analysis on a single dataset")

            self.stackedData = []
            for k in range(self.MATRIX_SIZE*self.MATRIX_SIZE):
                self.stackedData.append(-1)

            for bit in range(len(givenRawData)):
                if(givenRawData[bit] == 0):
                    self.stackedData[bit] = 0
                elif (givenRawData[bit] == len(self.FILES)):
                    self.stackedData[bit] = 0
                else:
                    self.stackedData[bit] = 1
        return self.stackedData
