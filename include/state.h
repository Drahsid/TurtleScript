#ifndef T_STATE_H
#define T_STATE_H

#include "turtleinttypes.h"

enum {
    REGISTER_INDEX_Z0 = 0,
    REGISTER_INDEX_PC,
    REGISTER_INDEX_LP,
    REGISTER_INDEX_SP,
    REGISTER_INDEX_R0,
    REGISTER_INDEX_R1,
    REGISTER_INDEX_A0,
    REGISTER_INDEX_A1,
    REGISTER_INDEX_A2,
    REGISTER_INDEX_A3,
    REGISTER_INDEX_T0,
    REGISTER_INDEX_T1,
    REGISTER_INDEX_T2,
    REGISTER_INDEX_T3,
    REGISTER_INDEX_S0,
    REGISTER_INDEX_S1,
    REGISTER_INDEX_S2,
    REGISTER_INDEX_S3,
    REGISTER_INDEX_FR,
    REGISTER_INDEX_19,
    REGISTER_INDEX_20,
    REGISTER_INDEX_21,
    REGISTER_INDEX_22,
    REGISTER_INDEX_23,
    REGISTER_INDEX_24,
    REGISTER_INDEX_25,
    REGISTER_INDEX_26,
    REGISTER_INDEX_27,
    REGISTER_INDEX_28,
    REGISTER_INDEX_29,
    REGISTER_INDEX_30,
    REGISTER_INDEX_31,
    REGISTER_INDEX_COUNT,
    REGISTER_INDEX_BOGUS = 0xFF
};

typedef union {
    struct {
        tu64 z0; // ZER0
        tu64 pc; // Program Counter
        tu64 lp; // Link Pointer
        tu64 sp; // Stack Pointer
        tu64 r0; // Return (0)
        tu64 r1; // Return (1)
        tu64 a0; // Argument (0)
        tu64 a1; // Argument (1)
        tu64 a2; // Argument (2)
        tu64 a3; // Argument (3)
        tu64 t0; // Temporary (0)
        tu64 t1; // Temporary (1)
        tu64 t2; // Temporary (2)
        tu64 t3; // Temporary (3)
        tu64 s0; // Saved (0)
        tu64 s1; // Saved (1)
        tu64 s2; // Saved (2)
        tu64 s3; // Saved (3)
        tu64 fr; // Flag Register
        tu64 reserved19;
        tu64 reserved20;
        tu64 reserved21;
        tu64 reserved22;
        tu64 reserved23;
        tu64 reserved24;
        tu64 reserved25;
        tu64 reserved26;
        tu64 reserved27;
        tu64 reserved28;
        tu64 reserved29;
        tu64 reserved30;
        tu64 reserved31;
    };
    struct {
        tu64 z[1]; // ZER0
        tu64 p[1]; // Program Counter
        tu64 l[1]; // Link Pointer
        tu64 _s[1]; // Stack Pointer
        tu64 r[2]; // Return
        tu64 a[4]; // Argument
        tu64 t[4]; // Temporary
        tu64 s[4]; // Saved
        tu64 f[1]; // Flag Register
        tu64 reserved[13]; // Dunno what to use these for yet
    };
    struct {
        tu64 regs[32];
    };
} TurtleRegisters;

typedef struct {
    TurtleRegisters regs;
    void* memory_code;
    void* memory_data;
} TurtleState;

extern const char gTurtleRegisterNames[][4];

#endif

