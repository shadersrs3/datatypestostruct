Add fields to a structure on a fixed size setting, 

example code for msvc based on main.cpp

#pragma pack(push, 4)

struct mystruct {
    uint8_t hello; // 0x0
    
    uint8_t unk_0[0xf]; // 0x1
    
    uint32_t gello[0x14]; // 0x10
    
    uint8_t unk_1[0x3a0]; // 0x60
    
    /* ^--- remove pad if needed */
};

#pragma pack(pop)
