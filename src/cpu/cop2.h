#pragma once

#include <cstdint>
#include <string>
#include <cstdio>
#include <vector>
#include <algorithm>

#include "litelib.h"
#include "glm/glm.hpp"

static constexpr uint32_t GTE_CONTROL_REGISTER_NUMBER = 32;
static constexpr uint32_t GTE_DATA_REGISTER_NUMBER = 32;

//---Data Register Shortcut-----------------------------------------------------------------------------------
#define SX0 reg.sxy0.y
#define SY0 reg.sxy0.x
#define SX1 reg.sxy1.y
#define SY1 reg.sxy1.x
#define SX2 reg.sxy2.y
#define SY2 reg.sxy2.x

#define MAC0 reg.mac0


//---Control Register Shortcut-----------------------------------------------------------------------------------

namespace gte
{
    union operation
	{
		uint32_t		word;
	
		lite::bitfield<11, 5> rd;				//rd index
		lite::bitfield<16, 5> rt;				//rt index
		lite::bitfield<21, 5> op;               //Cop0 Sub Operation
	};

	union function
	{
		uint32_t		word;
		
		lite::bitfield<0, 6>  realcmd;          //Real GTE Command Number (00h..3Fh) (used by hardware)
		lite::bitfield<10, 1> lm;				//Saturate IR1,IR2,IR3 result (0=To -8000h..+7FFFh, 1=To 0..+7FFFh)
        lite::bitfield<13, 2> trans_vect;		//(0=TR, 1=BK, 2=FC/Bugged, 3=None)
        lite::bitfield<15, 2> mult_vect;		//(0=V0, 1=V1, 2=V2, 3=IR/long)
        lite::bitfield<17, 2> mult_matrix;      //(0=Rotation. 1=Light, 2=Color, 3=Reserved)
        lite::bitfield<19, 1> sf;               //Shift Fraction in IR registers (0=No fraction, 1=12bit fraction)
        lite::bitfield<20, 5> fakecmd;          //Fake GTE Command Number (00h..1Fh) (ignored by hardware)
	};

    union flags
    {
        uint32_t        word;
        
        lite::bitfield<12, 1> ir0_sat;      //IR0 saturated to +0000h..+1000h
        lite::bitfield<13, 1> sy2_sat;      //SY2 saturated to -0400h..+03FFh
        lite::bitfield<14, 1> sx2_sat;      //SX2 saturated to -0400h..+03FFh
        lite::bitfield<15, 1> mac0_uflow;   //MAC0 Result larger than 31 bits and negative
        lite::bitfield<16, 1> mac0_oflow;   //MAC0 Result larger than 31 bits and positive
        lite::bitfield<17, 1> div_oflow;    //Divide overflow. RTPS/RTPT division result saturated to max=1FFFFh
        lite::bitfield<18, 1> sz3_sat;      //SZ3 or OTZ saturated to +0000h..+FFFFh
        lite::bitfield<19, 1> b_sat;        //Color-FIFO-B saturated to +00h..+FFh
        lite::bitfield<20, 1> g_sat;        //Color-FIFO-G saturated to +00h..+FFh    
        lite::bitfield<21, 1> r_sat;        //Color-FIFO-R saturated to +00h..+FFh
        lite::bitfield<22, 1> ir3_sat;      //IR3 saturated to +0000h..+7FFFh (lm=1) or to -8000h..+7FFFh (lm=0)
        lite::bitfield<23, 1> ir2_sat;      //IR2 saturated to +0000h..+7FFFh (lm=1) or to -8000h..+7FFFh (lm=0)
        lite::bitfield<24, 1> ir1_sat;      //IR1 saturated to +0000h..+7FFFh (lm=1) or to -8000h..+7FFFh (lm=0)
        lite::bitfield<25, 1> mac3_uflow;   //MAC3 Result larger than 43 bits and negative
        lite::bitfield<26, 1> mac2_uflow;   //MAC2 Result larger than 43 bits and negative
        lite::bitfield<27, 1> mac1_uflow;   //MAC1 Result larger than 43 bits and negative
        lite::bitfield<28, 1> mac3_oflow;   //MAC3 Result larger than 43 bits and positive
        lite::bitfield<29, 1> mac2_oflow;   //MAC2 Result larger than 43 bits and positive
        lite::bitfield<30, 1> mac1_ofloq;   //MAC1 Result larger than 43 bits and positive
        lite::bitfield<31, 1> checksum;     //Error Flag Checksum (Bit30..23, and 18..13 ORed together) (Read only)
        
