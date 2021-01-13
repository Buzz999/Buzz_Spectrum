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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ZXSpectrum.h"
#include "Z80.h"

Z80_Regs::Z80_Regs () {
  AF.D = 0;
  BC.D = 0;
  DE.D = 0;
  HL.D = 0;
  IX.D = 0;
  IY.D = 0;

  AF2.D = 0;
  BC2.D = 0;
  DE2.D = 0;
  HL2.D = 0;

  PC.D = 0;
  SP.D = 0;

  IFF1 = 0;
  IFF2 = 0;

  HALT = 0;
  IM = 0;

  I = 0;
  R = 0;
  R2 = 0;

  pending_irq = 0;
  pending_nmi = 0;
}

Z80_Regs::~Z80_Regs () {
}

Z80_Regs::Z80_Regs (const Z80_Regs &aZ80_Regs) {
  AF.W.l = aZ80_Regs.AF.W.l;
  BC.W.l = aZ80_Regs.BC.W.l;
  DE.W.l = aZ80_Regs.DE.W.l;
  HL.W.l = aZ80_Regs.HL.W.l;
  IX.W.l = aZ80_Regs.IX.W.l;
  IY.W.l = aZ80_Regs.IY.W.l;

  AF2.W.l = aZ80_Regs.AF2.W.l;
  BC2.W.l = aZ80_Regs.BC2.W.l;
  DE2.W.l = aZ80_Regs.DE2.W.l;
  HL2.W.l = aZ80_Regs.HL2.W.l;

  PC.W.l = aZ80_Regs.PC.W.l;
  SP.W.l = aZ80_Regs.SP.W.l;

  IFF1 = aZ80_Regs.IFF1;
  IFF2 = aZ80_Regs.IFF2;

  HALT = aZ80_Regs.HALT;
  IM = aZ80_Regs.IM;

  I = aZ80_Regs.I;
  R = aZ80_Regs.R;
  R2 = aZ80_Regs.R2;

  pending_irq = aZ80_Regs.pending_irq;
  pending_nmi = aZ80_Regs.pending_nmi;
}

Z80_Regs& Z80_Regs::operator=(const Z80_Regs &aZ80_Regs) {
  if (this == &aZ80_Regs) return *this;
  AF.W.l = aZ80_Regs.AF.W.l;
  BC.W.l = aZ80_Regs.BC.W.l;
  DE.W.l = aZ80_Regs.DE.W.l;
  HL.W.l = aZ80_Regs.HL.W.l;
  IX.W.l = aZ80_Regs.IX.W.l;
  IY.W.l = aZ80_Regs.IY.W.l;

  AF2.W.l = aZ80_Regs.AF2.W.l;
  BC2.W.l = aZ80_Regs.BC2.W.l;
  DE2.W.l = aZ80_Regs.DE2.W.l;
  HL2.W.l = aZ80_Regs.HL2.W.l;

  PC.W.l = aZ80_Regs.PC.W.l;
  SP.W.l = aZ80_Regs.SP.W.l;

  IFF1 = aZ80_Regs.IFF1;
  IFF2 = aZ80_Regs.IFF2;

  HALT = aZ80_Regs.HALT;
  IM = aZ80_Regs.IM;

  I = aZ80_Regs.I;
  R = aZ80_Regs.R;
  R2 = aZ80_Regs.R2;

  pending_irq = aZ80_Regs.pending_irq;
  pending_nmi = aZ80_Regs.pending_nmi;

  return *this;
}

void Z80_Regs::Dump (void) {
  printf ("AF:%04X HL:%04X DE:%04X BC:%04X PC:%04X SP:%04X IX:%04X IY:%04X  \n",
	  AF.W.l,HL.W.l,DE.W.l,BC.W.l,PC.W.l,SP.W.l,IX.W.l,IY.W.l);
}

Z80::Z80 (ZXMemory* aZXMemory, ZXULA* aZXULA, ZXIO* aZXIO) {
  theZXMemory = aZXMemory;
  theZXULA = aZXULA;
  theZXIO = aZXIO;

  Z80_ICount = 0;

  InitTables ();
}

Z80::~Z80 () {
}

uint8_t Z80::Z80_In(uint16_t Port) const{
  uint8_t data;
  theZXIO->CPUReadFromZXIO (Port, data);
  return data;
}

void Z80::Z80_Out(uint16_t Port, uint8_t Value) {
  theZXIO->CPUWriteToZXIO (Port, Value, Z80_ICount);
}

uint8_t Z80::Z80_RDMEM (uint16_t adr) const {
  uint8_t data;
  theZXMemory->ReadFromZXMemory (adr, data);
  return data;
}

void Z80::Z80_WRMEM(uint16_t adr, uint8_t data) {
  theZXMemory->WriteToZXMemory (adr, data);
  theZXULA->WriteToVideo (adr,data);
}

