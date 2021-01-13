/*
 *  Copyright (C) 2008-2018 Florent Bedoiseau (electronique.fb@free.fr)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma once

class ZXMemory;
class ZXIO;
class ZXULA;

#define Z80_IGNORE_INT  -1   /* Ignore interrupt                            */
#define Z80_NMI_INT     -2   /* Execute NMI                                 */
#if 1
typedef union {
  struct { uint8_t l,h,h2,h3; } __attribute__((aligned (4)))B;
  struct { uint16_t l,h; } __attribute__((aligned (4)))W;
  uint32_t D;
} __attribute__((aligned (4))) z80_pair;
#else
typedef union {
  struct { uint8_t l,h,h2,h3; } B;
  struct { uint16_t l,h; } W;
  uint32_t D;
} z80_pair;
#endif


/****************************************************************************/
/* The Z80 registers. HALT is set to 1 when the CPU is halted, the refresh  */
/* register is calculated as follows: refresh=(Regs.R&127)|(Regs.R2&128)    */
/****************************************************************************/
class Z80_Regs {
 public:
  Z80_Regs ();
  ~Z80_Regs ();
  Z80_Regs (const Z80_Regs &aRegs);
  Z80_Regs& operator=(const Z80_Regs &aRegs);
  void Dump (void);

  z80_pair AF;
  z80_pair BC;
  z80_pair DE;
  z80_pair HL;
  z80_pair IX;
  z80_pair IY;

  z80_pair AF2;
  z80_pair BC2;
  z80_pair DE2;
  z80_pair HL2;

  z80_pair PC;
  z80_pair SP;

  // Interrupt
  // IFF1 <= 0 after DI instr
  // IFF1 <= 1 after EI instr
  // IFF1 <= IFF2 after RETN instr
  // IFF2 <= 0 after DI instr
  // IFF2 <= 1 after EI instr
  unsigned IFF1; // Interrupt mask
  unsigned IFF2; // Flip-flop

  unsigned HALT; // = 1 after HALT, = 0 after Interrupt

  // Interrupt mode
  // IM <= 0 after IM_0 instr
  // IM <= 1 after IM_1 instr
  // IM <= 2 after IM_2 instr
  unsigned IM;

  // Interrupt vector
  unsigned I; // MSB of the indirect address. LSB is given by the data bus
  unsigned R; // Refresh register.
  unsigned R2;

  int pending_irq, pending_nmi;
};





#define DoIn(lo,hi)     Z80_In((lo)+(((uint16_t)(hi))<<8))
#define DoOut(lo,hi,v)  Z80_Out((lo)+(((uint16_t)(hi))<<8),v)

#define S_FLAG          0x80
#define Z_FLAG          0x40
#define H_FLAG          0x10
#define V_FLAG          0x04
#define N_FLAG          0x02
#define C_FLAG          0x01

#define M_SKIP_CALL     R.PC.W.l+=2
#define M_SKIP_JP       R.PC.W.l+=2
#define M_SKIP_JR       R.PC.W.l+=1
#define M_SKIP_RET

#define M_C     (R.AF.B.l&C_FLAG)
#define M_NC    (!M_C)
#define M_Z     (R.AF.B.l&Z_FLAG)
#define M_NZ    (!M_Z)
#define M_M     (R.AF.B.l&S_FLAG)
#define M_P     (!M_M)
#define M_PE    (R.AF.B.l&V_FLAG)
#define M_PO    (!M_PE)

#define M_XIX       ((R.IX.D+(int8_t)Z80_RDMEM_OPCODE())&0xFFFF)
#define M_XIY       ((R.IY.D+(int8_t)Z80_RDMEM_OPCODE())&0xFFFF)
#define M_RD_XHL    Z80_RDMEM(R.HL.D)


#define M_POP(Rg)											\
  R.Rg.D=Z80_RDMEM(R.SP.D)+(Z80_RDMEM((uint16_t)(R.SP.D+1))<<8); \
  R.SP.W.l+=2
#define M_PUSH(Rg)										\
  R.SP.W.l-=2;											\
  Z80_WRMEM(R.SP.D,R.Rg.B.l);	/* NS 980720 */			\
  Z80_WRMEM((uint16_t)(R.SP.D+1),R.Rg.B.h)	/* NS 980720 */
#define M_CALL									\
  {												\
	int q;										\
	q=Z80_RDMEM_OPCODE_U16();					\
	M_PUSH(PC);									\
	R.PC.D=q;									\
	Z80_ICount-=7;								\
  }
#define M_JP													\
  R.PC.D=Z80_RDMEM(R.PC.D)+((Z80_RDMEM((uint16_t)(R.PC.D+1)))<<8)	\
	;
#define M_JR											\
  R.PC.W.l+=((int8_t)Z80_RDMEM(R.PC.D))+1; Z80_ICount-=5	\
  ;
#define M_RET           M_POP(PC); Z80_ICount-=6	\
  ;
#define M_RST(Addr)     M_PUSH(PC); R.PC.D=Addr	\
									  ;
#define M_SET(Bit,Reg)  Reg|=1<<Bit
#define M_RES(Bit,Reg)  Reg&=~(1<<Bit)
#define M_BIT(Bit,Reg)								\
  R.AF.B.l=(R.AF.B.l&C_FLAG)|H_FLAG|				\
	((Reg&(1<<Bit))? ((Bit==7)?S_FLAG:0):Z_FLAG)
#define M_AND(Reg)      R.AF.B.h&=Reg; R.AF.B.l=ZSPTable[R.AF.B.h]|H_FLAG
#define M_OR(Reg)       R.AF.B.h|=Reg; R.AF.B.l=ZSPTable[R.AF.B.h]
#define M_XOR(Reg)      R.AF.B.h^=Reg; R.AF.B.l=ZSPTable[R.AF.B.h]
#define M_IN(Reg)								\
  Reg=DoIn(R.BC.B.l,R.BC.B.h);					\
  R.AF.B.l=(R.AF.B.l&C_FLAG)|ZSPTable[Reg]

#define M_RLCA									\
  R.AF.B.h=(R.AF.B.h<<1)|((R.AF.B.h&0x80)>>7);	\
  R.AF.B.l=(R.AF.B.l&0xEC)|(R.AF.B.h&C_FLAG)

#define M_RRCA									\
  R.AF.B.l=(R.AF.B.l&0xEC)|(R.AF.B.h&C_FLAG);	\
  R.AF.B.h=(R.AF.B.h>>1)|(R.AF.B.h<<7)

#define M_RLA										\
  {													\
	int i;											\
	i=R.AF.B.l&C_FLAG;								\
	R.AF.B.l=(R.AF.B.l&0xEC)|((R.AF.B.h&0x80)>>7);	\
	R.AF.B.h=(R.AF.B.h<<1)|i;						\
  }

#define M_RRA									\
  {												\
	int i;										\
	i=R.AF.B.l&C_FLAG;							\
	R.AF.B.l=(R.AF.B.l&0xEC)|(R.AF.B.h&0x01);	\
	R.AF.B.h=(R.AF.B.h>>1)|(i<<7);				\
  }

#define M_RLC(Reg)								\
  {												\
	int q;										\
	q=Reg>>7;									\
	Reg=(Reg<<1)|q;								\
	R.AF.B.l=ZSPTable[Reg]|q;					\
  }
#define M_RRC(Reg)								\
  {												\
	int q;										\
	q=Reg&1;									\
	Reg=(Reg>>1)|(q<<7);						\
	R.AF.B.l=ZSPTable[Reg]|q;					\
  }
#define M_RL(Reg)								\
  {												\
	int q;										\
	q=Reg>>7;									\
	Reg=(Reg<<1)|(R.AF.B.l&1);					\
	R.AF.B.l=ZSPTable[Reg]|q;					\
  }
#define M_RR(Reg)								\
  {												\
	int q;										\
	q=Reg&1;									\
	Reg=(Reg>>1)|(R.AF.B.l<<7);					\
	R.AF.B.l=ZSPTable[Reg]|q;					\
  }
#define M_SLL(Reg)								\
  {												\
	int q;										\
	q=Reg>>7;									\
	Reg=(Reg<<1)|1;								\
	R.AF.B.l=ZSPTable[Reg]|q;					\
  }
#define M_SLA(Reg)								\
  {												\
	int q;										\
	q=Reg>>7;									\
	Reg<<=1;									\
	R.AF.B.l=ZSPTable[Reg]|q;					\
  }
#define M_SRL(Reg)								\
  {												\
	int q;										\
	q=Reg&1;									\
	Reg>>=1;									\
	R.AF.B.l=ZSPTable[Reg]|q;					\
  }
#define M_SRA(Reg)								\
  {												\
	int q;										\
	q=Reg&1;									\
	Reg=(Reg>>1)|(Reg&0x80);					\
	R.AF.B.l=ZSPTable[Reg]|q;					\
  }

#define M_INC(Reg)									\
  ++Reg;											\
  R.AF.B.l=(R.AF.B.l&C_FLAG)|ZSTable[Reg]|			\
	((Reg==0x80)?V_FLAG:0)|((Reg&0x0F)?0:H_FLAG)

#define M_DEC(Reg)									\
  R.AF.B.l=(R.AF.B.l&C_FLAG)|N_FLAG|				\
	((Reg==0x80)?V_FLAG:0)|((Reg&0x0F)?0:H_FLAG);	\
  R.AF.B.l|=ZSTable[--Reg]

#define M_ADD(Reg)								\
  {												\
	int q;										\
	q=R.AF.B.h+Reg;								\
	R.AF.B.l=ZSTable[q&255]|((q&256)>>8)|		\
	  ((R.AF.B.h^q^Reg)&H_FLAG)|				\
	  (((Reg^R.AF.B.h^0x80)&(Reg^q)&0x80)>>5);	\
	R.AF.B.h=q;									\
  }

#define M_ADC(Reg)								\
  {												\
	int q;										\
	q=R.AF.B.h+Reg+(R.AF.B.l&1);				\
	R.AF.B.l=ZSTable[q&255]|((q&256)>>8)|		\
	  ((R.AF.B.h^q^Reg)&H_FLAG)|				\
	  (((Reg^R.AF.B.h^0x80)&(Reg^q)&0x80)>>5);	\
	R.AF.B.h=q;									\
  }

#define M_SUB(Reg)														\
  {																		\
	int q;																\
	q=R.AF.B.h-Reg;														\
	R.AF.B.l=ZSTable[q&255]|((q&256)>>8)|N_FLAG|						\
	  ((R.AF.B.h^q^Reg)&H_FLAG)|										\
	  (((Reg^R.AF.B.h)&(R.AF.B.h^q)&0x80)>>5);   /* MB & FB 220997 */	\
	R.AF.B.h=q;															\
  }

#define M_SBC(Reg)														\
  {																		\
	int q;																\
	q=R.AF.B.h-Reg-(R.AF.B.l&1);										\
	R.AF.B.l=ZSTable[q&255]|((q&256)>>8)|N_FLAG|						\
	  ((R.AF.B.h^q^Reg)&H_FLAG)|										\
	  (((Reg^R.AF.B.h)&(R.AF.B.h^q)&0x80)>>5);   /* MB & FB 220997 */	\
	R.AF.B.h=q;															\
  }

#define M_CP(Reg)														\
  {																		\
	int q;																\
	q=R.AF.B.h-Reg;														\
	R.AF.B.l=ZSTable[q&255]|((q&256)>>8)|N_FLAG|						\
	  ((R.AF.B.h^q^Reg)&H_FLAG)|										\
	  (((Reg^R.AF.B.h)&(R.AF.B.h^q)&0x80)>>5);   /* MB & FB 220997 */   \
  }

#define M_ADDW(Reg1,Reg2)						\
  {												\
	int q;										\
	q=R.Reg1.D+R.Reg2.D;						\
	R.AF.B.l=(R.AF.B.l&(S_FLAG|Z_FLAG|V_FLAG))|	\
	  (((R.Reg1.D^q^R.Reg2.D)&0x1000)>>8)|		\
	  ((q>>16)&1);								\
	R.Reg1.W.l=q;								\
  }

#define M_ADCW(Reg)											\
  {															\
	int q;													\
	q=R.HL.D+R.Reg.D+(R.AF.D&1);							\
	R.AF.B.l=(((R.HL.D^q^R.Reg.D)&0x1000)>>8)|				\
	  ((q>>16)&1)|											\
	  ((q&0x8000)>>8)|										\
	  ((q&65535)?0:Z_FLAG)|									\
	  (((R.Reg.D^R.HL.D^0x8000)&(R.Reg.D^q)&0x8000)>>13);	\
	R.HL.W.l=q;												\
  }

#define M_SBCW(Reg)														\
  {																		\
	int q;																\
	q=R.HL.D-R.Reg.D-(R.AF.D&1);										\
	R.AF.B.l=(((R.HL.D^q^R.Reg.D)&0x1000)>>8)|							\
	  ((q>>16)&1)|														\
	  ((q&0x8000)>>8)|													\
	  ((q&65535)?0:Z_FLAG)|												\
	  (((R.Reg.D^R.HL.D)&(R.HL.D^q)&0x8000)>>13)|	/* ASG 220997 */	\
	  N_FLAG;															\
	R.HL.W.l=q;															\
  }



class Z80 {
 public:
  Z80 (ZXMemory* aZXMemory, ZXULA* aZXULA, ZXIO* aZXIO);
  ~Z80 ();

  void Reset (void);             /* Reset registers to the initial values */
  int  Execute(int cycles);           /* Execute cycles T-States - returns number of cycles actually run */                 /* Execute until Z80_Running==0          */
  int Step(void);

  const Z80_Regs& Z80_GetRegs (void) const { return R; }
  Z80_Regs& Z80_ChangeRegs (void) { return R; }
  void Z80_SetRegs (const Z80_Regs& aRegs) { R = aRegs; }
  void Z80_Cause_Interrupt(int type);	/* NS 970904 */

 private:
  ZXMemory *theZXMemory;
  ZXULA *theZXULA;
  ZXIO *theZXIO;

  static unsigned cycles_xx[256];
  static unsigned cycles_xx_cb[256];
  static unsigned cycles_cb[256];
  static unsigned cycles_main[256];
  static unsigned cycles_ed[256];

  void InitTables (void);
  void Interrupt (void);

  Z80_Regs R; // Z80 registers

  int Z80_ICount;

  uint8_t PTable[512];
  uint8_t ZSTable[512];
  uint8_t ZSPTable[512];

