# TurtleScript
A pet project of mine that I slowly work on when I am bored. TurtleScript is intended to be a small VM-based scripting language that is intended to be statically-typed, procedural, and structured. I intend for it's syntax to be extremely similar to C. Currently, this can only assemble and disassemble some VM bytecode, but one day it might actually be usable.

## Intended syntax
My intended syntax is practically C, with some minor changes to make it more succinct, readable, and scripty. I intend to keep the language syntax verbose, so that code written is always explicit. I intend for this to feel like C, if C were a scripting language.
The syntax is subject to change as I actually develop the language.
```c
// structure typedefs can written more succinctly:
struct StructOfPrimitiveTypes {
    u64 unsinged_64_bit_value; // the primitive type for a 64-bit uint is 'u64'
    s64 singed_64_bit_value; // the primitive type for a 64-bit int is 's64'
    u32 unsinged_32_bit_value; // the primitive type for a 32-bit uint is 'u32'
    s32 singed_32_bit_value; // the primitive type for a 32-bit int is 's32'
    u16 unsinged_16_bit_value; // the primitive type for a 16-bit uint is 'u16'
    s16 singed_16_bit_value; // the primitive type for a 16-bit int is 's16'
    u8 unsinged_8_bit_value; // the primitive type for a 8-bit uint is 'u8'
    s8 singed_8_bit_value; // the primitive type for a 8-bit int is 's8'
    f32 float_value; // the primitive type for a single is 'f32
    f64 double_value; // the primitive type for a double is 'f64'
    void* pointer_to_void; // pointers are declared in the same way as in c
    u32* pointer_to_u32; // however pointers should have bounds-checking within the VM
    StructName* pointer_to_StructName; // structs inherently declare theirselves
};

static StructOfPrimitiveTypes sPrimitiveTypes; // the 'static' keyword means that this symbol is not visible outside of this module
StructOfPrimitiveTypes* gpPrimitiveTypes = &sPrimitiveTypes; // this, however, is

void StructOfPrimitiveTypes_Construct(StructOfPrimitiveTypes* this) {
    this->pointer_to_StructName = this;
}

struct Vec3 {
    f32 x;
    f32 y;
    f32 z;
};

// structured operator functions can be explicitly declared
// this will be used for non-primitives if the lvalue and rvalue point to the types in this signature at compile time
+(Vec3* lvalue, Vec3* rvalue) {
    lvalue->x += rvalue->x;
    lvalue->y += rvalue->y;
    lvalue->z += rvalue->z;
}

// Initializers work the same as C
Vec3 up = { 0, 1, 0 };

// the 'native' keyword symbolizes something that is from the parent program
native s32 printf(const char* format, ...);
native u32 deadbeef; // I may consider making native values unwritable for safety

// you must declare functions with empty parameters as having void parameters
s32 main(void) {
    u32 badfood0 = 0xBADFOOD0;

    printf("Hello, world! %X\n", deadbeef); // calls printf from the host program, must be explicitly registered
    printf("I just ate %X at %P\n", badfood0, &badfood0);

    return 1;
}
```