void Z80::call_opcode_dd_cb (uint8_t aCode)
{
  __label__
l_0, l_1, l_2, l_3, l_4, l_5, l_6, l_7, l_8, l_9, l_10, l_11, l_12, l_13, l_14,
l_15, l_16, l_17, l_18, l_19, l_20, l_21, l_22, l_23, l_24, l_25, l_26, l_27,
l_28, l_29, l_30, l_31, l_32, l_33, l_34, l_35, l_36, l_37, l_38, l_39, l_40,
l_41, l_42, l_43, l_44, l_45, l_46, l_47, l_48, l_49, l_50, l_51, l_52, l_53,
l_54, l_55, l_56, l_57, l_58, l_59, l_60, l_61, l_62, l_63, l_64, l_65, l_66,
l_67, l_68, l_69, l_70, l_71, l_72, l_73, l_74, l_75, l_76, l_77, l_78, l_79,
l_80, l_81, l_82, l_83, l_84, l_85, l_86, l_87, l_88, l_89, l_90, l_91, l_92,
l_93, l_94, l_95, l_96, l_97, l_98, l_99, l_100, l_101, l_102, l_103, l_104,
l_105, l_106, l_107, l_108, l_109, l_110, l_111, l_112, l_113, l_114, l_115,
l_116, l_117, l_118, l_119, l_120, l_121, l_122, l_123, l_124, l_125, l_126,
l_127, l_128, l_129, l_130, l_131, l_132, l_133, l_134, l_135, l_136, l_137,
l_138, l_139, l_140, l_141, l_142, l_143, l_144, l_145, l_146, l_147, l_148,
l_149, l_150, l_151, l_152, l_153, l_154, l_155, l_156, l_157, l_158, l_159,
l_160, l_161, l_162, l_163, l_164, l_165, l_166, l_167, l_168, l_169, l_170,
l_171, l_172, l_173, l_174, l_175, l_176, l_177, l_178, l_179, l_180, l_181,
l_182, l_183, l_184, l_185, l_186, l_187, l_188, l_189, l_190, l_191, l_192,
l_193, l_194, l_195, l_196, l_197, l_198, l_199, l_200, l_201, l_202, l_203,
l_204, l_205, l_206, l_207, l_208, l_209, l_210, l_211, l_212, l_213, l_214,
l_215, l_216, l_217, l_218, l_219, l_220, l_221, l_222, l_223, l_224, l_225,
l_226, l_227, l_228, l_229, l_230, l_231, l_232, l_233, l_234, l_235, l_236,
l_237, l_238, l_239, l_240, l_241, l_242, l_243, l_244, l_245, l_246, l_247,
l_248, l_249, l_250, l_251, l_252, l_253, l_254, l_255;

    static const void* const a_jump_table[256] =
	  { &&
l_0, && l_1, && l_2, && l_3, && l_4, && l_5, && l_6, && l_7, && l_8, && l_9, && l_10, && l_11, && l_12, && l_13, && l_14, &&
l_15, && l_16, && l_17, && l_18, && l_19, && l_20, && l_21, && l_22, && l_23, && l_24, && l_25, && l_26, && l_27, &&
l_28, && l_29, && l_30, && l_31, && l_32, && l_33, && l_34, && l_35, && l_36, && l_37, && l_38, && l_39, && l_40, &&
l_41, && l_42, && l_43, && l_44, && l_45, && l_46, && l_47, && l_48, && l_49, && l_50, && l_51, && l_52, && l_53, &&
l_54, && l_55, && l_56, && l_57, && l_58, && l_59, && l_60, && l_61, && l_62, && l_63, && l_64, && l_65, && l_66, &&
l_67, && l_68, && l_69, && l_70, && l_71, && l_72, && l_73, && l_74, && l_75, && l_76, && l_77, && l_78, && l_79, &&
l_80, && l_81, && l_82, && l_83, && l_84, && l_85, && l_86, && l_87, && l_88, && l_89, && l_90, && l_91, && l_92, &&
l_93, && l_94, && l_95, && l_96, && l_97, && l_98, && l_99, && l_100, && l_101, && l_102, && l_103, && l_104, &&
l_105, && l_106, && l_107, && l_108, && l_109, && l_110, && l_111, && l_112, && l_113, && l_114, && l_115, &&
l_116, && l_117, && l_118, && l_119, && l_120, && l_121, && l_122, && l_123, && l_124, && l_125, && l_126, &&
l_127, && l_128, && l_129, && l_130, && l_131, && l_132, && l_133, && l_134, && l_135, && l_136, && l_137, &&
l_138, && l_139, && l_140, && l_141, && l_142, && l_143, && l_144, && l_145, && l_146, && l_147, && l_148, &&
l_149, && l_150, && l_151, && l_152, && l_153, && l_154, && l_155, && l_156, && l_157, && l_158, && l_159, &&
l_160, && l_161, && l_162, && l_163, && l_164, && l_165, && l_166, && l_167, && l_168, && l_169, && l_170, &&
l_171, && l_172, && l_173, && l_174, && l_175, && l_176, && l_177, && l_178, && l_179, && l_180, && l_181, &&
l_182, && l_183, && l_184, && l_185, && l_186, && l_187, && l_188, && l_189, && l_190, && l_191, && l_192, &&
l_193, && l_194, && l_195, && l_196, && l_197, && l_198, && l_199, && l_200, && l_201, && l_202, && l_203, &&
l_204, && l_205, && l_206, && l_207, && l_208, && l_209, && l_210, && l_211, && l_212, && l_213, && l_214, &&
l_215, && l_216, && l_217, && l_218, && l_219, && l_220, && l_221, && l_222, && l_223, && l_224, && l_225, &&
l_226, && l_227, && l_228, && l_229, && l_230, && l_231, && l_232, && l_233, && l_234, && l_235, && l_236, &&
l_237, && l_238, && l_239, && l_240, && l_241, && l_242, && l_243, && l_244, && l_245, && l_246, && l_247, &&
l_248, && l_249, && l_250, && l_251, && l_252, && l_253, && l_254, && l_255 };

	 goto *a_jump_table[aCode];

  //switch(aCode){
  l_0:rlc_xix_b();return;
  l_1:rlc_xix_c();return;
  l_2:rlc_xix_d();return;
  l_3:rlc_xix_e();return;
  l_4:rlc_xix_h();return;
  l_5:rlc_xix_l();return;
  l_6:rlc_xix();return;
  l_7:rlc_xix_a();return;
  l_8:rrc_xix_b();return;
  l_9:rrc_xix_c();return;
  l_10:rrc_xix_d();return;
  l_11:rrc_xix_e();return;
  l_12:rrc_xix_h();return;
  l_13:rrc_xix_l();return;
  l_14:rrc_xix();return;
  l_15:rrc_xix_a();return;
  l_16:rl_xix_b();return;
  l_17:rl_xix_c();return;
  l_18:rl_xix_d();return;
  l_19:rl_xix_e();return;
  l_20:rl_xix_h();return;
  l_21:rl_xix_l();return;
  l_22:rl_xix();return;
  l_23:rl_xix_a();return;
  l_24:rr_xix_b();return;
  l_25:rr_xix_c();return;
  l_26:rr_xix_d();return;
  l_27:rr_xix_e();return;
  l_28:rr_xix_h();return;
  l_29:rr_xix_l();return;
  l_30:rr_xix();return;
  l_31:rr_xix_a();return;
  l_32:sla_xix_b();return;
  l_33:sla_xix_c();return;
  l_34:sla_xix_d();return;
  l_35:sla_xix_e();return;
  l_36:sla_xix_h();return;
  l_37:sla_xix_l();return;
  l_38:sla_xix();return;
  l_39:sla_xix_a();return;
  l_40:sra_xix_b();return;
  l_41:sra_xix_c();return;
  l_42:sra_xix_d();return;
  l_43:sra_xix_e();return;
  l_44:sra_xix_h();return;
  l_45:sra_xix_l();return;
  l_46:sra_xix();return;
  l_47:sra_xix_a();return;
  l_48:sll_xix_b();return;
  l_49:sll_xix_c();return;
  l_50:sll_xix_d();return;
  l_51:sll_xix_e();return;
  l_52:sll_xix_h();return;
  l_53:sll_xix_l();return;
  l_54:sll_xix();return;
  l_55:sll_xix_a();return;
  l_56:srl_xix_b();return;
  l_57:srl_xix_c();return;
  l_58:srl_xix_d();return;
  l_59:srl_xix_e();return;
  l_60:srl_xix_h();return;
  l_61:srl_xix_l();return;
  l_62:srl_xix();return;
  l_63:srl_xix_a();return;
  l_64:bit_0_xix_b();return;
  l_65:bit_0_xix_c();return;
  l_66:bit_0_xix_d();return;
  l_67:bit_0_xix_e();return;
  l_68:bit_0_xix_h();return;
  l_69:bit_0_xix_l();return;
  l_70:bit_0_xix();return;
  l_71:bit_0_xix_a();return;
  l_72:bit_1_xix_b();return;
  l_73:bit_1_xix_c();return;
  l_74:bit_1_xix_d();return;
  l_75:bit_1_xix_e();return;
  l_76:bit_1_xix_h();return;
  l_77:bit_1_xix_l();return;
  l_78:bit_1_xix();return;
  l_79:bit_1_xix_a();return;
  l_80:bit_2_xix_b();return;
  l_81:bit_2_xix_c();return;
  l_82:bit_2_xix_d();return;
  l_83:bit_2_xix_e();return;
  l_84:bit_2_xix_h();return;
  l_85:bit_2_xix_l();return;
  l_86:bit_2_xix();return;
  l_87:bit_2_xix_a();return;
  l_88:bit_3_xix_b();return;
  l_89:bit_3_xix_c();return;
  l_90:bit_3_xix_d();return;
  l_91:bit_3_xix_e();return;
  l_92:bit_3_xix_h();return;
  l_93:bit_3_xix_l();return;
  l_94:bit_3_xix();return;
  l_95:bit_3_xix_a();return;
  l_96:bit_4_xix_b();return;
  l_97:bit_4_xix_c();return;
  l_98:bit_4_xix_d();return;
  l_99:bit_4_xix_e();return;
  l_100:bit_4_xix_h();return;
  l_101:bit_4_xix_l();return;
  l_102:bit_4_xix();return;
  l_103:bit_4_xix_a();return;
  l_104:bit_5_xix_b();return;
  l_105:bit_5_xix_c();return;
  l_106:bit_5_xix_d();return;
  l_107:bit_5_xix_e();return;
  l_108:bit_5_xix_h();return;
  l_109:bit_5_xix_l();return;
  l_110:bit_5_xix();return;
  l_111:bit_5_xix_a();return;
  l_112:bit_6_xix_b();return;
  l_113:bit_6_xix_c();return;
  l_114:bit_6_xix_d();return;
  l_115:bit_6_xix_e();return;
  l_116:bit_6_xix_h();return;
  l_117:bit_6_xix_l();return;
  l_118:bit_6_xix();return;
  l_119:bit_6_xix_a();return;
  l_120:bit_7_xix_b();return;
  l_121:bit_7_xix_c();return;
  l_122:bit_7_xix_d();return;
  l_123:bit_7_xix_e();return;
  l_124:bit_7_xix_h();return;
  l_125:bit_7_xix_l();return;
  l_126:bit_7_xix();return;
  l_127:bit_7_xix_a();return;
  l_128:res_0_xix_b();return;
  l_129:res_0_xix_c();return;
  l_130:res_0_xix_d();return;
  l_131:res_0_xix_e();return;
  l_132:res_0_xix_h();return;
  l_133:res_0_xix_l();return;
  l_134:res_0_xix();return;
  l_135:res_0_xix_a();return;
  l_136:res_1_xix_b();return;
  l_137:res_1_xix_c();return;
  l_138:res_1_xix_d();return;
  l_139:res_1_xix_e();return;
  l_140:res_1_xix_h();return;
  l_141:res_1_xix_l();return;
  l_142:res_1_xix();return;
  l_143:res_1_xix_a();return;
  l_144:res_2_xix_b();return;
  l_145:res_2_xix_c();return;
  l_146:res_2_xix_d();return;
  l_147:res_2_xix_e();return;
  l_148:res_2_xix_h();return;
  l_149:res_2_xix_l();return;
  l_150:res_2_xix();return;
  l_151:res_2_xix_a();return;
  l_152:res_3_xix_b();return;
  l_153:res_3_xix_c();return;
  l_154:res_3_xix_d();return;
  l_155:res_3_xix_e();return;
  l_156:res_3_xix_h();return;
  l_157:res_3_xix_l();return;
  l_158:res_3_xix();return;
  l_159:res_3_xix_a();return;
  l_160:res_4_xix_b();return;
  l_161:res_4_xix_c();return;
  l_162:res_4_xix_d();return;
  l_163:res_4_xix_e();return;
  l_164:res_4_xix_h();return;
  l_165:res_4_xix_l();return;
  l_166:res_4_xix();return;
  l_167:res_4_xix_a();return;
  l_168:res_5_xix_b();return;
  l_169:res_5_xix_c();return;
  l_170:res_5_xix_d();return;
  l_171:res_5_xix_e();return;
  l_172:res_5_xix_h();return;
  l_173:res_5_xix_l();return;
  l_174:res_5_xix();return;
  l_175:res_5_xix_a();return;
  l_176:res_6_xix_b();return;
  l_177:res_6_xix_c();return;
  l_178:res_6_xix_d();return;
  l_179:res_6_xix_e();return;
  l_180:res_6_xix_h();return;
  l_181:res_6_xix_l();return;
  l_182:res_6_xix();return;
  l_183:res_6_xix_a();return;
  l_184:res_7_xix_b();return;
  l_185:res_7_xix_c();return;
  l_186:res_7_xix_d();return;
  l_187:res_7_xix_e();return;
  l_188:res_7_xix_h();return;
  l_189:res_7_xix_l();return;
  l_190:res_7_xix();return;
  l_191:res_7_xix_a();return;
  l_192:set_0_xix_b();return;
  l_193:set_0_xix_c();return;
  l_194:set_0_xix_d();return;
  l_195:set_0_xix_e();return;
  l_196:set_0_xix_h();return;
  l_197:set_0_xix_l();return;
  l_198:set_0_xix();return;
  l_199:set_0_xix_a();return;
  l_200:set_1_xix_b();return;
  l_201:set_1_xix_c();return;
  l_202:set_1_xix_d();return;
  l_203:set_1_xix_e();return;
  l_204:set_1_xix_h();return;
  l_205:set_1_xix_l();return;
  l_206:set_1_xix();return;
  l_207:set_1_xix_a();return;
  l_208:set_2_xix_b();return;
  l_209:set_2_xix_c();return;
  l_210:set_2_xix_d();return;
  l_211:set_2_xix_e();return;
  l_212:set_2_xix_h();return;
  l_213:set_2_xix_l();return;
  l_214:set_2_xix();return;
  l_215:set_2_xix_a();return;
  l_216:set_3_xix_b();return;
  l_217:set_3_xix_c();return;
  l_218:set_3_xix_d();return;
  l_219:set_3_xix_e();return;
  l_220:set_3_xix_h();return;
  l_221:set_3_xix_l();return;
  l_222:set_3_xix();return;
  l_223:set_3_xix_a();return;
  l_224:set_4_xix_b();return;
  l_225:set_4_xix_c();return;
  l_226:set_4_xix_d();return;
  l_227:set_4_xix_e();return;
  l_228:set_4_xix_h();return;
  l_229:set_4_xix_l();return;
  l_230:set_4_xix();return;
  l_231:set_4_xix_a();return;
  l_232:set_5_xix_b();return;
  l_233:set_5_xix_c();return;
  l_234:set_5_xix_d();return;
  l_235:set_5_xix_e();return;
  l_236:set_5_xix_h();return;
  l_237:set_5_xix_l();return;
  l_238:set_5_xix();return;
  l_239:set_5_xix_a();return;
  l_240:set_6_xix_b();return;
  l_241:set_6_xix_c();return;
  l_242:set_6_xix_d();return;
  l_243:set_6_xix_e();return;
  l_244:set_6_xix_h();return;
  l_245:set_6_xix_l();return;
  l_246:set_6_xix();return;
  l_247:set_6_xix_a();return;
  l_248:set_7_xix_b();return;
  l_249:set_7_xix_c();return;
  l_250:set_7_xix_d();return;
  l_251:set_7_xix_e();return;
  l_252:set_7_xix_h();return;
  l_253:set_7_xix_l();return;
  l_254:set_7_xix();return;
  l_255:set_7_xix_a();return;

  //}
}
void Z80::call_opcode_fd_cb(uint8_t aCode)
{
  __label__
l_0, l_1, l_2, l_3, l_4, l_5, l_6, l_7, l_8, l_9, l_10, l_11, l_12, l_13, l_14,
l_15, l_16, l_17, l_18, l_19, l_20, l_21, l_22, l_23, l_24, l_25, l_26, l_27,
l_28, l_29, l_30, l_31, l_32, l_33, l_34, l_35, l_36, l_37, l_38, l_39, l_40,
l_41, l_42, l_43, l_44, l_45, l_46, l_47, l_48, l_49, l_50, l_51, l_52, l_53,
l_54, l_55, l_56, l_57, l_58, l_59, l_60, l_61, l_62, l_63, l_64, l_65, l_66,
l_67, l_68, l_69, l_70, l_71, l_72, l_73, l_74, l_75, l_76, l_77, l_78, l_79,
l_80, l_81, l_82, l_83, l_84, l_85, l_86, l_87, l_88, l_89, l_90, l_91, l_92,
l_93, l_94, l_95, l_96, l_97, l_98, l_99, l_100, l_101, l_102, l_103, l_104,
l_105, l_106, l_107, l_108, l_109, l_110, l_111, l_112, l_113, l_114, l_115,
l_116, l_117, l_118, l_119, l_120, l_121, l_122, l_123, l_124, l_125, l_126,
l_127, l_128, l_129, l_130, l_131, l_132, l_133, l_134, l_135, l_136, l_137,
l_138, l_139, l_140, l_141, l_142, l_143, l_144, l_145, l_146, l_147, l_148,
l_149, l_150, l_151, l_152, l_153, l_154, l_155, l_156, l_157, l_158, l_159,
l_160, l_161, l_162, l_163, l_164, l_165, l_166, l_167, l_168, l_169, l_170,
l_171, l_172, l_173, l_174, l_175, l_176, l_177, l_178, l_179, l_180, l_181,
l_182, l_183, l_184, l_185, l_186, l_187, l_188, l_189, l_190, l_191, l_192,
l_193, l_194, l_195, l_196, l_197, l_198, l_199, l_200, l_201, l_202, l_203,
l_204, l_205, l_206, l_207, l_208, l_209, l_210, l_211, l_212, l_213, l_214,
l_215, l_216, l_217, l_218, l_219, l_220, l_221, l_222, l_223, l_224, l_225,
l_226, l_227, l_228, l_229, l_230, l_231, l_232, l_233, l_234, l_235, l_236,
l_237, l_238, l_239, l_240, l_241, l_242, l_243, l_244, l_245, l_246, l_247,
l_248, l_249, l_250, l_251, l_252, l_253, l_254, l_255;

    static const void* const a_jump_table[256] =
	  { &&
l_0, && l_1, && l_2, && l_3, && l_4, && l_5, && l_6, && l_7, && l_8, && l_9, && l_10, && l_11, && l_12, && l_13, && l_14, &&
l_15, && l_16, && l_17, && l_18, && l_19, && l_20, && l_21, && l_22, && l_23, && l_24, && l_25, && l_26, && l_27, &&
l_28, && l_29, && l_30, && l_31, && l_32, && l_33, && l_34, && l_35, && l_36, && l_37, && l_38, && l_39, && l_40, &&
l_41, && l_42, && l_43, && l_44, && l_45, && l_46, && l_47, && l_48, && l_49, && l_50, && l_51, && l_52, && l_53, &&
l_54, && l_55, && l_56, && l_57, && l_58, && l_59, && l_60, && l_61, && l_62, && l_63, && l_64, && l_65, && l_66, &&
l_67, && l_68, && l_69, && l_70, && l_71, && l_72, && l_73, && l_74, && l_75, && l_76, && l_77, && l_78, && l_79, &&
l_80, && l_81, && l_82, && l_83, && l_84, && l_85, && l_86, && l_87, && l_88, && l_89, && l_90, && l_91, && l_92, &&
l_93, && l_94, && l_95, && l_96, && l_97, && l_98, && l_99, && l_100, && l_101, && l_102, && l_103, && l_104, &&
l_105, && l_106, && l_107, && l_108, && l_109, && l_110, && l_111, && l_112, && l_113, && l_114, && l_115, &&
l_116, && l_117, && l_118, && l_119, && l_120, && l_121, && l_122, && l_123, && l_124, && l_125, && l_126, &&
l_127, && l_128, && l_129, && l_130, && l_131, && l_132, && l_133, && l_134, && l_135, && l_136, && l_137, &&
l_138, && l_139, && l_140, && l_141, && l_142, && l_143, && l_144, && l_145, && l_146, && l_147, && l_148, &&
l_149, && l_150, && l_151, && l_152, && l_153, && l_154, && l_155, && l_156, && l_157, && l_158, && l_159, &&
l_160, && l_161, && l_162, && l_163, && l_164, && l_165, && l_166, && l_167, && l_168, && l_169, && l_170, &&
l_171, && l_172, && l_173, && l_174, && l_175, && l_176, && l_177, && l_178, && l_179, && l_180, && l_181, &&
l_182, && l_183, && l_184, && l_185, && l_186, && l_187, && l_188, && l_189, && l_190, && l_191, && l_192, &&
l_193, && l_194, && l_195, && l_196, && l_197, && l_198, && l_199, && l_200, && l_201, && l_202, && l_203, &&
l_204, && l_205, && l_206, && l_207, && l_208, && l_209, && l_210, && l_211, && l_212, && l_213, && l_214, &&
l_215, && l_216, && l_217, && l_218, && l_219, && l_220, && l_221, && l_222, && l_223, && l_224, && l_225, &&
l_226, && l_227, && l_228, && l_229, && l_230, && l_231, && l_232, && l_233, && l_234, && l_235, && l_236, &&
l_237, && l_238, && l_239, && l_240, && l_241, && l_242, && l_243, && l_244, && l_245, && l_246, && l_247, &&
l_248, && l_249, && l_250, && l_251, && l_252, && l_253, && l_254, && l_255 };

	 goto *a_jump_table[aCode];

  //switch(aCode){
  l_0:rlc_xiy_b();return;
  l_1:rlc_xiy_c();return;
  l_2:rlc_xiy_d();return;
  l_3:rlc_xiy_e();return;
  l_4:rlc_xiy_h();return;
  l_5:rlc_xiy_l();return;
  l_6:rlc_xiy();return;
  l_7:rlc_xiy_a();return;
  l_8:rrc_xiy_b();return;
  l_9:rrc_xiy_c();return;
  l_10:rrc_xiy_d();return;
  l_11:rrc_xiy_e();return;
  l_12:rrc_xiy_h();return;
  l_13:rrc_xiy_l();return;
  l_14:rrc_xiy();return;
  l_15:rrc_xiy_a();return;
  l_16:rl_xiy_b();return;
  l_17:rl_xiy_c();return;
  l_18:rl_xiy_d();return;
  l_19:rl_xiy_e();return;
  l_20:rl_xiy_h();return;
  l_21:rl_xiy_l();return;
  l_22:rl_xiy();return;
  l_23:rl_xiy_a();return;
  l_24:rr_xiy_b();return;
  l_25:rr_xiy_c();return;
  l_26:rr_xiy_d();return;
  l_27:rr_xiy_e();return;
  l_28:rr_xiy_h();return;
  l_29:rr_xiy_l();return;
  l_30:rr_xiy();return;
  l_31:rr_xiy_a();return;
  l_32:sla_xiy_b();return;
  l_33:sla_xiy_c();return;
  l_34:sla_xiy_d();return;
  l_35:sla_xiy_e();return;
  l_36:sla_xiy_h();return;
  l_37:sla_xiy_l();return;
  l_38:sla_xiy();return;
  l_39:sla_xiy_a();return;
  l_40:sra_xiy_b();return;
  l_41:sra_xiy_c();return;
  l_42:sra_xiy_d();return;
  l_43:sra_xiy_e();return;
  l_44:sra_xiy_h();return;
  l_45:sra_xiy_l();return;
  l_46:sra_xiy();return;
  l_47:sra_xiy_a();return;
  l_48:sll_xiy_b();return;
  l_49:sll_xiy_c();return;
  l_50:sll_xiy_d();return;
  l_51:sll_xiy_e();return;
  l_52:sll_xiy_h();return;
  l_53:sll_xiy_l();return;
  l_54:sll_xiy();return;
  l_55:sll_xiy_a();return;
  l_56:srl_xiy_b();return;
  l_57:srl_xiy_c();return;
  l_58:srl_xiy_d();return;
  l_59:srl_xiy_e();return;
  l_60:srl_xiy_h();return;
  l_61:srl_xiy_l();return;
  l_62:srl_xiy();return;
  l_63:srl_xiy_a();return;
  l_64:bit_0_xiy_b();return;
  l_65:bit_0_xiy_c();return;
  l_66:bit_0_xiy_d();return;
  l_67:bit_0_xiy_e();return;
  l_68:bit_0_xiy_h();return;
  l_69:bit_0_xiy_l();return;
  l_70:bit_0_xiy();return;
  l_71:bit_0_xiy_a();return;
  l_72:bit_1_xiy_b();return;
  l_73:bit_1_xiy_c();return;
  l_74:bit_1_xiy_d();return;
  l_75:bit_1_xiy_e();return;
  l_76:bit_1_xiy_h();return;
  l_77:bit_1_xiy_l();return;
  l_78:bit_1_xiy();return;
  l_79:bit_1_xiy_a();return;
  l_80:bit_2_xiy_b();return;
  l_81:bit_2_xiy_c();return;
  l_82:bit_2_xiy_d();return;
  l_83:bit_2_xiy_e();return;
  l_84:bit_2_xiy_h();return;
  l_85:bit_2_xiy_l();return;
  l_86:bit_2_xiy();return;
  l_87:bit_2_xiy_a();return;
  l_88:bit_3_xiy_b();return;
  l_89:bit_3_xiy_c();return;
  l_90:bit_3_xiy_d();return;
  l_91:bit_3_xiy_e();return;
  l_92:bit_3_xiy_h();return;
  l_93:bit_3_xiy_l();return;
  l_94:bit_3_xiy();return;
  l_95:bit_3_xiy_a();return;
  l_96:bit_4_xiy_b();return;
  l_97:bit_4_xiy_c();return;
  l_98:bit_4_xiy_d();return;
  l_99:bit_4_xiy_e();return;
  l_100:bit_4_xiy_h();return;
  l_101:bit_4_xiy_l();return;
  l_102:bit_4_xiy();return;
  l_103:bit_4_xiy_a();return;
  l_104:bit_5_xiy_b();return;
  l_105:bit_5_xiy_c();return;
  l_106:bit_5_xiy_d();return;
  l_107:bit_5_xiy_e();return;
  l_108:bit_5_xiy_h();return;
  l_109:bit_5_xiy_l();return;
  l_110:bit_5_xiy();return;
  l_111:bit_5_xiy_a();return;
  l_112:bit_6_xiy_b();return;
  l_113:bit_6_xiy_c();return;
  l_114:bit_6_xiy_d();return;
  l_115:bit_6_xiy_e();return;
  l_116:bit_6_xiy_h();return;
  l_117:bit_6_xiy_l();return;
  l_118:bit_6_xiy();return;
  l_119:bit_6_xiy_a();return;
  l_120:bit_7_xiy_b();return;
  l_121:bit_7_xiy_c();return;
  l_122:bit_7_xiy_d();return;
  l_123:bit_7_xiy_e();return;
  l_124:bit_7_xiy_h();return;
  l_125:bit_7_xiy_l();return;
  l_126:bit_7_xiy();return;
  l_127:bit_7_xiy_a();return;
  l_128:res_0_xiy_b();return;
  l_129:res_0_xiy_c();return;
  l_130:res_0_xiy_d();return;
  l_131:res_0_xiy_e();return;
  l_132:res_0_xiy_h();return;
  l_133:res_0_xiy_l();return;
  l_134:res_0_xiy();return;
  l_135:res_0_xiy_a();return;
  l_136:res_1_xiy_b();return;
  l_137:res_1_xiy_c();return;
  l_138:res_1_xiy_d();return;
  l_139:res_1_xiy_e();return;
  l_140:res_1_xiy_h();return;
  l_141:res_1_xiy_l();return;
  l_142:res_1_xiy();return;
  l_143:res_1_xiy_a();return;
  l_144:res_2_xiy_b();return;
  l_145:res_2_xiy_c();return;
  l_146:res_2_xiy_d();return;
  l_147:res_2_xiy_e();return;
  l_148:res_2_xiy_h();return;
  l_149:res_2_xiy_l();return;
  l_150:res_2_xiy();return;
  l_151:res_2_xiy_a();return;
  l_152:res_3_xiy_b();return;
  l_153:res_3_xiy_c();return;
  l_154:res_3_xiy_d();return;
  l_155:res_3_xiy_e();return;
  l_156:res_3_xiy_h();return;
  l_157:res_3_xiy_l();return;
  l_158:res_3_xiy();return;
  l_159:res_3_xiy_a();return;
  l_160:res_4_xiy_b();return;
  l_161:res_4_xiy_c();return;
  l_162:res_4_xiy_d();return;
  l_163:res_4_xiy_e();return;
  l_164:res_4_xiy_h();return;
  l_165:res_4_xiy_l();return;
  l_166:res_4_xiy();return;
  l_167:res_4_xiy_a();return;
  l_168:res_5_xiy_b();return;
  l_169:res_5_xiy_c();return;
  l_170:res_5_xiy_d();return;
  l_171:res_5_xiy_e();return;
  l_172:res_5_xiy_h();return;
  l_173:res_5_xiy_l();return;
  l_174:res_5_xiy();return;
  l_175:res_5_xiy_a();return;
  l_176:res_6_xiy_b();return;
  l_177:res_6_xiy_c();return;
  l_178:res_6_xiy_d();return;
  l_179:res_6_xiy_e();return;
  l_180:res_6_xiy_h();return;
  l_181:res_6_xiy_l();return;
  l_182:res_6_xiy();return;
  l_183:res_6_xiy_a();return;
  l_184:res_7_xiy_b();return;
  l_185:res_7_xiy_c();return;
  l_186:res_7_xiy_d();return;
  l_187:res_7_xiy_e();return;
  l_188:res_7_xiy_h();return;
  l_189:res_7_xiy_l();return;
  l_190:res_7_xiy();return;
  l_191:res_7_xiy_a();return;
  l_192:set_0_xiy_b();return;
  l_193:set_0_xiy_c();return;
  l_194:set_0_xiy_d();return;
  l_195:set_0_xiy_e();return;
  l_196:set_0_xiy_h();return;
  l_197:set_0_xiy_l();return;
  l_198:set_0_xiy();return;
  l_199:set_0_xiy_a();return;
  l_200:set_1_xiy_b();return;
  l_201:set_1_xiy_c();return;
  l_202:set_1_xiy_d();return;
  l_203:set_1_xiy_e();return;
  l_204:set_1_xiy_h();return;
  l_205:set_1_xiy_l();return;
  l_206:set_1_xiy();return;
  l_207:set_1_xiy_a();return;
  l_208:set_2_xiy_b();return;
  l_209:set_2_xiy_c();return;
  l_210:set_2_xiy_d();return;
  l_211:set_2_xiy_e();return;
  l_212:set_2_xiy_h();return;
  l_213:set_2_xiy_l();return;
  l_214:set_2_xiy();return;
  l_215:set_2_xiy_a();return;
  l_216:set_3_xiy_b();return;
  l_217:set_3_xiy_c();return;
  l_218:set_3_xiy_d();return;
  l_219:set_3_xiy_e();return;
  l_220:set_3_xiy_h();return;
  l_221:set_3_xiy_l();return;
  l_222:set_3_xiy();return;
  l_223:set_3_xiy_a();return;
  l_224:set_4_xiy_b();return;
  l_225:set_4_xiy_c();return;
  l_226:set_4_xiy_d();return;
  l_227:set_4_xiy_e();return;
  l_228:set_4_xiy_h();return;
  l_229:set_4_xiy_l();return;
  l_230:set_4_xiy();return;
  l_231:set_4_xiy_a();return;
  l_232:set_5_xiy_b();return;
  l_233:set_5_xiy_c();return;
  l_234:set_5_xiy_d();return;
  l_235:set_5_xiy_e();return;
  l_236:set_5_xiy_h();return;
  l_237:set_5_xiy_l();return;
  l_238:set_5_xiy();return;
  l_239:set_5_xiy_a();return;
  l_240:set_6_xiy_b();return;
  l_241:set_6_xiy_c();return;
  l_242:set_6_xiy_d();return;
  l_243:set_6_xiy_e();return;
  l_244:set_6_xiy_h();return;
  l_245:set_6_xiy_l();return;
  l_246:set_6_xiy();return;
  l_247:set_6_xiy_a();return;
  l_248:set_7_xiy_b();return;
  l_249:set_7_xiy_c();return;
  l_250:set_7_xiy_d();return;
  l_251:set_7_xiy_e();return;
  l_252:set_7_xiy_h();return;
  l_253:set_7_xiy_l();return;
  l_254:set_7_xiy();return;
  l_255:set_7_xiy_a();return;
  //}
}