  uint8_t Z80_In(uint16_t Port) const;
  void Z80_Out(uint16_t Port, uint8_t Value);
  uint8_t Z80_RDMEM (uint16_t A) const;
  void Z80_WRMEM(uint16_t A, uint8_t V);
  uint16_t Z80_RDMEM_U16 (uint16_t A);
  void Z80_WRMEM_U16 (uint16_t A,uint16_t V);

  uint8_t Z80_RDMEM_OPCODE (void);
  uint16_t Z80_RDMEM_OPCODE_U16 (void);

  uint8_t M_RD_XIX(void);
  uint8_t M_RD_XIY(void);
  void M_WR_XIX(uint8_t a);
  void M_WR_XIY(uint8_t a);

  void call_opcode_dd_cb (uint8_t aCode);
  void call_opcode_fd_cb (uint8_t aCode);
  void call_opcode_cb (uint8_t aCode);
  void call_opcode_dd (uint8_t aCode);
  void call_opcode_ed (uint8_t aCode);
  void call_opcode_fd (uint8_t aCode);
  void call_opcode_main (uint8_t aCode);


  // inline now
inline void dd_cb(void) {
 uint8_t opcode;
 opcode=Z80_RDMEM((R.PC.D+1)&0xFFFF);
 Z80_ICount-=cycles_xx_cb[opcode];
 call_opcode_dd_cb (opcode);
 ++R.PC.W.l;
}

inline void fd_cb(void) {
 uint8_t opcode;
 opcode=Z80_RDMEM((R.PC.D+1)&0xFFFF);
 Z80_ICount-=cycles_xx_cb[opcode];
 call_opcode_fd_cb (opcode);
 ++R.PC.W.l;
}

inline void cb(void) {
 uint8_t opcode;
 ++R.R;
 opcode=Z80_RDMEM(R.PC.D);
 R.PC.W.l++;
 Z80_ICount-=cycles_cb[opcode];
 call_opcode_cb (opcode);
}

inline void dd(void) {
 uint8_t opcode;
 ++R.R;
 opcode=Z80_RDMEM(R.PC.D);
 R.PC.W.l++;
 Z80_ICount-=cycles_xx[opcode];
 call_opcode_dd (opcode);
}

inline void ed(void) {
 uint8_t opcode;
 ++R.R;
 opcode=Z80_RDMEM(R.PC.D);
 R.PC.W.l++;
 Z80_ICount-=cycles_ed[opcode];
 call_opcode_ed (opcode);
}

inline void fd (void) {
 uint8_t opcode;
 ++R.R;
 opcode=Z80_RDMEM(R.PC.D);
 R.PC.W.l++;
 Z80_ICount-=cycles_xx[opcode];
 call_opcode_fd (opcode);
}

inline void ei(void) {
  uint8_t opcode;
  /* If interrupts were disabled, execute one more instruction and check the */
  /* IRQ line. If not, simply set interrupt flip-flop 2                      */
  if (!R.IFF1)
    {
      R.IFF1=R.IFF2=1;
      ++R.R;
      opcode=Z80_RDMEM(R.PC.D);

      R.PC.W.l++;
      Z80_ICount-=cycles_main[opcode];
      call_opcode_main (opcode);

      Interrupt(/*Z80_IRQ*/);	/* NS 970904 */
    }
  else
    R.IFF2=1;
}

#define adc_a_xhl() { uint8_t i=M_RD_XHL; M_ADC(i); }
#define adc_a_xix() { uint8_t i=M_RD_XIX();  M_ADC(i); }
#define adc_a_xiy() { uint8_t i=M_RD_XIY(); M_ADC(i); }
#define adc_a_a() { M_ADC(R.AF.B.h); }
#define adc_a_b() { M_ADC(R.BC.B.h); }
#define adc_a_c() { M_ADC(R.BC.B.l); }
#define adc_a_d() { M_ADC(R.DE.B.h); }
#define adc_a_e() { M_ADC(R.DE.B.l); }
#define adc_a_h() { M_ADC(R.HL.B.h); }
#define adc_a_l() { M_ADC(R.HL.B.l); }
#define adc_a_ixl() { M_ADC(R.IX.B.l); }
#define adc_a_ixh() { M_ADC(R.IX.B.h); }
#define adc_a_iyl() { M_ADC(R.IY.B.l); }
#define adc_a_iyh() { M_ADC(R.IY.B.h); }
#define adc_a_uint8_t() { uint8_t i=Z80_RDMEM_OPCODE(); M_ADC(i); }



  #define  adc_hl_bc() { M_ADCW(BC); }
  #define  adc_hl_de() { M_ADCW(DE); }
  #define  adc_hl_hl() { M_ADCW(HL); }
  #define  adc_hl_sp() { M_ADCW(SP); }

  #define  add_a_xhl() { uint8_t i=M_RD_XHL; M_ADD(i); }
  #define  add_a_xix() { uint8_t i=M_RD_XIX(); M_ADD(i); }
  #define  add_a_xiy() { uint8_t i=M_RD_XIY(); M_ADD(i); }
  #define  add_a_a() { M_ADD(R.AF.B.h); }
  #define  add_a_b() { M_ADD(R.BC.B.h); }
  #define  add_a_c() { M_ADD(R.BC.B.l); }
  #define  add_a_d() { M_ADD(R.DE.B.h); }
  #define  add_a_e() { M_ADD(R.DE.B.l); }
  #define  add_a_h() { M_ADD(R.HL.B.h); }
  #define  add_a_l() { M_ADD(R.HL.B.l); }
  #define  add_a_ixl() { M_ADD(R.IX.B.l); }
  #define  add_a_ixh() { M_ADD(R.IX.B.h); }
  #define  add_a_iyl() { M_ADD(R.IY.B.l); }
  #define  add_a_iyh() { M_ADD(R.IY.B.h); }
  #define  add_a_uint8_t() { uint8_t i=Z80_RDMEM_OPCODE(); M_ADD(i); }

  #define  add_hl_bc() { M_ADDW(HL,BC); }
  #define  add_hl_de() { M_ADDW(HL,DE); }
  #define  add_hl_hl() { M_ADDW(HL,HL); }
  #define  add_hl_sp() { M_ADDW(HL,SP); }
  #define  add_ix_bc() { M_ADDW(IX,BC); }
  #define  add_ix_de() { M_ADDW(IX,DE); }
  #define  add_ix_ix() { M_ADDW(IX,IX); }
  #define  add_ix_sp() { M_ADDW(IX,SP); }
  #define  add_iy_bc() { M_ADDW(IY,BC); }
  #define  add_iy_de() { M_ADDW(IY,DE); }
  #define  add_iy_iy() { M_ADDW(IY,IY); }
  #define  add_iy_sp() { M_ADDW(IY,SP); }

  #define  and_xhl() { uint8_t i=M_RD_XHL; M_AND(i); }
  #define  and_xix() { uint8_t i=M_RD_XIX(); M_AND(i); }
  #define  and_xiy() { uint8_t i=M_RD_XIY(); M_AND(i); }
  #define  and_a() { R.AF.B.l=ZSPTable[R.AF.B.h]|H_FLAG; }
  #define  and_b() { M_AND(R.BC.B.h); }
  #define  and_c() { M_AND(R.BC.B.l); }
  #define  and_d() { M_AND(R.DE.B.h); }
  #define  and_e() { M_AND(R.DE.B.l); }
  #define  and_h() { M_AND(R.HL.B.h); }
  #define  and_l() { M_AND(R.HL.B.l); }
  #define  and_ixh() { M_AND(R.IX.B.h); }
  #define  and_ixl() { M_AND(R.IX.B.l); }
  #define  and_iyh() { M_AND(R.IY.B.h); }
  #define  and_iyl() { M_AND(R.IY.B.l); }
  #define  and_uint8_t() { uint8_t i=Z80_RDMEM_OPCODE(); M_AND(i); }

  #define  bit_0_xhl() { uint8_t i=M_RD_XHL; M_BIT(0,i); }
  #define  bit_0_xix() { uint8_t i=M_RD_XIX(); M_BIT(0,i); }
  #define  bit_0_xix_b() { R.BC.B.h=M_RD_XIX(); M_BIT(0,R.BC.B.h); }
  #define  bit_0_xix_c() { R.BC.B.l=M_RD_XIX(); M_BIT(0,R.BC.B.l); }
  #define  bit_0_xix_d() { R.DE.B.h=M_RD_XIX(); M_BIT(0,R.DE.B.h); }
  #define  bit_0_xix_e() { R.DE.B.l=M_RD_XIX(); M_BIT(0,R.DE.B.l); }
  #define  bit_0_xix_h() { R.HL.B.h=M_RD_XIX(); M_BIT(0,R.HL.B.h); }
  #define  bit_0_xix_l() { R.HL.B.l=M_RD_XIX(); M_BIT(0,R.HL.B.l); }
  #define  bit_0_xix_a() { R.AF.B.h=M_RD_XIX(); M_BIT(0,R.AF.B.h); }
  #define  bit_0_xiy() { uint8_t i=M_RD_XIY(); M_BIT(0,i); }
  #define  bit_0_xiy_b() { R.BC.B.h=M_RD_XIY(); M_BIT(0,R.BC.B.h); }
  #define  bit_0_xiy_c() { R.BC.B.l=M_RD_XIY(); M_BIT(0,R.BC.B.l); }
  #define  bit_0_xiy_d() { R.DE.B.h=M_RD_XIY(); M_BIT(0,R.DE.B.h); }
  #define  bit_0_xiy_e() { R.DE.B.l=M_RD_XIY(); M_BIT(0,R.DE.B.l); }
  #define  bit_0_xiy_h() { R.HL.B.h=M_RD_XIY(); M_BIT(0,R.HL.B.h); }
  #define  bit_0_xiy_l() { R.HL.B.l=M_RD_XIY(); M_BIT(0,R.HL.B.l); }
  #define  bit_0_xiy_a() { R.AF.B.h=M_RD_XIY(); M_BIT(0,R.AF.B.h); }
  #define  bit_0_a() { M_BIT(0,R.AF.B.h); }
  #define  bit_0_b() { M_BIT(0,R.BC.B.h); }
  #define  bit_0_c() { M_BIT(0,R.BC.B.l); }
  #define  bit_0_d() { M_BIT(0,R.DE.B.h); }
  #define  bit_0_e() { M_BIT(0,R.DE.B.l); }
  #define  bit_0_h() { M_BIT(0,R.HL.B.h); }
  #define  bit_0_l() { M_BIT(0,R.HL.B.l); }

  #define  bit_1_xhl() { uint8_t i=M_RD_XHL; M_BIT(1,i); }
  #define  bit_1_xix() { uint8_t i=M_RD_XIX(); M_BIT(1,i); }
  #define  bit_1_xix_b() { R.BC.B.h=M_RD_XIX(); M_BIT(1,R.BC.B.h); }
  #define  bit_1_xix_c() { R.BC.B.l=M_RD_XIX(); M_BIT(1,R.BC.B.l); }
  #define  bit_1_xix_d() { R.DE.B.h=M_RD_XIX(); M_BIT(1,R.DE.B.h); }
  #define  bit_1_xix_e() { R.DE.B.l=M_RD_XIX(); M_BIT(1,R.DE.B.l); }
  #define  bit_1_xix_h() { R.HL.B.h=M_RD_XIX(); M_BIT(1,R.HL.B.h); }
  #define  bit_1_xix_l() { R.HL.B.l=M_RD_XIX(); M_BIT(1,R.HL.B.l); }
  #define  bit_1_xix_a() { R.AF.B.h=M_RD_XIX(); M_BIT(1,R.AF.B.h); }
  #define  bit_1_xiy() { uint8_t i=M_RD_XIY(); M_BIT(1,i); }
  #define  bit_1_xiy_b() { R.BC.B.h=M_RD_XIY(); M_BIT(1,R.BC.B.h); }
  #define  bit_1_xiy_c() { R.BC.B.l=M_RD_XIY(); M_BIT(1,R.BC.B.l); }
  #define  bit_1_xiy_d() { R.DE.B.h=M_RD_XIY(); M_BIT(1,R.DE.B.h); }
  #define  bit_1_xiy_e() { R.DE.B.l=M_RD_XIY(); M_BIT(1,R.DE.B.l); }
  #define  bit_1_xiy_h() { R.HL.B.h=M_RD_XIY(); M_BIT(1,R.HL.B.h); }
  #define  bit_1_xiy_l() { R.HL.B.l=M_RD_XIY(); M_BIT(1,R.HL.B.l); }
  #define  bit_1_xiy_a() { R.AF.B.h=M_RD_XIY(); M_BIT(1,R.AF.B.h); }
  #define  bit_1_a() { M_BIT(1,R.AF.B.h); }
  #define  bit_1_b() { M_BIT(1,R.BC.B.h); }
  #define  bit_1_c() { M_BIT(1,R.BC.B.l); }
  #define  bit_1_d() { M_BIT(1,R.DE.B.h); }
  #define  bit_1_e() { M_BIT(1,R.DE.B.l); }
  #define  bit_1_h() { M_BIT(1,R.HL.B.h); }
  #define  bit_1_l() { M_BIT(1,R.HL.B.l); }

