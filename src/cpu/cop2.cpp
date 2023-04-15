#include <loguru.hpp>
#include "cop2.h"
#include "cpu_short_pipe.h"

Cop2::Cop2(CPU* instance)
{
	//Link cop2 to cpu
	cpu = instance;
	
    //Reset Cop2 Registers
	std::memset(reg.data, 0x0, sizeof(uint32_t)*GTE_DATA_REGISTER_NUMBER);
	std::memset(reg.ctrl, 0x0, sizeof(uint32_t)*GTE_CONTROL_REGISTER_NUMBER);

	//Init GTE Command Dictionaries
	commandSet =
	{
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"rtps", &Cop2::cmd_rtps, 15},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"nclip", &Cop2::cmd_nclip, 8},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"op", &Cop2::cmd_op, 6},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"dcps", &Cop2::cmd_dcps, 8},
		{"intpl", &Cop2::cmd_intpl, 8},
		{"mvmva", &Cop2::cmd_mvmva, 8,},
		{"ncds", &Cop2::cmd_ncds, 19},
		{"cpd", &Cop2::cmd_cdp, 13},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"ncdt", &Cop2::cmd_ncdt, 44},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"nccs", &Cop2::cmd_nccs, 17},
		{"cc", &Cop2::cmd_cc, 11},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"ncs", &Cop2::cmd_ncs, 14},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"nct", &Cop2::cmd_nct, 30},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"sqr", &Cop2::cmd_sqr, 5},
		{"dcpl", &Cop2::cmd_dcpl, 8},
		{"dcpt", &Cop2::cmd_dpct, 17}, 
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"avsz3", &Cop2::cmd_avsz3, 5},
		{"avsz4", &Cop2::cmd_avsz4, 6},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"rtpt", &Cop2::cmd_rtpt, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"invalid command", &Cop2::cmd_unknown, 0},
		{"gpf", &Cop2::cmd_gpf, 5},
		{"gpl", &Cop2::cmd_gpl, 5},
		{"ncct", &Cop2::cmd_ncct, 39}
	};
};

Cop2::~Cop2()
{

};

bool Cop2::reset()
{
    //Reset Cop0 Registers
	std::memset(reg.data, 0x0, sizeof(uint32_t)*GTE_DATA_REGISTER_NUMBER);
	std::memset(reg.ctrl, 0x0, sizeof(uint32_t)*GTE_CONTROL_REGISTER_NUMBER);

	return true;
}

bool Cop2::execute(uint32_t cofun)
{
	//Parse Current Operation
	currentOperation.word = cofun;

	switch(currentOperation.op)
	{
	case 0x00:	//mfc2 rt, rd
		if (currentOperation.rt != 0)
			cpu->gpr[currentOperation.rt] = reg.data[currentOperation.rd];
			LOG_F(3, "COP2 - mfc2 $%d, $%d", (uint8_t)currentOperation.rt, (uint8_t)currentOperation.rd);
		break;

	case 0x02:	//cfc2 rt, rd
		if (currentOperation.rt != 0)
			cpu->gpr[currentOperation.rt] = reg.ctrl[currentOperation.rd];
			LOG_F(3, "COP2 - cfc2 $%d, $%d", (uint8_t)currentOperation.rt, (uint8_t)currentOperation.rd);
		break;

	case 0x04:	//mtc2 rt, rd
		reg.data[currentOperation.rd] = cpu->gpr[currentOperation.rt];
		LOG_F(3, "COP2 - mtc2 $%d, $%d", (uint8_t)currentOperation.rt, (uint8_t)currentOperation.rd);
		break;

	case 0x06:	//ctc2 rt, rd
		reg.ctrl[currentOperation.rd] = cpu->gpr[currentOperation.rt];
		LOG_F(3, "COP2 - cfc2 $%d, $%d", (uint8_t)currentOperation.rt, (uint8_t)currentOperation.rd);
		break;
	
	default:
		//Parse Current GTE Function
		currentFunction.word = cofun;

		bool bResult;
		LOG_F(1, "COP2 - Command %s", commandSet[currentFunction.realcmd].mnemonic.c_str());
		bResult = (this->*commandSet[currentFunction.realcmd].operate)();
		if (!bResult)
			LOG_F(ERROR, "COP2 - Unimplemented Command %s", commandSet[currentFunction.realcmd].mnemonic.c_str());
	}

	return true;
}