void Z80::call_opcode_cb(uint8_t aCode)
{
  __label__
l_0, l_1, l_2, l_3, l_4, l_5, l_6, l_7, l_8, l_9, l_10, l_11, l_12, l_13, l_14,
l_15, l_16, l_17, l_18, l_19, l_20, l_21, l_22, l_23, l_24, l_25, l_26, l_27,
l_28, l_29, l_30, l_31, l_32, l_33, l_34, l_35, l_36, l_37, l_38, l_39, l_40,
l_41, l_42, l_43, l_44, l_45, l_46, l_47, l_48, l_49, l_50, l_51, l_52, l_53,
l_54, l_55, l_56, l_57, l_58, l_59, l_60, l_61, l_62, l_63, l_64, l_65, l_66,
l_67, l_68, l_69, l_70, l_71, l_72, l_73, l_74, l_75, l_76, l_77, l_78, l_79,
l_80, l_81, l_82, l_83, l_84, l_85, l_86, l_87, l_88, l_89, l_90, l_91, l_92,
l_93, l_94, l_95, l_96, l_97, l_98, l_99, l_100, l_101, l_102, l_103, l_104,
l_105, l_106, l_107, l_108, l_109, l_110, l_111, l_112, l_113, l_114, l_115,
l_116, l_117, l_118, l_119, l_120, l_121, l_122, l_123, l_124, l_125, l_126,
l_127, l_128, l_129, l_130, l_131, l_132, l_133, l_134, l_135, l_136, l_137,
l_138, l_139, l_140, l_141, l_142, l_143, l_144, l_145, l_146, l_147, l_148,
l_149, l_150, l_151, l_152, l_153, l_154, l_155, l_156, l_157, l_158, l_159,
l_160, l_161, l_162, l_163, l_164, l_165, l_166, l_167, l_168, l_169, l_170,
l_171, l_172, l_173, l_174, l_175, l_176, l_177, l_178, l_179, l_180, l_181,
l_182, l_183, l_184, l_185, l_186, l_187, l_188, l_189, l_190, l_191, l_192,
l_193, l_194, l_195, l_196, l_197, l_198, l_199, l_200, l_201, l_202, l_203,
l_204, l_205, l_206, l_207, l_208, l_209, l_210, l_211, l_212, l_213, l_214,
l_215, l_216, l_217, l_218, l_219, l_220, l_221, l_222, l_223, l_224, l_225,
l_226, l_227, l_228, l_229, l_230, l_231, l_232, l_233, l_234, l_235, l_236,
l_237, l_238, l_239, l_240, l_241, l_242, l_243, l_244, l_245, l_246, l_247,
l_248, l_249, l_250, l_251, l_252, l_253, l_254, l_255;

    static const void* const a_jump_table[256] =
	  { &&
l_0, && l_1, && l_2, && l_3, && l_4, && l_5, && l_6, && l_7, && l_8, && l_9, && l_10, && l_11, && l_12, && l_13, && l_14, &&
l_15, && l_16, && l_17, && l_18, && l_19, && l_20, && l_21, && l_22, && l_23, && l_24, && l_25, && l_26, && l_27, &&
l_28, && l_29, && l_30, && l_31, && l_32, && l_33, && l_34, && l_35, && l_36, && l_37, && l_38, && l_39, && l_40, &&
l_41, && l_42, && l_43, && l_44, && l_45, && l_46, && l_47, && l_48, && l_49, && l_50, && l_51, && l_52, && l_53, &&
l_54, && l_55, && l_56, && l_57, && l_58, && l_59, && l_60, && l_61, && l_62, && l_63, && l_64, && l_65, && l_66, &&
l_67, && l_68, && l_69, && l_70, && l_71, && l_72, && l_73, && l_74, && l_75, && l_76, && l_77, && l_78, && l_79, &&
l_80, && l_81, && l_82, && l_83, && l_84, && l_85, && l_86, && l_87, && l_88, && l_89, && l_90, && l_91, && l_92, &&
l_93, && l_94, && l_95, && l_96, && l_97, && l_98, && l_99, && l_100, && l_101, && l_102, && l_103, && l_104, &&
l_105, && l_106, && l_107, && l_108, && l_109, && l_110, && l_111, && l_112, && l_113, && l_114, && l_115, &&
l_116, && l_117, && l_118, && l_119, && l_120, && l_121, && l_122, && l_123, && l_124, && l_125, && l_126, &&
l_127, && l_128, && l_129, && l_130, && l_131, && l_132, && l_133, && l_134, && l_135, && l_136, && l_137, &&
l_138, && l_139, && l_140, && l_141, && l_142, && l_143, && l_144, && l_145, && l_146, && l_147, && l_148, &&
l_149, && l_150, && l_151, && l_152, && l_153, && l_154, && l_155, && l_156, && l_157, && l_158, && l_159, &&
l_160, && l_161, && l_162, && l_163, && l_164, && l_165, && l_166, && l_167, && l_168, && l_169, && l_170, &&
l_171, && l_172, && l_173, && l_174, && l_175, && l_176, && l_177, && l_178, && l_179, && l_180, && l_181, &&
l_182, && l_183, && l_184, && l_185, && l_186, && l_187, && l_188, && l_189, && l_190, && l_191, && l_192, &&
l_193, && l_194, && l_195, && l_196, && l_197, && l_198, && l_199, && l_200, && l_201, && l_202, && l_203, &&
l_204, && l_205, && l_206, && l_207, && l_208, && l_209, && l_210, && l_211, && l_212, && l_213, && l_214, &&
l_215, && l_216, && l_217, && l_218, && l_219, && l_220, && l_221, && l_222, && l_223, && l_224, && l_225, &&
l_226, && l_227, && l_228, && l_229, && l_230, && l_231, && l_232, && l_233, && l_234, && l_235, && l_236, &&
l_237, && l_238, && l_239, && l_240, && l_241, && l_242, && l_243, && l_244, && l_245, && l_246, && l_247, &&
l_248, && l_249, && l_250, && l_251, && l_252, && l_253, && l_254, && l_255 };

	 goto *a_jump_table[aCode];

  //switch(aCode){
  l_0:rlc_b();return;
  l_1:rlc_c();return;
  l_2:rlc_d();return;
  l_3:rlc_e();return;
  l_4:rlc_h();return;
  l_5:rlc_l();return;
  l_6:rlc_xhl();return;
  l_7:rlc_a();return;
  l_8:rrc_b();return;
  l_9:rrc_c();return;
  l_10:rrc_d();return;
  l_11:rrc_e();return;
  l_12:rrc_h();return;
  l_13:rrc_l();return;
  l_14:rrc_xhl();return;
  l_15:rrc_a();return;
  l_16:rl_b();return;
  l_17:rl_c();return;
  l_18:rl_d();return;
  l_19:rl_e();return;
  l_20:rl_h();return;
  l_21:rl_l();return;
  l_22:rl_xhl();return;
  l_23:rl_a();return;
  l_24:rr_b();return;
  l_25:rr_c();return;
  l_26:rr_d();return;
  l_27:rr_e();return;
  l_28:rr_h();return;
  l_29:rr_l();return;
  l_30:rr_xhl();return;
  l_31:rr_a();return;
  l_32:sla_b();return;
  l_33:sla_c();return;
  l_34:sla_d();return;
  l_35:sla_e();return;
  l_36:sla_h();return;
  l_37:sla_l();return;
  l_38:sla_xhl();return;
  l_39:sla_a();return;
  l_40:sra_b();return;
  l_41:sra_c();return;
  l_42:sra_d();return;
  l_43:sra_e();return;
  l_44:sra_h();return;
  l_45:sra_l();return;
  l_46:sra_xhl();return;
  l_47:sra_a();return;
  l_48:sll_b();return;
  l_49:sll_c();return;
  l_50:sll_d();return;
  l_51:sll_e();return;
  l_52:sll_h();return;
  l_53:sll_l();return;
  l_54:sll_xhl();return;
  l_55:sll_a();return;
  l_56:srl_b();return;
  l_57:srl_c();return;
  l_58:srl_d();return;
  l_59:srl_e();return;
  l_60:srl_h();return;
  l_61:srl_l();return;
  l_62:srl_xhl();return;
  l_63:srl_a();return;
  l_64:bit_0_b();return;
  l_65:bit_0_c();return;
  l_66:bit_0_d();return;
  l_67:bit_0_e();return;
  l_68:bit_0_h();return;
  l_69:bit_0_l();return;
  l_70:bit_0_xhl();return;
  l_71:bit_0_a();return;
  l_72:bit_1_b();return;
  l_73:bit_1_c();return;
  l_74:bit_1_d();return;
  l_75:bit_1_e();return;
  l_76:bit_1_h();return;
  l_77:bit_1_l();return;
  l_78:bit_1_xhl();return;
  l_79:bit_1_a();return;
  l_80:bit_2_b();return;
  l_81:bit_2_c();return;
  l_82:bit_2_d();return;
  l_83:bit_2_e();return;
  l_84:bit_2_h();return;
  l_85:bit_2_l();return;
  l_86:bit_2_xhl();return;
  l_87:bit_2_a();return;
  l_88:bit_3_b();return;
  l_89:bit_3_c();return;
  l_90:bit_3_d();return;
  l_91:bit_3_e();return;
  l_92:bit_3_h();return;
  l_93:bit_3_l();return;
  l_94:bit_3_xhl();return;
  l_95:bit_3_a();return;
  l_96:bit_4_b();return;
  l_97:bit_4_c();return;
  l_98:bit_4_d();return;
  l_99:bit_4_e();return;
  l_100:bit_4_h();return;
  l_101:bit_4_l();return;
  l_102:bit_4_xhl();return;
  l_103:bit_4_a();return;
  l_104:bit_5_b();return;
  l_105:bit_5_c();return;
  l_106:bit_5_d();return;
  l_107:bit_5_e();return;
  l_108:bit_5_h();return;
  l_109:bit_5_l();return;
  l_110:bit_5_xhl();return;
  l_111:bit_5_a();return;
  l_112:bit_6_b();return;
  l_113:bit_6_c();return;
  l_114:bit_6_d();return;
  l_115:bit_6_e();return;
  l_116:bit_6_h();return;
  l_117:bit_6_l();return;
  l_118:bit_6_xhl();return;
  l_119:bit_6_a();return;
  l_120:bit_7_b();return;
  l_121:bit_7_c();return;
  l_122:bit_7_d();return;
  l_123:bit_7_e();return;
  l_124:bit_7_h();return;
  l_125:bit_7_l();return;
  l_126:bit_7_xhl();return;
  l_127:bit_7_a();return;
  l_128:res_0_b();return;
  l_129:res_0_c();return;
  l_130:res_0_d();return;
  l_131:res_0_e();return;
  l_132:res_0_h();return;
  l_133:res_0_l();return;
  l_134:res_0_xhl();return;
  l_135:res_0_a();return;
  l_136:res_1_b();return;
  l_137:res_1_c();return;
  l_138:res_1_d();return;
  l_139:res_1_e();return;
  l_140:res_1_h();return;
  l_141:res_1_l();return;
  l_142:res_1_xhl();return;
  l_143:res_1_a();return;
  l_144:res_2_b();return;
  l_145:res_2_c();return;
  l_146:res_2_d();return;
  l_147:res_2_e();return;
  l_148:res_2_h();return;
  l_149:res_2_l();return;
  l_150:res_2_xhl();return;
  l_151:res_2_a();return;
  l_152:res_3_b();return;
  l_153:res_3_c();return;
  l_154:res_3_d();return;
  l_155:res_3_e();return;
  l_156:res_3_h();return;
  l_157:res_3_l();return;
  l_158:res_3_xhl();return;
  l_159:res_3_a();return;
  l_160:res_4_b();return;
  l_161:res_4_c();return;
  l_162:res_4_d();return;
  l_163:res_4_e();return;
  l_164:res_4_h();return;
  l_165:res_4_l();return;
  l_166:res_4_xhl();return;
  l_167:res_4_a();return;
  l_168:res_5_b();return;
  l_169:res_5_c();return;
  l_170:res_5_d();return;
  l_171:res_5_e();return;
  l_172:res_5_h();return;
  l_173:res_5_l();return;
  l_174:res_5_xhl();return;
  l_175:res_5_a();return;
  l_176:res_6_b();return;
  l_177:res_6_c();return;
  l_178:res_6_d();return;
  l_179:res_6_e();return;
  l_180:res_6_h();return;
  l_181:res_6_l();return;
  l_182:res_6_xhl();return;
  l_183:res_6_a();return;
  l_184:res_7_b();return;
  l_185:res_7_c();return;
  l_186:res_7_d();return;
  l_187:res_7_e();return;
  l_188:res_7_h();return;
  l_189:res_7_l();return;
  l_190:res_7_xhl();return;
  l_191:res_7_a();return;
  l_192:set_0_b();return;
  l_193:set_0_c();return;
  l_194:set_0_d();return;
  l_195:set_0_e();return;
  l_196:set_0_h();return;
  l_197:set_0_l();return;
  l_198:set_0_xhl();return;
  l_199:set_0_a();return;
  l_200:set_1_b();return;
  l_201:set_1_c();return;
  l_202:set_1_d();return;
  l_203:set_1_e();return;
  l_204:set_1_h();return;
  l_205:set_1_l();return;
  l_206:set_1_xhl();return;
  l_207:set_1_a();return;
  l_208:set_2_b();return;
  l_209:set_2_c();return;
  l_210:set_2_d();return;
  l_211:set_2_e();return;
  l_212:set_2_h();return;
  l_213:set_2_l();return;
  l_214:set_2_xhl();return;
  l_215:set_2_a();return;
  l_216:set_3_b();return;
  l_217:set_3_c();return;
  l_218:set_3_d();return;
  l_219:set_3_e();return;
  l_220:set_3_h();return;
  l_221:set_3_l();return;
  l_222:set_3_xhl();return;
  l_223:set_3_a();return;
  l_224:set_4_b();return;
  l_225:set_4_c();return;
  l_226:set_4_d();return;
  l_227:set_4_e();return;
  l_228:set_4_h();return;
  l_229:set_4_l();return;
  l_230:set_4_xhl();return;
  l_231:set_4_a();return;
  l_232:set_5_b();return;
  l_233:set_5_c();return;
  l_234:set_5_d();return;
  l_235:set_5_e();return;
  l_236:set_5_h();return;
  l_237:set_5_l();return;
  l_238:set_5_xhl();return;
  l_239:set_5_a();return;
  l_240:set_6_b();return;
  l_241:set_6_c();return;
  l_242:set_6_d();return;
  l_243:set_6_e();return;
  l_244:set_6_h();return;
  l_245:set_6_l();return;
  l_246:set_6_xhl();return;
  l_247:set_6_a();return;
  l_248:set_7_b();return;
  l_249:set_7_c();return;
  l_250:set_7_d();return;
  l_251:set_7_e();return;
  l_252:set_7_h();return;
  l_253:set_7_l();return;
  l_254:set_7_xhl();return;
  l_255:set_7_a();return;
  //}
}