  #define  bit_2_xhl() { uint8_t i=M_RD_XHL; M_BIT(2,i); }
  #define  bit_2_xix() { uint8_t i=M_RD_XIX(); M_BIT(2,i); }
  #define  bit_2_xix_b() { R.BC.B.h=M_RD_XIX(); M_BIT(2,R.BC.B.h); }
  #define  bit_2_xix_c() { R.BC.B.l=M_RD_XIX(); M_BIT(2,R.BC.B.l); }
  #define  bit_2_xix_d() { R.DE.B.h=M_RD_XIX(); M_BIT(2,R.DE.B.h); }
  #define  bit_2_xix_e() { R.DE.B.l=M_RD_XIX(); M_BIT(2,R.DE.B.l); }
  #define  bit_2_xix_h() { R.HL.B.h=M_RD_XIX(); M_BIT(2,R.HL.B.h); }
  #define  bit_2_xix_l() { R.HL.B.l=M_RD_XIX(); M_BIT(2,R.HL.B.l); }
  #define  bit_2_xix_a() { R.AF.B.h=M_RD_XIX(); M_BIT(2,R.AF.B.h); }
  #define  bit_2_xiy() { uint8_t i=M_RD_XIY(); M_BIT(2,i); }
  #define  bit_2_xiy_b() { R.BC.B.h=M_RD_XIY(); M_BIT(2,R.BC.B.h); }
  #define  bit_2_xiy_c() { R.BC.B.l=M_RD_XIY(); M_BIT(2,R.BC.B.l); }
  #define  bit_2_xiy_d() { R.DE.B.h=M_RD_XIY(); M_BIT(2,R.DE.B.h); }
  #define  bit_2_xiy_e() { R.DE.B.l=M_RD_XIY(); M_BIT(2,R.DE.B.l); }
  #define  bit_2_xiy_h() { R.HL.B.h=M_RD_XIY(); M_BIT(2,R.HL.B.h); }
  #define  bit_2_xiy_l() { R.HL.B.l=M_RD_XIY(); M_BIT(2,R.HL.B.l); }
  #define  bit_2_xiy_a() { R.AF.B.h=M_RD_XIY(); M_BIT(2,R.AF.B.h); }
  #define  bit_2_a() { M_BIT(2,R.AF.B.h); }
  #define  bit_2_b() { M_BIT(2,R.BC.B.h); }
  #define  bit_2_c() { M_BIT(2,R.BC.B.l); }
  #define  bit_2_d() { M_BIT(2,R.DE.B.h); }
  #define  bit_2_e() { M_BIT(2,R.DE.B.l); }
  #define  bit_2_h() { M_BIT(2,R.HL.B.h); }
  #define  bit_2_l() { M_BIT(2,R.HL.B.l); }

  #define  bit_3_xhl() { uint8_t i=M_RD_XHL; M_BIT(3,i); }
  #define  bit_3_xix() { uint8_t i=M_RD_XIX(); M_BIT(3,i); }
  #define  bit_3_xix_b() { R.BC.B.h=M_RD_XIX(); M_BIT(3,R.BC.B.h); }
  #define  bit_3_xix_c() { R.BC.B.l=M_RD_XIX(); M_BIT(3,R.BC.B.l); }
  #define  bit_3_xix_d() { R.DE.B.h=M_RD_XIX(); M_BIT(3,R.DE.B.h); }
  #define  bit_3_xix_e() { R.DE.B.l=M_RD_XIX(); M_BIT(3,R.DE.B.l); }
  #define  bit_3_xix_h() { R.HL.B.h=M_RD_XIX(); M_BIT(3,R.HL.B.h); }
  #define  bit_3_xix_l() { R.HL.B.l=M_RD_XIX(); M_BIT(3,R.HL.B.l); }
  #define  bit_3_xix_a() { R.AF.B.h=M_RD_XIX(); M_BIT(3,R.AF.B.h); }
  #define  bit_3_xiy() { uint8_t i=M_RD_XIY(); M_BIT(3,i); }
  #define  bit_3_xiy_b() { R.BC.B.h=M_RD_XIY(); M_BIT(3,R.BC.B.h); }
  #define  bit_3_xiy_c() { R.BC.B.l=M_RD_XIY(); M_BIT(3,R.BC.B.l); }
  #define  bit_3_xiy_d() { R.DE.B.h=M_RD_XIY(); M_BIT(3,R.DE.B.h); }
  #define  bit_3_xiy_e() { R.DE.B.l=M_RD_XIY(); M_BIT(3,R.DE.B.l); }
  #define  bit_3_xiy_h() { R.HL.B.h=M_RD_XIY(); M_BIT(3,R.HL.B.h); }
  #define  bit_3_xiy_l() { R.HL.B.l=M_RD_XIY(); M_BIT(3,R.HL.B.l); }
  #define  bit_3_xiy_a() { R.AF.B.h=M_RD_XIY(); M_BIT(3,R.AF.B.h); }
  #define  bit_3_a() { M_BIT(3,R.AF.B.h); }
  #define  bit_3_b() { M_BIT(3,R.BC.B.h); }
  #define  bit_3_c() { M_BIT(3,R.BC.B.l); }
  #define  bit_3_d() { M_BIT(3,R.DE.B.h); }
  #define  bit_3_e() { M_BIT(3,R.DE.B.l); }
  #define  bit_3_h() { M_BIT(3,R.HL.B.h); }
  #define  bit_3_l() { M_BIT(3,R.HL.B.l); }

  #define  bit_4_xhl() { uint8_t i=M_RD_XHL; M_BIT(4,i); }
  #define  bit_4_xix() { uint8_t i=M_RD_XIX(); M_BIT(4,i); }
  #define  bit_4_xix_b() { R.BC.B.h=M_RD_XIX(); M_BIT(4,R.BC.B.h); }
  #define  bit_4_xix_c() { R.BC.B.l=M_RD_XIX(); M_BIT(4,R.BC.B.l); }
  #define  bit_4_xix_d() { R.DE.B.h=M_RD_XIX(); M_BIT(4,R.DE.B.h); }
  #define  bit_4_xix_e() { R.DE.B.l=M_RD_XIX(); M_BIT(4,R.DE.B.l); }
  #define  bit_4_xix_h() { R.HL.B.h=M_RD_XIX(); M_BIT(4,R.HL.B.h); }
  #define  bit_4_xix_l() { R.HL.B.l=M_RD_XIX(); M_BIT(4,R.HL.B.l); }
  #define  bit_4_xix_a() { R.AF.B.h=M_RD_XIX(); M_BIT(4,R.AF.B.h); }
  #define  bit_4_xiy() { uint8_t i=M_RD_XIY(); M_BIT(4,i); }
  #define  bit_4_xiy_b() { R.BC.B.h=M_RD_XIY(); M_BIT(4,R.BC.B.h); }
  #define  bit_4_xiy_c() { R.BC.B.l=M_RD_XIY(); M_BIT(4,R.BC.B.l); }
  #define  bit_4_xiy_d() { R.DE.B.h=M_RD_XIY(); M_BIT(4,R.DE.B.h); }
  #define  bit_4_xiy_e() { R.DE.B.l=M_RD_XIY(); M_BIT(4,R.DE.B.l); }
  #define  bit_4_xiy_h() { R.HL.B.h=M_RD_XIY(); M_BIT(4,R.HL.B.h); }
  #define  bit_4_xiy_l() { R.HL.B.l=M_RD_XIY(); M_BIT(4,R.HL.B.l); }
  #define  bit_4_xiy_a() { R.AF.B.h=M_RD_XIY(); M_BIT(4,R.AF.B.h); }
  #define  bit_4_a() { M_BIT(4,R.AF.B.h); }
  #define  bit_4_b() { M_BIT(4,R.BC.B.h); }
  #define  bit_4_c() { M_BIT(4,R.BC.B.l); }
  #define  bit_4_d() { M_BIT(4,R.DE.B.h); }
  #define  bit_4_e() { M_BIT(4,R.DE.B.l); }
  #define  bit_4_h() { M_BIT(4,R.HL.B.h); }
  #define  bit_4_l() { M_BIT(4,R.HL.B.l); }

  #define  bit_5_xhl() { uint8_t i=M_RD_XHL; M_BIT(5,i); }
  #define  bit_5_xix() { uint8_t i=M_RD_XIX(); M_BIT(5,i); }
  #define  bit_5_xix_b() { R.BC.B.h=M_RD_XIX(); M_BIT(5,R.BC.B.h); }
  #define  bit_5_xix_c() { R.BC.B.l=M_RD_XIX(); M_BIT(5,R.BC.B.l); }
  #define  bit_5_xix_d() { R.DE.B.h=M_RD_XIX(); M_BIT(5,R.DE.B.h); }
  #define  bit_5_xix_e() { R.DE.B.l=M_RD_XIX(); M_BIT(5,R.DE.B.l); }
  #define  bit_5_xix_h() { R.HL.B.h=M_RD_XIX(); M_BIT(5,R.HL.B.h); }
  #define  bit_5_xix_l() { R.HL.B.l=M_RD_XIX(); M_BIT(5,R.HL.B.l); }
  #define  bit_5_xix_a() { R.AF.B.h=M_RD_XIX(); M_BIT(5,R.AF.B.h); }
  #define  bit_5_xiy() { uint8_t i=M_RD_XIY(); M_BIT(5,i); }
  #define  bit_5_xiy_b() { R.BC.B.h=M_RD_XIY(); M_BIT(5,R.BC.B.h); }
  #define  bit_5_xiy_c() { R.BC.B.l=M_RD_XIY(); M_BIT(5,R.BC.B.l); }
  #define  bit_5_xiy_d() { R.DE.B.h=M_RD_XIY(); M_BIT(5,R.DE.B.h); }
  #define  bit_5_xiy_e() { R.DE.B.l=M_RD_XIY(); M_BIT(5,R.DE.B.l); }
  #define  bit_5_xiy_h() { R.HL.B.h=M_RD_XIY(); M_BIT(5,R.HL.B.h); }
  #define  bit_5_xiy_l() { R.HL.B.l=M_RD_XIY(); M_BIT(5,R.HL.B.l); }
  #define  bit_5_xiy_a() { R.AF.B.h=M_RD_XIY(); M_BIT(5,R.AF.B.h); }
  #define  bit_5_a() { M_BIT(5,R.AF.B.h); }
  #define  bit_5_b() { M_BIT(5,R.BC.B.h); }
  #define  bit_5_c() { M_BIT(5,R.BC.B.l); }
  #define  bit_5_d() { M_BIT(5,R.DE.B.h); }
  #define  bit_5_e() { M_BIT(5,R.DE.B.l); }
  #define  bit_5_h() { M_BIT(5,R.HL.B.h); }
  #define  bit_5_l() { M_BIT(5,R.HL.B.l); }

  #define  bit_6_xhl() { uint8_t i=M_RD_XHL; M_BIT(6,i); }
  #define  bit_6_xix() { uint8_t i=M_RD_XIX(); M_BIT(6,i); }
  #define  bit_6_xix_b() { R.BC.B.h=M_RD_XIX(); M_BIT(6,R.BC.B.h); }
  #define  bit_6_xix_c() { R.BC.B.l=M_RD_XIX(); M_BIT(6,R.BC.B.l); }
  #define  bit_6_xix_d() { R.DE.B.h=M_RD_XIX(); M_BIT(6,R.DE.B.h); }
  #define  bit_6_xix_e() { R.DE.B.l=M_RD_XIX(); M_BIT(6,R.DE.B.l); }
  #define  bit_6_xix_h() { R.HL.B.h=M_RD_XIX(); M_BIT(6,R.HL.B.h); }
  #define  bit_6_xix_l() { R.HL.B.l=M_RD_XIX(); M_BIT(6,R.HL.B.l); }
  #define  bit_6_xix_a() { R.AF.B.h=M_RD_XIX(); M_BIT(6,R.AF.B.h); }
  #define  bit_6_xiy() { uint8_t i=M_RD_XIY(); M_BIT(6,i); }
  #define  bit_6_xiy_b() { R.BC.B.h=M_RD_XIY(); M_BIT(6,R.BC.B.h); }
  #define  bit_6_xiy_c() { R.BC.B.l=M_RD_XIY(); M_BIT(6,R.BC.B.l); }
  #define  bit_6_xiy_d() { R.DE.B.h=M_RD_XIY(); M_BIT(6,R.DE.B.h); }
  #define  bit_6_xiy_e() { R.DE.B.l=M_RD_XIY(); M_BIT(6,R.DE.B.l); }
  #define  bit_6_xiy_h() { R.HL.B.h=M_RD_XIY(); M_BIT(6,R.HL.B.h); }
  #define  bit_6_xiy_l() { R.HL.B.l=M_RD_XIY(); M_BIT(6,R.HL.B.l); }
  #define  bit_6_xiy_a() { R.AF.B.h=M_RD_XIY(); M_BIT(6,R.AF.B.h); }
  #define  bit_6_a() { M_BIT(6,R.AF.B.h); }
  #define  bit_6_b() { M_BIT(6,R.BC.B.h); }
  #define  bit_6_c() { M_BIT(6,R.BC.B.l); }
  #define  bit_6_d() { M_BIT(6,R.DE.B.h); }
  #define  bit_6_e() { M_BIT(6,R.DE.B.l); }
  #define  bit_6_h() { M_BIT(6,R.HL.B.h); }
  #define  bit_6_l() { M_BIT(6,R.HL.B.l); }

  #define  bit_7_xhl() { uint8_t i=M_RD_XHL; M_BIT(7,i); }
  #define  bit_7_xix() { uint8_t i=M_RD_XIX(); M_BIT(7,i); }
  #define  bit_7_xix_b() { R.BC.B.h=M_RD_XIX(); M_BIT(7,R.BC.B.h); }
  #define  bit_7_xix_c() { R.BC.B.l=M_RD_XIX(); M_BIT(7,R.BC.B.l); }
  #define  bit_7_xix_d() { R.DE.B.h=M_RD_XIX(); M_BIT(7,R.DE.B.h); }
  #define  bit_7_xix_e() { R.DE.B.l=M_RD_XIX(); M_BIT(7,R.DE.B.l); }
  #define  bit_7_xix_h() { R.HL.B.h=M_RD_XIX(); M_BIT(7,R.HL.B.h); }
  #define  bit_7_xix_l() { R.HL.B.l=M_RD_XIX(); M_BIT(7,R.HL.B.l); }
  #define  bit_7_xix_a() { R.AF.B.h=M_RD_XIX(); M_BIT(7,R.AF.B.h); }
  #define  bit_7_xiy() { uint8_t i=M_RD_XIY(); M_BIT(7,i); }
  #define  bit_7_xiy_b() { R.BC.B.h=M_RD_XIY(); M_BIT(7,R.BC.B.h); }
  #define  bit_7_xiy_c() { R.BC.B.l=M_RD_XIY(); M_BIT(7,R.BC.B.l); }
  #define  bit_7_xiy_d() { R.DE.B.h=M_RD_XIY(); M_BIT(7,R.DE.B.h); }
  #define  bit_7_xiy_e() { R.DE.B.l=M_RD_XIY(); M_BIT(7,R.DE.B.l); }
  #define  bit_7_xiy_h() { R.HL.B.h=M_RD_XIY(); M_BIT(7,R.HL.B.h); }
  #define  bit_7_xiy_l() { R.HL.B.l=M_RD_XIY(); M_BIT(7,R.HL.B.l); }
  #define  bit_7_xiy_a() { R.AF.B.h=M_RD_XIY(); M_BIT(7,R.AF.B.h); }
  #define  bit_7_a() { M_BIT(7,R.AF.B.h); }
  #define  bit_7_b() { M_BIT(7,R.BC.B.h); }
  #define  bit_7_c() { M_BIT(7,R.BC.B.l); }
  #define  bit_7_d() { M_BIT(7,R.DE.B.h); }
  #define  bit_7_e() { M_BIT(7,R.DE.B.l); }
  #define  bit_7_h() { M_BIT(7,R.HL.B.h); }
  #define  bit_7_l() { M_BIT(7,R.HL.B.l); }

