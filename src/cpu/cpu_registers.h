#pragma once

//CPU Constant Definitions
constexpr auto ICACHE_SIZE = 0x1000;				//4 KB
constexpr auto DCACHE_SIZE = 0x0400;				//1 KB
constexpr auto DCACHE_EN1_MASK = 0x00000008;
constexpr auto DCACHE_EN2_MASK = 0x00000080;
constexpr auto ICACHE_EN_MASK = 0x00000800;

// Decoding Unions
namespace cpu
{
	union Instruction
	{
		uint32_t		word;

		BitField<26, 6> op;		//opcode
		BitField<21, 5> rs;		//rs index
		BitField<16, 5> rt;		//rt index
		BitField<11, 5> rd;		//rd index
		BitField<6, 5>	shamt;	//shift amount
		BitField<0, 6>	funct;	//function
		BitField<0, 16> imm;	//immediate value
		BitField<0, 26> tgt;	//target
		BitField<0, 25> cofun;	//coprocessor function
		BitField<25, 1>	cop;	//cop	
	};

	union CauseRegister
	{
		uint32_t		word;

		BitField<2, 5>	excode;	//exception code
		BitField<8, 2>	ipsw;	//Software Interrupt Pending
		BitField<10, 6>	iphw;	//Hardware Interrupt Pending
		BitField<28, 2>	ce;		//Coprocessor number exception
		BitField<31, 1>	bd;		//Branch Delay Exception
	};

	union StatusRegister
	{
		uint32_t		word;

		BitField<0, 1>	iec;	//Current Interrupt enable
		BitField<1, 1>	kuc;	//Current Kernel/User mode
		BitField<2, 1>	iep;	//Previous Interrupt enable
		BitField<3, 1>	kup;	//Previous Kernel/User mode
		BitField<4, 1>	ieo;	//Old Interrupt enable
		BitField<5, 1>	kuo;	//Old Kernel/User mode
		BitField<8, 2>	imsw;	//Sotfware Interrupt Mask
		BitField<10, 6>	imhw;	//Hardware Interrupt Mask
		BitField<16, 1>	isc;	//Isolate Cache
		BitField<17, 1>	swc;	//Swappe Cache mode
		BitField<18, 1>	pz;		//When set parity bit are written az 0's
		BitField<19, 1>	cm;		//Set if last access to D-Cache contained actual data
		BitField<20, 1>	pe;		//Cache parity check
		BitField<21, 1>	ts;		//TLB Shutdown
		BitField<22, 1>	bev;	//Boot Exception Vector (0 = RAM, 1 = ROM)
		BitField<25, 1>	re;		//Reverse Endianess (0 = normal endianess, 1 = 0 reversed endianess)
		BitField<28, 1>	cu0;	//COP0 Enable
		BitField<29, 1> cu1;	//COP1 Enable
		BitField<30, 1>	cu2;	//COP2 Enable
		BitField<31, 1>	cu3;	//COP3 Enable

		BitField<0, 6>	stk;	//Interrupt Enable / Kernel Mode Stack
	};

	//Pipeline Buffers, used to pass Signals and Data from one stage to the next
	struct ifidBuffer
	{
		uint32_t	pc;
		uint32_t	instr;
		bool		branchDelaySlot;
	};

	struct idexBuffer
	{
		uint32_t	pc;
		uint32_t	regA;	//Content of rs (aka base)
		uint32_t	regB;	//Content of rt
		uint32_t	imm;	//32bit Sign extendend immediate value
		uint8_t		rs;
		uint8_t		rd;
		uint8_t		rt;
		uint8_t		op;
		uint8_t		funct;
		uint32_t	tgt;
		uint8_t		shamt;
		uint32_t	cofun;
		bool		cop;
		bool		branchDelaySlot;
	};

	struct exmemBuffer
	{
		uint32_t	pc;
		uint32_t	aluRes;
		uint8_t		rd;
		uint32_t	regB;
		uint8_t		bytes;
		bool		readMem;
		bool		readMemUnaligned;
		bool		readMemSignExt;
		bool		writeMem;
		bool		writeMemUnaligned;
		bool		memToReg;
		bool		aluToReg;

	};
	struct memwbBuffer
	{
		uint32_t	pc;
		uint32_t	memData;
		uint32_t	aluRes;
		uint8_t		rd;
		bool		memToReg;
		bool		aluToReg;
	};

	enum class exceptionCause
	{
		interrupt = 0x00,
		addrerrload = 0x04,
		addrerrstore = 0x05,
		instrbuserr = 0x06,
		databusserr = 0x07,
		syscall = 0x08,
		breakpoint = 0x09,
		resinst = 0x0a,
		copunusable = 0x0b,
		overflow = 0x0c
	};

	enum class interruptCause
	{
		vblank = 0x00,
		gpu = 0x01,
		cdrom = 0x02,
		dma = 0x03,
		timer0 = 0x04,
		timer1 = 0x05,
		timer2 = 0x06,
		byterecv = 0x07,
		sio = 0x08,
		spu = 0x09,
		controller = 0x0a
	};
};