void Z80::call_opcode_dd(uint8_t aCode)
{
  __label__
l_0, l_1, l_2, l_3, l_4, l_5, l_6, l_7, l_8, l_9, l_10, l_11, l_12, l_13, l_14,
l_15, l_16, l_17, l_18, l_19, l_20, l_21, l_22, l_23, l_24, l_25, l_26, l_27,
l_28, l_29, l_30, l_31, l_32, l_33, l_34, l_35, l_36, l_37, l_38, l_39, l_40,
l_41, l_42, l_43, l_44, l_45, l_46, l_47, l_48, l_49, l_50, l_51, l_52, l_53,
l_54, l_55, l_56, l_57, l_58, l_59, l_60, l_61, l_62, l_63, l_64, l_65, l_66,
l_67, l_68, l_69, l_70, l_71, l_72, l_73, l_74, l_75, l_76, l_77, l_78, l_79,
l_80, l_81, l_82, l_83, l_84, l_85, l_86, l_87, l_88, l_89, l_90, l_91, l_92,
l_93, l_94, l_95, l_96, l_97, l_98, l_99, l_100, l_101, l_102, l_103, l_104,
l_105, l_106, l_107, l_108, l_109, l_110, l_111, l_112, l_113, l_114, l_115,
l_116, l_117, l_118, l_119, l_120, l_121, l_122, l_123, l_124, l_125, l_126,
l_127, l_128, l_129, l_130, l_131, l_132, l_133, l_134, l_135, l_136, l_137,
l_138, l_139, l_140, l_141, l_142, l_143, l_144, l_145, l_146, l_147, l_148,
l_149, l_150, l_151, l_152, l_153, l_154, l_155, l_156, l_157, l_158, l_159,
l_160, l_161, l_162, l_163, l_164, l_165, l_166, l_167, l_168, l_169, l_170,
l_171, l_172, l_173, l_174, l_175, l_176, l_177, l_178, l_179, l_180, l_181,
l_182, l_183, l_184, l_185, l_186, l_187, l_188, l_189, l_190, l_191, l_192,
l_193, l_194, l_195, l_196, l_197, l_198, l_199, l_200, l_201, l_202, l_203,
l_204, l_205, l_206, l_207, l_208, l_209, l_210, l_211, l_212, l_213, l_214,
l_215, l_216, l_217, l_218, l_219, l_220, l_221, l_222, l_223, l_224, l_225,
l_226, l_227, l_228, l_229, l_230, l_231, l_232, l_233, l_234, l_235, l_236,
l_237, l_238, l_239, l_240, l_241, l_242, l_243, l_244, l_245, l_246, l_247,
l_248, l_249, l_250, l_251, l_252, l_253, l_254, l_255;

    static const void* const a_jump_table[256] =
	  { &&
l_0, && l_1, && l_2, && l_3, && l_4, && l_5, && l_6, && l_7, && l_8, && l_9, && l_10, && l_11, && l_12, && l_13, && l_14, &&
l_15, && l_16, && l_17, && l_18, && l_19, && l_20, && l_21, && l_22, && l_23, && l_24, && l_25, && l_26, && l_27, &&
l_28, && l_29, && l_30, && l_31, && l_32, && l_33, && l_34, && l_35, && l_36, && l_37, && l_38, && l_39, && l_40, &&
l_41, && l_42, && l_43, && l_44, && l_45, && l_46, && l_47, && l_48, && l_49, && l_50, && l_51, && l_52, && l_53, &&
l_54, && l_55, && l_56, && l_57, && l_58, && l_59, && l_60, && l_61, && l_62, && l_63, && l_64, && l_65, && l_66, &&
l_67, && l_68, && l_69, && l_70, && l_71, && l_72, && l_73, && l_74, && l_75, && l_76, && l_77, && l_78, && l_79, &&
l_80, && l_81, && l_82, && l_83, && l_84, && l_85, && l_86, && l_87, && l_88, && l_89, && l_90, && l_91, && l_92, &&
l_93, && l_94, && l_95, && l_96, && l_97, && l_98, && l_99, && l_100, && l_101, && l_102, && l_103, && l_104, &&
l_105, && l_106, && l_107, && l_108, && l_109, && l_110, && l_111, && l_112, && l_113, && l_114, && l_115, &&
l_116, && l_117, && l_118, && l_119, && l_120, && l_121, && l_122, && l_123, && l_124, && l_125, && l_126, &&
l_127, && l_128, && l_129, && l_130, && l_131, && l_132, && l_133, && l_134, && l_135, && l_136, && l_137, &&
l_138, && l_139, && l_140, && l_141, && l_142, && l_143, && l_144, && l_145, && l_146, && l_147, && l_148, &&
l_149, && l_150, && l_151, && l_152, && l_153, && l_154, && l_155, && l_156, && l_157, && l_158, && l_159, &&
l_160, && l_161, && l_162, && l_163, && l_164, && l_165, && l_166, && l_167, && l_168, && l_169, && l_170, &&
l_171, && l_172, && l_173, && l_174, && l_175, && l_176, && l_177, && l_178, && l_179, && l_180, && l_181, &&
l_182, && l_183, && l_184, && l_185, && l_186, && l_187, && l_188, && l_189, && l_190, && l_191, && l_192, &&
l_193, && l_194, && l_195, && l_196, && l_197, && l_198, && l_199, && l_200, && l_201, && l_202, && l_203, &&
l_204, && l_205, && l_206, && l_207, && l_208, && l_209, && l_210, && l_211, && l_212, && l_213, && l_214, &&
l_215, && l_216, && l_217, && l_218, && l_219, && l_220, && l_221, && l_222, && l_223, && l_224, && l_225, &&
l_226, && l_227, && l_228, && l_229, && l_230, && l_231, && l_232, && l_233, && l_234, && l_235, && l_236, &&
l_237, && l_238, && l_239, && l_240, && l_241, && l_242, && l_243, && l_244, && l_245, && l_246, && l_247, &&
l_248, && l_249, && l_250, && l_251, && l_252, && l_253, && l_254, && l_255 };

	 goto *a_jump_table[aCode];

  //switch(aCode){
  l_0:no_op();return;
  l_1:no_op();return;
  l_2:no_op();return;
  l_3:no_op();return;
  l_4:no_op();return;
  l_5:no_op();return;
  l_6:no_op();return;
  l_7:no_op();return;
  l_8:no_op();return;
  l_9:add_ix_bc();return;
  l_10:no_op();return;
  l_11:no_op();return;
  l_12:no_op();return;
  l_13:no_op();return;
  l_14:no_op();return;
  l_15:no_op();return;
  l_16:no_op();return;
  l_17:no_op();return;
  l_18:no_op();return;
  l_19:no_op();return;
  l_20:no_op();return;
  l_21:no_op();return;
  l_22:no_op();return;
  l_23:no_op();return;
  l_24:no_op();return;
  l_25:add_ix_de();return;
  l_26:no_op();return;
  l_27:no_op();return;
  l_28:no_op();return;
  l_29:no_op();return;
  l_30:no_op();return;
  l_31:no_op();return;
  l_32:no_op();return;
  l_33:ld_ix_uint16_t();return;
  l_34:ld_xuint16_t_ix();return;
  l_35:inc_ix();return;
  l_36:inc_ixh();return;
  l_37:dec_ixh();return;
  l_38:ld_ixh_uint8_t();return;
  l_39:no_op();return;
  l_40:no_op();return;
  l_41:add_ix_ix();return;
  l_42:ld_ix_xuint16_t();return;
  l_43:dec_ix();return;
  l_44:inc_ixl();return;
  l_45:dec_ixl();return;
  l_46:ld_ixl_uint8_t();return;
  l_47:no_op();return;
  l_48:no_op();return;
  l_49:no_op();return;
  l_50:no_op();return;
  l_51:no_op();return;
  l_52:inc_xix();return;
  l_53:dec_xix();return;
  l_54:ld_xix_uint8_t();return;
  l_55:no_op();return;
  l_56:no_op();return;
  l_57:add_ix_sp();return;
  l_58:no_op();return;
  l_59:no_op();return;
  l_60:no_op();return;
  l_61:no_op();return;
  l_62:no_op();return;
  l_63:no_op();return;
  l_64:no_op();return;
  l_65:no_op();return;
  l_66:no_op();return;
  l_67:no_op();return;
  l_68:ld_b_ixh();return;
  l_69:ld_b_ixl();return;
  l_70:ld_b_xix();return;
  l_71:no_op();return;
  l_72:no_op();return;
  l_73:no_op();return;
  l_74:no_op();return;
  l_75:no_op();return;
  l_76:ld_c_ixh();return;
  l_77:ld_c_ixl();return;
  l_78:ld_c_xix();return;
  l_79:no_op();return;
  l_80:no_op();return;
  l_81:no_op();return;
  l_82:no_op();return;
  l_83:no_op();return;
  l_84:ld_d_ixh();return;
  l_85:ld_d_ixl();return;
  l_86:ld_d_xix();return;
  l_87:no_op();return;
  l_88:no_op();return;
  l_89:no_op();return;
  l_90:no_op();return;
  l_91:no_op();return;
  l_92:ld_e_ixh();return;
  l_93:ld_e_ixl();return;
  l_94:ld_e_xix();return;
  l_95:no_op();return;
  l_96:ld_ixh_b();return;
  l_97:ld_ixh_c();return;
  l_98:ld_ixh_d();return;
  l_99:ld_ixh_e();return;
  l_100:ld_ixh_ixh();return;
  l_101:ld_ixh_ixl();return;
  l_102:ld_h_xix();return;
  l_103:ld_ixh_a();return;
  l_104:ld_ixl_b();return;
  l_105:ld_ixl_c();return;
  l_106:ld_ixl_d();return;
  l_107:ld_ixl_e();return;
  l_108:ld_ixl_ixh();return;
  l_109:ld_ixl_ixl();return;
  l_110:ld_l_xix();return;
  l_111:ld_ixl_a();return;
  l_112:ld_xix_b();return;
  l_113:ld_xix_c();return;
  l_114:ld_xix_d();return;
  l_115:ld_xix_e();return;
  l_116:ld_xix_h();return;
  l_117:ld_xix_l();return;
  l_118:no_op();return;
  l_119:ld_xix_a();return;
  l_120:no_op();return;
  l_121:no_op();return;
  l_122:no_op();return;
  l_123:no_op();return;
  l_124:ld_a_ixh();return;
  l_125:ld_a_ixl();return;
  l_126:ld_a_xix();return;
  l_127:no_op();return;
  l_128:no_op();return;
  l_129:no_op();return;
  l_130:no_op();return;
  l_131:no_op();return;
  l_132:add_a_ixh();return;
  l_133:add_a_ixl();return;
  l_134:add_a_xix();return;
  l_135:no_op();return;
  l_136:no_op();return;
  l_137:no_op();return;
  l_138:no_op();return;
  l_139:no_op();return;
  l_140:adc_a_ixh();return;
  l_141:adc_a_ixl();return;
  l_142:adc_a_xix();return;
  l_143:no_op();return;
  l_144:no_op();return;
  l_145:no_op();return;
  l_146:no_op();return;
  l_147:no_op();return;
  l_148:sub_ixh();return;
  l_149:sub_ixl();return;
  l_150:sub_xix();return;
  l_151:no_op();return;
  l_152:no_op();return;
  l_153:no_op();return;
  l_154:no_op();return;
  l_155:no_op();return;
  l_156:sbc_a_ixh();return;
  l_157:sbc_a_ixl();return;
  l_158:sbc_a_xix();return;
  l_159:no_op();return;
  l_160:no_op();return;
  l_161:no_op();return;
  l_162:no_op();return;
  l_163:no_op();return;
  l_164:and_ixh();return;
  l_165:and_ixl();return;
  l_166:and_xix();return;
  l_167:no_op();return;
  l_168:no_op();return;
  l_169:no_op();return;
  l_170:no_op();return;
  l_171:no_op();return;
  l_172:xor_ixh();return;
  l_173:xor_ixl();return;
  l_174:xor_xix();return;
  l_175:no_op();return;
  l_176:no_op();return;
  l_177:no_op();return;
  l_178:no_op();return;
  l_179:no_op();return;
  l_180:or_ixh();return;
  l_181:or_ixl();return;
  l_182:or_xix();return;
  l_183:no_op();return;
  l_184:no_op();return;
  l_185:no_op();return;
  l_186:no_op();return;
  l_187:no_op();return;
  l_188:cp_ixh();return;
  l_189:cp_ixl();return;
  l_190:cp_xix();return;
  l_191:no_op();return;
  l_192:no_op();return;
  l_193:no_op();return;
  l_194:no_op();return;
  l_195:no_op();return;
  l_196:no_op();return;
  l_197:no_op();return;
  l_198:no_op();return;
  l_199:no_op();return;
  l_200:no_op();return;
  l_201:no_op();return;
  l_202:no_op();return;
  l_203:dd_cb();return;
  l_204:no_op();return;
  l_205:no_op();return;
  l_206:no_op();return;
  l_207:no_op();return;
  l_208:no_op();return;
  l_209:no_op();return;
  l_210:no_op();return;
  l_211:no_op();return;
  l_212:no_op();return;
  l_213:no_op();return;
  l_214:no_op();return;
  l_215:no_op();return;
  l_216:no_op();return;
  l_217:no_op();return;
  l_218:no_op();return;
  l_219:no_op();return;
  l_220:no_op();return;
  l_221:no_op();return;
  l_222:no_op();return;
  l_223:no_op();return;
  l_224:no_op();return;
  l_225:pop_ix();return;
  l_226:no_op();return;
  l_227:ex_xsp_ix();return;
  l_228:no_op();return;
  l_229:push_ix();return;
  l_230:no_op();return;
  l_231:no_op();return;
  l_232:no_op();return;
  l_233:jp_ix();return;
  l_234:no_op();return;
  l_235:no_op();return;
  l_236:no_op();return;
  l_237:no_op();return;
  l_238:no_op();return;
  l_239:no_op();return;
  l_240:no_op();return;
  l_241:no_op();return;
  l_242:no_op();return;
  l_243:no_op();return;
  l_244:no_op();return;
  l_245:no_op();return;
  l_246:no_op();return;
  l_247:no_op();return;
  l_248:no_op();return;
  l_249:ld_sp_ix();return;
  l_250:no_op();return;
  l_251:no_op();return;
  l_252:no_op();return;
  l_253:no_op();return;
  l_254:no_op();return;
  l_255:no_op();return;
  //}
}
void Z80::call_opcode_ed(uint8_t aCode)
{
  __label__
l_0, l_1, l_2, l_3, l_4, l_5, l_6, l_7, l_8, l_9, l_10, l_11, l_12, l_13, l_14,
l_15, l_16, l_17, l_18, l_19, l_20, l_21, l_22, l_23, l_24, l_25, l_26, l_27,
l_28, l_29, l_30, l_31, l_32, l_33, l_34, l_35, l_36, l_37, l_38, l_39, l_40,
l_41, l_42, l_43, l_44, l_45, l_46, l_47, l_48, l_49, l_50, l_51, l_52, l_53,
l_54, l_55, l_56, l_57, l_58, l_59, l_60, l_61, l_62, l_63, l_64, l_65, l_66,
l_67, l_68, l_69, l_70, l_71, l_72, l_73, l_74, l_75, l_76, l_77, l_78, l_79,
l_80, l_81, l_82, l_83, l_84, l_85, l_86, l_87, l_88, l_89, l_90, l_91, l_92,
l_93, l_94, l_95, l_96, l_97, l_98, l_99, l_100, l_101, l_102, l_103, l_104,
l_105, l_106, l_107, l_108, l_109, l_110, l_111, l_112, l_113, l_114, l_115,
l_116, l_117, l_118, l_119, l_120, l_121, l_122, l_123, l_124, l_125, l_126,
l_127, l_128, l_129, l_130, l_131, l_132, l_133, l_134, l_135, l_136, l_137,
l_138, l_139, l_140, l_141, l_142, l_143, l_144, l_145, l_146, l_147, l_148,
l_149, l_150, l_151, l_152, l_153, l_154, l_155, l_156, l_157, l_158, l_159,
l_160, l_161, l_162, l_163, l_164, l_165, l_166, l_167, l_168, l_169, l_170,
l_171, l_172, l_173, l_174, l_175, l_176, l_177, l_178, l_179, l_180, l_181,
l_182, l_183, l_184, l_185, l_186, l_187, l_188, l_189, l_190, l_191, l_192,
l_193, l_194, l_195, l_196, l_197, l_198, l_199, l_200, l_201, l_202, l_203,
l_204, l_205, l_206, l_207, l_208, l_209, l_210, l_211, l_212, l_213, l_214,
l_215, l_216, l_217, l_218, l_219, l_220, l_221, l_222, l_223, l_224, l_225,
l_226, l_227, l_228, l_229, l_230, l_231, l_232, l_233, l_234, l_235, l_236,
l_237, l_238, l_239, l_240, l_241, l_242, l_243, l_244, l_245, l_246, l_247,
l_248, l_249, l_250, l_251, l_252, l_253, l_254, l_255;

    static const void* const a_jump_table[256] =
	  { &&
l_0, && l_1, && l_2, && l_3, && l_4, && l_5, && l_6, && l_7, && l_8, && l_9, && l_10, && l_11, && l_12, && l_13, && l_14, &&
l_15, && l_16, && l_17, && l_18, && l_19, && l_20, && l_21, && l_22, && l_23, && l_24, && l_25, && l_26, && l_27, &&
l_28, && l_29, && l_30, && l_31, && l_32, && l_33, && l_34, && l_35, && l_36, && l_37, && l_38, && l_39, && l_40, &&
l_41, && l_42, && l_43, && l_44, && l_45, && l_46, && l_47, && l_48, && l_49, && l_50, && l_51, && l_52, && l_53, &&
l_54, && l_55, && l_56, && l_57, && l_58, && l_59, && l_60, && l_61, && l_62, && l_63, && l_64, && l_65, && l_66, &&
l_67, && l_68, && l_69, && l_70, && l_71, && l_72, && l_73, && l_74, && l_75, && l_76, && l_77, && l_78, && l_79, &&
l_80, && l_81, && l_82, && l_83, && l_84, && l_85, && l_86, && l_87, && l_88, && l_89, && l_90, && l_91, && l_92, &&
l_93, && l_94, && l_95, && l_96, && l_97, && l_98, && l_99, && l_100, && l_101, && l_102, && l_103, && l_104, &&
l_105, && l_106, && l_107, && l_108, && l_109, && l_110, && l_111, && l_112, && l_113, && l_114, && l_115, &&
l_116, && l_117, && l_118, && l_119, && l_120, && l_121, && l_122, && l_123, && l_124, && l_125, && l_126, &&
l_127, && l_128, && l_129, && l_130, && l_131, && l_132, && l_133, && l_134, && l_135, && l_136, && l_137, &&
l_138, && l_139, && l_140, && l_141, && l_142, && l_143, && l_144, && l_145, && l_146, && l_147, && l_148, &&
l_149, && l_150, && l_151, && l_152, && l_153, && l_154, && l_155, && l_156, && l_157, && l_158, && l_159, &&
l_160, && l_161, && l_162, && l_163, && l_164, && l_165, && l_166, && l_167, && l_168, && l_169, && l_170, &&
l_171, && l_172, && l_173, && l_174, && l_175, && l_176, && l_177, && l_178, && l_179, && l_180, && l_181, &&
l_182, && l_183, && l_184, && l_185, && l_186, && l_187, && l_188, && l_189, && l_190, && l_191, && l_192, &&
l_193, && l_194, && l_195, && l_196, && l_197, && l_198, && l_199, && l_200, && l_201, && l_202, && l_203, &&
l_204, && l_205, && l_206, && l_207, && l_208, && l_209, && l_210, && l_211, && l_212, && l_213, && l_214, &&
l_215, && l_216, && l_217, && l_218, && l_219, && l_220, && l_221, && l_222, && l_223, && l_224, && l_225, &&
l_226, && l_227, && l_228, && l_229, && l_230, && l_231, && l_232, && l_233, && l_234, && l_235, && l_236, &&
l_237, && l_238, && l_239, && l_240, && l_241, && l_242, && l_243, && l_244, && l_245, && l_246, && l_247, &&
l_248, && l_249, && l_250, && l_251, && l_252, && l_253, && l_254, && l_255 };

	 goto *a_jump_table[aCode];

  //switch(aCode){
  l_0:nop();return;
  l_1:nop();return;
  l_2:nop();return;
  l_3:nop();return;
  l_4:nop();return;
  l_5:nop();return;
  l_6:nop();return;
  l_7:nop();return;
  l_8:nop();return;
  l_9:nop();return;
  l_10:nop();return;
  l_11:nop();return;
  l_12:nop();return;
  l_13:nop();return;
  l_14:nop();return;
  l_15:nop();return;
  l_16:nop();return;
  l_17:nop();return;
  l_18:nop();return;
  l_19:nop();return;
  l_20:nop();return;
  l_21:nop();return;
  l_22:nop();return;
  l_23:nop();return;
  l_24:nop();return;
  l_25:nop();return;
  l_26:nop();return;
  l_27:nop();return;
  l_28:nop();return;
  l_29:nop();return;
  l_30:nop();return;
  l_31:nop();return;
  l_32:nop();return;
  l_33:nop();return;
  l_34:nop();return;
  l_35:nop();return;
  l_36:nop();return;
  l_37:nop();return;
  l_38:nop();return;
  l_39:nop();return;
  l_40:nop();return;
  l_41:nop();return;
  l_42:nop();return;
  l_43:nop();return;
  l_44:nop();return;
  l_45:nop();return;
  l_46:nop();return;
  l_47:nop();return;
  l_48:nop();return;
  l_49:nop();return;
  l_50:nop();return;
  l_51:nop();return;
  l_52:nop();return;
  l_53:nop();return;
  l_54:nop();return;
  l_55:nop();return;
  l_56:nop();return;
  l_57:nop();return;
  l_58:nop();return;
  l_59:nop();return;
  l_60:nop();return;
  l_61:nop();return;
  l_62:nop();return;
  l_63:nop();return;
  l_64:in_b_c();return;
  l_65:out_c_b();return;
  l_66:sbc_hl_bc();return;
  l_67:ld_xuint16_t_bc();return;
  l_68:neg();return;
  l_69:retn();return;
  l_70:im_0();return;
  l_71:ld_i_a();return;
  l_72:in_c_c();return;
  l_73:out_c_c();return;
  l_74:adc_hl_bc();return;
  l_75:ld_bc_xuint16_t();return;
  l_76:neg();return;
  l_77:reti();return;
  l_78:im_0();return;
  l_79:ld_r_a();return;
  l_80:in_d_c();return;
  l_81:out_c_d();return;
  l_82:sbc_hl_de();return;
  l_83:ld_xuint16_t_de();return;
  l_84:neg();return;
  l_85:retn();return;
  l_86:im_1();return;
  l_87:ld_a_i();return;
  l_88:in_e_c();return;
  l_89:out_c_e();return;
  l_90:adc_hl_de();return;
  l_91:ld_de_xuint16_t();return;
  l_92:neg();return;
  l_93:reti();return;
  l_94:im_2();return;
  l_95:ld_a_r();return;
  l_96:in_h_c();return;
  l_97:out_c_h();return;
  l_98:sbc_hl_hl();return;
  l_99:ld_xuint16_t_hl();return;
  l_100:neg();return;
  l_101:retn();return;
  l_102:im_0();return;
  l_103:rrd();return;
  l_104:in_l_c();return;
  l_105:out_c_l();return;
  l_106:adc_hl_hl();return;
  l_107:ld_hl_xuint16_t();return;
  l_108:neg();return;
  l_109:reti();return;
  l_110:im_0();return;
  l_111:rld();return;
  l_112:in_0_c();return;
  l_113:out_c_0();return;
  l_114:sbc_hl_sp();return;
  l_115:ld_xuint16_t_sp();return;
  l_116:neg();return;
  l_117:retn();return;
  l_118:im_1();return;
  l_119:nop();return;
  l_120:in_a_c();return;
  l_121:out_c_a();return;
  l_122:adc_hl_sp();return;
  l_123:ld_sp_xuint16_t();return;
  l_124:neg();return;
  l_125:reti();return;
  l_126:im_2();return;
  l_127:nop();return;
  l_128:nop();return;
  l_129:nop();return;
  l_130:nop();return;
  l_131:nop();return;
  l_132:nop();return;
  l_133:nop();return;
  l_134:nop();return;
  l_135:nop();return;
  l_136:nop();return;
  l_137:nop();return;
  l_138:nop();return;
  l_139:nop();return;
  l_140:nop();return;
  l_141:nop();return;
  l_142:nop();return;
  l_143:nop();return;
  l_144:nop();return;
  l_145:nop();return;
  l_146:nop();return;
  l_147:nop();return;
  l_148:nop();return;
  l_149:nop();return;
  l_150:nop();return;
  l_151:nop();return;
  l_152:nop();return;
  l_153:nop();return;
  l_154:nop();return;
  l_155:nop();return;
  l_156:nop();return;
  l_157:nop();return;
  l_158:nop();return;
  l_159:nop();return;
  l_160:ldi();return;
  l_161:cpi();return;
  l_162:ini();return;
  l_163:outi();return;
  l_164:nop();return;
  l_165:nop();return;
  l_166:nop();return;
  l_167:nop();return;
  l_168:ldd();return;
  l_169:cpd();return;
  l_170:ind();return;
  l_171:outd();return;
  l_172:nop();return;
  l_173:nop();return;
  l_174:nop();return;
  l_175:nop();return;
  l_176:ldir();return;
  l_177:cpir();return;
  l_178:inir();return;
  l_179:otir();return;
  l_180:nop();return;
  l_181:nop();return;
  l_182:nop();return;
  l_183:nop();return;
  l_184:lddr();return;
  l_185:cpdr();return;
  l_186:indr();return;
  l_187:otdr();return;
  l_188:nop();return;
  l_189:nop();return;
  l_190:nop();return;
  l_191:nop();return;
  l_192:nop();return;
  l_193:nop();return;
  l_194:nop();return;
  l_195:nop();return;
  l_196:nop();return;
  l_197:nop();return;
  l_198:nop();return;
  l_199:nop();return;
  l_200:nop();return;
  l_201:nop();return;
  l_202:nop();return;
  l_203:nop();return;
  l_204:nop();return;
  l_205:nop();return;
  l_206:nop();return;
  l_207:nop();return;
  l_208:nop();return;
  l_209:nop();return;
  l_210:nop();return;
  l_211:nop();return;
  l_212:nop();return;
  l_213:nop();return;
  l_214:nop();return;
  l_215:nop();return;
  l_216:nop();return;
  l_217:nop();return;
  l_218:nop();return;
  l_219:nop();return;
  l_220:nop();return;
  l_221:nop();return;
  l_222:nop();return;
  l_223:nop();return;
  l_224:nop();return;
  l_225:nop();return;
  l_226:nop();return;
  l_227:nop();return;
  l_228:nop();return;
  l_229:nop();return;
  l_230:nop();return;
  l_231:nop();return;
  l_232:nop();return;
  l_233:nop();return;
  l_234:nop();return;
  l_235:nop();return;
  l_236:nop();return;
  l_237:nop();return;
  l_238:nop();return;
  l_239:nop();return;
  l_240:nop();return;
  l_241:nop();return;
  l_242:nop();return;
  l_243:nop();return;
  l_244:nop();return;
  l_245:nop();return;
  l_246:nop();return;
  l_247:nop();return;
  l_248:nop();return;
  l_249:nop();return;
  l_250:nop();return;
  l_251:nop();return;
  l_252:nop();return;
  l_253:nop();return;
  l_254:patch();return;
  l_255:nop();return;
  //}
}
void Z80::call_opcode_fd(uint8_t aCode)
{
  __label__
l_0, l_1, l_2, l_3, l_4, l_5, l_6, l_7, l_8, l_9, l_10, l_11, l_12, l_13, l_14,
l_15, l_16, l_17, l_18, l_19, l_20, l_21, l_22, l_23, l_24, l_25, l_26, l_27,
l_28, l_29, l_30, l_31, l_32, l_33, l_34, l_35, l_36, l_37, l_38, l_39, l_40,
l_41, l_42, l_43, l_44, l_45, l_46, l_47, l_48, l_49, l_50, l_51, l_52, l_53,
l_54, l_55, l_56, l_57, l_58, l_59, l_60, l_61, l_62, l_63, l_64, l_65, l_66,
l_67, l_68, l_69, l_70, l_71, l_72, l_73, l_74, l_75, l_76, l_77, l_78, l_79,
l_80, l_81, l_82, l_83, l_84, l_85, l_86, l_87, l_88, l_89, l_90, l_91, l_92,
l_93, l_94, l_95, l_96, l_97, l_98, l_99, l_100, l_101, l_102, l_103, l_104,
l_105, l_106, l_107, l_108, l_109, l_110, l_111, l_112, l_113, l_114, l_115,
l_116, l_117, l_118, l_119, l_120, l_121, l_122, l_123, l_124, l_125, l_126,
l_127, l_128, l_129, l_130, l_131, l_132, l_133, l_134, l_135, l_136, l_137,
l_138, l_139, l_140, l_141, l_142, l_143, l_144, l_145, l_146, l_147, l_148,
l_149, l_150, l_151, l_152, l_153, l_154, l_155, l_156, l_157, l_158, l_159,
l_160, l_161, l_162, l_163, l_164, l_165, l_166, l_167, l_168, l_169, l_170,
l_171, l_172, l_173, l_174, l_175, l_176, l_177, l_178, l_179, l_180, l_181,
l_182, l_183, l_184, l_185, l_186, l_187, l_188, l_189, l_190, l_191, l_192,
l_193, l_194, l_195, l_196, l_197, l_198, l_199, l_200, l_201, l_202, l_203,
l_204, l_205, l_206, l_207, l_208, l_209, l_210, l_211, l_212, l_213, l_214,
l_215, l_216, l_217, l_218, l_219, l_220, l_221, l_222, l_223, l_224, l_225,
l_226, l_227, l_228, l_229, l_230, l_231, l_232, l_233, l_234, l_235, l_236,
l_237, l_238, l_239, l_240, l_241, l_242, l_243, l_244, l_245, l_246, l_247,
l_248, l_249, l_250, l_251, l_252, l_253, l_254, l_255;

    static const void* const a_jump_table[256] =
	  { &&
l_0, && l_1, && l_2, && l_3, && l_4, && l_5, && l_6, && l_7, && l_8, && l_9, && l_10, && l_11, && l_12, && l_13, && l_14, &&
l_15, && l_16, && l_17, && l_18, && l_19, && l_20, && l_21, && l_22, && l_23, && l_24, && l_25, && l_26, && l_27, &&
l_28, && l_29, && l_30, && l_31, && l_32, && l_33, && l_34, && l_35, && l_36, && l_37, && l_38, && l_39, && l_40, &&
l_41, && l_42, && l_43, && l_44, && l_45, && l_46, && l_47, && l_48, && l_49, && l_50, && l_51, && l_52, && l_53, &&
l_54, && l_55, && l_56, && l_57, && l_58, && l_59, && l_60, && l_61, && l_62, && l_63, && l_64, && l_65, && l_66, &&
l_67, && l_68, && l_69, && l_70, && l_71, && l_72, && l_73, && l_74, && l_75, && l_76, && l_77, && l_78, && l_79, &&
l_80, && l_81, && l_82, && l_83, && l_84, && l_85, && l_86, && l_87, && l_88, && l_89, && l_90, && l_91, && l_92, &&
l_93, && l_94, && l_95, && l_96, && l_97, && l_98, && l_99, && l_100, && l_101, && l_102, && l_103, && l_104, &&
l_105, && l_106, && l_107, && l_108, && l_109, && l_110, && l_111, && l_112, && l_113, && l_114, && l_115, &&
l_116, && l_117, && l_118, && l_119, && l_120, && l_121, && l_122, && l_123, && l_124, && l_125, && l_126, &&
l_127, && l_128, && l_129, && l_130, && l_131, && l_132, && l_133, && l_134, && l_135, && l_136, && l_137, &&
l_138, && l_139, && l_140, && l_141, && l_142, && l_143, && l_144, && l_145, && l_146, && l_147, && l_148, &&
l_149, && l_150, && l_151, && l_152, && l_153, && l_154, && l_155, && l_156, && l_157, && l_158, && l_159, &&
l_160, && l_161, && l_162, && l_163, && l_164, && l_165, && l_166, && l_167, && l_168, && l_169, && l_170, &&
l_171, && l_172, && l_173, && l_174, && l_175, && l_176, && l_177, && l_178, && l_179, && l_180, && l_181, &&
l_182, && l_183, && l_184, && l_185, && l_186, && l_187, && l_188, && l_189, && l_190, && l_191, && l_192, &&
l_193, && l_194, && l_195, && l_196, && l_197, && l_198, && l_199, && l_200, && l_201, && l_202, && l_203, &&
l_204, && l_205, && l_206, && l_207, && l_208, && l_209, && l_210, && l_211, && l_212, && l_213, && l_214, &&
l_215, && l_216, && l_217, && l_218, && l_219, && l_220, && l_221, && l_222, && l_223, && l_224, && l_225, &&
l_226, && l_227, && l_228, && l_229, && l_230, && l_231, && l_232, && l_233, && l_234, && l_235, && l_236, &&
l_237, && l_238, && l_239, && l_240, && l_241, && l_242, && l_243, && l_244, && l_245, && l_246, && l_247, &&
l_248, && l_249, && l_250, && l_251, && l_252, && l_253, && l_254, && l_255 };

	 goto *a_jump_table[aCode];

  //switch(aCode){
  l_0:no_op();return;
  l_1:no_op();return;
  l_2:no_op();return;
  l_3:no_op();return;
  l_4:no_op();return;
  l_5:no_op();return;
  l_6:no_op();return;
  l_7:no_op();return;
  l_8:no_op();return;
  l_9:add_iy_bc();return;
  l_10:no_op();return;
  l_11:no_op();return;
  l_12:no_op();return;
  l_13:no_op();return;
  l_14:no_op();return;
  l_15:no_op();return;
  l_16:no_op();return;
  l_17:no_op();return;
  l_18:no_op();return;
  l_19:no_op();return;
  l_20:no_op();return;
  l_21:no_op();return;
  l_22:no_op();return;
  l_23:no_op();return;
  l_24:no_op();return;
  l_25:add_iy_de();return;
  l_26:no_op();return;
  l_27:no_op();return;
  l_28:no_op();return;
  l_29:no_op();return;
  l_30:no_op();return;
  l_31:no_op();return;
  l_32:no_op();return;
  l_33:ld_iy_uint16_t();return;
  l_34:ld_xuint16_t_iy();return;
  l_35:inc_iy();return;
  l_36:inc_iyh();return;
  l_37:dec_iyh();return;
  l_38:ld_iyh_uint8_t();return;
  l_39:no_op();return;
  l_40:no_op();return;
  l_41:add_iy_iy();return;
  l_42:ld_iy_xuint16_t();return;
  l_43:dec_iy();return;
  l_44:inc_iyl();return;
  l_45:dec_iyl();return;
  l_46:ld_iyl_uint8_t();return;
  l_47:no_op();return;
  l_48:no_op();return;
  l_49:no_op();return;
  l_50:no_op();return;
  l_51:no_op();return;
  l_52:inc_xiy();return;
  l_53:dec_xiy();return;
  l_54:ld_xiy_uint8_t();return;
  l_55:no_op();return;
  l_56:no_op();return;
  l_57:add_iy_sp();return;
  l_58:no_op();return;
  l_59:no_op();return;
  l_60:no_op();return;
  l_61:no_op();return;
  l_62:no_op();return;
  l_63:no_op();return;
  l_64:no_op();return;
  l_65:no_op();return;
  l_66:no_op();return;
  l_67:no_op();return;
  l_68:ld_b_iyh();return;
  l_69:ld_b_iyl();return;
  l_70:ld_b_xiy();return;
  l_71:no_op();return;
  l_72:no_op();return;
  l_73:no_op();return;
  l_74:no_op();return;
  l_75:no_op();return;
  l_76:ld_c_iyh();return;
  l_77:ld_c_iyl();return;
  l_78:ld_c_xiy();return;
  l_79:no_op();return;
  l_80:no_op();return;
  l_81:no_op();return;
  l_82:no_op();return;
  l_83:no_op();return;
  l_84:ld_d_iyh();return;
  l_85:ld_d_iyl();return;
  l_86:ld_d_xiy();return;
  l_87:no_op();return;
  l_88:no_op();return;
  l_89:no_op();return;
  l_90:no_op();return;
  l_91:no_op();return;
  l_92:ld_e_iyh();return;
  l_93:ld_e_iyl();return;
  l_94:ld_e_xiy();return;
  l_95:no_op();return;
  l_96:ld_iyh_b();return;
  l_97:ld_iyh_c();return;
  l_98:ld_iyh_d();return;
  l_99:ld_iyh_e();return;
  l_100:ld_iyh_iyh();return;
  l_101:ld_iyh_iyl();return;
  l_102:ld_h_xiy();return;
  l_103:ld_iyh_a();return;
  l_104:ld_iyl_b();return;
  l_105:ld_iyl_c();return;
  l_106:ld_iyl_d();return;
  l_107:ld_iyl_e();return;
  l_108:ld_iyl_iyh();return;
  l_109:ld_iyl_iyl();return;
  l_110:ld_l_xiy();return;
  l_111:ld_iyl_a();return;
  l_112:ld_xiy_b();return;
  l_113:ld_xiy_c();return;
  l_114:ld_xiy_d();return;
  l_115:ld_xiy_e();return;
  l_116:ld_xiy_h();return;
  l_117:ld_xiy_l();return;
  l_118:no_op();return;
  l_119:ld_xiy_a();return;
  l_120:no_op();return;
  l_121:no_op();return;
  l_122:no_op();return;
  l_123:no_op();return;
  l_124:ld_a_iyh();return;
  l_125:ld_a_iyl();return;
  l_126:ld_a_xiy();return;
  l_127:no_op();return;
  l_128:no_op();return;
  l_129:no_op();return;
  l_130:no_op();return;
  l_131:no_op();return;
  l_132:add_a_iyh();return;
  l_133:add_a_iyl();return;
  l_134:add_a_xiy();return;
  l_135:no_op();return;
  l_136:no_op();return;
  l_137:no_op();return;
  l_138:no_op();return;
  l_139:no_op();return;
  l_140:adc_a_iyh();return;
  l_141:adc_a_iyl();return;
  l_142:adc_a_xiy();return;
  l_143:no_op();return;
  l_144:no_op();return;
  l_145:no_op();return;
  l_146:no_op();return;
  l_147:no_op();return;
  l_148:sub_iyh();return;
  l_149:sub_iyl();return;
  l_150:sub_xiy();return;
  l_151:no_op();return;
  l_152:no_op();return;
  l_153:no_op();return;
  l_154:no_op();return;
  l_155:no_op();return;
  l_156:sbc_a_iyh();return;
  l_157:sbc_a_iyl();return;
  l_158:sbc_a_xiy();return;
  l_159:no_op();return;
  l_160:no_op();return;
  l_161:no_op();return;
  l_162:no_op();return;
  l_163:no_op();return;
  l_164:and_iyh();return;
  l_165:and_iyl();return;
  l_166:and_xiy();return;
  l_167:no_op();return;
  l_168:no_op();return;
  l_169:no_op();return;
  l_170:no_op();return;
  l_171:no_op();return;
  l_172:xor_iyh();return;
  l_173:xor_iyl();return;
  l_174:xor_xiy();return;
  l_175:no_op();return;
  l_176:no_op();return;
  l_177:no_op();return;
  l_178:no_op();return;
  l_179:no_op();return;
  l_180:or_iyh();return;
  l_181:or_iyl();return;
  l_182:or_xiy();return;
  l_183:no_op();return;
  l_184:no_op();return;
  l_185:no_op();return;
  l_186:no_op();return;
  l_187:no_op();return;
  l_188:cp_iyh();return;
  l_189:cp_iyl();return;
  l_190:cp_xiy();return;
  l_191:no_op();return;
  l_192:no_op();return;
  l_193:no_op();return;
  l_194:no_op();return;
  l_195:no_op();return;
  l_196:no_op();return;
  l_197:no_op();return;
  l_198:no_op();return;
  l_199:no_op();return;
  l_200:no_op();return;
  l_201:no_op();return;
  l_202:no_op();return;
  l_203:fd_cb();return;
  l_204:no_op();return;
  l_205:no_op();return;
  l_206:no_op();return;
  l_207:no_op();return;
  l_208:no_op();return;
  l_209:no_op();return;
  l_210:no_op();return;
  l_211:no_op();return;
  l_212:no_op();return;
  l_213:no_op();return;
  l_214:no_op();return;
  l_215:no_op();return;
  l_216:no_op();return;
  l_217:no_op();return;
  l_218:no_op();return;
  l_219:no_op();return;
  l_220:no_op();return;
  l_221:no_op();return;
  l_222:no_op();return;
  l_223:no_op();return;
  l_224:no_op();return;
  l_225:pop_iy();return;
  l_226:no_op();return;
  l_227:ex_xsp_iy();return;
  l_228:no_op();return;
  l_229:push_iy();return;
  l_230:no_op();return;
  l_231:no_op();return;
  l_232:no_op();return;
  l_233:jp_iy();return;
  l_234:no_op();return;
  l_235:no_op();return;
  l_236:no_op();return;
  l_237:no_op();return;
  l_238:no_op();return;
  l_239:no_op();return;
  l_240:no_op();return;
  l_241:no_op();return;
  l_242:no_op();return;
  l_243:no_op();return;
  l_244:no_op();return;
  l_245:no_op();return;
  l_246:no_op();return;
  l_247:no_op();return;
  l_248:no_op();return;
  l_249:ld_sp_iy();return;
  l_250:no_op();return;
  l_251:no_op();return;
  l_252:no_op();return;
  l_253:no_op();return;
  l_254:no_op();return;
  l_255:no_op();return;
  //}
}
void Z80::call_opcode_main(uint8_t aCode)
{
  __label__
l_0, l_1, l_2, l_3, l_4, l_5, l_6, l_7, l_8, l_9, l_10, l_11, l_12, l_13, l_14,
l_15, l_16, l_17, l_18, l_19, l_20, l_21, l_22, l_23, l_24, l_25, l_26, l_27,
l_28, l_29, l_30, l_31, l_32, l_33, l_34, l_35, l_36, l_37, l_38, l_39, l_40,
l_41, l_42, l_43, l_44, l_45, l_46, l_47, l_48, l_49, l_50, l_51, l_52, l_53,
l_54, l_55, l_56, l_57, l_58, l_59, l_60, l_61, l_62, l_63, l_64, l_65, l_66,
l_67, l_68, l_69, l_70, l_71, l_72, l_73, l_74, l_75, l_76, l_77, l_78, l_79,
l_80, l_81, l_82, l_83, l_84, l_85, l_86, l_87, l_88, l_89, l_90, l_91, l_92,
l_93, l_94, l_95, l_96, l_97, l_98, l_99, l_100, l_101, l_102, l_103, l_104,
l_105, l_106, l_107, l_108, l_109, l_110, l_111, l_112, l_113, l_114, l_115,
l_116, l_117, l_118, l_119, l_120, l_121, l_122, l_123, l_124, l_125, l_126,
l_127, l_128, l_129, l_130, l_131, l_132, l_133, l_134, l_135, l_136, l_137,
l_138, l_139, l_140, l_141, l_142, l_143, l_144, l_145, l_146, l_147, l_148,
l_149, l_150, l_151, l_152, l_153, l_154, l_155, l_156, l_157, l_158, l_159,
l_160, l_161, l_162, l_163, l_164, l_165, l_166, l_167, l_168, l_169, l_170,
l_171, l_172, l_173, l_174, l_175, l_176, l_177, l_178, l_179, l_180, l_181,
l_182, l_183, l_184, l_185, l_186, l_187, l_188, l_189, l_190, l_191, l_192,
l_193, l_194, l_195, l_196, l_197, l_198, l_199, l_200, l_201, l_202, l_203,
l_204, l_205, l_206, l_207, l_208, l_209, l_210, l_211, l_212, l_213, l_214,
l_215, l_216, l_217, l_218, l_219, l_220, l_221, l_222, l_223, l_224, l_225,
l_226, l_227, l_228, l_229, l_230, l_231, l_232, l_233, l_234, l_235, l_236,
l_237, l_238, l_239, l_240, l_241, l_242, l_243, l_244, l_245, l_246, l_247,
l_248, l_249, l_250, l_251, l_252, l_253, l_254, l_255;

    static const void* const a_jump_table[256] =
	  { &&
l_0, && l_1, && l_2, && l_3, && l_4, && l_5, && l_6, && l_7, && l_8, && l_9, && l_10, && l_11, && l_12, && l_13, && l_14, &&
l_15, && l_16, && l_17, && l_18, && l_19, && l_20, && l_21, && l_22, && l_23, && l_24, && l_25, && l_26, && l_27, &&
l_28, && l_29, && l_30, && l_31, && l_32, && l_33, && l_34, && l_35, && l_36, && l_37, && l_38, && l_39, && l_40, &&
l_41, && l_42, && l_43, && l_44, && l_45, && l_46, && l_47, && l_48, && l_49, && l_50, && l_51, && l_52, && l_53, &&
l_54, && l_55, && l_56, && l_57, && l_58, && l_59, && l_60, && l_61, && l_62, && l_63, && l_64, && l_65, && l_66, &&
l_67, && l_68, && l_69, && l_70, && l_71, && l_72, && l_73, && l_74, && l_75, && l_76, && l_77, && l_78, && l_79, &&
l_80, && l_81, && l_82, && l_83, && l_84, && l_85, && l_86, && l_87, && l_88, && l_89, && l_90, && l_91, && l_92, &&
l_93, && l_94, && l_95, && l_96, && l_97, && l_98, && l_99, && l_100, && l_101, && l_102, && l_103, && l_104, &&
l_105, && l_106, && l_107, && l_108, && l_109, && l_110, && l_111, && l_112, && l_113, && l_114, && l_115, &&
l_116, && l_117, && l_118, && l_119, && l_120, && l_121, && l_122, && l_123, && l_124, && l_125, && l_126, &&
l_127, && l_128, && l_129, && l_130, && l_131, && l_132, && l_133, && l_134, && l_135, && l_136, && l_137, &&
l_138, && l_139, && l_140, && l_141, && l_142, && l_143, && l_144, && l_145, && l_146, && l_147, && l_148, &&
l_149, && l_150, && l_151, && l_152, && l_153, && l_154, && l_155, && l_156, && l_157, && l_158, && l_159, &&
l_160, && l_161, && l_162, && l_163, && l_164, && l_165, && l_166, && l_167, && l_168, && l_169, && l_170, &&
l_171, && l_172, && l_173, && l_174, && l_175, && l_176, && l_177, && l_178, && l_179, && l_180, && l_181, &&
l_182, && l_183, && l_184, && l_185, && l_186, && l_187, && l_188, && l_189, && l_190, && l_191, && l_192, &&
l_193, && l_194, && l_195, && l_196, && l_197, && l_198, && l_199, && l_200, && l_201, && l_202, && l_203, &&
l_204, && l_205, && l_206, && l_207, && l_208, && l_209, && l_210, && l_211, && l_212, && l_213, && l_214, &&
l_215, && l_216, && l_217, && l_218, && l_219, && l_220, && l_221, && l_222, && l_223, && l_224, && l_225, &&
l_226, && l_227, && l_228, && l_229, && l_230, && l_231, && l_232, && l_233, && l_234, && l_235, && l_236, &&
l_237, && l_238, && l_239, && l_240, && l_241, && l_242, && l_243, && l_244, && l_245, && l_246, && l_247, &&
l_248, && l_249, && l_250, && l_251, && l_252, && l_253, && l_254, && l_255 };

	 goto *a_jump_table[aCode];

  //switch(aCode){
  l_0:nop();return;
  l_1:ld_bc_uint16_t();return;
  l_2:ld_xbc_a();return;
  l_3:inc_bc();return;
  l_4:inc_b();return;
  l_5:dec_b();return;
  l_6:ld_b_uint8_t();return;
  l_7:rlca();return;
  l_8:ex_af_af();return;
  l_9:add_hl_bc();return;
  l_10:ld_a_xbc();return;
  l_11:dec_bc();return;
  l_12:inc_c();return;
  l_13:dec_c();return;
  l_14:ld_c_uint8_t();return;
  l_15:rrca();return;
  l_16:djnz();return;
  l_17:ld_de_uint16_t();return;
  l_18:ld_xde_a();return;
  l_19:inc_de();return;
  l_20:inc_d();return;
  l_21:dec_d();return;
  l_22:ld_d_uint8_t();return;
  l_23:rla();return;
  l_24:jr();return;
  l_25:add_hl_de();return;
  l_26:ld_a_xde();return;
  l_27:dec_de();return;
  l_28:inc_e();return;
  l_29:dec_e();return;
  l_30:ld_e_uint8_t();return;
  l_31:rra();return;
  l_32:jr_nz();return;
  l_33:ld_hl_uint16_t();return;
  l_34:ld_xuint16_t_hl();return;
  l_35:inc_hl();return;
  l_36:inc_h();return;
  l_37:dec_h();return;
  l_38:ld_h_uint8_t();return;
  l_39:daa();return;
  l_40:jr_z();return;
  l_41:add_hl_hl();return;
  l_42:ld_hl_xuint16_t();return;
  l_43:dec_hl();return;
  l_44:inc_l();return;
  l_45:dec_l();return;
  l_46:ld_l_uint8_t();return;
  l_47:cpl();return;
  l_48:jr_nc();return;
  l_49:ld_sp_uint16_t();return;
  l_50:ld_xuint8_t_a();return;
  l_51:inc_sp();return;
  l_52:inc_xhl();return;
  l_53:dec_xhl();return;
  l_54:ld_xhl_uint8_t();return;
  l_55:scf();return;
  l_56:jr_c();return;
  l_57:add_hl_sp();return;
  l_58:ld_a_xuint8_t();return;
  l_59:dec_sp();return;
  l_60:inc_a();return;
  l_61:dec_a();return;
  l_62:ld_a_uint8_t();return;
  l_63:ccf();return;
  l_64:ld_b_b();return;
  l_65:ld_b_c();return;
  l_66:ld_b_d();return;
  l_67:ld_b_e();return;
  l_68:ld_b_h();return;
  l_69:ld_b_l();return;
  l_70:ld_b_xhl();return;
  l_71:ld_b_a();return;
  l_72:ld_c_b();return;
  l_73:ld_c_c();return;
  l_74:ld_c_d();return;
  l_75:ld_c_e();return;
  l_76:ld_c_h();return;
  l_77:ld_c_l();return;
  l_78:ld_c_xhl();return;
  l_79:ld_c_a();return;
  l_80:ld_d_b();return;
  l_81:ld_d_c();return;
  l_82:ld_d_d();return;
  l_83:ld_d_e();return;
  l_84:ld_d_h();return;
  l_85:ld_d_l();return;
  l_86:ld_d_xhl();return;
  l_87:ld_d_a();return;
  l_88:ld_e_b();return;
  l_89:ld_e_c();return;
  l_90:ld_e_d();return;
  l_91:ld_e_e();return;
  l_92:ld_e_h();return;
  l_93:ld_e_l();return;
  l_94:ld_e_xhl();return;
  l_95:ld_e_a();return;
  l_96:ld_h_b();return;
  l_97:ld_h_c();return;
  l_98:ld_h_d();return;
  l_99:ld_h_e();return;
  l_100:ld_h_h();return;
  l_101:ld_h_l();return;
  l_102:ld_h_xhl();return;
  l_103:ld_h_a();return;
  l_104:ld_l_b();return;
  l_105:ld_l_c();return;
  l_106:ld_l_d();return;
  l_107:ld_l_e();return;
  l_108:ld_l_h();return;
  l_109:ld_l_l();return;
  l_110:ld_l_xhl();return;
  l_111:ld_l_a();return;
  l_112:ld_xhl_b();return;
  l_113:ld_xhl_c();return;
  l_114:ld_xhl_d();return;
  l_115:ld_xhl_e();return;
  l_116:ld_xhl_h();return;
  l_117:ld_xhl_l();return;
  l_118:halt();return;
  l_119:ld_xhl_a();return;
  l_120:ld_a_b();return;
  l_121:ld_a_c();return;
  l_122:ld_a_d();return;
  l_123:ld_a_e();return;
  l_124:ld_a_h();return;
  l_125:ld_a_l();return;
  l_126:ld_a_xhl();return;
  l_127:ld_a_a();return;
  l_128:add_a_b();return;
  l_129:add_a_c();return;
  l_130:add_a_d();return;
  l_131:add_a_e();return;
  l_132:add_a_h();return;
  l_133:add_a_l();return;
  l_134:add_a_xhl();return;
  l_135:add_a_a();return;
  l_136:adc_a_b();return;
  l_137:adc_a_c();return;
  l_138:adc_a_d();return;
  l_139:adc_a_e();return;
  l_140:adc_a_h();return;
  l_141:adc_a_l();return;
  l_142:adc_a_xhl();return;
  l_143:adc_a_a();return;
  l_144:sub_b();return;
  l_145:sub_c();return;
  l_146:sub_d();return;
  l_147:sub_e();return;
  l_148:sub_h();return;
  l_149:sub_l();return;
  l_150:sub_xhl();return;
  l_151:sub_a();return;
  l_152:sbc_a_b();return;
  l_153:sbc_a_c();return;
  l_154:sbc_a_d();return;
  l_155:sbc_a_e();return;
  l_156:sbc_a_h();return;
  l_157:sbc_a_l();return;
  l_158:sbc_a_xhl();return;
  l_159:sbc_a_a();return;
  l_160:and_b();return;
  l_161:and_c();return;
  l_162:and_d();return;
  l_163:and_e();return;
  l_164:and_h();return;
  l_165:and_l();return;
  l_166:and_xhl();return;
  l_167:and_a();return;
  l_168:xor_b();return;
  l_169:xor_c();return;
  l_170:xor_d();return;
  l_171:xor_e();return;
  l_172:xor_h();return;
  l_173:xor_l();return;
  l_174:xor_xhl();return;
  l_175:xor_a();return;
  l_176:or_b();return;
  l_177:or_c();return;
  l_178:or_d();return;
  l_179:or_e();return;
  l_180:or_h();return;
  l_181:or_l();return;
  l_182:or_xhl();return;
  l_183:or_a();return;
  l_184:cp_b();return;
  l_185:cp_c();return;
  l_186:cp_d();return;
  l_187:cp_e();return;
  l_188:cp_h();return;
  l_189:cp_l();return;
  l_190:cp_xhl();return;
  l_191:cp_a();return;
  l_192:ret_nz();return;
  l_193:pop_bc();return;
  l_194:jp_nz();return;
  l_195:jp();return;
  l_196:call_nz();return;
  l_197:push_bc();return;
  l_198:add_a_uint8_t();return;
  l_199:rst_00();return;
  l_200:ret_z();return;
  l_201:ret();return;
  l_202:jp_z();return;
  l_203:cb();return;
  l_204:call_z();return;
  l_205:call();return;
  l_206:adc_a_uint8_t();return;
  l_207:rst_08();return;
  l_208:ret_nc();return;
  l_209:pop_de();return;
  l_210:jp_nc();return;
  l_211:out_uint8_t_a();return;
  l_212:call_nc();return;
  l_213:push_de();return;
  l_214:sub_uint8_t();return;
  l_215:rst_10();return;
  l_216:ret_c();return;
  l_217:exx();return;
  l_218:jp_c();return;
  l_219:in_a_uint8_t();return;
  l_220:call_c();return;
  l_221:dd();return;
  l_222:sbc_a_uint8_t();return;
  l_223:rst_18();return;
  l_224:ret_po();return;
  l_225:pop_hl();return;
  l_226:jp_po();return;
  l_227:ex_xsp_hl();return;
  l_228:call_po();return;
  l_229:push_hl();return;
  l_230:and_uint8_t();return;
  l_231:rst_20();return;
  l_232:ret_pe();return;
  l_233:jp_hl();return;
  l_234:jp_pe();return;
  l_235:ex_de_hl();return;
  l_236:call_pe();return;
  l_237:ed();return;
  l_238:xor_uint8_t();return;
  l_239:rst_28();return;
  l_240:ret_p();return;
  l_241:pop_af();return;
  l_242:jp_p();return;
  l_243:di();return;
  l_244:call_p();return;
  l_245:push_af();return;
  l_246:or_uint8_t();return;
  l_247:rst_30();return;
  l_248:ret_m();return;
  l_249:ld_sp_hl();return;
  l_250:jp_m();return;
  l_251:ei();return;
  l_252:call_m();return;
  l_253:fd();return;
  l_254:cp_uint8_t();return;
  l_255:rst_38();return;
  //}
}

