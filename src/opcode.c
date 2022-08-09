#include "opcode.h"
#include "state.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define WORKING_BUFFER_SIZE (64)
#define WIDE_SHIFT (sizeof(tu32) * 8)
#define WIDE_MASK (0xFFFFFFFF)

static const char sDelimiter[] = ",";
static const char sSpacer[] = " ";
static const char sHex[] = "0x";

const char gInstructionJBNames[INSTRUCTION_JB_COUNT + 1][16] = {
    "J",
    "JE",
    "JG",
    "JF",
    "JAL",
    "JEL",
    "JGL",
    "JFL",
    "B",
    "BE",
    "BG",
    "BF",
    "JB_BOGUS"
};

const char gInstructionMEMNames[INSTRUCTION_MEM_COUNT + 1][16] = {
    "S8",
    "S16",
    "S32",
    "S64",
    "L8",
    "L16",
    "L32",
    "L64",
    "MEM_BOGUS"
};

const char gInstructionREGNames[INSTRUCTION_REG_COUNT + 1][16] = {
    "AND",
    "OR",
    "CE",
    "CG",
    "ADDU",
    "ADD",
    "MUL",
    "DIV",
    "SR",
    "SL",
    "REG_BOGUS"
};

const char gInstructionNONames[INSTRUCTION_NO_TYPE_COUNT + 1][16] = {
    "NOP",
    "TRAP",
    "NO_BOGUS"
};

static void _Opcode_DebugPrintf(const char* format, const char* fn, const char* file, const int line, ...) {
    va_list arg;

    printf("[%s, %s:%d] ", fn, file, line);

    va_start(arg, format);
    vfprintf(stdout, format, arg);
    va_end(arg);
}

#if _MSC_VER
#define Opcode_DebugPrintf(FORMAT, ...)
#else
#define Opcode_DebugPrintf(FORMAT, ...) _Opcode_DebugPrintf(FORMAT, __FUNCTION__, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif

static void Opcode_Memcpy(char* dest, char* src, size_t length) {
    while (length--) {
        *dest++ = *src++;
    }
}

static char* Opcode_GetDelimStart(char* substr) {
    if (substr == T_NULLPOINTER) {
        Opcode_DebugPrintf("Opcode_GetDelimStart: substr is null!\n");
        return T_NULLPOINTER;
    }

    while (*substr == sDelimiter[0] || *substr == sSpacer[0]) {
        substr++;
    }

    return substr;
}

static char* Opcode_GetDelimEnd(char* substr) {
    char* nexstr = T_NULLPOINTER;

    if (substr == T_NULLPOINTER) {
        Opcode_DebugPrintf("Opcode_GetDelimEnd: substr is null!\n");
        return T_NULLPOINTER;
    }

    // get end of rs identifier
    nexstr = strstr(substr, sDelimiter) - strlen(sDelimiter);

    // no delim? check for a space instead
    if (nexstr == T_NULLPOINTER || nexstr == -1) {
        nexstr = strstr(substr, sSpacer);// - strlen(sSpacer);
    }

    // end must be the end of the string
    if (nexstr == T_NULLPOINTER || nexstr == -1) {
        nexstr = substr + strlen(substr);
    }

    return nexstr;
}

static tu32 Opcode_GetMnemonic_OpcodeSpace(char* str) {
    tu32 len = strlen(str);
    tu32 spaces = 8 - len;

    str = str + len;
    memset(str, sSpacer[0], spaces);

    return len + spaces;
}

static tu32 Opcode_GetMnemonic_Regs(char* str, tu8 rs, tu8 rl, tu8 rr) {
    tu32 len = strlen(str);
    tu32 prev = 0;

    if (rs != 0xFF) {
        strcpy(str + len, sSpacer);
        len = strlen(str);

        strcpy(str + len, gTurtleRegisterNames[rs]);
        len = strlen(str);

        prev = 1;
    }

    if (rl != 0xFF) {
        if (prev) {
            strcpy(str + len, sDelimiter);
            len = strlen(str);
        }

        strcpy(str + len, sSpacer);
        len = strlen(str);

        strcpy(str + len, gTurtleRegisterNames[rl]);
        len = strlen(str);

        prev = 1;
    }

    if (rr != 0xFF) {
        if (prev) {
            strcpy(str + len, sDelimiter);
            len = strlen(str);
        }

        strcpy(str + len, sSpacer);
        len = strlen(str);

        strcpy(str + len, gTurtleRegisterNames[rr]);
    }

    return strlen(str);
}

static tu32 Opcode_GetMnemonic_IMM(char* str, tu8 use_imm, tu32 len) {
    if (use_imm == 2) {
        strcpy(str + len, "IE");
        return T_TRUE;
    }
    else if (use_imm == 1) {
        strcpy(str + len, "I");
        return T_TRUE;
    }

    return T_FALSE;
}