  #define  call_c() { if (M_C) { M_CALL; } else { M_SKIP_CALL; } }
  #define  call_m() { if (M_M) { M_CALL; } else { M_SKIP_CALL; } }
  #define  call_nc() { if (M_NC) { M_CALL; } else { M_SKIP_CALL; } }
  #define  call_nz() { if (M_NZ) { M_CALL; } else { M_SKIP_CALL; } }
  #define  call_p() { if (M_P) { M_CALL; } else { M_SKIP_CALL; } }
  #define  call_pe() { if (M_PE) { M_CALL; } else { M_SKIP_CALL; } }
  #define  call_po() { if (M_PO) { M_CALL; } else { M_SKIP_CALL; } }
  #define  call_z() { if (M_Z) { M_CALL; } else { M_SKIP_CALL; } }
  #define  call() { M_CALL; }

  #define  ccf() { R.AF.B.l=((R.AF.B.l&0xED)|((R.AF.B.l&1)<<4))^1; }

  #define  cp_xhl() { uint8_t i=M_RD_XHL; M_CP(i); }
  #define  cp_xix() { uint8_t i=M_RD_XIX(); M_CP(i); }
  #define  cp_xiy() { uint8_t i=M_RD_XIY(); M_CP(i); }
  #define  cp_a() { M_CP(R.AF.B.h); }
  #define  cp_b() { M_CP(R.BC.B.h); }
  #define  cp_c() { M_CP(R.BC.B.l); }
  #define  cp_d() { M_CP(R.DE.B.h); }
  #define  cp_e() { M_CP(R.DE.B.l); }
  #define  cp_h() { M_CP(R.HL.B.h); }
  #define  cp_l() { M_CP(R.HL.B.l); }
  #define  cp_ixh() { M_CP(R.IX.B.h); }
  #define  cp_ixl() { M_CP(R.IX.B.l); }
  #define  cp_iyh() { M_CP(R.IY.B.h); }
  #define  cp_iyl() { M_CP(R.IY.B.l); }
  #define  cp_uint8_t() { uint8_t i=Z80_RDMEM_OPCODE(); M_CP(i); }

  inline void cpd(void)
	{
	  uint8_t i,j;
	  i=Z80_RDMEM(R.HL.D); // TO DO
	  j=R.AF.B.h-i;
	  --R.HL.W.l;
	  --R.BC.W.l;
	  R.AF.B.l=(R.AF.B.l&C_FLAG)|ZSTable[j]|
		((R.AF.B.h^i^j)&H_FLAG)|(R.BC.D? V_FLAG:0)|N_FLAG; // TO DO
	}

  inline void cpdr(void)
	{
	  cpd ();
	  if (R.BC.D && !(R.AF.B.l&Z_FLAG)) { Z80_ICount-=5; R.PC.W.l-=2; }// TO DO
	}

  inline void cpi(void)
	{
	  uint8_t i,j;
	  i=Z80_RDMEM(R.HL.D);// TO DO
	  j=R.AF.B.h-i;
	  ++R.HL.W.l;
	  --R.BC.W.l;
	  R.AF.B.l=(R.AF.B.l&C_FLAG)|ZSTable[j]|
		((R.AF.B.h^i^j)&H_FLAG)|(R.BC.D? V_FLAG:0)|N_FLAG;// TO DO
	}

  inline void cpir(void)
	{
	  cpi ();
	  if (R.BC.D && !(R.AF.B.l&Z_FLAG)) { Z80_ICount-=5; R.PC.W.l-=2; }// TO DO
	}

 #define cpl() { R.AF.B.h^=0xFF; R.AF.B.l|=(H_FLAG|N_FLAG); }

  inline void daa (void) {
	uint8_t cf, nf, hf, lo, hi, diff;
	cf = R.AF.B.l & C_FLAG;
	nf = R.AF.B.l & N_FLAG;
	hf = R.AF.B.l & H_FLAG;
	lo = R.AF.B.h & 15;
	hi = R.AF.B.h / 16;
	if (cf) {
	  diff = (lo <= 9 && !hf) ? 0x60 : 0x66;
	}
	else {
	  if (lo >= 10) {
		diff = hi <= 8 ? 0x06 : 0x66;
	  }
	  else {
		if (hi >= 10) {
		  diff = hf ? 0x66 : 0x60;
		}
		else {
		  diff = hf ? 0x06 : 0x00;
		}
	  }
	}
	if (nf) R.AF.B.h -= diff;
	else R.AF.B.h += diff;

	R.AF.B.l = ZSPTable[R.AF.B.h] | (R.AF.B.l & N_FLAG);
	if (cf || (lo <= 9 ? hi >= 10 : hi >= 9)) R.AF.B.l |= C_FLAG;
	if (nf ? hf && lo <= 5 : lo >= 10)	R.AF.B.l |= H_FLAG;
  }

  inline void dec_xhl(void)
	{
	  uint8_t i;
	  i=Z80_RDMEM(R.HL.D);// TO DO
	  M_DEC(i);
	  Z80_WRMEM(R.HL.D,i);// TO DO
	}
  inline void dec_xix(void)
	{
	  uint8_t i;
	  int j;
	  j=M_XIX;
	  i=Z80_RDMEM(j);
	  M_DEC(i);
	  Z80_WRMEM(j,i);
	}
  inline void dec_xiy(void)
	{
	  uint8_t i;
	  int j;
	  j=M_XIY;
	  i=Z80_RDMEM(j);
	  M_DEC(i);
	  Z80_WRMEM(j,i);
	}

  #define  dec_a() { M_DEC(R.AF.B.h); }
  #define  dec_b() { M_DEC(R.BC.B.h); }
  #define  dec_c() { M_DEC(R.BC.B.l); }
  #define  dec_d() { M_DEC(R.DE.B.h); }
  #define  dec_e() { M_DEC(R.DE.B.l); }
  #define  dec_h() { M_DEC(R.HL.B.h); }
  #define  dec_l() { M_DEC(R.HL.B.l); }
  #define  dec_ixh() { M_DEC(R.IX.B.h); }
  #define  dec_ixl() { M_DEC(R.IX.B.l); }
  #define  dec_iyh() { M_DEC(R.IY.B.h); }
  #define  dec_iyl() { M_DEC(R.IY.B.l); }

  #define  dec_bc() { --R.BC.W.l; }
  #define  dec_de() { --R.DE.W.l; }
  #define  dec_hl() { --R.HL.W.l; }
  #define  dec_ix() { --R.IX.W.l; }
  #define  dec_iy() { --R.IY.W.l; }
  #define  dec_sp() { --R.SP.W.l; }

  #define  di() { R.IFF1=R.IFF2=0; }

  #define  djnz() { if (--R.BC.B.h) { M_JR; } else { M_SKIP_JR; } }

  inline void ex_xsp_hl(void)
	{
	  int i;
	  i=Z80_RDMEM_U16(R.SP.D);// TO DO
	  Z80_WRMEM_U16(R.SP.D,R.HL.D);// TO DO
	  R.HL.D=i;// TO DO
	}

  inline void ex_xsp_ix(void)
	{
	  int i;
	  i=Z80_RDMEM_U16(R.SP.D);// TO DO
	  Z80_WRMEM_U16(R.SP.D,R.IX.D);// TO DO
	  R.IX.D=i;// TO DO
	}

  inline void ex_xsp_iy(void)
	{
	  int i;
	  i=Z80_RDMEM_U16(R.SP.D);// TO DO
	  Z80_WRMEM_U16(R.SP.D,R.IY.D);
	  R.IY.D=i;
	}

  inline void ex_af_af(void)
	{
	  int i;
	  i=R.AF.D;// TO DO
	  R.AF.D=R.AF2.D;// TO DO
	  R.AF2.D=i;// TO DO
	}

  inline void ex_de_hl(void)
	{
	  int i;
	  i=R.DE.D;// TO DO
	  R.DE.D=R.HL.D;// TO DO
	  R.HL.D=i;// TO DO
	}

  inline void exx(void)
	{
	  int i;
	  i=R.BC.D;// TO DO
	  R.BC.D=R.BC2.D;// TO DO
	  R.BC2.D=i;// TO DO
	  i=R.DE.D;// TO DO
	  R.DE.D=R.DE2.D;// TO DO
	  R.DE2.D=i;// TO DO
	  i=R.HL.D;// TO DO
	  R.HL.D=R.HL2.D;// TO DO
	  R.HL2.D=i;// TO DO
	}

  inline void halt(void)
	{
	  --R.PC.W.l;
	  R.HALT=1;
	  if (Z80_ICount>0) Z80_ICount=0;
	}

  inline void im_0(void) { R.IM=0; }
  inline void im_1(void) { R.IM=1; }
  inline void im_2(void) { R.IM=2; }

  inline void in_a_c(void) { M_IN(R.AF.B.h); }
  inline void in_b_c(void) { M_IN(R.BC.B.h); }
  inline void in_c_c(void) { M_IN(R.BC.B.l); }
  inline void in_d_c(void) { M_IN(R.DE.B.h); }
  inline void in_e_c(void) { M_IN(R.DE.B.l); }
  inline void in_h_c(void) { M_IN(R.HL.B.h); }
  inline void in_l_c(void) { M_IN(R.HL.B.l); }
  inline void in_0_c(void) { uint8_t i; M_IN(i); }

  inline void in_a_uint8_t(void)
	{
	  uint8_t i=Z80_RDMEM_OPCODE();
	  R.AF.B.h=DoIn(i,R.AF.B.h);
	}

  inline void inc_xhl(void)
	{
	  uint8_t i;
	  i=Z80_RDMEM(R.HL.D);
	  M_INC(i);
	  Z80_WRMEM(R.HL.D,i);
	}
  inline void inc_xix(void)
	{
	  uint8_t i;
	  int j;
	  j=M_XIX;
	  i=Z80_RDMEM(j);
	  M_INC(i);
	  Z80_WRMEM(j,i);
	}
  inline void inc_xiy(void)
	{
	  uint8_t i;
	  int j;
	  j=M_XIY;
	  i=Z80_RDMEM(j);
	  M_INC(i);
	  Z80_WRMEM(j,i);
	}

  #define  inc_a() { M_INC(R.AF.B.h); }
  #define  inc_b() { M_INC(R.BC.B.h); }
  #define  inc_c() { M_INC(R.BC.B.l); }
  #define  inc_d() { M_INC(R.DE.B.h); }
  #define  inc_e() { M_INC(R.DE.B.l); }
  #define  inc_h() { M_INC(R.HL.B.h); }
  #define  inc_l() { M_INC(R.HL.B.l); }
  #define  inc_ixh() { M_INC(R.IX.B.h); }
  #define  inc_ixl() { M_INC(R.IX.B.l); }
  #define  inc_iyh() { M_INC(R.IY.B.h); }
  #define  inc_iyl() { M_INC(R.IY.B.l); }

  #define  inc_bc() { ++R.BC.W.l; }
  #define  inc_de() { ++R.DE.W.l; }
  #define  inc_hl() { ++R.HL.W.l; }
  #define  inc_ix() { ++R.IX.W.l; }
  #define  inc_iy() { ++R.IY.W.l; }
  #define  inc_sp() { ++R.SP.W.l; }

  inline void ind(void)
	{
	  Z80_WRMEM(R.HL.D,DoIn(R.BC.B.l,R.BC.B.h));
	  --R.BC.B.h;
	  --R.HL.W.l;
	  R.AF.B.l=(R.BC.B.h)? N_FLAG:(N_FLAG|Z_FLAG);
	}

  inline void indr(void)
	{
	  ind ();
	  if (R.BC.B.h) { Z80_ICount-=5; R.PC.W.l-=2; }
	}

  inline void ini(void)
	{
	  Z80_WRMEM(R.HL.D,DoIn(R.BC.B.l,R.BC.B.h));// TO DO
	  --R.BC.B.h;
	  ++R.HL.W.l;
	  R.AF.B.l=(R.BC.B.h)? N_FLAG:(N_FLAG|Z_FLAG);
	}

  inline void inir(void)
	{
	  ini ();
	  if (R.BC.B.h) { Z80_ICount-=5; R.PC.W.l-=2; }
	}

  /*inline void jp(void) { M_JP; } -NS- changed to speed up busy loops */
  inline void jp(void)
	{
	  uint32_t oldpc,newpc;

	  oldpc = R.PC.D-1;// TO DO
	  M_JP;
	  newpc = R.PC.D;
	  if (newpc == oldpc) { if (Z80_ICount > 0) Z80_ICount = 0; } /* speed up busy loop */
	  else if (newpc == oldpc-1 && Z80_RDMEM(newpc) == 0x00) /* NOP - JP */
		{ if (Z80_ICount > 0) Z80_ICount = 0; }
	  else if (newpc == oldpc-3 && Z80_RDMEM(newpc) == 0x31) /* LD SP,#xxxx - Galaga */
		{ if (Z80_ICount > 10) Z80_ICount = 10; }
	  else if (newpc == oldpc-1 && Z80_RDMEM(newpc) == 0xfb && R.pending_irq == Z80_IGNORE_INT)	/* EI - JP */
		{ if (Z80_ICount > 4) Z80_ICount = 4; }
	}

  #define  jp_hl() { R.PC.D=R.HL.D; }
  #define  jp_ix() { R.PC.D=R.IX.D; }
  #define  jp_iy() { R.PC.D=R.IY.D; }
  #define  jp_c() { if (M_C) { M_JP; } else { M_SKIP_JP; } }
  #define  jp_m() { if (M_M) { M_JP; } else { M_SKIP_JP; } }
  #define  jp_nc() { if (M_NC) { M_JP; } else { M_SKIP_JP; } }
  #define  jp_nz() { if (M_NZ) { M_JP; } else { M_SKIP_JP; } }
  #define  jp_p() { if (M_P) { M_JP; } else { M_SKIP_JP; } }
  #define  jp_pe() { if (M_PE) { M_JP; } else { M_SKIP_JP; } }
  #define  jp_po() { if (M_PO) { M_JP; } else { M_SKIP_JP; } }
  #define  jp_z() { if (M_Z) { M_JP; } else { M_SKIP_JP; } }