/* Get next opcode argument and increment program counter */
uint8_t Z80::Z80_RDMEM_OPCODE (void) {
  uint8_t retval;
  retval=Z80_RDMEM(R.PC.W.l);
  R.PC.W.l++;
  return retval;
}

uint16_t Z80::Z80_RDMEM_U16 (uint16_t A) {
  uint16_t lsb, msb;
  uint16_t retval;
  lsb=(uint16_t) Z80_RDMEM(A);
  msb=(uint16_t) Z80_RDMEM(A+1);
  retval = lsb + (msb <<8);

  return retval;
}

void Z80::Z80_WRMEM_U16 (uint16_t A,uint16_t V) {
 Z80_WRMEM (A,V&255);
 Z80_WRMEM ((uint16_t)(A+1),V>>8);
}

uint16_t Z80::Z80_RDMEM_OPCODE_U16 (void) {
 uint16_t i;
 i=(uint16_t) Z80_RDMEM_OPCODE();
 i+=(uint16_t) Z80_RDMEM_OPCODE()<<8;
 return i;
}

uint8_t Z80::M_RD_XIX(void) {
  uint16_t i;
  i=M_XIX;
  return Z80_RDMEM(i);
}

uint8_t Z80::M_RD_XIY(void) {
  uint16_t i;
  i=M_XIY;
  return Z80_RDMEM(i);
}

