// LC4.c: Defines simulator functions for executing instructions
#include "LC4.h"
#include <stdio.h>
void PrintBinary(MachineState* CPU, FILE* output);

#define INSN_OP(I) ((I) >> 12) // Get Opcode
#define INSN_11_9(I) (((I) >> 9) & 0x7) // Get I[11:9]
#define INSN_8_6(I) (((I) >> 6) & 0x7) // Get I[8:6]
#define INSN_5_3(I) (((I) >> 3) & 0x7) // Get I[5:3]
#define INSN_2_0(I) (I & 0x7) // Get I[2:0]

#define INSN_3_0(I) (I & 0xF) // Get I[3:0] = UIMM4
#define INSN_4_0(I) (I & 0x1F) // Get I[4:0] = IMM5
#define INSN_5_0(I) (I & 0x3F) // Get I[5:0] = IMM6
#define INSN_6_0(I) (I & 0x7F) // Get I[6:0] = IMM7
#define INSN_7_0(I) (I & 0xFF) // Get I[7:0] = IMM8
#define INSN_8_0(I) (I & 0x1FF) // Get I[8:0] = IMM9
#define INSN_10_0(I) (I & 0x7FF) // Get I[10:0] = IMM11



/*
 * Helper for printing the binary representation of the instruction
 */
void PrintBinary(MachineState* CPU, FILE* output)
{
     unsigned short value = CPU -> memory[CPU -> PC]; // instruction decimal
     int bit = 0; // counter for bits

    for (bit = 0; bit < 16; bit++) { // print out binary bit by bit
        fprintf(output, "%d", (value >> (15 - bit)) & 0x1);
    }
    
    fprintf(output, " "); // Add space
}


/*
 * Reset the machine state as Pennsim would do
 */
void Reset(MachineState* CPU)
{
     int i = 0; // For loop counters
     CPU -> PC = 0x8200; // Set starting PC to x8200
    
     CPU -> PSR = 0;    // Set PSR to x0000
     for (i = 0; i < 8; i++) { // Set all registers to x0000
          CPU -> R[i] = 0; // Current register value set to x0000
     }
    
     ClearSignals(CPU); // Clear control signals
}


/*
 * Clear all of the control signals (set to 0)
 */
void ClearSignals(MachineState* CPU)
{
    CPU -> rsMux_CTL = 0; // rsMux_CTL = x0000
    CPU -> rtMux_CTL = 0; // rtMux_CTL = x0000
    CPU -> rdMux_CTL = 0; // rdMux_CTL = x0000
    
    CPU -> regFile_WE = 0; // regFile_WE = x0000
    CPU -> NZP_WE = 0; // NZP_WE = x0000
    CPU -> DATA_WE = 0; // DATA_WE = x0000

    CPU -> regInputVal = 0; // regInputVal = x0000
    CPU -> NZPVal = 0; // NZPVal = x0000
    CPU -> dmemAddr = 0; // dmemAddr = x0000
    CPU -> dmemValue = 0; // dmemValue = x0000
}


/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState* CPU, FILE* output)
{
    fprintf(output, "%04X ", CPU -> PC);
    PrintBinary(CPU, output); // print out binary
    
    if (CPU -> regFile_WE == 1) {
        // Print out Register written into and Value written in Register
        fprintf(output, "%d %d %04X ", CPU -> regFile_WE, CPU -> regInputVal, CPU -> R[CPU -> regInputVal]);
    } else {
        // Print out 0 and 0000 for respective sections 
        fprintf(output, "%d %d %04X ", 0, 0, 0);
    }
    
    fprintf(output, "%X ", CPU -> NZP_WE); // NZP_WE value
    
    if (CPU -> NZP_WE == 1) {
        // Print out value written in NZP Register
        fprintf(output, "%d ", CPU -> NZPVal);
    } else {
        // Print out 0000 for section
        fprintf(output, "%d ", 0);
    }
    
    fprintf(output, "%X ", CPU -> DATA_WE); // DATA_WE value
    
    // Print out Data Memory Address and Value Loaded/Stored in Memory and end line
    fprintf(output, "%04X %04X\n", CPU -> dmemAddr, CPU -> dmemValue);
}