  /*inline void jr(void) { M_JR; }	-NS- changed to speed up busy loops */
  inline void jr(void)
	{
	  uint32_t oldpc,newpc;

	  oldpc = R.PC.D-1;
	  M_JR;
	  newpc = R.PC.D;
	  if (newpc == oldpc) { if (Z80_ICount > 0) Z80_ICount = 0; } /* speed up busy loop */
	  else if (newpc == oldpc-1 && Z80_RDMEM(newpc) == 0xfb && R.pending_irq == Z80_IGNORE_INT)	/* EI - 1942 */
		{ if (Z80_ICount > 4) Z80_ICount = 4; }
	}

  #define  jr_c() { if (M_C) { M_JR; } else { M_SKIP_JR; } }
  #define  jr_nc() { if (M_NC) { M_JR; } else { M_SKIP_JR; } }
  #define  jr_nz() { if (M_NZ) { M_JR; } else { M_SKIP_JR; } }
  #define  jr_z() { if (M_Z) { M_JR; } else { M_SKIP_JR; } }

  #define  ld_xbc_a() { Z80_WRMEM(R.BC.D,R.AF.B.h); }
  #define  ld_xde_a() { Z80_WRMEM(R.DE.D,R.AF.B.h); }
  #define  ld_xhl_a() { Z80_WRMEM(R.HL.D,R.AF.B.h); }
  #define  ld_xhl_b() { Z80_WRMEM(R.HL.D,R.BC.B.h); }
  #define  ld_xhl_c() { Z80_WRMEM(R.HL.D,R.BC.B.l); }
  #define  ld_xhl_d() { Z80_WRMEM(R.HL.D,R.DE.B.h); }
  #define  ld_xhl_e() { Z80_WRMEM(R.HL.D,R.DE.B.l); }
  #define  ld_xhl_h() { Z80_WRMEM(R.HL.D,R.HL.B.h); }
  #define  ld_xhl_l() { Z80_WRMEM(R.HL.D,R.HL.B.l); }
  #define  ld_xhl_uint8_t() { uint8_t i=Z80_RDMEM_OPCODE(); Z80_WRMEM(R.HL.D,i); }
  #define  ld_xix_a() { M_WR_XIX(R.AF.B.h); }
  #define  ld_xix_b() { M_WR_XIX(R.BC.B.h); }
  #define  ld_xix_c() { M_WR_XIX(R.BC.B.l); }
  #define  ld_xix_d() { M_WR_XIX(R.DE.B.h); }
  #define  ld_xix_e() { M_WR_XIX(R.DE.B.l); }
  #define  ld_xix_h() { M_WR_XIX(R.HL.B.h); }
  #define  ld_xix_l() { M_WR_XIX(R.HL.B.l); }

  inline void ld_xix_uint8_t(void )
	{
	  int i,j;
	  i=M_XIX;
	  j=Z80_RDMEM_OPCODE();
	  Z80_WRMEM(i,j);
	}

  #define  ld_xiy_a() { M_WR_XIY(R.AF.B.h); }
  #define  ld_xiy_b() { M_WR_XIY(R.BC.B.h); }
  #define  ld_xiy_c() { M_WR_XIY(R.BC.B.l); }
  #define  ld_xiy_d() { M_WR_XIY(R.DE.B.h); }
  #define  ld_xiy_e() { M_WR_XIY(R.DE.B.l); }
  #define  ld_xiy_h() { M_WR_XIY(R.HL.B.h); }
  #define  ld_xiy_l() { M_WR_XIY(R.HL.B.l); }

  inline void ld_xiy_uint8_t(void)
	{
	  int i,j;
	  i=M_XIY;
	  j=Z80_RDMEM_OPCODE();
	  Z80_WRMEM(i,j);
	}

  #define  ld_xuint8_t_a() { int i=Z80_RDMEM_OPCODE_U16(); Z80_WRMEM(i,R.AF.B.h); }
  #define  ld_xuint16_t_bc() { Z80_WRMEM_U16(Z80_RDMEM_OPCODE_U16(),R.BC.D); }
  #define  ld_xuint16_t_de() { Z80_WRMEM_U16(Z80_RDMEM_OPCODE_U16(),R.DE.D); }
  #define  ld_xuint16_t_hl() { Z80_WRMEM_U16(Z80_RDMEM_OPCODE_U16(),R.HL.D); }
  #define  ld_xuint16_t_ix() { Z80_WRMEM_U16(Z80_RDMEM_OPCODE_U16(),R.IX.D); }
  #define  ld_xuint16_t_iy() { Z80_WRMEM_U16(Z80_RDMEM_OPCODE_U16(),R.IY.D); }
  #define  ld_xuint16_t_sp() { Z80_WRMEM_U16(Z80_RDMEM_OPCODE_U16(),R.SP.D); }
  #define  ld_a_xbc() { R.AF.B.h=Z80_RDMEM(R.BC.D); }
  #define  ld_a_xde() { R.AF.B.h=Z80_RDMEM(R.DE.D); }
  #define  ld_a_xhl() { R.AF.B.h=M_RD_XHL; }
  #define  ld_a_xix() { R.AF.B.h=M_RD_XIX(); }
  #define  ld_a_xiy() { R.AF.B.h=M_RD_XIY(); }
  #define  ld_a_xuint8_t() { int i=Z80_RDMEM_OPCODE_U16(); R.AF.B.h=Z80_RDMEM(i); }

  #define  ld_a_uint8_t() { R.AF.B.h=Z80_RDMEM_OPCODE(); }
  #define  ld_b_uint8_t() { R.BC.B.h=Z80_RDMEM_OPCODE(); }
  #define  ld_c_uint8_t() { R.BC.B.l=Z80_RDMEM_OPCODE(); }
  #define  ld_d_uint8_t() { R.DE.B.h=Z80_RDMEM_OPCODE(); }
  #define  ld_e_uint8_t() { R.DE.B.l=Z80_RDMEM_OPCODE(); }
  #define  ld_h_uint8_t() { R.HL.B.h=Z80_RDMEM_OPCODE(); }
  #define  ld_l_uint8_t() { R.HL.B.l=Z80_RDMEM_OPCODE(); }
  #define  ld_ixh_uint8_t() { R.IX.B.h=Z80_RDMEM_OPCODE(); }
  #define  ld_ixl_uint8_t() { R.IX.B.l=Z80_RDMEM_OPCODE(); }
  #define  ld_iyh_uint8_t() { R.IY.B.h=Z80_RDMEM_OPCODE(); }
  #define  ld_iyl_uint8_t() { R.IY.B.l=Z80_RDMEM_OPCODE(); }

  #define  ld_b_xhl() { R.BC.B.h=M_RD_XHL; }
  #define  ld_c_xhl() { R.BC.B.l=M_RD_XHL; }
  #define  ld_d_xhl() { R.DE.B.h=M_RD_XHL; }
  #define  ld_e_xhl() { R.DE.B.l=M_RD_XHL; }
  #define  ld_h_xhl() { R.HL.B.h=M_RD_XHL; }
  #define  ld_l_xhl() { R.HL.B.l=M_RD_XHL; }
  #define  ld_b_xix() { R.BC.B.h=M_RD_XIX(); }
  #define  ld_c_xix() { R.BC.B.l=M_RD_XIX(); }
  #define  ld_d_xix() { R.DE.B.h=M_RD_XIX(); }
  #define  ld_e_xix() { R.DE.B.l=M_RD_XIX(); }
  #define  ld_h_xix() { R.HL.B.h=M_RD_XIX(); }
  #define  ld_l_xix() { R.HL.B.l=M_RD_XIX(); }
  #define  ld_b_xiy() { R.BC.B.h=M_RD_XIY(); }
  #define  ld_c_xiy() { R.BC.B.l=M_RD_XIY(); }
  #define  ld_d_xiy() { R.DE.B.h=M_RD_XIY(); }
  #define  ld_e_xiy() { R.DE.B.l=M_RD_XIY(); }
  #define  ld_h_xiy() { R.HL.B.h=M_RD_XIY(); }
  #define  ld_l_xiy() { R.HL.B.l=M_RD_XIY(); }
  #define  ld_a_a() { }
  #define  ld_a_b() { R.AF.B.h=R.BC.B.h; }
  #define  ld_a_c() { R.AF.B.h=R.BC.B.l; }
  #define  ld_a_d() { R.AF.B.h=R.DE.B.h; }
  #define  ld_a_e() { R.AF.B.h=R.DE.B.l; }
  #define  ld_a_h() { R.AF.B.h=R.HL.B.h; }
  #define  ld_a_l() { R.AF.B.h=R.HL.B.l; }
  #define  ld_a_ixh() { R.AF.B.h=R.IX.B.h; }
  #define  ld_a_ixl() { R.AF.B.h=R.IX.B.l; }
  #define  ld_a_iyh() { R.AF.B.h=R.IY.B.h; }
  #define  ld_a_iyl() { R.AF.B.h=R.IY.B.l; }
  #define  ld_b_b() { }
  #define  ld_b_a() { R.BC.B.h=R.AF.B.h; }
  #define  ld_b_c() { R.BC.B.h=R.BC.B.l; }
  #define  ld_b_d() { R.BC.B.h=R.DE.B.h; }
  #define  ld_b_e() { R.BC.B.h=R.DE.B.l; }
  #define  ld_b_h() { R.BC.B.h=R.HL.B.h; }
  #define  ld_b_l() { R.BC.B.h=R.HL.B.l; }
  #define  ld_b_ixh() { R.BC.B.h=R.IX.B.h; }
  #define  ld_b_ixl() { R.BC.B.h=R.IX.B.l; }
  #define  ld_b_iyh() { R.BC.B.h=R.IY.B.h; }
  #define  ld_b_iyl() { R.BC.B.h=R.IY.B.l; }
  #define  ld_c_c() { }
  #define  ld_c_a() { R.BC.B.l=R.AF.B.h; }
  #define  ld_c_b() { R.BC.B.l=R.BC.B.h; }
  #define  ld_c_d() { R.BC.B.l=R.DE.B.h; }
  #define  ld_c_e() { R.BC.B.l=R.DE.B.l; }
  #define  ld_c_h() { R.BC.B.l=R.HL.B.h; }
  #define  ld_c_l() { R.BC.B.l=R.HL.B.l; }
  #define  ld_c_ixh() { R.BC.B.l=R.IX.B.h; }
  #define  ld_c_ixl() { R.BC.B.l=R.IX.B.l; }
  #define  ld_c_iyh() { R.BC.B.l=R.IY.B.h; }
  #define  ld_c_iyl() { R.BC.B.l=R.IY.B.l; }
  #define  ld_d_d() { }
  #define  ld_d_a() { R.DE.B.h=R.AF.B.h; }
  #define  ld_d_c() { R.DE.B.h=R.BC.B.l; }
  #define  ld_d_b() { R.DE.B.h=R.BC.B.h; }
  #define  ld_d_e() { R.DE.B.h=R.DE.B.l; }
  #define  ld_d_h() { R.DE.B.h=R.HL.B.h; }
  #define  ld_d_l() { R.DE.B.h=R.HL.B.l; }
  #define  ld_d_ixh() { R.DE.B.h=R.IX.B.h; }
  #define  ld_d_ixl() { R.DE.B.h=R.IX.B.l; }
  #define  ld_d_iyh() { R.DE.B.h=R.IY.B.h; }
  #define  ld_d_iyl() { R.DE.B.h=R.IY.B.l; }
  #define  ld_e_e() { }
  #define  ld_e_a() { R.DE.B.l=R.AF.B.h; }
  #define  ld_e_c() { R.DE.B.l=R.BC.B.l; }
  #define  ld_e_b() { R.DE.B.l=R.BC.B.h; }
  #define  ld_e_d() { R.DE.B.l=R.DE.B.h; }
  #define  ld_e_h() { R.DE.B.l=R.HL.B.h; }
  #define  ld_e_l() { R.DE.B.l=R.HL.B.l; }
  #define  ld_e_ixh() { R.DE.B.l=R.IX.B.h; }
  #define  ld_e_ixl() { R.DE.B.l=R.IX.B.l; }
  #define  ld_e_iyh() { R.DE.B.l=R.IY.B.h; }
  #define  ld_e_iyl() { R.DE.B.l=R.IY.B.l; }
  #define  ld_h_h() { }
  #define  ld_h_a() { R.HL.B.h=R.AF.B.h; }
  #define  ld_h_c() { R.HL.B.h=R.BC.B.l; }
  #define  ld_h_b() { R.HL.B.h=R.BC.B.h; }
  #define  ld_h_e() { R.HL.B.h=R.DE.B.l; }
  #define  ld_h_d() { R.HL.B.h=R.DE.B.h; }
  #define  ld_h_l() { R.HL.B.h=R.HL.B.l; }
  #define  ld_l_l() { }
  #define  ld_l_a() { R.HL.B.l=R.AF.B.h; }
  #define  ld_l_c() { R.HL.B.l=R.BC.B.l; }
  #define  ld_l_b() { R.HL.B.l=R.BC.B.h; }
  #define  ld_l_e() { R.HL.B.l=R.DE.B.l; }
  #define  ld_l_d() { R.HL.B.l=R.DE.B.h; }
  #define  ld_l_h() { R.HL.B.l=R.HL.B.h; }
  #define  ld_ixh_a() { R.IX.B.h=R.AF.B.h; }
  #define  ld_ixh_b() { R.IX.B.h=R.BC.B.h; }
  #define  ld_ixh_c() { R.IX.B.h=R.BC.B.l; }
  #define  ld_ixh_d() { R.IX.B.h=R.DE.B.h; }
  #define  ld_ixh_e() { R.IX.B.h=R.DE.B.l; }
  #define  ld_ixh_ixh() { }
  #define  ld_ixh_ixl() { R.IX.B.h=R.IX.B.l; }
  #define  ld_ixl_a() { R.IX.B.l=R.AF.B.h; }
  #define  ld_ixl_b() { R.IX.B.l=R.BC.B.h; }
  #define  ld_ixl_c() { R.IX.B.l=R.BC.B.l; }
  #define  ld_ixl_d() { R.IX.B.l=R.DE.B.h; }
  #define  ld_ixl_e() { R.IX.B.l=R.DE.B.l; }
  #define  ld_ixl_ixh() { R.IX.B.l=R.IX.B.h; }
  #define  ld_ixl_ixl() { }
  #define  ld_iyh_a() { R.IY.B.h=R.AF.B.h; }
  #define  ld_iyh_b() { R.IY.B.h=R.BC.B.h; }
  #define  ld_iyh_c() { R.IY.B.h=R.BC.B.l; }
  #define  ld_iyh_d() { R.IY.B.h=R.DE.B.h; }
  #define  ld_iyh_e() { R.IY.B.h=R.DE.B.l; }
  #define  ld_iyh_iyh() { }
  #define  ld_iyh_iyl() { R.IY.B.h=R.IY.B.l; }
  #define  ld_iyl_a() { R.IY.B.l=R.AF.B.h; }
  #define  ld_iyl_b() { R.IY.B.l=R.BC.B.h; }
  #define  ld_iyl_c() { R.IY.B.l=R.BC.B.l; }
  #define  ld_iyl_d() { R.IY.B.l=R.DE.B.h; }
  #define  ld_iyl_e() { R.IY.B.l=R.DE.B.l; }
  #define  ld_iyl_iyh() { R.IY.B.l=R.IY.B.h; }