void Z80::M_WR_XIX(uint8_t a) {
  uint16_t i;
  i=M_XIX;
  Z80_WRMEM(i,a);
}

void Z80::M_WR_XIY(uint8_t a) {
  uint16_t i;
  i=M_XIY;
  Z80_WRMEM(i,a);
}

unsigned Z80::cycles_main[256]=
{
  4,10,7,6,4,4,7,4,
  4,11,7,6,4,4,7,4,
  8,10,7,6,4,4,7,4,
  7,11,7,6,4,4,7,4,
  7,10,16,6,4,4,7,4,
  7,11,16,6,4,4,7,4,
  7,10,13,6,11,11,10,4,
  7,11,13,6,4,4,7,4,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  7,7,7,7,7,7,4,7,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,
  5,10,10,10,10,11,7,11,
  5,4,10,0,10,10,7,11,
  5,10,10,11,10,11,7,11,
  5,4,10,11,10,0,7,11,
  5,10,10,19,10,11,7,11,
  5,4,10,4,10,0,7,11,
  5,10,10,4,10,11,7,11,
  5,6,10,4,10,0,7,11
};

unsigned Z80::cycles_cb[256]=
{
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,12,8,
  8,8,8,8,8,8,12,8,
  8,8,8,8,8,8,12,8,
  8,8,8,8,8,8,12,8,
  8,8,8,8,8,8,12,8,
  8,8,8,8,8,8,12,8,
  8,8,8,8,8,8,12,8,
  8,8,8,8,8,8,12,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8,
  8,8,8,8,8,8,15,8
};