/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState* CPU, FILE* output)
{
    // Consider TRAP/RTI/HICONST/CONST/LDR/STR within this function
    unsigned short rs = 0;
    unsigned short rt = 0;
    unsigned short rd = 0;
    int offset = 0;
    int im9 = 0;
    int uim8 = 0;
    unsigned short opcode = INSN_OP(CPU -> memory[CPU -> PC]); // Get Opcode
    
    if (CPU -> PC == 0x80FF) {
        return 1;
        
    } else {
        if (opcode == 0) { // branch
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            } else {
                BranchOp(CPU, output); // Run Branch Parser
                return 0;
            }
            
        } else if (opcode == 1) { // arithmetic
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            } else {
                ArithmeticOp(CPU, output); // Run Arithmetic Parser
                return 0;
            }
            
        } else if (opcode == 2) { // comparative
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            } else {
                ComparativeOp(CPU, output); // Run Comparative Parser
                return 0;
            }
            
        } else if (opcode == 4) { // jump subroutine
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            } else {
                JSROp(CPU, output); // Run Jump Subroutine Parser
                return 0;
            }
            
        } else if (opcode == 5) { // logical
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            } else {
                LogicalOp(CPU, output); // Run Logical Parser
                return 0;
            }
            
        } else if (opcode == 6) { // ldr
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            rs = INSN_8_6(CPU -> memory[CPU -> PC]); // get RS
            rd = INSN_11_9(CPU -> memory[CPU -> PC]); // get RD
            offset = INSN_5_0(CPU -> memory[CPU -> PC]); // IMM6
            
            if (offset >> 5 == 1) { // Sign extend
                offset = offset | 0xFFC0;
            }
            
            if (rd > 7 | rs > 7) { // Invalid registers
                fprintf(stderr, "error: Invalid registers\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            CPU -> rdMux_CTL = 0; // set RD control signal to 0
            CPU -> rsMux_CTL = 0; // set RS control signal to 0
            CPU -> rtMux_CTL = 0; // set RT control signal to 0
            CPU -> regFile_WE = 1; // set register file write enable to 0
            CPU -> NZP_WE = 1; // set NZP write enable to 1
            CPU -> DATA_WE = 0; // set data write enable to 0
            
            CPU -> dmemAddr = (short int)CPU -> R[rs] + offset;
            
            if (((CPU -> PSR) >> 15 != 1 && CPU -> dmemAddr >= 0xA000) || (((CPU -> PSR) >> 15 != 1) && CPU -> dmemAddr >= 0xC000) ||
                (CPU -> dmemAddr >= 0x8000 && CPU -> dmemAddr <= 0x9FFF) || CPU -> dmemAddr < 0x2000) { // Bad values
                fprintf(stderr, "error: Invalid Data Address\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            CPU -> dmemValue = CPU -> memory[CPU -> dmemAddr];
            
            CPU -> R[rd] = CPU -> memory[CPU -> dmemAddr]; // Store from data to RD
            SetNZP(CPU, CPU -> R[rd]); // Set new NZP
            
            CPU -> regInputVal = rd; // regInputVal = register being stored into
            
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1; // PC = PC + 1
            return 0;
            
        } else if (opcode == 7) { // str
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            rs = INSN_8_6(CPU -> memory[CPU -> PC]); // get RS
            rt = INSN_11_9(CPU -> memory[CPU -> PC]); // get RT
            offset = INSN_5_0(CPU -> memory[CPU -> PC]); // IMM6
            
            if (offset >> 5 == 1) { // Sign extend
                offset = offset | 0xFFC0;
            }

            if (rs > 7 | rt > 7) { // Invalid registers
                fprintf(stderr, "error: Invalid registers\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            CPU -> rdMux_CTL = 0; // set RD control signal to 0
            CPU -> rsMux_CTL = 0; // set RS control signal to 0
            CPU -> rtMux_CTL = 1; // set RT control signal to 1
            CPU -> regFile_WE = 0; // set register file write enable to 0
            CPU -> NZP_WE = 0; // set NZP write enable to 1
            CPU -> DATA_WE = 1; // set data write enable to 0
            
            CPU -> dmemAddr = (short int)CPU ->R[rs] + offset;
            
            if (((CPU -> PSR) >> 15 != 1 && CPU -> dmemAddr >= 0xA000) || (((CPU -> PSR) >> 15 != 1) && CPU -> dmemAddr >= 0xC000) ||
                (CPU -> dmemAddr >= 0x8000 && CPU -> dmemAddr <= 0x9FFF) || CPU -> dmemAddr < 0x2000) { // Bad values
                fprintf(stderr, "error: Invalid Data Address\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            CPU -> dmemValue = (short int)CPU -> R[rt];
            CPU -> memory[CPU -> dmemAddr] = CPU -> R[rt]; // store rt in datamem
            
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1; // PC = PC + 1
            return 0;
            
        } else if (opcode == 8) { // RTI
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            CPU -> rdMux_CTL = 0; // set RD control signal to 0
            CPU -> rsMux_CTL = 0; // set RS control signal to 0
            CPU -> rtMux_CTL = 0; // set RT control signal to 0
            CPU -> regFile_WE = 0; // set register file write enable to 0
            CPU -> NZP_WE = 0; // set NZP write enable to 1
            CPU -> DATA_WE = 0; // set data write enable to 0

            CPU -> dmemAddr = 0; // set dmemAddr to 0
            CPU -> dmemValue = 0; // set dmemValue to 0
            
            CPU -> PSR  = CPU -> PSR & (0x7FFF); // PSR[15] = 0
            
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> R[7]; // PC = R7
            return 0;
            
        } else if (opcode == 9) { // constant
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            rd = INSN_11_9(CPU -> memory[CPU -> PC]); // get rd

            im9 = INSN_8_0(CPU -> memory[CPU -> PC]); // IMM9
            
            if (im9 >> 8 == 1) { // Sign extend
                im9 = im9 | 0xFE00;
            }
            
            if (rd > 7) { // Invalid registers
                fprintf(stderr, "error: Invalid registers\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            CPU -> rdMux_CTL = 0; // set RD control signal to 0
            CPU -> rsMux_CTL = 0; // set RS control signal to 0
            CPU -> rtMux_CTL = 0; // set RT control signal to 0
            CPU -> regFile_WE = 1; // set register file write enable to 0
            CPU -> NZP_WE = 1; // set NZP write enable to 1
            CPU -> DATA_WE = 0; // set data write enable to 0

            CPU -> dmemAddr = 0; // set dmemAddr to 0
            CPU -> dmemValue = 0; // set dmemValue to 0
            
            CPU -> R[rd] = im9; // RD = sext(IMM9)
            CPU -> regInputVal = rd; // Store register number
            SetNZP(CPU, CPU -> R[rd]); // Set NZP based on result
            
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1; // PC = PC + 1
            return 0;
            
        } else if (opcode == 10) { // shift
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            } else {
                ShiftModOp(CPU, output); // Run Shift Parser
                return 0;
            }
            
        } else if (opcode == 12) { // jump
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            } else {
                JumpOp(CPU, output); // Run Jump Parser
                return 0;
            }
            
        } else if (opcode == 13) { // hi-constant
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            rd = INSN_11_9(CPU -> memory[CPU -> PC]); // get rd

            uim8 = INSN_7_0(CPU -> memory[CPU -> PC]); // UIMM8
            
            if (rd > 7) { // Invalid registers
                fprintf(stderr, "error: Invalid registers\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            CPU -> rdMux_CTL = 0; // set RD control signal to 0
            CPU -> rsMux_CTL = 0; // set RS control signal to 0
            CPU -> rtMux_CTL = 0; // set RT control signal to 0
            CPU -> regFile_WE = 1; // set register file write enable to 0
            CPU -> NZP_WE = 1; // set NZP write enable to 1
            CPU -> DATA_WE = 0; // set data write enable to 0

            CPU -> dmemAddr = 0; // set dmemAddr to 0
            CPU -> dmemValue = 0; // set dmemValue to 0
            
            CPU -> R[rd] = (CPU -> R[rd] & 0xFF) | (uim8 << 8); // RD = RD & 0xFF | UIMM8 << 8
            CPU -> regInputVal = rd; // Store register number
            SetNZP(CPU, CPU -> R[rd]); // Set NZP based on result
            
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1; // PC = PC + 1
            return 0;
            
        } else if (opcode == 15) { // TRAP
            if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
                fprintf(stderr, "Error: Trying to Execute Code in Data Memory\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            rd = 0x7; // get rd = 7
            uim8 = INSN_7_0(CPU -> memory[CPU -> PC]); // UIMM8
            
            if (rd > 7) { // Invalid registers
                fprintf(stderr, "error: Invalid registers\n");
                CPU -> PC = 0x80FF;
                return 0;
            }
            
            CPU -> rdMux_CTL = 1; // set RD control signal to 1
            CPU -> rsMux_CTL = 0; // set RS control signal to 0
            CPU -> rtMux_CTL = 0; // set RT control signal to 0
            CPU -> regFile_WE = 1; // set register file write enable to 0
            CPU -> NZP_WE = 1; // set NZP write enable to 1
            CPU -> DATA_WE = 0; // set data write enable to 0

            CPU -> dmemAddr = 0; // set dmemAddr to 0
            CPU -> dmemValue = 0; // set dmemValue to 0
            
            CPU -> R[rd] = CPU -> PC + 1; // R7 = PC + 1
            CPU -> regInputVal = rd; // Store register number
            SetNZP(CPU, CPU -> R[rd]); // Set NZP based on result
            
            CPU -> PSR = CPU -> PSR | (0x8000); // PSR[15] = 1
            
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = (0x8000 | uim8); // PC = (0x8000 | uIMM8)
            return 0;
            
        } else { // Error
            fprintf(stderr, "error: Invalid Opcode\n");
            CPU -> PC = 0x80FF;
            return 0;
        }
        
        CPU -> PC = CPU -> PC + 1; // PC = PC + 1 by default
        return 0;
    }
    
}



//////////////// PARSING HELPER FUNCTIONS ///////////////////////////



/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState* CPU, FILE* output)
{
    // Check what we are testing for (N, Z, P, or combination)
    // Compare with current NZP value and update PC value
    unsigned short subOp = INSN_11_9(CPU -> memory[CPU -> PC]); // get sub-opcode
    short offset = INSN_8_0(CPU -> memory[CPU -> PC]);
    
    if (offset >> 8 == 1) { // Sign extend
        offset = offset | 0xFE00;
    }
    
    CPU -> rdMux_CTL = 0; // set RD control signal to 0
    CPU -> rsMux_CTL = 0; // set RS control signal to 0
    CPU -> rtMux_CTL = 0; // set RT control signal to 0
    CPU -> regFile_WE = 0; // set register file write enable to 0
    CPU -> NZP_WE = 0; // set NZP write enable to 0
    CPU -> DATA_WE = 0; // set data write enable to 0

    CPU -> dmemAddr = 0; // set dmemAddr to 0
    CPU -> dmemValue = 0; // set dmemValue to 0
    
    if (subOp == 0) { // NOP
        WriteOut(CPU, output); // Write output into file
        CPU -> PC = CPU -> PC + 1;
        
    } else if (subOp == 1) { // BRp
        if (INSN_2_0(CPU -> PSR) == 1) {
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1 + offset;
        } else {
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1;
        }
        
    } else if (subOp == 2) { // BRz
        if (INSN_2_0(CPU -> PSR) == 2) {
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1 + offset;
        } else {
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1;
        }
        
    } else if (subOp == 3) { // BRzp
        if (INSN_2_0(CPU -> PSR) == 1 || INSN_2_0(CPU -> PSR) == 2) {
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1 + offset;
        } else {
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1;
        }
        
    } else if (subOp == 4) { // BRn
        if (INSN_2_0(CPU -> PSR) == 4) {
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1 + offset;
        } else {
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1;
        }
        
    } else if (subOp == 5) { // BRnp
       if (INSN_2_0(CPU -> PSR) == 1 || INSN_2_0(CPU -> PSR) == 4) {
           WriteOut(CPU, output); // Write output into file
           CPU -> PC = CPU -> PC + 1 + offset;
        } else {
           WriteOut(CPU, output); // Write output into file
           CPU -> PC = CPU -> PC + 1;
        }
        
    } else if (subOp == 6) { // BRnz
        if (INSN_2_0(CPU -> PSR) == 2 || INSN_2_0(CPU -> PSR) == 4) {
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1 + offset;
        } else {
            WriteOut(CPU, output); // Write output into file
            CPU -> PC = CPU -> PC + 1;
        }
        
    } else if (subOp == 7) { // BRnzp
        WriteOut(CPU, output); // Write output into file
        CPU -> PC = CPU -> PC + 1 + offset;
        
    } 
    
}

/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState* CPU, FILE* output)
{
    // Determine sub-opcode
    // Update register values based on sub-opcode values
    unsigned short subOp = INSN_5_3(CPU -> memory[CPU -> PC]); // get sub-opcode
    unsigned short rd = INSN_11_9(CPU -> memory[CPU -> PC]); // get rd number
    unsigned short rs = INSN_8_6(CPU -> memory[CPU -> PC]); // get rs number
    unsigned short rt = INSN_2_0(CPU -> memory[CPU -> PC]); // get rt number
    short imm5 = INSN_4_0(CPU -> memory[CPU -> PC]); // get imm5
    if (imm5 >> 4 == 1) {
        imm5 = imm5 | 0xFFE0;
    }
    
    if (rd > 7 | rs > 7 | rt > 7) { // Invalid registers
        fprintf(stderr, "error: Invalid registers\n");
        CPU -> PC = 0x80FF;
        return;
    }
                                    
    CPU -> rdMux_CTL = 0; // set RD control signal to 0
    CPU -> rsMux_CTL = 0; // set RS control signal to 0
    CPU -> rtMux_CTL = 0; // set RT control signal to 0
    CPU -> regFile_WE = 1; // set register file write enable to 1
    CPU -> NZP_WE = 1; // set NZP write enable to 1
    CPU -> DATA_WE = 0; // set data write enable to 0

    CPU -> dmemAddr = 0; // set dmemAddr to 0
    CPU -> dmemValue = 0; // set dmemValue to 0
                                    
    if (subOp == 0) { // Addition subOp
        CPU -> R[rd] = (short int)CPU -> R[rs] + (short int)CPU -> R[rt]; // R[rd] = R[rs] + R[rt]
        
    } else if (subOp == 1) { // Multiplication subOp
        CPU -> R[rd] = (short int)CPU -> R[rs] * (short int)CPU -> R[rt]; // R[rd] = R[rs] * R[rt]
        
    } else if (subOp == 2) { // Subtraction subOp
        CPU -> R[rd] = (short int)CPU -> R[rs] - (short int)CPU -> R[rt]; // R[rd] = R[rs] - R[rt]
        
    } else if (subOp == 3) { // Division subOp
        CPU -> R[rd] = (short int)CPU -> R[rs] / (short int)CPU -> R[rt]; // R[rd] = R[rs] / R[rt]
        
    } else { // Immediate Addition subOp
        CPU -> R[rd] = (short int)(CPU -> R[rs]) + (short int)(imm5); // R[rd] = R[rs] + IMM5
    }
                                    
    CPU -> regInputVal = rd; // set to register being updated
    SetNZP(CPU, CPU -> R[rd]); // set NZPVal based on result
    
    WriteOut(CPU, output); // Write output into file
    CPU -> PC = CPU -> PC + 1; // PC = PC + 1
}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState* CPU, FILE* output)
{
    // Determine sub-opcode and set NZP value based on contents
    unsigned short subOp = ((CPU -> memory[CPU -> PC]) >> 7) & 0x3; // Get I[8:7]
    unsigned short rs = INSN_11_9(CPU -> memory[CPU -> PC]); // Get rs
    unsigned short rt = INSN_2_0(CPU -> memory[CPU -> PC]); // Get rt
    short imm7 = INSN_6_0(CPU -> memory[CPU -> PC]); // Get IMM7
    unsigned short uimm7 = INSN_6_0(CPU -> memory[CPU -> PC]); // Get UIMM7
    unsigned int uRS = CPU -> R[rs]; // unsigned RS value
    unsigned int uRT = CPU -> R[rt]; // unsigned RT value
    
    if (imm7 >> 6 == 1) { // Sign Extend
        imm7 = imm7 | 0xFF80;
    }
    
    if (rs > 7 | rt > 7) { // Invalid registers
        fprintf(stderr, "error: Invalid registers\n");
        CPU -> PC = 0x80FF;
        return;
    }
    
    CPU -> rdMux_CTL = 0; // set RD control signal to 0
    CPU -> rsMux_CTL = 2; // set RS control signal to I[11:9]
    CPU -> rtMux_CTL = 0; // set RT control signal to 0
    CPU -> regFile_WE = 0; // set register file write enable to 1
    CPU -> NZP_WE = 1; // set NZP write enable to 1
    CPU -> DATA_WE = 0; // set data write enable to 0

    CPU -> dmemAddr = 0; // set dmemAddr to 0
    CPU -> dmemValue = 0; // set dmemValue to 0
    
    if (subOp == 0) { // CMP
        SetNZP(CPU, (short int)CPU -> R[rs] - (short int)CPU -> R[rt]);
        
    } else if (subOp == 1) { // CMPU
        SetNZP(CPU, uRS - uRT);
        
    } else if (subOp == 2) { // CMPI
        SetNZP(CPU, (short int)CPU -> R[rs] - imm7);
        
    } else { // CMPIU
        SetNZP(CPU, uRS - uimm7);
        
    }
    
    CPU -> regInputVal = 0; // set to what is stored in RD
    
    WriteOut(CPU, output); // Write output into file
    CPU -> PC = CPU -> PC + 1; // PC = PC + 1
}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState* CPU, FILE* output)
{
    // Determine sub-opcode
    // Set specified register based on contents and operation
    unsigned short subOp = INSN_5_3(CPU -> memory[CPU -> PC]); // get sub-opcode
    unsigned short rd = INSN_11_9(CPU -> memory[CPU -> PC]); // get rd number
    unsigned short rs = INSN_8_6(CPU -> memory[CPU -> PC]); // get rs number
    unsigned short rt = INSN_2_0(CPU -> memory[CPU -> PC]); // get rt number
    int imm5 = INSN_4_0(CPU -> memory[CPU -> PC]); // IMM5
    if (imm5 >> 4 == 1) {
        imm5 = imm5 | 0xFFE0;
    }
    
    if (rd > 7 | rs > 7 | rt > 7) { // Invalid registers
        fprintf(stderr, "error: Invalid registers\n");
        CPU -> PC = 0x80FF;
        return;
    }
    
    CPU -> rdMux_CTL = 0; // set RD control signal to 0
    CPU -> rsMux_CTL = 0; // set RS control signal to 0
    CPU -> rtMux_CTL = 0; // set RT control signal to 0
    CPU -> regFile_WE = 1; // set register file write enable to 1
    CPU -> NZP_WE = 1; // set NZP write enable to 1
    CPU -> DATA_WE = 0; // set data write enable to 0

    CPU -> dmemAddr = 0; // set dmemAddr to 0
    CPU -> dmemValue = 0; // set dmemValue to 0
    
    if (subOp == 0) { // AND subOp
        CPU -> R[rd] = (short int)CPU -> R[rs] & (short int)CPU -> R[rt]; // R[rd] = R[rs] & R[rt]
        
    } else if (subOp == 1) { // NOT subOp
        CPU -> R[rd] = !(short int)CPU -> R[rs]; // R[rd] = !R[rs]
        
    } else if (subOp == 2) { // OR subOp
        CPU -> R[rd] = (short int)CPU -> R[rs] | (short int)CPU -> R[rt]; // R[rd] = R[rs] | R[rt]
        
    } else if (subOp == 3) { // XOR subOp
        CPU -> R[rd] = (short int)CPU -> R[rs] ^ (short int)CPU -> R[rt]; // R[rd] = R[rs] ^ R[rt]
        
    } else if (subOp == 4) { // Immediate AND subOp
        CPU -> R[rd] = (short int)CPU -> R[rs] & imm5; // R[rd] = R[rs] & IMM5
    } else { // ERROR
        
    }
                                    
    CPU -> regInputVal = rd; // set to register value we set
    SetNZP(CPU, CPU -> R[rd]); // set NZPVal based on result
    
    WriteOut(CPU, output); // Write output into file
    CPU -> PC = CPU -> PC + 1; // PC = PC + 1
}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState* CPU, FILE* output)
{
    unsigned short subOp = (CPU -> memory[CPU -> PC] >> 11) & (0x1); // Get subOp
    unsigned short rs = INSN_8_6(CPU -> memory[CPU -> PC]); // Get RS
    
    if (rs > 7) { // Invalid registers
        fprintf(stderr, "error: Invalid registers\n");
        CPU -> PC = 0x80FF;
        return;
    }
    
    CPU -> rdMux_CTL = 0; // set RD control signal to 0
    CPU -> rsMux_CTL = 0; // set RS control signal to 0
    CPU -> rtMux_CTL = 0; // set RT control signal to 0
    CPU -> regFile_WE = 0; // set register file write enable to 0
    CPU -> NZP_WE = 0; // set NZP write enable to 0
    CPU -> DATA_WE = 0; // set data write enable to 0

    CPU -> dmemAddr = 0; // set dmemAddr to 0
    CPU -> dmemValue = 0; // set dmemValue to 0
    
    
    if (subOp == 0) { // Register
        
        WriteOut(CPU, output); // Write output into file
        CPU -> PC = CPU -> R[rs];
        
    } else { // Immediate
        
        WriteOut(CPU, output); // Write output into file
        CPU -> PC = CPU -> PC + 1 + INSN_10_0(CPU -> memory[CPU -> PC]);
        
    }
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState* CPU, FILE* output)
{
    unsigned short subOp = (CPU -> memory[CPU -> PC] >> 11) & (0x1); // Get subOp
    unsigned short rs = INSN_8_6(CPU -> memory[CPU -> PC]); // Get RS
    
    if (rs > 7) { // Invalid registers
        fprintf(stderr, "error: Invalid registers\n");
        CPU -> PC = 0x80FF;
        return;
    }
    
    CPU -> rdMux_CTL = 1; // set RD control signal to 7
    CPU -> rsMux_CTL = 0; // set RS control signal to 0
    CPU -> rtMux_CTL = 0; // set RT control signal to 0
    CPU -> regFile_WE = 1; // set register file write enable to 1
    CPU -> NZP_WE = 0; // set NZP write enable to 0
    CPU -> DATA_WE = 0; // set data write enable to 0

    CPU -> dmemAddr = 0; // set dmemAddr to 0
    CPU -> dmemValue = 0; // set dmemValue to 0
    
    if (subOp == 1) { // Immediate 
        CPU -> R[7] = CPU -> PC;
        
        WriteOut(CPU, output); // Write output into file
        CPU -> PC = (CPU -> PC & 0x8000) | INSN_10_0(CPU -> memory[CPU -> PC] << 4);
        
    } else { // Register
        CPU -> R[7] = CPU -> PC;
        
        WriteOut(CPU, output); // Write output into file
        CPU -> PC = CPU -> R[rs]; // PC = value in rs
        
    }
    
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState* CPU, FILE* output)
{
    unsigned short subOp = ((CPU -> memory[CPU -> PC]) >> 4) & 0x3; // Get I[6:5]
    unsigned short rd = INSN_11_9(CPU -> memory[CPU -> PC]); // get rd number
    unsigned short rs = INSN_8_6(CPU -> memory[CPU -> PC]); // get rs number
    unsigned short rt = INSN_2_0(CPU -> memory[CPU -> PC]); // get rt number
    unsigned short u4 = INSN_3_0(CPU -> memory[CPU -> PC]); // UIMM4
    
    if (rd > 7 | rs > 7 | rt > 7) { // Invalid registers
        fprintf(stderr, "error: Invalid registers\n");
        CPU -> PC = 0x80FF;
        return;
    }
    
    CPU -> rdMux_CTL = 0; // set RD control signal to 0
    CPU -> rsMux_CTL = 0; // set RS control signal to 0
    CPU -> rtMux_CTL = 0; // set RT control signal to 0
    CPU -> regFile_WE = 1; // set register file write enable to 1
    CPU -> NZP_WE = 1; // set NZP write enable to 1
    CPU -> DATA_WE = 0; // set data write enable to 0

    CPU -> dmemAddr = 0; // set dmemAddr to 0
    CPU -> dmemValue = 0; // set dmemValue to 0
    
    if (subOp == 0) { // SLL
        CPU -> R[rd] = CPU -> R[rs] << u4;
        
    } else if (subOp == 1) { // SRA
        CPU -> R[rd] = (signed) (CPU -> R[rs]) >> u4;
        
    } else if (subOp == 2) { // SRL
        CPU -> R[rd] = (unsigned) (CPU -> R[rs]) >> u4;
        
    } else if (subOp == 3) { // subOp == 3 -> MOD
        CPU -> R[rd] = CPU -> R[rs] % CPU -> R[rt];
        
    } else {
        
    }
    
    CPU -> regInputVal = CPU -> R[rd]; // set to what is stored in RD
    SetNZP(CPU, CPU -> R[rd]); // set NZPVal based on result
    
    WriteOut(CPU, output); // Write output into file
    CPU -> PC = CPU -> PC + 1; // PC = PC + 1
}

/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result)
{
    unsigned short currNZP = INSN_2_0(CPU -> PSR); // Get current NZP value of PSR
    if (result < 0) { // negative value
        if (currNZP == 1) { // P bit = 1
            CPU -> PSR -= 1; // remove P bit
            CPU -> PSR += 4; // add N bit
        } else if (currNZP == 2) { // Z bit = 1
            CPU -> PSR -= 2; // remove Z bit
            CPU -> PSR += 4; // add N bit
        } else if (currNZP == 0) { // reset
            CPU -> PSR += 4; // add N bit
        } else {
            return; // nothing to adjust
        }
    } else if (result > 0) { // positive value
        if (currNZP == 4) { // N bit = 1
            CPU -> PSR -= 4; // remove N bit
            CPU -> PSR += 1; // add P bit
        } else if (currNZP == 2) { // Z bit = 1
            CPU -> PSR -= 2; // remove Z bit
            CPU -> PSR += 1; // add P bit
        } else if (currNZP == 0) { // reset
            CPU -> PSR += 1; // add P bit
        } else {
            return; // nothing to adjust
        }
    } else { // zero value
        if (currNZP == 4) { // N bit = 1
            CPU -> PSR -= 4; // remove N bit
            CPU -> PSR += 2; // add Z bit
        } else if (currNZP == 1) { // P bit = 1
            CPU -> PSR -= 1; // remove P bit
            CPU -> PSR += 2; // add Z bit
        } else if (currNZP == 0) { // reset
            CPU -> PSR += 2; // add Z bit
        } else {
            return; // nothing to adjust
        }
    }
    
    CPU -> NZPVal = INSN_2_0(CPU -> PSR); // Set new NZPVal
}