  #define  ld_iyl_iyl() { }
  #define  ld_bc_xuint16_t() { R.BC.D=Z80_RDMEM_U16(Z80_RDMEM_OPCODE_U16()); }
  #define  ld_bc_uint16_t() { R.BC.D=Z80_RDMEM_OPCODE_U16(); }
  #define  ld_de_xuint16_t() { R.DE.D=Z80_RDMEM_U16(Z80_RDMEM_OPCODE_U16()); }
  #define  ld_de_uint16_t() { R.DE.D=Z80_RDMEM_OPCODE_U16(); }
  #define  ld_hl_xuint16_t() { R.HL.D=Z80_RDMEM_U16(Z80_RDMEM_OPCODE_U16()); }
  #define  ld_hl_uint16_t() { R.HL.D=Z80_RDMEM_OPCODE_U16(); }
  #define  ld_ix_xuint16_t() { R.IX.D=Z80_RDMEM_U16(Z80_RDMEM_OPCODE_U16()); }
  #define  ld_ix_uint16_t() { R.IX.D=Z80_RDMEM_OPCODE_U16(); }
  #define  ld_iy_xuint16_t() { R.IY.D=Z80_RDMEM_U16(Z80_RDMEM_OPCODE_U16()); }
  #define  ld_iy_uint16_t() { R.IY.D=Z80_RDMEM_OPCODE_U16(); }
  #define  ld_sp_xuint16_t() { R.SP.D=Z80_RDMEM_U16(Z80_RDMEM_OPCODE_U16()); }
  #define  ld_sp_uint16_t() { R.SP.D=Z80_RDMEM_OPCODE_U16(); }
  #define  ld_sp_hl() { R.SP.D=R.HL.D; }
  #define  ld_sp_ix() { R.SP.D=R.IX.D; }
  #define  ld_sp_iy() { R.SP.D=R.IY.D; }

  inline void ld_a_i(void)
	{
	  R.AF.B.h=R.I;
	  R.AF.B.l=(R.AF.B.l&C_FLAG)|ZSTable[R.I]|(R.IFF2<<2);
	}
  inline void ld_i_a(void) { R.I=R.AF.B.h; }
  inline void ld_a_r(void)
	{
	  R.AF.B.h=(R.R&127)|(R.R2&128);
	  R.AF.B.l=(R.AF.B.l&C_FLAG)|ZSTable[R.AF.B.h]|(R.IFF2<<2);
	}
  inline void ld_r_a(void) { R.R=R.R2=R.AF.B.h; }

  inline void ldd(void)
	{
	  Z80_WRMEM(R.DE.D,Z80_RDMEM(R.HL.D));
	  --R.DE.W.l;
	  --R.HL.W.l;
	  --R.BC.W.l;
	  R.AF.B.l=(R.AF.B.l&0xE9)|(R.BC.D? V_FLAG:0);
	}
  inline void lddr(void)
	{
	  ldd ();
	  if (R.BC.D) { Z80_ICount-=5; R.PC.W.l-=2; }
	}
  inline void ldi(void)
	{
	  Z80_WRMEM(R.DE.D,Z80_RDMEM(R.HL.D));
	  ++R.DE.W.l;
	  ++R.HL.W.l;
	  --R.BC.W.l;
	  R.AF.B.l=(R.AF.B.l&0xE9)|(R.BC.D? V_FLAG:0);
	}
  inline void ldir(void)
	{
	  ldi ();
	  if (R.BC.D) { Z80_ICount-=5; R.PC.W.l-=2; }
	}

  inline void neg(void)
	{
	  uint8_t i;
	  i=R.AF.B.h;
	  R.AF.B.h=0;
	  M_SUB(i);
	}


  #define  nop() { }

  #define  or_xhl() { uint8_t i=M_RD_XHL; M_OR(i); }
  #define  or_xix() { uint8_t i=M_RD_XIX(); M_OR(i); }
  #define  or_xiy() { uint8_t i=M_RD_XIY(); M_OR(i); }
  #define  or_a() { R.AF.B.l=ZSPTable[R.AF.B.h]; }
  #define  or_b() { M_OR(R.BC.B.h); }
  #define  or_c() { M_OR(R.BC.B.l); }
  #define  or_d() { M_OR(R.DE.B.h); }
  #define  or_e() { M_OR(R.DE.B.l); }
  #define  or_h() { M_OR(R.HL.B.h); }
  #define  or_l() { M_OR(R.HL.B.l); }
  #define  or_ixh() { M_OR(R.IX.B.h); }
  #define  or_ixl() { M_OR(R.IX.B.l); }
  #define  or_iyh() { M_OR(R.IY.B.h); }
  #define  or_iyl() { M_OR(R.IY.B.l); }
  #define  or_uint8_t() { uint8_t i=Z80_RDMEM_OPCODE(); M_OR(i); }

  inline void outd(void)
	{
	  DoOut (R.BC.B.l,R.BC.B.h,Z80_RDMEM(R.HL.D));
	  --R.BC.B.h;
	  --R.HL.W.l;
	  R.AF.B.l=(R.BC.B.h)? N_FLAG:(Z_FLAG|N_FLAG);
	}
  inline void otdr(void)
	{
	  outd ();
	  if (R.BC.B.h) { Z80_ICount-=5; R.PC.W.l-=2; }
	}
  inline void outi(void)
	{
	  DoOut (R.BC.B.l,R.BC.B.h,Z80_RDMEM(R.HL.D));
	  --R.BC.B.h;
	  ++R.HL.W.l;
	  R.AF.B.l=(R.BC.B.h)? N_FLAG:(Z_FLAG|N_FLAG);
	}
  inline void otir(void)
	{
	  outi ();
	  if (R.BC.B.h) { Z80_ICount-=5; R.PC.W.l-=2; }
	}

  inline void out_c_a(void) { DoOut(R.BC.B.l,R.BC.B.h,R.AF.B.h); }
  inline void out_c_b(void) { DoOut(R.BC.B.l,R.BC.B.h,R.BC.B.h); }
  inline void out_c_c(void) { DoOut(R.BC.B.l,R.BC.B.h,R.BC.B.l); }
  inline void out_c_d(void) { DoOut(R.BC.B.l,R.BC.B.h,R.DE.B.h); }
  inline void out_c_e(void) { DoOut(R.BC.B.l,R.BC.B.h,R.DE.B.l); }
  inline void out_c_h(void) { DoOut(R.BC.B.l,R.BC.B.h,R.HL.B.h); }
  inline void out_c_l(void) { DoOut(R.BC.B.l,R.BC.B.h,R.HL.B.l); }
  inline void out_c_0(void) { DoOut(R.BC.B.l,R.BC.B.h,0); }
  inline void out_uint8_t_a(void)
	{
	  uint8_t i=Z80_RDMEM_OPCODE();
	  DoOut(i,R.AF.B.h,R.AF.B.h);
	}

  #define  pop_af() { M_POP(AF); }
  #define  pop_bc() { M_POP(BC); }
  #define  pop_de() { M_POP(DE); }
  #define  pop_hl() { M_POP(HL); }
  #define  pop_ix() { M_POP(IX); }
  #define  pop_iy() { M_POP(IY); }

  #define  push_af() { M_PUSH(AF); }
  #define  push_bc() { M_PUSH(BC); }
  #define  push_de() { M_PUSH(DE); }
  #define  push_hl() { M_PUSH(HL); }
  #define  push_ix() { M_PUSH(IX); }
  #define  push_iy() { M_PUSH(IY); }

  #define  res_0_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_RES(0,i); Z80_WRMEM(R.HL.D,i); }
  #define  res_0_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_RES(0,i); Z80_WRMEM(j,i); }
  #define  res_0_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_RES(0,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_0_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_RES(0,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_0_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_RES(0,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_0_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_RES(0,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_0_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_RES(0,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_0_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_RES(0,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_0_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_RES(0,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_0_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_RES(0,i); Z80_WRMEM(j,i); }
  #define  res_0_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_RES(0,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_0_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_RES(0,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_0_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_RES(0,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_0_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_RES(0,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_0_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_RES(0,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_0_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_RES(0,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_0_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_RES(0,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_0_a() { M_RES(0,R.AF.B.h); }
  #define  res_0_b() { M_RES(0,R.BC.B.h); }
  #define  res_0_c() { M_RES(0,R.BC.B.l); }
  #define  res_0_d() { M_RES(0,R.DE.B.h); }
  #define  res_0_e() { M_RES(0,R.DE.B.l); }
  #define  res_0_h() { M_RES(0,R.HL.B.h); }
  #define  res_0_l() { M_RES(0,R.HL.B.l); }

  #define  res_1_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_RES(1,i); Z80_WRMEM(R.HL.D,i); }
  #define  res_1_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_RES(1,i); Z80_WRMEM(j,i); }
  #define  res_1_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_RES(1,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_1_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_RES(1,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_1_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_RES(1,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_1_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_RES(1,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_1_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_RES(1,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_1_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_RES(1,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_1_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_RES(1,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_1_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_RES(1,i); Z80_WRMEM(j,i); }
  #define  res_1_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_RES(1,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_1_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_RES(1,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_1_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_RES(1,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_1_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_RES(1,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_1_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_RES(1,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_1_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_RES(1,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_1_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_RES(1,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_1_a() { M_RES(1,R.AF.B.h); }
  #define  res_1_b() { M_RES(1,R.BC.B.h); }
  #define  res_1_c() { M_RES(1,R.BC.B.l); }
  #define  res_1_d() { M_RES(1,R.DE.B.h); }
  #define  res_1_e() { M_RES(1,R.DE.B.l); }
  #define  res_1_h() { M_RES(1,R.HL.B.h); }
  #define  res_1_l() { M_RES(1,R.HL.B.l); }

  #define  res_2_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_RES(2,i); Z80_WRMEM(R.HL.D,i); }
  #define  res_2_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_RES(2,i); Z80_WRMEM(j,i); }
  #define  res_2_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_RES(2,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_2_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_RES(2,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_2_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_RES(2,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_2_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_RES(2,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_2_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_RES(2,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_2_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_RES(2,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_2_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_RES(2,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_2_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_RES(2,i); Z80_WRMEM(j,i); }
  #define  res_2_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_RES(2,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_2_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_RES(2,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_2_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_RES(2,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_2_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_RES(2,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_2_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_RES(2,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_2_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_RES(2,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_2_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_RES(2,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_2_a() { M_RES(2,R.AF.B.h); }
  #define  res_2_b() { M_RES(2,R.BC.B.h); }
  #define  res_2_c() { M_RES(2,R.BC.B.l); }
  #define  res_2_d() { M_RES(2,R.DE.B.h); }
  #define  res_2_e() { M_RES(2,R.DE.B.l); }
  #define  res_2_h() { M_RES(2,R.HL.B.h); }
  #define  res_2_l() { M_RES(2,R.HL.B.l); }

  #define  res_3_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_RES(3,i); Z80_WRMEM(R.HL.D,i); }
  #define  res_3_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_RES(3,i); Z80_WRMEM(j,i); }
  #define  res_3_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_RES(3,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_3_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_RES(3,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_3_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_RES(3,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_3_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_RES(3,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_3_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_RES(3,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_3_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_RES(3,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_3_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_RES(3,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_3_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_RES(3,i); Z80_WRMEM(j,i); }
  #define  res_3_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_RES(3,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_3_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_RES(3,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_3_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_RES(3,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_3_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_RES(3,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_3_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_RES(3,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_3_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_RES(3,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_3_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_RES(3,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_3_a() { M_RES(3,R.AF.B.h); }
  #define  res_3_b() { M_RES(3,R.BC.B.h); }
  #define  res_3_c() { M_RES(3,R.BC.B.l); }
  #define  res_3_d() { M_RES(3,R.DE.B.h); }
  #define  res_3_e() { M_RES(3,R.DE.B.l); }
  #define  res_3_h() { M_RES(3,R.HL.B.h); }
  #define  res_3_l() { M_RES(3,R.HL.B.l); }

  #define  res_4_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_RES(4,i); Z80_WRMEM(R.HL.D,i); }
  #define  res_4_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_RES(4,i); Z80_WRMEM(j,i); }
  #define  res_4_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_RES(4,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_4_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_RES(4,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_4_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_RES(4,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_4_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_RES(4,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_4_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_RES(4,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_4_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_RES(4,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_4_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_RES(4,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_4_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_RES(4,i); Z80_WRMEM(j,i); }
  #define  res_4_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_RES(4,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_4_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_RES(4,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_4_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_RES(4,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_4_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_RES(4,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_4_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_RES(4,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_4_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_RES(4,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_4_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_RES(4,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_4_a() { M_RES(4,R.AF.B.h); }
  #define  res_4_b() { M_RES(4,R.BC.B.h); }
  #define  res_4_c() { M_RES(4,R.BC.B.l); }
  #define  res_4_d() { M_RES(4,R.DE.B.h); }
  #define  res_4_e() { M_RES(4,R.DE.B.l); }
  #define  res_4_h() { M_RES(4,R.HL.B.h); }
  #define  res_4_l() { M_RES(4,R.HL.B.l); }