static void Opcode_GetMnemonic_R_OR_I(char* str, tu8 use_imm) {
    tu32 len = strlen(str);
    if (!Opcode_GetMnemonic_IMM(str, use_imm, len)) {
        strcpy(str + len, "R");
    }
}

static void Opcode_GetMnemonic_JB(TurtleOpcode_Extended* src, char* out) {
    tu32 len = 0;
    tu64 imm = 0;
    tu8 index = src->opcode.index;
    tu8 flags = src->opcode.flags;
    tu8 extended = flags & INSTRUCTION_FLAG_EXTENDED;
    tu8 use_imm = flags & INSTRUCTION_FLAG_IMMEDIATE;
    tu8 space_imm = 1;
    tu8 rs = 0xFF;
    tu8 rl = 0xFF;
    tu8 rr = 0xFF;

    if (extended) {
        imm = ((tu64)src->opcode.imm << WIDE_SHIFT) | ((tu64)src->imm & WIDE_MASK);
        use_imm = 2;
    }
    else if (use_imm) {
        imm = src->opcode.imm;
        use_imm = 1;
    }
    else {
        use_imm = 0;
        rs = src->opcode.rs; // might have some bogus results sometimes?
    }

    if (index < INSTRUCTION_JB_COUNT) {
        strcpy(out, gInstructionJBNames[index]);
    }
    else {
        strcpy(out, gInstructionJBNames[INSTRUCTION_JB_COUNT]);
        return;
    }

    switch (index) {
        case (INSTRUCTION_JB_JUMP):
        case (INSTRUCTION_JB_JUMP_ON_FLAG):
        case (INSTRUCTION_JB_JUMP_AND_LINK):
        case (INSTRUCTION_JB_JUMP_ON_FLAG_AND_LINK):
        case (INSTRUCTION_JB_BRANCH):
        case (INSTRUCTION_JB_BRANCH_ON_FLAG): {
            space_imm = 0;
            break;
        }
        case (INSTRUCTION_JB_JUMP_EQUALS):
        case (INSTRUCTION_JB_JUMP_GREATER_THAN):
        case (INSTRUCTION_JB_JUMP_EQUALS_AND_LINK):
        case (INSTRUCTION_JB_JUMP_GREATER_THAN_AND_LINK):
        case (INSTRUCTION_JB_BRANCH_EQUALS):
        case (INSTRUCTION_JB_BRANCH_GREATER_THAN): {
            rl = src->opcode.rl;
            rr = src->opcode.rr;
            break;
        }
        default: {
            strcpy(out, gInstructionJBNames[INSTRUCTION_JB_COUNT]);
            return;
            break;
        }
    }

    Opcode_GetMnemonic_R_OR_I(out, use_imm);
    len = Opcode_GetMnemonic_OpcodeSpace(out);
    len = Opcode_GetMnemonic_Regs(out, rs, rl, rr);

    if (use_imm) {
        if (space_imm) {
            strcpy(out + len, sDelimiter);
            len += strlen(sDelimiter);

            strcpy(out + len, sSpacer);
            len += strlen(sSpacer);
        }

        strcpy(out + len, sHex);
        len += strlen(sHex);

        if (extended) {
            len += sprintf(out + len, "%llX", imm);
        }
        else {
            len += sprintf(out + len, "%X", (tu32)imm);
        }
    }
}

static void Opcode_GetMnemonic_MEM(TurtleOpcode_Extended* src, char* out) {
    tu8 index = src->opcode.index;
    tu8 flags = src->opcode.flags;
}

static void Opcode_GetMnemonic_REG(TurtleOpcode_Extended* src, char* out) {
    tu8 index = src->opcode.index;
    tu8 flags = src->opcode.flags;
}

static void Opcode_GetMnemonic_NO(TurtleOpcode_Extended* src, char* out) {
    tu8 index = src->opcode.index;
    tu8 flags = src->opcode.flags;
}

static void Opcode_BufferToLower(char* src) {
    while (src[0] != '\0') {
        src[0] = tolower(src[0]);
        src++;
    }
}

static void Opcode_BufferToUpper(char* src) {
    while (src[0] != '\0') {
        src[0] = toupper(src[0]);
        src++;
    }
}

static tu32 Opcode_GetRegIndexFromName(char* src) {
    tu32 rndex = 0;
    tu32 sanity = 0;
    char buffer[WORKING_BUFFER_SIZE];

    memset(buffer, 0, sizeof(buffer));

    strcpy(buffer, src);
    Opcode_BufferToLower(buffer);

    // check for name
    for (rndex = 0; rndex < REGISTER_INDEX_COUNT; rndex++) {
        if (strcmp(gTurtleRegisterNames[rndex], buffer) == 0) {
            sanity = 1;
            break;
        }
    }

    if (!sanity) {
        // check for index
        rndex = atoi(buffer);
    }

    if (rndex >= REGISTER_INDEX_COUNT) {
        rndex = 0xFF;
    }
    else if (rndex < 0) {
        rndex = 0xFF;
    }

    return rndex;
}

