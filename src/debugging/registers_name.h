#pragma once

#include <string>

//Registers names
std::string cpuRegisterName[32] = { "zr", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
									"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};

std::string cop0RegisterName[32] = { "", "", "", "BPC", "", "BDA", "JDEST", "DCIC", "BadVaddr", "BDAM", "", "BPCM", "SR",
									"Cause", "EPC", "PDId", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "" };

std::string cop2RegisterName[64] = { "VXY0", "VZ0", "VXY1", "VZ1", "VXY2", "VZ2", "RGBC", "OTZ", "IR0", "IR1", "IR2", "IR3", "SXY0", 
									  "SXY1", "SXY2", "SXYP", "SZ0", "SZ1", "SZ2", "SZ3", "RGB0", "RGB1", "RGB2", "RES1", "MAC0", 
									  "MAC1", "MAC2", "MAC3", "IRGB", "ORGB", "LZCS", "LZCR", "RT11-12", "RT13-21", "RT22-23", "RT31-32",
									  "RT33", "TRX", "TRY", "TRZ", "L11-12", "L13-21", "L22-23", "L31-32", "L33", "RBK", "GBK", "BBK",
									  "LR1-R2", "LR3-G1", "LG2-G3", "LB1-B2", "LB4", "RFC", "GFC", "BFC", "OFX", "OFY", "H", "DQA", "DQB",
									  "ZSF3", "ZSF4", "FLAG"};