  #define  res_5_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_RES(5,i); Z80_WRMEM(R.HL.D,i); }
  #define  res_5_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_RES(5,i); Z80_WRMEM(j,i); }
  #define  res_5_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_RES(5,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_5_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_RES(5,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_5_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_RES(5,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_5_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_RES(5,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_5_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_RES(5,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_5_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_RES(5,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_5_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_RES(5,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_5_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_RES(5,i); Z80_WRMEM(j,i); }
  #define  res_5_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_RES(5,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_5_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_RES(5,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_5_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_RES(5,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_5_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_RES(5,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_5_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_RES(5,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_5_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_RES(5,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_5_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_RES(5,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_5_a() { M_RES(5,R.AF.B.h); }
  #define  res_5_b() { M_RES(5,R.BC.B.h); }
  #define  res_5_c() { M_RES(5,R.BC.B.l); }
  #define  res_5_d() { M_RES(5,R.DE.B.h); }
  #define  res_5_e() { M_RES(5,R.DE.B.l); }
  #define  res_5_h() { M_RES(5,R.HL.B.h); }
  #define  res_5_l() { M_RES(5,R.HL.B.l); }

  #define  res_6_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_RES(6,i); Z80_WRMEM(R.HL.D,i); }
  #define  res_6_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_RES(6,i); Z80_WRMEM(j,i); }
  #define  res_6_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_RES(6,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_6_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_RES(6,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_6_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_RES(6,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_6_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_RES(6,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_6_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_RES(6,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_6_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_RES(6,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_6_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_RES(6,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_6_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_RES(6,i); Z80_WRMEM(j,i); }
  #define  res_6_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_RES(6,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_6_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_RES(6,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_6_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_RES(6,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_6_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_RES(6,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_6_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_RES(6,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_6_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_RES(6,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_6_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_RES(6,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_6_a() { M_RES(6,R.AF.B.h); }
  #define  res_6_b() { M_RES(6,R.BC.B.h); }
  #define  res_6_c() { M_RES(6,R.BC.B.l); }
  #define  res_6_d() { M_RES(6,R.DE.B.h); }
  #define  res_6_e() { M_RES(6,R.DE.B.l); }
  #define  res_6_h() { M_RES(6,R.HL.B.h); }
  #define  res_6_l() { M_RES(6,R.HL.B.l); }

  #define  res_7_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_RES(7,i); Z80_WRMEM(R.HL.D,i); }
  #define  res_7_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_RES(7,i); Z80_WRMEM(j,i); }
  #define  res_7_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_RES(7,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_7_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_RES(7,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_7_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_RES(7,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_7_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_RES(7,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_7_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_RES(7,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_7_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_RES(7,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_7_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_RES(7,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_7_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_RES(7,i); Z80_WRMEM(j,i); }
  #define  res_7_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_RES(7,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  res_7_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_RES(7,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  res_7_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_RES(7,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  res_7_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_RES(7,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  res_7_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_RES(7,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  res_7_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_RES(7,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  res_7_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_RES(7,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  res_7_a() { M_RES(7,R.AF.B.h); }
  #define  res_7_b() { M_RES(7,R.BC.B.h); }
  #define  res_7_c() { M_RES(7,R.BC.B.l); }
  #define  res_7_d() { M_RES(7,R.DE.B.h); }
  #define  res_7_e() { M_RES(7,R.DE.B.l); }
  #define  res_7_h() { M_RES(7,R.HL.B.h); }
  #define  res_7_l() { M_RES(7,R.HL.B.l); }

  #define  ret() { M_RET; }
  #define  ret_c() { if (M_C) { M_RET; } else { M_SKIP_RET; } }
  #define  ret_m() { if (M_M) { M_RET; } else { M_SKIP_RET; } }
  #define  ret_nc() { if (M_NC) { M_RET; } else { M_SKIP_RET; } }
  #define  ret_nz() { if (M_NZ) { M_RET; } else { M_SKIP_RET; } }
  #define  ret_p() { if (M_P) { M_RET; } else { M_SKIP_RET; } }
  #define  ret_pe() { if (M_PE) { M_RET; } else { M_SKIP_RET; } }
  #define  ret_po() { if (M_PO) { M_RET; } else { M_SKIP_RET; } }
  #define  ret_z() { if (M_Z) { M_RET; } else { M_SKIP_RET; } }


  #define  reti() { M_RET; }
  #define  retn() { R.IFF1=R.IFF2; M_RET; }

  #define  rl_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_RL(i); Z80_WRMEM(R.HL.D,i); }
  #define  rl_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_RL(i); Z80_WRMEM(j,i); }
  #define  rl_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_RL(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  rl_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_RL(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  rl_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_RL(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  rl_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_RL(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  rl_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_RL(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  rl_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_RL(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  rl_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_RL(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  rl_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_RL(i); Z80_WRMEM(j,i); }
  #define  rl_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_RL(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  rl_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_RL(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  rl_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_RL(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  rl_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_RL(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  rl_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_RL(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  rl_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_RL(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  rl_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_RL(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  rl_a() { M_RL(R.AF.B.h); }
  #define  rl_b() { M_RL(R.BC.B.h); }
  #define  rl_c() { M_RL(R.BC.B.l); }
  #define  rl_d() { M_RL(R.DE.B.h); }
  #define  rl_e() { M_RL(R.DE.B.l); }
  #define  rl_h() { M_RL(R.HL.B.h); }
  #define  rl_l() { M_RL(R.HL.B.l); }

  #define  rla()  { M_RLA; }

  #define  rlc_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_RLC(i); Z80_WRMEM(R.HL.D,i); }
  #define  rlc_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_RLC(i); Z80_WRMEM(j,i); }
  #define  rlc_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_RLC(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  rlc_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_RLC(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  rlc_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_RLC(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  rlc_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_RLC(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  rlc_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_RLC(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  rlc_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_RLC(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  rlc_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_RLC(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  rlc_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_RLC(i); Z80_WRMEM(j,i); }
  #define  rlc_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_RLC(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  rlc_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_RLC(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  rlc_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_RLC(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  rlc_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_RLC(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  rlc_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_RLC(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  rlc_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_RLC(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  rlc_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_RLC(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  rlc_a() { M_RLC(R.AF.B.h); }
  #define  rlc_b() { M_RLC(R.BC.B.h); }
  #define  rlc_c() { M_RLC(R.BC.B.l); }
  #define  rlc_d() { M_RLC(R.DE.B.h); }
  #define  rlc_e() { M_RLC(R.DE.B.l); }
  #define  rlc_h() { M_RLC(R.HL.B.h); }
  #define  rlc_l() { M_RLC(R.HL.B.l); }

  #define  rlca()  { M_RLCA; }

  inline void rld(void)
	{
	  uint8_t i;
	  i=Z80_RDMEM(R.HL.D);
	  Z80_WRMEM(R.HL.D,(i<<4)|(R.AF.B.h&0x0F));
	  R.AF.B.h=(R.AF.B.h&0xF0)|(i>>4);
	  R.AF.B.l=(R.AF.B.l&C_FLAG)|ZSPTable[R.AF.B.h];
	}


  #define  rr_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_RR(i); Z80_WRMEM(R.HL.D,i); }
  #define  rr_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_RR(i); Z80_WRMEM(j,i); }
  #define  rr_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_RR(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  rr_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_RR(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  rr_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_RR(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  rr_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_RR(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  rr_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_RR(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  rr_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_RR(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  rr_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_RR(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  rr_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_RR(i); Z80_WRMEM(j,i); }
  #define  rr_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_RR(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  rr_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_RR(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  rr_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_RR(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  rr_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_RR(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  rr_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_RR(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  rr_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_RR(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  rr_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_RR(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  rr_a() { M_RR(R.AF.B.h); }
  #define  rr_b() { M_RR(R.BC.B.h); }
  #define  rr_c() { M_RR(R.BC.B.l); }
  #define  rr_d() { M_RR(R.DE.B.h); }
  #define  rr_e() { M_RR(R.DE.B.l); }
  #define  rr_h() { M_RR(R.HL.B.h); }
  #define  rr_l() { M_RR(R.HL.B.l); }

  #define  rra()  { M_RRA; }

  #define  rrc_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_RRC(i); Z80_WRMEM(R.HL.D,i); }
  #define  rrc_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_RRC(i); Z80_WRMEM(j,i); }
  #define  rrc_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_RRC(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  rrc_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_RRC(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  rrc_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_RRC(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  rrc_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_RRC(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  rrc_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_RRC(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  rrc_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_RRC(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  rrc_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_RRC(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  rrc_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_RRC(i); Z80_WRMEM(j,i); }
  #define  rrc_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_RRC(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define  rrc_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_RRC(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define  rrc_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_RRC(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define  rrc_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_RRC(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define  rrc_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_RRC(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define  rrc_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_RRC(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define  rrc_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_RRC(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define  rrc_a() { M_RRC(R.AF.B.h); }
  #define  rrc_b() { M_RRC(R.BC.B.h); }
  #define  rrc_c() { M_RRC(R.BC.B.l); }
  #define  rrc_d() { M_RRC(R.DE.B.h); }
  #define  rrc_e() { M_RRC(R.DE.B.l); }
  #define  rrc_h() { M_RRC(R.HL.B.h); }
  #define  rrc_l() { M_RRC(R.HL.B.l); }

  #define  rrca()  { M_RRCA; }

  inline void rrd(void)
	{
	  uint8_t i;
	  i=Z80_RDMEM(R.HL.D);
	  Z80_WRMEM(R.HL.D,(i>>4)|(R.AF.B.h<<4));
	  R.AF.B.h=(R.AF.B.h&0xF0)|(i&0x0F);
	  R.AF.B.l=(R.AF.B.l&C_FLAG)|ZSPTable[R.AF.B.h];
	}

  #define rst_00() { M_RST(0x00); }
  #define rst_08() { M_RST(0x08); }
  #define rst_10() { M_RST(0x10); }
  #define rst_18() { M_RST(0x18); }
  #define rst_20() { M_RST(0x20); }
  #define rst_28() { M_RST(0x28); }
  #define rst_30() { M_RST(0x30); }
  #define rst_38() { M_RST(0x38); }

  #define sbc_a_uint8_t() { uint8_t i=Z80_RDMEM_OPCODE(); M_SBC(i); }
  #define sbc_a_xhl() { uint8_t i=M_RD_XHL; M_SBC(i); }
  #define sbc_a_xix() { uint8_t i=M_RD_XIX(); M_SBC(i); }
  #define sbc_a_xiy() { uint8_t i=M_RD_XIY(); M_SBC(i); }
  #define sbc_a_a() { M_SBC(R.AF.B.h); }
  #define sbc_a_b() { M_SBC(R.BC.B.h); }
  #define sbc_a_c() { M_SBC(R.BC.B.l); }
  #define sbc_a_d() { M_SBC(R.DE.B.h); }
  #define sbc_a_e() { M_SBC(R.DE.B.l); }
  #define sbc_a_h() { M_SBC(R.HL.B.h); }
  #define sbc_a_l() { M_SBC(R.HL.B.l); }
  #define sbc_a_ixh() { M_SBC(R.IX.B.h); }
  #define sbc_a_ixl() { M_SBC(R.IX.B.l); }
  #define sbc_a_iyh() { M_SBC(R.IY.B.h); }
  #define sbc_a_iyl() { M_SBC(R.IY.B.l); }

  #define sbc_hl_bc() { M_SBCW(BC); }
  #define sbc_hl_de() { M_SBCW(DE); }
  #define sbc_hl_hl() { M_SBCW(HL); }
  #define sbc_hl_sp() { M_SBCW(SP); }

  #define scf() { R.AF.B.l=(R.AF.B.l&0xEC)|C_FLAG; }

  #define set_0_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_SET(0,i); Z80_WRMEM(R.HL.D,i); }
  #define set_0_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_SET(0,i); Z80_WRMEM(j,i); }
  #define set_0_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_SET(0,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_0_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_SET(0,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_0_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_SET(0,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_0_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_SET(0,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_0_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_SET(0,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_0_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_SET(0,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_0_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_SET(0,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_0_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_SET(0,i); Z80_WRMEM(j,i); }
  #define set_0_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_SET(0,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_0_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_SET(0,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_0_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_SET(0,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_0_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_SET(0,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_0_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_SET(0,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_0_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_SET(0,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_0_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_SET(0,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_0_a() { M_SET(0,R.AF.B.h); }
  #define set_0_b() { M_SET(0,R.BC.B.h); }
  #define set_0_c() { M_SET(0,R.BC.B.l); }
  #define set_0_d() { M_SET(0,R.DE.B.h); }
  #define set_0_e() { M_SET(0,R.DE.B.l); }
  #define set_0_h() { M_SET(0,R.HL.B.h); }
  #define set_0_l() { M_SET(0,R.HL.B.l); }

  #define set_1_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_SET(1,i); Z80_WRMEM(R.HL.D,i); }
  #define set_1_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_SET(1,i); Z80_WRMEM(j,i); }
  #define set_1_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_SET(1,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_1_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_SET(1,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_1_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_SET(1,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_1_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_SET(1,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_1_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_SET(1,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_1_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_SET(1,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_1_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_SET(1,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_1_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_SET(1,i); Z80_WRMEM(j,i); }
  #define set_1_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_SET(1,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_1_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_SET(1,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_1_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_SET(1,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_1_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_SET(1,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_1_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_SET(1,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_1_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_SET(1,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_1_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_SET(1,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_1_a() { M_SET(1,R.AF.B.h); }
  #define set_1_b() { M_SET(1,R.BC.B.h); }
  #define set_1_c() { M_SET(1,R.BC.B.l); }
  #define set_1_d() { M_SET(1,R.DE.B.h); }
  #define set_1_e() { M_SET(1,R.DE.B.l); }
  #define set_1_h() { M_SET(1,R.HL.B.h); }
  #define set_1_l() { M_SET(1,R.HL.B.l); }

  #define set_2_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_SET(2,i); Z80_WRMEM(R.HL.D,i); }
  #define set_2_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_SET(2,i); Z80_WRMEM(j,i); }
  #define set_2_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_SET(2,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_2_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_SET(2,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_2_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_SET(2,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_2_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_SET(2,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_2_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_SET(2,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_2_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_SET(2,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_2_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_SET(2,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_2_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_SET(2,i); Z80_WRMEM(j,i); }
  #define set_2_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_SET(2,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_2_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_SET(2,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_2_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_SET(2,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_2_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_SET(2,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_2_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_SET(2,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_2_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_SET(2,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_2_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_SET(2,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_2_a() { M_SET(2,R.AF.B.h); }
  #define set_2_b() { M_SET(2,R.BC.B.h); }
  #define set_2_c() { M_SET(2,R.BC.B.l); }
  #define set_2_d() { M_SET(2,R.DE.B.h); }
  #define set_2_e() { M_SET(2,R.DE.B.l); }
  #define set_2_h() { M_SET(2,R.HL.B.h); }
  #define set_2_l() { M_SET(2,R.HL.B.l); }