unsigned Z80::cycles_xx_cb[]=
{
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  20,20,20,20,20,20,20,20,
  20,20,20,20,20,20,20,20,
  20,20,20,20,20,20,20,20,
  20,20,20,20,20,20,20,20,
  20,20,20,20,20,20,20,20,
  20,20,20,20,20,20,20,20,
  20,20,20,20,20,20,20,20,
  20,20,20,20,20,20,20,20,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23,
  23,23,23,23,23,23,23,23
};

unsigned Z80::cycles_xx[]=
{
  0,0,0,0,0,0,0,0,
  0,15,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,15,0,0,0,0,0,0,
  0,14,20,10,9,9,9,0,
  0,15,20,10,9,9,9,0,
  0,0,0,0,23,23,19,0,
  0,15,0,0,0,0,0,0,
  0,0,0,0,9,9,19,0,
  0,0,0,0,9,9,19,0,
  0,0,0,0,9,9,19,0,
  0,0,0,0,9,9,19,0,
  9,9,9,9,9,9,19,9,	/* ASG 220997 */
  9,9,9,9,9,9,19,9,	/* ASG 220997 */
  19,19,19,19,19,19,19,19,
  0,0,0,0,9,9,19,0,
  0,0,0,0,9,9,19,0,
  0,0,0,0,9,9,19,0,
  0,0,0,0,9,9,19,0,
  0,0,0,0,9,9,19,0,
  0,0,0,0,9,9,19,0,
  0,0,0,0,9,9,19,0,
  0,0,0,0,9,9,19,0,
  0,0,0,0,9,9,19,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,14,0,23,0,15,0,0,
  0,8,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,10,0,0,0,0,0,0
};