        inline void reset()     { word = 0x00000000; };
        inline void update()  { checksum = (bool)((word & 0x7f87e000) != 0x00000000);};
    };

    union registers
    {
        struct
        {
            uint32_t    data[GTE_DATA_REGISTER_NUMBER];      //Data Registers
            uint32_t    ctrl[GTE_CONTROL_REGISTER_NUMBER];   //Control Register
        };
        
    #pragma pack(push, 1)
        struct
        {
            //Data Registers
            lite::vec4t<int16_t> v0;
            lite::vec4t<int16_t> v1;
            lite::vec4t<int16_t> v2;
            lite::vec4t<uint8_t> rgbc;
            uint16_t    otz;
            uint16_t    pad4;
            int16_t     ir1;
            uint16_t    pad5;
            int16_t     ir2;
            uint16_t    pad6;
            int16_t     ir3;
            uint16_t    pad7;
            lite::vec2t<int16_t> sxy0;
            lite::vec2t<int16_t> sxy1;
            lite::vec2t<int16_t> sxy2;
            lite::vec2t<int16_t> sxyp;
            uint16_t    sz0;
            uint16_t    pad8;
            uint16_t    sz1;
            uint16_t    pad9;
            uint16_t    sz2;
            uint16_t    pad10;
            uint16_t    sz3;
            uint16_t    pad11;
            lite::vec4t<uint8_t> rgb0;
            lite::vec4t<uint8_t> rgb1;
            lite::vec4t<uint8_t> rgb2;
            uint32_t    res1;
            int32_t     mac0;
            int32_t     mac1;
            int32_t     mac2;
            int32_t     mac3;
            uint32_t    irgb;
            uint32_t    orgb;
            int32_t     lzcs;
            int32_t     lzcr;

            //Control Registers
            int16_t     rtm[3][3];
            uint16_t    pad12;
            lite::vec3t<int32_t> tr;
            int16_t     lsm[3][3];
            uint16_t    pad13;
            lite::vec3t<int32_t> rgb_bk;
            int16_t     lcm[3][3];
            uint16_t    pad14;
            lite::vec3t<int32_t> rgb_far;
            lite::vec2t<int32_t> of;
            uint16_t    h;
            uint16_t    pad15;
            int16_t     dqa;
            uint16_t    pad16;
            int32_t     dqb;
            int16_t     zsf3;
            uint16_t    pad17;
            int16_t     zsf4;
            uint16_t    pad18;
            gte::flags flag;    
        };
    #pragma pack(pop)    
    };
}

class CPU;

class Cop2
{
public:
    Cop2(CPU* instance);
    ~Cop2();

    //Cop2 Interface
    bool reset();
    bool execute (uint32_t cofun);

    //Helper Functions
    bool checkOverflow(int64_t value, uint8_t size);
    bool checkUnderflow(int64_t value, uint8_t size);

public:
    //GTE Registers
    gte::registers    reg;

private:
    //GTE Command
    bool cmd_unknown();
    bool cmd_rtps();
    bool cmd_nclip();
    bool cmd_op();
    bool cmd_dcps();
    bool cmd_intpl();
    bool cmd_mvmva();
    bool cmd_ncds();
    bool cmd_cdp();
    bool cmd_ncdt();
    bool cmd_nccs();
    bool cmd_cc();
    bool cmd_ncs();
    bool cmd_nct();
    bool cmd_sqr();
    bool cmd_dcpl();
    bool cmd_dpct();
    bool cmd_avsz3();
    bool cmd_avsz4();
    bool cmd_rtpt();
    bool cmd_gpf();
    bool cmd_gpl();
    bool cmd_ncct();

private:
	CPU* cpu;

    //Running COP2 Operation and GTE Function
    gte::operation currentOperation;
    gte::function  currentFunction;

    //Full set of CPU Instruction Dictionaries
	struct GTECOMMAND
	{
		std::string mnemonic;
		bool(Cop2::* operate)() = nullptr;
        uint32_t clockticks;
	};

    std::vector<GTECOMMAND> commandSet;		//Full GTE Command Set
};