#pragma once

#include <stdio.h>
#include <stdint.h>
#include "Common/Typedefs.hpp"
#include "INESBus.hpp"
#include "Mapper/Mapper.hpp"

struct Sprite {
    u8 y;
    u8 tileNum;
    u8 attr;
    u8 x;
    u8 id;
};

struct SpriteRenderEntity {
    u8 lo;
    u8 hi;
    u8 attr;
    u8 counter;
    u8 id;
    bool flipHorizontally;
    bool flipVertically;
    bool isActive;
    int shifted = 0;
    
    void shift() {
        if (shifted == 7) {
            isActive = false;
            shifted = 0;
            return;
        }
        
        if (flipHorizontally) {
            lo >>= 1;
            hi >>= 1;
        } else {
            lo <<= 1;
            hi <<= 1;
        }
        
        shifted++;
    }
};

class PPU : public INESBus {
private:
    //Registers
    u8 ppuctrl = 0; //$2000
    u8 ppumask = 0; //$2001
    u8 ppustatus = 0x80; //$2002
    u8 ppustatus_cpy = 0;
    u8 oamaddr = 0; //$2003
    u8 oamdata = 0; //$2004
    u8 ppuscroll = 0; //$2005
    u8 ppu_read_buffer = 0;
    u8 ppu_read_buffer_cpy = 0;
    
    u32 palette[64] = { 
        4283716692, 4278197876, 4278718608, 4281335944, 4282646628, 4284219440, 4283696128, 4282128384,
        4280297984, 4278729216, 4278206464, 4278205440, 4278202940, 4278190080, 4278190080, 4278190080,
        4288190104, 4278734020, 4281348844, 4284227300, 4287108272, 4288681060, 4288160288, 4286069760,
        4283718144, 4280840704, 4278746112, 4278220328, 4278216312, 4278190080, 4278190080, 4278190080,
        4293717740, 4283210476, 4286086380, 4289749740, 4293154028, 4293679284, 4293683812, 4292118560,
        4288719360, 4285842432, 4283224096, 4281912428, 4281906380, 4282137660, 4278190080, 4278190080,
        4293717740, 4289252588, 4290559212, 4292129516, 4293701356, 4293701332, 4293702832, 4293182608,
        4291613304, 4290043512, 4289258128, 4288209588, 4288730852, 4288717472, 4278190080, 4278190080
    }; 
    
    //BG
    u8 bg_palette[16] = { 0 };
    u8 vram[2048] = { 0 };
    u16 v = 0, t = 0, v1 = 0;
    u8 x = 0;
    int w = 0;
    u8 ntbyte, attrbyte, patternlow, patternhigh;
    u16 bgShiftRegLo;
    u16 bgShiftRegHi;
    u16 attrShiftReg1;
    u16 attrShiftReg2;
    
    //Sprites
    u8 sprite_palette[16] = { 0 };
    u16 spritePatternLowAddr, spritePatternHighAddr;
    int primaryOAMCursor = 0;
    int secondaryOAMCursor = 0;
    Sprite primaryOAM[64];
    Sprite secondaryOAM[8];
    Sprite tmpOAM;
    bool inRange = false;
    int inRangeCycles = 8;
    int spriteHeight = 8;
    int spriteDelayCounter = 4;
    std::vector<SpriteRenderEntity> spriteRenderEntities;
    SpriteRenderEntity out;

    Mapper* mapper;
    
    int scanLine = 0;
    int dot = 0;
    int pixelIndex = 0;
    bool odd = false;
    bool nmiOccured = false;
    int mirroring;
    
    //methods
    inline void copyHorizontalBits();
    inline void copyVerticalBits();
    inline bool isRenderingDisabled();
    inline void emitPixel();
    inline void loadRegisters();
    inline void fetchTiles();
    inline void xIncrement();
    inline void yIncrement();
    inline void reloadShiftersAndShift();
    inline void decrementSpriteCounters();
    u16 getSpritePatternAddress(const Sprite&, bool);
    void evalSprites();
    bool inYRange(const Sprite&);
    bool isUninit(const Sprite&);
    
public:
    PPU(Mapper* mapper) : mapper(mapper) { };
    
    //cpu address space
    u8 read(u16 address);
    void write(u16 address, u8 data);
    
    //ppu address space
    u8 ppuread(u16 address);
    void ppuwrite(u16 address, u8 data);
    void setMirroring(int mirroring) { this->mirroring = mirroring; }
    
    void tick();
    void copyOAM(u8, int);
    u8 readOAM(int);
    bool genNMI();
    bool generateFrame;
    void printState();
    uint32_t buffer[256*240]  = { 0 };
};