unsigned Z80::cycles_ed[256]=
{
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  12,12,15,20,8,8,8,9,
  12,12,15,20,8,8,8,9,
  12,12,15,20,8,8,8,9,
  12,12,15,20,8,8,8,9,
  12,12,15,20,8,8,8,18,
  12,12,15,20,8,8,8,18,
  12,12,15,20,8,8,8,0,
  12,12,15,20,8,8,8,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  16,16,16,16,0,0,0,0,
  16,16,16,16,0,0,0,0,
  16,16,16,16,0,0,0,0,  /* ASG 220997 */
  16,16,16,16,0,0,0,0,	/* ASG 220997 */
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0
};



/****************************************************************************/
/* Reset registers to their initial values                                  */
/****************************************************************************/
void Z80::Reset (void) {
  Z80_Regs aRegs;
  Z80_SetRegs (aRegs);
}

/****************************************************************************/
/* Initialise the various lookup tables used by the emulation code          */
/****************************************************************************/
void Z80::InitTables (void)
{
 uint8_t zs;
 int i,p;
 for (i=0;i<256;++i)
 {
  zs=0;
  if (i==0)
   zs|=Z_FLAG;
  if (i&0x80)
   zs|=S_FLAG;
  p=0;
  if (i&1) ++p;
  if (i&2) ++p;
  if (i&4) ++p;
  if (i&8) ++p;
  if (i&16) ++p;
  if (i&32) ++p;
  if (i&64) ++p;
  if (i&128) ++p;
  PTable[i]=(p&1)? 0:V_FLAG;
  ZSTable[i]=zs;
  ZSPTable[i]=zs|PTable[i];
 }
 for (i=0;i<256;++i)
 {
  ZSTable[i+256]=ZSTable[i]|C_FLAG;
  ZSPTable[i+256]=ZSPTable[i]|C_FLAG;
  PTable[i+256]=PTable[i]|C_FLAG;
 }
}

/****************************************************************************/
/* Issue an interrupt if necessary                                          */
/****************************************************************************/
void Z80::Interrupt (void/*int j*/) {
  //printf ("Z80::Interrupt\n");
  if (R.pending_irq == Z80_IGNORE_INT && R.pending_nmi == 0) return;	/* NS 970904 */
  if (R.pending_nmi != 0 || R.IFF1) {

    /* Clear interrupt flip-flop 1 */
    R.IFF1=0;
    /* Check if processor was halted */
    if (R.HALT) {
      ++R.PC.W.l;
      R.HALT=0;
    }

    if (R.pending_nmi != 0) {
      R.pending_nmi = 0;
      M_PUSH (PC);
      R.PC.D=0x0066;
    }
    else {
      int j;
      j = R.pending_irq;	/* NS 970904 */
      R.pending_irq = Z80_IGNORE_INT;	/* NS 970904 */

      /* Interrupt mode 2. Call [R.I:datauint8_t] */
      if (R.IM==2) {
	M_PUSH (PC);
	R.PC.D=Z80_RDMEM_U16((j&255)|(R.I<<8));
      }
      else /* Interrupt mode 1. RST 38h */
	if (R.IM==1) {
	  Z80_ICount-=cycles_main[0xFF];
	  call_opcode_main (0xFF);
	}
	else {
	  /* Interrupt mode 0. We check for CALL and JP instructions, if neither  */
	  /* of these were found we assume a 1 uint8_t opcode was placed on the      */
	  /* databus                                                              */
	  printf ("Interrupt : MODE 0\n");
	  switch (j&0xFF0000) {
	  case 0xCD0000:	/* bugfix NS 970904 */
	    M_PUSH(PC);
	  case 0xC30000:	/* bugfix NS 970904 */
	    R.PC.D=j&0xFFFF;
	    break;
	  default:
	    j&=255;
	    Z80_ICount-=cycles_main[j];
	    call_opcode_main (j);
	    break;
	  }
	}
    }
  }
}


/*******************************/
/* Execute IPeriod T-States.   */
/*******************************/
int Z80::Execute(int cycles) {
 uint8_t opcode;
 Z80_ICount=cycles;	/* NS 970904 */

 do {
   if (R.pending_nmi != 0 || R.pending_irq != Z80_IGNORE_INT) Interrupt();	/* NS 970901 */
   ++R.R;
   opcode=Z80_RDMEM(R.PC.D);

   R.PC.W.l++;
   Z80_ICount-=cycles_main[opcode];
   //(*(opcode_main[opcode]))();
   call_opcode_main (opcode);
 }
 while (Z80_ICount>0);

 return cycles - Z80_ICount;	/* NS 970904 */
}

int Z80::Step(void) {
 uint8_t opcode;
 Z80_ICount=0;	/* NS 970904 */

 if (R.pending_nmi != 0 || R.pending_irq != Z80_IGNORE_INT) Interrupt();	/* NS 970901 */
 ++R.R;
 opcode=Z80_RDMEM(R.PC.D);

 R.PC.W.l++;
 Z80_ICount = cycles_main[opcode];

 call_opcode_main (opcode);


 return Z80_ICount;	/* NS 970904 */
}

void Z80::Z80_Cause_Interrupt(int type) {
  if (type == Z80_NMI_INT) {
    printf ("Z80::Z80_Cause_Interrupt : Z80_NMI_INT\n");
    R.pending_nmi = 1;
  }
  else if (type != Z80_IGNORE_INT) {
    //printf ("Z80::Z80_Cause_Interrupt : Z80_INT\n");
    R.pending_irq = type;
  }
}

// End of file