static void Opcode_GetOp(char* src, char* buffer) {
    char* nexstr = T_NULLPOINTER;
    tu32 length = T_NULL;

    nexstr = strstr(src, sSpacer);
    length = nexstr - src;

    // IE
    if (src[length - 1] == 'E') {
        memcpy(buffer, src, length - 2);
    }
    // I or R
    else {
        memcpy(buffer, src, length - 1);
    }

    Opcode_BufferToUpper(buffer);
}

static tu32 Opcode_AssembleNextReg(TurtleOpcode_Extended* out, char** _substr, char* srcend, tu32 rbuff) {
    tu32 reg = 0;
    char* substr = *_substr;
    char* nexstr = T_NULLPOINTER;
    char* endstr = T_NULLPOINTER;
    char buff[WORKING_BUFFER_SIZE];

    memset(buff, 0, sizeof(buff));

    substr = Opcode_GetDelimStart(substr);
    if (substr < srcend) {
        nexstr = Opcode_GetDelimEnd(substr);
        if (nexstr <= srcend) {
            endstr = nexstr + 1; // move to the end of the register keyword
            Opcode_Memcpy(buff, substr, endstr - substr);

            substr = endstr + 1;
            reg = Opcode_GetRegIndexFromName(buff);

            if (rbuff == 0) {
                out->opcode.rs = reg;
            }
            else if (rbuff == 1) {
                out->opcode.rl = reg;
            }
            else {
                out->opcode.rr = reg;
            }

            *_substr = substr;
            return T_FALSE;
        }
    }

    *_substr = substr;
    return T_TRUE;
}

static tu32 Opcode_AssembleImm(TurtleOpcode_Extended* out, char** _substr, char* srcend, tu32 extended) {
    char* substr = *_substr;
    char* nexstr = T_NULLPOINTER;
    tu64 imm = 0;
    char buff[WORKING_BUFFER_SIZE];

    memset(buff, 0, sizeof(buff));

    substr = Opcode_GetDelimStart(substr);
    if (substr < srcend) {
        nexstr = Opcode_GetDelimEnd(substr);
        if (nexstr <= srcend) {
            Opcode_Memcpy(buff, substr, nexstr - substr);
            if (extended) {
                imm = strtoull(buff, &nexstr, 0);
                out->opcode.imm = imm >> WIDE_SHIFT;
                out->imm = imm & WIDE_MASK;
            }
            else {
                imm = strtoul(buff, &nexstr, 0);
                out->opcode.imm = (tu32)imm;
            }

            substr = nexstr;
            *_substr = substr;
            return T_FALSE;
        }
    }

    *_substr = substr;
    return T_TRUE;
}