//-------------------------------------------------------------------------------------------
// HELPER FUNCTIONS
//-------------------------------------------------------------------------------------------
bool Cop2::checkOverflow(int64_t value, uint8_t size)
{
	bool result;

	result = (value > 0) && ((value >> size) != 0);
	return result;
}

bool Cop2::checkUnderflow(int64_t value, uint8_t size)
{
	return checkOverflow(-value, size);
}

//-------------------------------------------------------------------------------------------
// GTE COMMANDS
//-------------------------------------------------------------------------------------------
bool Cop2::cmd_unknown()
{
	return false;
}

bool Cop2::cmd_rtps()
{
	return false;
}

bool Cop2::cmd_nclip()
{
	// in:      SXY0,SXY1,SXY2    Screen coordinates                  [1,15,0]
	// out:     MAC0              Outerproduct of SXY1 and SXY2 with  [1,31,0]
    //                            SXY0 as origin.

	// Calculation:
	// [1,31,0] MAC0 = F[SX0*SY1+SX1*SY2+SX2*SY0-SX0*SY2-SX1*SY0-SX2*SY1] [1,43,0]

	//Reset Flags
	reg.flag.reset();

	int64_t res;
	res = (int64_t)(SX0*SY1+SX1*SY2+SX2*SY0-SX0*SY2-SX1*SY0-SX2*SY1);
	MAC0 = (int32_t)res;

	//Set Flags
	reg.flag.mac0_oflow =(bool)checkOverflow(res, 31);
	reg.flag.mac0_uflow =(bool)checkUnderflow(res, 31);

	//Update Flag Checksum
	reg.flag.update();

	return true;
}

bool Cop2::cmd_op()
{
	return false;
}

bool Cop2::cmd_dcps()
{
	return false;
}

bool Cop2::cmd_intpl()
{
	return false;
}

bool Cop2::cmd_mvmva()
{
	return false;
}

bool Cop2::cmd_ncds()
{
	// In:  V0                Normal vector                       [1,3,12]
    // 	    BK                Background color       RBK,GBK,BBK  [1,19,12]
    //      RGB               Primary color          R,G,B,CODE   [0,8,0]
    //      LLM               Light matrix                        [1,3,12]
    //      LCM               Color matrix                        [1,3,12]
    //      IR0               Interpolation value                 [1,3,12]
    // Out: RGBn              RGB fifo.              Rn,Gn,Bn,CDn [0,8,0]
    //      [IR1,IR2,IR3]     Color vector                        [1,11,4]
    //      [MAC1,MAC2,MAC3]  Color vector                        [1,27,4]

	// Calculation:
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	// [1,19,12] MAC2=A2[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	// [1,19,12] MAC3=A3[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	// [1,19,12] MAC2=A2[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	// [1,19,12] MAC3=A3[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	// [1,27,4]  MAC1=A1[R*IR1 + IR0*(Lm_B1[RFC-R*IR1])]              [1,27,16][lm=0]
	// [1,27,4]  MAC2=A2[G*IR2 + IR0*(Lm_B2[GFC-G*IR2])]              [1,27,16][lm=0]
	// [1,27,4]  MAC3=A3[B*IR3 + IR0*(Lm_B3[BFC-B*IR3])]              [1,27,16][lm=0]
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,27,4][lm=1]
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,27,4][lm=1]
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,27,4][lm=1]
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]

	//Reset Flags
	reg.flag.reset();



	//Update Flag Checksum
	reg.flag.update();

	return true;
}

bool Cop2::cmd_cdp()
{
	return false;
}

bool Cop2::cmd_ncdt()
{
	return false;
}

bool Cop2::cmd_nccs()
{
	return false;
}