  #define set_3_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_SET(3,i); Z80_WRMEM(R.HL.D,i); }
  #define set_3_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_SET(3,i); Z80_WRMEM(j,i); }
  #define set_3_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_SET(3,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_3_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_SET(3,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_3_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_SET(3,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_3_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_SET(3,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_3_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_SET(3,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_3_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_SET(3,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_3_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_SET(3,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_3_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_SET(3,i); Z80_WRMEM(j,i); }
  #define set_3_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_SET(3,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_3_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_SET(3,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_3_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_SET(3,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_3_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_SET(3,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_3_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_SET(3,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_3_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_SET(3,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_3_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_SET(3,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_3_a() { M_SET(3,R.AF.B.h); }
  #define set_3_b() { M_SET(3,R.BC.B.h); }
  #define set_3_c() { M_SET(3,R.BC.B.l); }
  #define set_3_d() { M_SET(3,R.DE.B.h); }
  #define set_3_e() { M_SET(3,R.DE.B.l); }
  #define set_3_h() { M_SET(3,R.HL.B.h); }
  #define set_3_l() { M_SET(3,R.HL.B.l); }

  #define set_4_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_SET(4,i); Z80_WRMEM(R.HL.D,i); }
  #define set_4_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_SET(4,i); Z80_WRMEM(j,i); }
  #define set_4_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_SET(4,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_4_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_SET(4,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_4_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_SET(4,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_4_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_SET(4,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_4_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_SET(4,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_4_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_SET(4,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_4_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_SET(4,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_4_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_SET(4,i); Z80_WRMEM(j,i); }
  #define set_4_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_SET(4,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_4_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_SET(4,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_4_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_SET(4,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_4_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_SET(4,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_4_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_SET(4,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_4_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_SET(4,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_4_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_SET(4,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_4_a() { M_SET(4,R.AF.B.h); }
  #define set_4_b() { M_SET(4,R.BC.B.h); }
  #define set_4_c() { M_SET(4,R.BC.B.l); }
  #define set_4_d() { M_SET(4,R.DE.B.h); }
  #define set_4_e() { M_SET(4,R.DE.B.l); }
  #define set_4_h() { M_SET(4,R.HL.B.h); }
  #define set_4_l() { M_SET(4,R.HL.B.l); }

  #define set_5_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_SET(5,i); Z80_WRMEM(R.HL.D,i); }
  #define set_5_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_SET(5,i); Z80_WRMEM(j,i); }
  #define set_5_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_SET(5,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_5_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_SET(5,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_5_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_SET(5,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_5_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_SET(5,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_5_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_SET(5,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_5_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_SET(5,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_5_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_SET(5,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_5_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_SET(5,i); Z80_WRMEM(j,i); }
  #define set_5_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_SET(5,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_5_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_SET(5,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_5_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_SET(5,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_5_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_SET(5,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_5_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_SET(5,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_5_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_SET(5,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_5_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_SET(5,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_5_a() { M_SET(5,R.AF.B.h); }
  #define set_5_b() { M_SET(5,R.BC.B.h); }
  #define set_5_c() { M_SET(5,R.BC.B.l); }
  #define set_5_d() { M_SET(5,R.DE.B.h); }
  #define set_5_e() { M_SET(5,R.DE.B.l); }
  #define set_5_h() { M_SET(5,R.HL.B.h); }
  #define set_5_l() { M_SET(5,R.HL.B.l); }

  #define set_6_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_SET(6,i); Z80_WRMEM(R.HL.D,i); }
  #define set_6_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_SET(6,i); Z80_WRMEM(j,i); }
  #define set_6_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_SET(6,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_6_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_SET(6,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_6_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_SET(6,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_6_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_SET(6,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_6_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_SET(6,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_6_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_SET(6,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_6_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_SET(6,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_6_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_SET(6,i); Z80_WRMEM(j,i); }
  #define set_6_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_SET(6,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_6_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_SET(6,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_6_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_SET(6,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_6_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_SET(6,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_6_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_SET(6,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_6_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_SET(6,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_6_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_SET(6,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_6_a() { M_SET(6,R.AF.B.h); }
  #define set_6_b() { M_SET(6,R.BC.B.h); }
  #define set_6_c() { M_SET(6,R.BC.B.l); }
  #define set_6_d() { M_SET(6,R.DE.B.h); }
  #define set_6_e() { M_SET(6,R.DE.B.l); }
  #define set_6_h() { M_SET(6,R.HL.B.h); }
  #define set_6_l() { M_SET(6,R.HL.B.l); }

  #define set_7_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_SET(7,i); Z80_WRMEM(R.HL.D,i); }
  #define set_7_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_SET(7,i); Z80_WRMEM(j,i); }
  #define set_7_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_SET(7,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_7_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_SET(7,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_7_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_SET(7,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_7_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_SET(7,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_7_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_SET(7,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_7_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_SET(7,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_7_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_SET(7,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_7_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_SET(7,i); Z80_WRMEM(j,i); }
  #define set_7_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_SET(7,R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define set_7_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_SET(7,R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define set_7_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_SET(7,R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define set_7_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_SET(7,R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define set_7_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_SET(7,R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define set_7_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_SET(7,R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define set_7_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_SET(7,R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define set_7_a() { M_SET(7,R.AF.B.h); }
  #define set_7_b() { M_SET(7,R.BC.B.h); }
  #define set_7_c() { M_SET(7,R.BC.B.l); }
  #define set_7_d() { M_SET(7,R.DE.B.h); }
  #define set_7_e() { M_SET(7,R.DE.B.l); }
  #define set_7_h() { M_SET(7,R.HL.B.h); }
  #define set_7_l() { M_SET(7,R.HL.B.l); }


  #define sla_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_SLA(i); Z80_WRMEM(R.HL.D,i); }
  #define sla_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_SLA(i); Z80_WRMEM(j,i); }
  #define sla_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_SLA(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define sla_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_SLA(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define sla_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_SLA(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define sla_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_SLA(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define sla_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_SLA(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define sla_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_SLA(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define sla_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_SLA(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define sla_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_SLA(i); Z80_WRMEM(j,i); }
  #define sla_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_SLA(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define sla_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_SLA(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define sla_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_SLA(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define sla_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_SLA(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define sla_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_SLA(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define sla_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_SLA(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define sla_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_SLA(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define sla_a() { M_SLA(R.AF.B.h); }
  #define sla_b() { M_SLA(R.BC.B.h); }
  #define sla_c() { M_SLA(R.BC.B.l); }
  #define sla_d() { M_SLA(R.DE.B.h); }
  #define sla_e() { M_SLA(R.DE.B.l); }
  #define sla_h() { M_SLA(R.HL.B.h); }
  #define sla_l() { M_SLA(R.HL.B.l); }


  #define sll_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_SLL(i); Z80_WRMEM(R.HL.D,i); }
  #define sll_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_SLL(i); Z80_WRMEM(j,i); }
  #define sll_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_SLL(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define sll_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_SLL(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define sll_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_SLL(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define sll_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_SLL(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define sll_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_SLL(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define sll_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_SLL(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define sll_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_SLL(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define sll_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_SLL(i); Z80_WRMEM(j,i); }
  #define sll_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_SLL(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define sll_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_SLL(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define sll_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_SLL(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define sll_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_SLL(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define sll_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_SLL(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define sll_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_SLL(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define sll_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_SLL(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define sll_a() { M_SLL(R.AF.B.h); }
  #define sll_b() { M_SLL(R.BC.B.h); }
  #define sll_c() { M_SLL(R.BC.B.l); }
  #define sll_d() { M_SLL(R.DE.B.h); }
  #define sll_e() { M_SLL(R.DE.B.l); }
  #define sll_h() { M_SLL(R.HL.B.h); }
  #define sll_l() { M_SLL(R.HL.B.l); }

  #define sra_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_SRA(i); Z80_WRMEM(R.HL.D,i); }
  #define sra_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_SRA(i); Z80_WRMEM(j,i); }
  #define sra_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_SRA(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define sra_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_SRA(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define sra_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_SRA(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define sra_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_SRA(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define sra_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_SRA(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define sra_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_SRA(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define sra_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_SRA(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define sra_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_SRA(i); Z80_WRMEM(j,i); }
  #define sra_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_SRA(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define sra_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_SRA(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define sra_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_SRA(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define sra_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_SRA(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define sra_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_SRA(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define sra_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_SRA(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define sra_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_SRA(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define sra_a() { M_SRA(R.AF.B.h); }
  #define sra_b() { M_SRA(R.BC.B.h); }
  #define sra_c() { M_SRA(R.BC.B.l); }
  #define sra_d() { M_SRA(R.DE.B.h); }
  #define sra_e() { M_SRA(R.DE.B.l); }
  #define sra_h() { M_SRA(R.HL.B.h); }
  #define sra_l() { M_SRA(R.HL.B.l); }

  #define srl_xhl() { uint8_t i=Z80_RDMEM(R.HL.D); M_SRL(i); Z80_WRMEM(R.HL.D,i); }
  #define srl_xix() { int j=M_XIX; uint8_t i=Z80_RDMEM(j); M_SRL(i); Z80_WRMEM(j,i); }
  #define srl_xix_a() { int j=M_XIX; R.AF.B.h=Z80_RDMEM(j); M_SRL(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define srl_xix_b() { int j=M_XIX; R.BC.B.h=Z80_RDMEM(j); M_SRL(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define srl_xix_c() { int j=M_XIX; R.BC.B.l=Z80_RDMEM(j); M_SRL(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define srl_xix_d() { int j=M_XIX; R.DE.B.h=Z80_RDMEM(j); M_SRL(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define srl_xix_e() { int j=M_XIX; R.DE.B.l=Z80_RDMEM(j); M_SRL(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define srl_xix_h() { int j=M_XIX; R.HL.B.h=Z80_RDMEM(j); M_SRL(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define srl_xix_l() { int j=M_XIX; R.HL.B.l=Z80_RDMEM(j); M_SRL(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define srl_xiy() { int j=M_XIY; uint8_t i=Z80_RDMEM(j); M_SRL(i); Z80_WRMEM(j,i); }
  #define srl_xiy_a() { int j=M_XIY; R.AF.B.h=Z80_RDMEM(j); M_SRL(R.AF.B.h); Z80_WRMEM(j,R.AF.B.h); }
  #define srl_xiy_b() { int j=M_XIY; R.BC.B.h=Z80_RDMEM(j); M_SRL(R.BC.B.h); Z80_WRMEM(j,R.BC.B.h); }
  #define srl_xiy_c() { int j=M_XIY; R.BC.B.l=Z80_RDMEM(j); M_SRL(R.BC.B.l); Z80_WRMEM(j,R.BC.B.l); }
  #define srl_xiy_d() { int j=M_XIY; R.DE.B.h=Z80_RDMEM(j); M_SRL(R.DE.B.h); Z80_WRMEM(j,R.DE.B.h); }
  #define srl_xiy_e() { int j=M_XIY; R.DE.B.l=Z80_RDMEM(j); M_SRL(R.DE.B.l); Z80_WRMEM(j,R.DE.B.l); }
  #define srl_xiy_h() { int j=M_XIY; R.HL.B.h=Z80_RDMEM(j); M_SRL(R.HL.B.h); Z80_WRMEM(j,R.HL.B.h); }
  #define srl_xiy_l() { int j=M_XIY; R.HL.B.l=Z80_RDMEM(j); M_SRL(R.HL.B.l); Z80_WRMEM(j,R.HL.B.l); }
  #define srl_a() { M_SRL(R.AF.B.h); }
  #define srl_b() { M_SRL(R.BC.B.h); }
  #define srl_c() { M_SRL(R.BC.B.l); }
  #define srl_d() { M_SRL(R.DE.B.h); }
  #define srl_e() { M_SRL(R.DE.B.l); }
  #define srl_h() { M_SRL(R.HL.B.h); }
  #define srl_l() { M_SRL(R.HL.B.l); }

  #define sub_xhl() { uint8_t i=M_RD_XHL; M_SUB(i); }
  #define sub_xix() { uint8_t i=M_RD_XIX(); M_SUB(i); }
  #define sub_xiy() { uint8_t i=M_RD_XIY(); M_SUB(i); }
  #define sub_a() { R.AF.D=Z_FLAG|N_FLAG; }
  #define sub_b() { M_SUB(R.BC.B.h); }
  #define sub_c() { M_SUB(R.BC.B.l); }
  #define sub_d() { M_SUB(R.DE.B.h); }
  #define sub_e() { M_SUB(R.DE.B.l); }
  #define sub_h() { M_SUB(R.HL.B.h); }
  #define sub_l() { M_SUB(R.HL.B.l); }
  #define sub_ixh() { M_SUB(R.IX.B.h); }
  #define sub_ixl() { M_SUB(R.IX.B.l); }
  #define sub_iyh() { M_SUB(R.IY.B.h); }
  #define sub_iyl() { M_SUB(R.IY.B.l); }
  #define sub_uint8_t() { uint8_t i=Z80_RDMEM_OPCODE(); M_SUB(i); }

  #define xor_xhl() { uint8_t i=M_RD_XHL; M_XOR(i); }
  #define xor_xix() { uint8_t i=M_RD_XIX(); M_XOR(i); }
  #define xor_xiy() { uint8_t i=M_RD_XIY(); M_XOR(i); }
  #define xor_a() { R.AF.D=Z_FLAG|V_FLAG; }
  #define xor_b() { M_XOR(R.BC.B.h); }
  #define xor_c() { M_XOR(R.BC.B.l); }
  #define xor_d() { M_XOR(R.DE.B.h); }
  #define xor_e() { M_XOR(R.DE.B.l); }
  #define xor_h() { M_XOR(R.HL.B.h); }
  #define xor_l() { M_XOR(R.HL.B.l); }
  #define xor_ixh() { M_XOR(R.IX.B.h); }
  #define xor_ixl() { M_XOR(R.IX.B.l); }
  #define xor_iyh() { M_XOR(R.IY.B.h); }
  #define xor_iyl() { M_XOR(R.IY.B.l); }
  #define xor_uint8_t() { uint8_t i=Z80_RDMEM_OPCODE(); M_XOR(i); }

  #define no_op() { --R.PC.W.l; }
  #define patch() { }
};

