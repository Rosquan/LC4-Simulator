/*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include "loader.h"

// memory array location
unsigned short memoryAddress;

/*
 * Read an object file and modify the machine state as described in the writeup
 */
int ReadObjectFile(char* filename, MachineState* CPU) {
    FILE* file;
    unsigned short word; // Current word we are looking at
    unsigned short numContents; // Number of contents in a header
    int i; // For loop counter
    
    file = fopen(filename, "rb"); // Open in read binary form
    if (file == NULL) { // Error opening object file
        fprintf(stderr, "error2: ReadObjectFile() failed\n");
        fclose(file); // Close file
        return -1; // Failure to Read
    } else {
        // Store info in file into memoryAddress
        while (feof(file) == 0) {
            if (ferror(file) != 0) { // Error during getc()
                fprintf(stderr, "error2: ReadObjectFile() failed\n");
                fclose(file); // Close file
                return -1; // Failure to read
            }
            
            word = (fgetc(file) << 8) | fgetc(file); // Creates the word
            
            if (word == 51934 || word == 56026) { // word = CADE or DADA
                memoryAddress = (fgetc(file) << 8) | fgetc(file); // Get address
                numContents = (fgetc(file) << 8) | fgetc(file); // Get num of contents
                
                for (int i = 0; i < numContents; i++) {
                    CPU -> memory[memoryAddress] = (fgetc(file) << 8) | fgetc(file); // Store instruction
                    memoryAddress++; // Increment to next address line
                }
            } else { // word != CADE or DADA
                continue;
            }

        }
        
        fclose(file); // Close file
        return 0; // Successful Read
    }
}