bool Cop2::cmd_cc()
{
	return false;
}

bool Cop2::cmd_ncs()
{
	return false;
}

bool Cop2::cmd_nct()
{
	return false;
}

bool Cop2::cmd_sqr()
{
	return false;
}

bool Cop2::cmd_dcpl()
{
	return false;
}

bool Cop2::cmd_dpct()
{
	return false;
}

bool Cop2::cmd_avsz3()
{
	//Average of three Z values

	//cop2r24    1xS32 MAC0                  32bit Maths Accumulators (Value)
	//cop2r16-19 4xU16 SZ0,SZ1,SZ2,SZ3       Screen Z-coordinate FIFO   (4 stages)
	//cop2r61-62 2xS16 ZSF3,ZSF4             Average Z scale factors
	//cop2r7     1xU16 OTZ                   Average Z value

	//MAC0 =  ZSF3*(SZ1+SZ2+SZ3)       ;for AVSZ3
  	//OTZ  =  MAC0 >> 12               ;for both (saturated to 0..FFFFh)
	//MAC0 = (int16_t)ZSF3 * ((uint16_t)SZ1 + (uint16_t)SZ2 + (uint16_t)SZ3);
	//OTZ = (int32_t)MAC0 >> 12;
	//LOG_F(WARNING, "avsz in [%08x, %08x, %08x, %08x]  out [%08x, %08x]", ZSF3, SZ1, SZ2, SZ3, MAC0, OTZ);

	return false;
}

bool Cop2::cmd_avsz4()
{
	return false;
}

