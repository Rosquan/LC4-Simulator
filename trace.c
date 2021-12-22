/*
 * trace.c: location of main() to start the simulator
 */

#include "loader.h"

// Global variable defining the current state of the machine
MachineState* CPU;

int main(int argc, char** argv)
{
    FILE* output_file; // Output file
    int i = 1; // Counter for arguments
    int state = 0; // Machine State
    CPU = malloc(sizeof(MachineState)); // Allocate memory for CPU
    memset(CPU, 0, sizeof(MachineState)); // Set memory contents to zero
    
    if (argc < 3) { // Filename and an obj not written
        fprintf(stderr, "Error: <filename.txt> and <first.obj> not written\n");
        free(CPU);
        return -1;
    } else { // Something written as argument
        output_file = fopen(argv[1], "w");
        if (output_file == NULL) { // Check if successful open
            fprintf(stderr, "Error: <filename.txt> could not be open\n");
            fclose(output_file);
            free(CPU);
            return -1;
        }
        
        for (i = 2; i < argc; i++) { // Write all data into memory
            if (ReadObjectFile(argv[i], CPU) == 1) {
                free(CPU);
                return -1; // Error during ReadObjectFile()
            } 
        }
        
    }
    
    Reset(CPU);
    ClearSignals(CPU);
    
    while (UpdateMachineState(CPU, output_file) == 0) {
        continue;
    }


    fclose(output_file); // Close file 
    free(CPU); // Free up memory
    return 0;
}