static tu32 Opcode_AssembleJB(TurtleOpcode_Extended* out, char* src, char* buffer) {
    tu32 index = T_NULL;
    tu32 rbuff = T_NULL;
    tu32 reg = T_NULL;
    char* substr = T_NULLPOINTER;
    char* nexstr = T_NULLPOINTER;
    char* srcend = src + strlen(src);
    
    for (index = 0; index < INSTRUCTION_JB_COUNT + 1; index++) {
        if (strcmp(buffer, gInstructionJBNames[index]) == 0) {
            out->opcode.type = INSTRUCTION_JB_TYPE;
            out->opcode.index = index;

            if (index >= INSTRUCTION_JB_COUNT) {
                Opcode_DebugPrintf("index is bogus?\n");
                out->opcode.index = 0xFF;
            }

            substr = src + strlen(gInstructionJBNames[index]);

            // Register
            if (toupper(substr[0]) == 'R') {
                // remove beginning space
                substr++; // remove 'R'

                switch (out->opcode.index) {
                    // rs
                    case (INSTRUCTION_JB_JUMP):
                    case (INSTRUCTION_JB_JUMP_ON_FLAG):
                    case (INSTRUCTION_JB_JUMP_AND_LINK):
                    case (INSTRUCTION_JB_JUMP_ON_FLAG_AND_LINK):
                    case (INSTRUCTION_JB_BRANCH):
                    case (INSTRUCTION_JB_BRANCH_ON_FLAG): {
                        Opcode_AssembleNextReg(out, &substr, srcend, 0);
                        break;
                    }
                    // rs, rl, rr
                    case (INSTRUCTION_JB_JUMP_EQUALS):
                    case (INSTRUCTION_JB_JUMP_GREATER_THAN):
                    case (INSTRUCTION_JB_JUMP_EQUALS_AND_LINK):
                    case (INSTRUCTION_JB_JUMP_GREATER_THAN_AND_LINK):
                    case (INSTRUCTION_JB_BRANCH_EQUALS):
                    case (INSTRUCTION_JB_BRANCH_GREATER_THAN): {
                        for (rbuff = 0; rbuff < 3; rbuff++) {
                            if (Opcode_AssembleNextReg(out, &substr, srcend, rbuff)) {
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            else if (toupper(substr[0] == 'I')) {
                tu32 extended = 0;

                out->opcode.flags |= INSTRUCTION_FLAG_IMMEDIATE;

                // check for rl, rr, imm
                substr++; // remove I
                if (toupper(substr[0] == 'E')) {
                    substr++; // remove E
                    extended = 1;
                    out->opcode.flags |= INSTRUCTION_FLAG_EXTENDED;
                }

                switch (out->opcode.index) {
                    // imm
                    case (INSTRUCTION_JB_JUMP):
                    case (INSTRUCTION_JB_JUMP_ON_FLAG):
                    case (INSTRUCTION_JB_JUMP_AND_LINK):
                    case (INSTRUCTION_JB_JUMP_ON_FLAG_AND_LINK):
                    case (INSTRUCTION_JB_BRANCH):
                    case (INSTRUCTION_JB_BRANCH_ON_FLAG): {
                        Opcode_AssembleImm(out, &substr, srcend, extended);
                        break;
                    }
                    // rl, rr, imm
                    case (INSTRUCTION_JB_JUMP_EQUALS):
                    case (INSTRUCTION_JB_JUMP_GREATER_THAN):
                    case (INSTRUCTION_JB_JUMP_EQUALS_AND_LINK):
                    case (INSTRUCTION_JB_JUMP_GREATER_THAN_AND_LINK):
                    case (INSTRUCTION_JB_BRANCH_EQUALS):
                    case (INSTRUCTION_JB_BRANCH_GREATER_THAN): {
                        for (rbuff = 1; rbuff < 3; rbuff++) {
                            if (Opcode_AssembleNextReg(out, &substr, srcend, rbuff)) {
                                break;
                            }
                        }

                        Opcode_AssembleImm(out, &substr, srcend, extended);
                        break;
                    }
                }
            }

            return T_TRUE;
        }
    }

    return T_FALSE;
}

static tu32 Opcode_AssembleMEM(TurtleOpcode_Extended* out, char* src, char* buffer) {
    return T_FALSE;
}

static tu32 Opcode_AssembleREG(TurtleOpcode_Extended* out, char* src, char* buffer) {
    return T_FALSE;
}

static tu32 Opcode_AssembleNO(TurtleOpcode_Extended* out, char* src, char* buffer) {
    return T_FALSE;
}

void Opcode_GetMnemonic(TurtleOpcode_Extended* src, char* out) {
    switch(src->opcode.type) {
        case (INSTRUCTION_JB_TYPE): {
            Opcode_GetMnemonic_JB(src, out);
            break;
        }
        case (INSTRUCTION_MEM_TYPE): {
            Opcode_GetMnemonic_MEM(src, out);
            break;
        }
        case (INSTRUCTION_REG_TYPE): {
            Opcode_GetMnemonic_REG(src, out);
            break;
        }
        case (INSTRUCTION_NO_TYPE): {
            Opcode_GetMnemonic_NO(src, out);
            break;
        }
        default: {
            strcpy(out, "NOP    # TYPE_BOGUS");
            break;
        }
    }

}

tu32 Opcode_Assemble(TurtleOpcode_Extended* out, char* src) {
    char buffer[WORKING_BUFFER_SIZE];

    if (strlen(sDelimiter) > 1) {
        Opcode_DebugPrintf("Opcode_Assemble: delimiter (%s) is larger than 1 character, assembler behavior may be wrong!\n", sDelimiter);
    }

    if (strlen(sSpacer) > 1) {
        Opcode_DebugPrintf("Opcode_Assemble: sSpacer (%s) is larger than 1 character, assembler behavior may be wrong!\n", sSpacer);
    }

    memset(buffer, 0, sizeof(buffer));

    if (src == T_NULLPOINTER) {
        Opcode_DebugPrintf("Opcode_Assemble: src is null!\n");
        return T_FALSE;
    }

    Opcode_GetOp(src, buffer);
    if (Opcode_AssembleJB(out, src, buffer)) {
        return T_TRUE;
    }
    else if (Opcode_AssembleMEM(out, src, buffer)) {
        return T_TRUE;
    }
    else if (Opcode_AssembleREG(out, src, buffer)) {
        return T_TRUE;
    }
    else if (Opcode_AssembleNO(out, src, buffer)) {
        return T_TRUE;
    }

    return T_FALSE;
}