bool Cop2::cmd_rtpt()
{
	
	//RTPT - Perspective Transformation (triple)

	//Reset flags
	reg.flag.reset();

	//cop2r8     1xS16 IR0                   16bit Accumulator (Interpolate)
	//cop2r9-11  3xS16 IR1,IR2,IR3           16bit Accumulator (Vector)
	//cop2r24    1xS32 MAC0                  32bit Maths Accumulators (Value)
	//cop2r25-27 3xS32 MAC1,MAC2,MAC3        32bit Maths Accumulators (Vector)
	//cop2r37-39 3xS32 TRX,TRY,TRZ           Translation vector  (X,Y,Z)
	//cop2r32-36 9xS16 RT11RT12,..,RT33      Rotation matrix     (3x3)
	//cop2r56-57 2x 32 OFX,OFY               Screen offset       (X,Y)
	//cop2r12-15 6xS16 SXY0,SXY1,SXY2,SXYP   Screen XY-coordinate FIFO
	//cop2r16-19 4xU16 SZ0,SZ1,SZ2,SZ3       Screen Z-coordinate FIFO   (4 stages)
	//cop2r59      S16 DQA                   Depth queing parameter A (coeff)
	//cop2r60       32 DQB                   Depth queing parameter B (offset)
	//cop2r58      U16 H                     Projection plane distance.
	//cop2r0-1   3xS16 VXY0,VZ0              V0 (X,Y,Z)
	//cop2r2-3   3xS16 VXY0,VZ0              V1 (X,Y,Z)
	//cop2r4-5   3xS16 VXY0,VZ0              V2 (X,Y,Z)

	// IR1 = MAC1 = (TRX*1000h + RT11*VX0 + RT12*VY0 + RT13*VZ0) SAR (sf*12)
	// IR2 = MAC2 = (TRY*1000h + RT21*VX0 + RT22*VY0 + RT23*VZ0) SAR (sf*12)
	// IR3 = MAC3 = (TRZ*1000h + RT31*VX0 + RT32*VY0 + RT33*VZ0) SAR (sf*12)
	// SZ3 = MAC3 SAR ((1-sf)*12)                           ;ScreenZ FIFO 0..+FFFFh
	// MAC0=(((H*20000h/SZ3)+1)/2)*IR1+OFX, SX2=MAC0/10000h ;ScrX FIFO -400h..+3FFh
	// MAC0=(((H*20000h/SZ3)+1)/2)*IR2+OFY, SY2=MAC0/10000h ;ScrY FIFO -400h..+3FFh
	// MAC0=(((H*20000h/SZ3)+1)/2)*DQA+DQB, IR0=MAC0/1000h  ;Depth cueing 0..+1000h

	// auto pushSXY = [&](uint32_t SXY)
	// {
	// 	SXY0 = SXY1;
	// 	SXY1 = SXY2;
	// 	SXY2 = SXYP;
	// 	SXY2 = SXY;
	// 	return 0;
	// };

	// auto pushSZ = [&](uint32_t SZ)
	// {
	// 	SZ0 = SZ1;
	// 	SZ1 = SZ2;
	// 	SZ2 = SZ3;
	// 	SZ3 = SZ;
	// 	return 0;
	// };
	
	// lite::fixpnt<int16_t, 3, 12> rt[3][3];
	// lite::fixpnt<int16_t, 7, 8> dqa, dqb;
	// lite::fixpnt<int32_t, 15, 16> ofx, ofy;
	// lite::fixpnt<int64_t, 51> x, y, z;
	// lite::fixpnt<int64_t, 47> a, b;
	// lite::fixpnt<int64_t, 44> u;
	// int16_t sx, sy;
	// float tmp;

	// //Load Rotation Matrix
	// rt[0][0].raw(R11); rt[0][1].raw(R12); rt[0][2].raw(R13);
	// rt[1][0].raw(R21); rt[1][1].raw(R22); rt[1][2].raw(R23);
	// rt[2][0].raw(R31); rt[2][1].raw(R32); rt[2][2].raw(R33);

	// //Load Screen Offset Values
	// ofx.raw(OFX); ofy.raw(OFY);

	// //Load Depth Queue Interpolation Values
	// dqa.raw(DQA); dqb.raw(DQB);

	// //------------------V0
	// x = (int32_t)TRX + (float)rt[0][0] * (int16_t)VX0 + (float)rt[0][1] * (int16_t)VY0 + (float)rt[0][2] * (int16_t)VZ0;
	// y = (int32_t)TRY + (float)rt[1][0] * (int16_t)VX0 + (float)rt[1][1] * (int16_t)VY0 + (float)rt[1][2] * (int16_t)VZ0; 
	// z = (int32_t)TRZ + (float)rt[2][0] * (int16_t)VX0 + (float)rt[2][1] * (int16_t)VY0 + (float)rt[2][2] * (int16_t)VZ0;
	// MAC1 = (int32_t)x; IR1 = std::clamp((int32_t)MAC1, -32768, 32767);
	// MAC2 = (int32_t)y; IR2 = std::clamp((int32_t)MAC2, -32768, 32767);
	// MAC3 = (int32_t)z; IR3 = std::clamp((int32_t)MAC3, -32768, 32767);
	// pushSZ(std::clamp((int32_t)MAC3, 0, 65535));

	// tmp = (float)H / (float)SZ3;
	// a = (float)ofx + (int16_t)IR1 * tmp; MAC0 = (int32_t)a; sx = std::clamp((int32_t)MAC0, -1024, 1023);
	// b = (float)ofy + (int16_t)IR2 * tmp; MAC0 = (int32_t)b; sy = std::clamp((int32_t)MAC0, -1024, 1023);
	// pushSXY((sx << 16) | (sy &0x0000fff));

	// u = (float)dqb + (float)dqa * ((uint16_t)H / (uint16_t)SZ3); MAC0 = (int32_t)u; IR0 = std::clamp((int32_t)MAC0, 0, 4096);

	// //------------------V1
	// x = (int32_t)TRX + (float)rt[0][0] * (int16_t)VX1 + (float)rt[0][1] * (int16_t)VY1 + (float)rt[0][2] * (int16_t)VZ1;
	// y = (int32_t)TRY + (float)rt[1][0] * (int16_t)VX1 + (float)rt[1][1] * (int16_t)VY1 + (float)rt[1][2] * (int16_t)VZ1; 
	// z = (int32_t)TRZ + (float)rt[2][0] * (int16_t)VX1 + (float)rt[2][1] * (int16_t)VY1 + (float)rt[2][2] * (int16_t)VZ1;
	// MAC1 = (int32_t)x; IR1 = std::clamp((int32_t)MAC1, -32768, 32767);
	// MAC2 = (int32_t)y; IR2 = std::clamp((int32_t)MAC2, -32768, 32767);
	// MAC3 = (int32_t)z; IR3 = std::clamp((int32_t)MAC3, -32768, 32767);
	// pushSZ(std::clamp((int32_t)MAC3, 0, 65535));
	
	// tmp = (float)H / (float)SZ3;
	// a = (float)ofx + (int16_t)IR1 * tmp; MAC0 = (int32_t)a; sx = std::clamp((int32_t)MAC0, -1024, 1023);
	// b = (float)ofy + (int16_t)IR2 * tmp; MAC0 = (int32_t)b; sy = std::clamp((int32_t)MAC0, -1024, 1023);
	// pushSXY((sx << 16) | (sy &0x0000fff));

	// u = (float)dqb + (float)dqa * ((uint16_t)H / (uint16_t)SZ3); MAC0 = (int32_t)u; IR0 = std::clamp((int32_t)MAC0, 0, 4096);

	// //------------------V2
	// x = (int32_t)TRX + (float)rt[0][0] * (int16_t)VX2 + (float)rt[0][1] * (int16_t)VY2 + (float)rt[0][2] * (int16_t)VZ2;
	// y = (int32_t)TRY + (float)rt[1][0] * (int16_t)VX2 + (float)rt[1][1] * (int16_t)VY2 + (float)rt[1][2] * (int16_t)VZ2; 
	// z = (int32_t)TRZ + (float)rt[2][0] * (int16_t)VX2 + (float)rt[2][1] * (int16_t)VY2 + (float)rt[2][2] * (int16_t)VZ2;
	// MAC1 = (int32_t)x; IR1 = std::clamp((int32_t)MAC1, -32768, 32767);
	// MAC2 = (int32_t)y; IR2 = std::clamp((int32_t)MAC2, -32768, 32767);
	// MAC3 = (int32_t)z; IR3 = std::clamp((int32_t)MAC3, -32768, 32767);
	// pushSZ(std::clamp((int32_t)MAC3, 0, 65535));

	// tmp = (float)H / (float)SZ3;
	// a = (float)ofx + (int16_t)IR1 * tmp; MAC0 = (int32_t)a; sx = std::clamp((int32_t)MAC0, -1024, 1023);
	// b = (float)ofy + (int16_t)IR2 * tmp; MAC0 = (int32_t)b; sy = std::clamp((int32_t)MAC0, -1024, 1023);
	// pushSXY((sx << 16) | (sy &0x0000fff));

	// u = (float)dqb + (float)dqa * ((uint16_t)H / (uint16_t)SZ3); MAC0 = (int32_t)u; IR0 = std::clamp((int32_t)MAC0, 0, 4096);

	// LOG_F(WARNING, "RT : (%f) (%f) (%f)", (float)rt[0][0], (float)rt[0][1], (float)rt[0][2]);
	// LOG_F(WARNING, "RT : (%f) (%f) (%f)", (float)rt[1][0], (float)rt[1][1], (float)rt[1][2]);
	// LOG_F(WARNING, "RT : (%f) (%f) (%f)", (float)rt[2][0], (float)rt[2][1], (float)rt[2][2]);
	// LOG_F(WARNING, "TR : (%d) (%d) (%d)", (int32_t)TRX, (int32_t)TRY, (int32_t)TRZ);
	// LOG_F(WARNING, "OF : (%f) (%f)     ", (float)ofx, (float)ofy);
	// LOG_F(WARNING, "DQ : (%f) (%f)     ", (float)dqa, (float)dqb);
	// LOG_F(WARNING, "H  : (%d)          ", (uint16_t)H);
	// LOG_F(WARNING, "TMP: (%f)          ", tmp);

	// LOG_F(WARNING, "V0 : (%d) (%d) (%d)", (int16_t)VX0, (int16_t)VY0, (int16_t)VZ0 );
	// LOG_F(WARNING, "V1 : (%d) (%d) (%d)", (int16_t)VX1, (int16_t)VY1, (int16_t)VZ1 );
	// LOG_F(WARNING, "V2 : (%d) (%d) (%d)", (int16_t)VX2, (int16_t)VY2, (int16_t)VZ2 );

	// LOG_F(WARNING, "S0 : (%d) (%d) (%d)", (int16_t)SX0, (int16_t)SY0, (int16_t)SZ1 );
	// LOG_F(WARNING, "S1 : (%d) (%d) (%d)", (int16_t)SX1, (int16_t)SY1, (int16_t)SZ2 );
	// LOG_F(WARNING, "S2 : (%d) (%d) (%d)", (int16_t)SX2, (int16_t)SY2, (int16_t)SZ3 );
	
	// LOG_F(WARNING, "MAC: (%d) (%d) (%d) (%d)", (int32_t)MAC0, (int32_t)MAC1, (int32_t)MAC2, (int32_t)MAC3);
	// LOG_F(WARNING, "IR:  (%d) (%d) (%d)", (int16_t)IR1, (int16_t)IR2, (int16_t)IR3);

	//LOG_F(WARNING, "V0d: (%d) (%d) (%d)", (int16_t)reg.data[0] >> 16, (int16_t)reg.data[0] & 0x0000ffff, (int16_t)reg.data[1] & 0x0000ffff );
	//LOG_F(WARNING, "V1d: (%d) (%d) (%d)", (int16_t)reg.data[2] >> 16, (int16_t)reg.data[2] & 0x0000ffff, (int16_t)reg.data[3] & 0x0000ffff );
	//LOG_F(WARNING, "V2d: (%d) (%d) (%d)", (int16_t)reg.data[4] >> 16, (int16_t)reg.data[4] & 0x0000ffff, (int16_t)reg.data[5] & 0x0000ffff );
	// LOG_F(WARNING, "V0x: (0x%04x) (0x%04x) (0x%04x)", reg.data[0] >> 16, reg.data[0] & 0x0000ffff, reg.data[1]);
	// LOG_F(WARNING, "V1x: (0x%04x) (0x%04x) (0x%04x)", reg.data[2] >> 16, reg.data[2] & 0x0000ffff, reg.data[3]);
	// LOG_F(WARNING, "V2x: (0x%04x) (0x%04x) (0x%04x)", reg.data[4] >> 16, reg.data[4] & 0x0000ffff, reg.data[5]);
	// LOG_F(WARNING, "V0d: (%d) (%d) (%d)", (int16_t)(reg.data[0] >> 16), (int16_t)(reg.data[0] & 0x0000ffff), (int16_t)reg.data[1]);
	// LOG_F(WARNING, "V1d: (%d) (%d) (%d)", (int16_t)(reg.data[2] >> 16), (int16_t)(reg.data[2] & 0x0000ffff), (int16_t)reg.data[3]);
	// LOG_F(WARNING, "V2d: (%d) (%d) (%d)", (int16_t)(reg.data[4] >> 16), (int16_t)(reg.data[4] & 0x0000ffff), (int16_t)reg.data[5]);
	// LOG_F(WARNING, "---------------");

	// LOG_F(WARNING, "V0 : (%d) (%d) (%d)", reg.v0.x, reg.v0.y, reg.v0.z);
	// LOG_F(WARNING, "V1 : (%d) (%d) (%d)", reg.v1.x, reg.v1.y, reg.v1.z);
	// LOG_F(WARNING, "V2 : (%d) (%d) (%d)", reg.v2.x, reg.v2.y, reg.v2.z);
	// LOG_F(WARNING, "---------------");
	
	//Update Flag Checksum
	reg.flag.update();
	return false;
}

bool Cop2::cmd_gpf()
{
	return false;
}

bool Cop2::cmd_gpl()
{
	return false;
}

bool Cop2::cmd_ncct()
{
	return false;
}