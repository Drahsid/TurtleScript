#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "turtleinttypes.h"
#include "opcode.h"
#include "state.h"

#define BUFFER_LENGTH (0x100)

static char buffer[BUFFER_LENGTH];

void ZeroInitOpcode(TurtleOpcode_Extended* thisx) {
    memset(thisx, 0, sizeof(TurtleOpcode_Extended));
    memset(buffer, 0, sizeof(buffer));
}

void ReadFile(const char* path) {
    TurtleOpcode_Extended opcode;
    tu32 whitespace = T_NULL;
    FILE* file = T_NULLPOINTER;
    char line_buffer[BUFFER_LENGTH];

    ZeroInitOpcode(&opcode);
    
    file = fopen(path, "r");
    if (file == T_NULLPOINTER) {
        printf("Failed to open file %s\n", path);
        return;
    }

    while (fgets(line_buffer, BUFFER_LENGTH, file)) {
        whitespace = strcspn(line_buffer, "\r\n"); // WINDOWS AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAH
        line_buffer[whitespace] = '\0';

        whitespace = strcspn(line_buffer, "\n"); // end line at end of line
        line_buffer[whitespace] = '\0';

        whitespace = strcspn(line_buffer, "//"); // remove comments
        line_buffer[whitespace] = '\0';

        whitespace = strcspn(line_buffer, "#"); // remove comments
        line_buffer[whitespace] = '\0';

        whitespace = 0;

        // skip empy lines and whitespace
        char* ch = line_buffer;
        while(*ch != '\0') {
            if (isspace(*ch)) {
                whitespace++;
                break;
            }
            ch++;
        }

        if (whitespace >= strlen(line_buffer)) {
            whitespace = 1;
        }
        else {
            whitespace = 0;
        }

        // if our line is not empty, assemble the contents of the line
        if (whitespace == 0) {
            printf("\nassembling line %s\n", line_buffer);
            ZeroInitOpcode(&opcode);
            Opcode_Assemble(&opcode, line_buffer);
            Opcode_GetMnemonic(&opcode, buffer);
            if ((opcode.opcode.flags & INSTRUCTION_FLAG_EXTENDED) != 0) {
                printf("/* [%X] %016llX */ %s\n", opcode.imm, opcode.opcode.dirty, buffer);
            }
            else {
                printf("/* %016llX */      %s \n", opcode.opcode.dirty, buffer);
            }
        }
    }

}

int main(int argc, char* argv[]) {
    TurtleOpcode_Extended opcode;

    printf("Sanity checks %zX %zX\n", sizeof(TurtleOpcode), sizeof(TurtleOpcode_Extended));
    printf("Opcode test display: /* INSTRUCTION HEX */ MNEMONIC\n");

    printf("Constructed by hand: \n");
    // JELI A0, S0, 0xDEAD00'00000000
    ZeroInitOpcode(&opcode);
    opcode.opcode.type = INSTRUCTION_JB_TYPE;
    opcode.opcode.index = INSTRUCTION_JB_JUMP_EQUALS_AND_LINK;
    opcode.opcode.flags = INSTRUCTION_FLAG_IMMEDIATE;
    opcode.opcode.rl = REGISTER_INDEX_A0;
    opcode.opcode.rr = REGISTER_INDEX_S0;
    opcode.opcode.imm = 0xDEAD0000;
    Opcode_GetMnemonic(&opcode, buffer);
    printf("/* %016llX */      %s // should disassemble to JELI a0, s0, 0xDEAD0000\n", opcode.opcode.dirty, buffer);

    // JELI Z0, A0, 0xDEAD0000'00000000'BEEF0000'00000000
    ZeroInitOpcode(&opcode);
    opcode.opcode.type = INSTRUCTION_JB_TYPE;
    opcode.opcode.index = INSTRUCTION_JB_JUMP_EQUALS_AND_LINK;
    opcode.opcode.flags = INSTRUCTION_FLAG_IMMEDIATE | INSTRUCTION_FLAG_EXTENDED;
    opcode.opcode.rl = REGISTER_INDEX_Z0;
    opcode.opcode.rr = REGISTER_INDEX_A0;
    opcode.opcode.imm = 0xDEAD0000;
    opcode.imm = 0xBEEF0000;
    Opcode_GetMnemonic(&opcode, buffer);
    printf("/* [%X] %016llX */ %s // should disassemble to JELIE z0, a0, 0xDEAD0000BEEF0000\n", opcode.imm, opcode.opcode.dirty, buffer);

    // BER Z0, S0, A1
    ZeroInitOpcode(&opcode);
    opcode.opcode.type = INSTRUCTION_JB_TYPE;
    opcode.opcode.index = INSTRUCTION_JB_BRANCH_EQUALS;
    opcode.opcode.flags = 0;
    opcode.opcode.rl = REGISTER_INDEX_Z0;
    opcode.opcode.rr = REGISTER_INDEX_S0;
    opcode.opcode.rs = REGISTER_INDEX_A1;
    opcode.opcode.imm = 0;
    Opcode_GetMnemonic(&opcode, buffer);
    printf("/* %016llX */      %s // should disassemble to BER a1, z0, s0\n", opcode.opcode.dirty, buffer);

    printf("\nConstructed by assembler: \n");
    // JALR A3
    ZeroInitOpcode(&opcode);
    Opcode_Assemble(&opcode, "JALR a3");
    Opcode_GetMnemonic(&opcode, buffer);
    printf("/* %016llX */      %s // should assemble to JALR a3\n", opcode.opcode.dirty, buffer);

    // JALR A0, A1 [bogus rl and rr]
    ZeroInitOpcode(&opcode);
    Opcode_Assemble(&opcode, "JALR a0, a1, a2");
    Opcode_GetMnemonic(&opcode, buffer);
    printf("/* %016llX */      %s // should assemble to JALR a0, a1, a2; should disassemble to JALR a0 since the other regs are bogus for this op\n", opcode.opcode.dirty, buffer);

    ZeroInitOpcode(&opcode);
    Opcode_Assemble(&opcode, "JIE 0xDEADBEEFBEEFDEAD");
    Opcode_GetMnemonic(&opcode, buffer);
    printf("/* [%X] %016llX */      %s // should assemble to JIE 0xDEADBEEFBEEFDEAD\n", opcode.imm, opcode.opcode.dirty, buffer);


    printf("Constructed by assembler through file:\n////////////////////////\n");
    ReadFile("tests/assemble.s");
    printf("\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\n");

#if _MSC_VER && _DEBUG
    system("pause");
#endif

    return 0;
}
