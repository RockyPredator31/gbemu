#include "cpu.h"

void cpu_init(CPU *cpu, GB_Version gbv)
{
    switch (gbv)
    {
    case DMG:
        cpu->a = 0x01;
        cpu->f = 0xB0; // z=1 | n=0 | h=1 | c=1
        cpu->b = 0x00;
        cpu->c = 0x13;
        cpu->d = 0x00;
        cpu->e = 0xD8;
        cpu->h = 0x01;
        cpu->l = 0x4D;
        cpu->pc = 0x0100;
        cpu->sp = 0xFFFE;

        cpu->halted = false;
        cpu->stopped = false;
        cpu->cycles = 0;
        break;

    default:
        break;
    }
}

// ============ Flag SET ===========================
void cpu_set_z(CPU *cpu) { cpu->f = cpu->f | 0x80u; }
void cpu_set_n(CPU *cpu) { cpu->f = cpu->f | 0x40u; }
void cpu_set_h(CPU *cpu) { cpu->f = cpu->f | 0x20u; }
void cpu_set_c(CPU *cpu) { cpu->f = cpu->f | 0x10u; }

// =============== Flag CLEAR =======================
void cpu_clear_z(CPU *cpu) { cpu->f = cpu->f & (~0x80u); }
void cpu_clear_n(CPU *cpu) { cpu->f = cpu->f & (~0x40u); }
void cpu_clear_h(CPU *cpu) { cpu->f = cpu->f & (~0x20u); }
void cpu_clear_c(CPU *cpu) { cpu->f = cpu->f & (~0x10u); }

uint8_t cpu_fetch(GameBoy *gb)
{
    uint8_t opcode = memory_read(gb, gb->cpu.pc);
    gb->cpu.pc++;
    return opcode;
}

void cpu_step(GameBoy *gb)
{
    uint8_t opcode = cpu_fetch(gb);
    cpu_decode_and_execute(gb, opcode);
}

void cpu_decode_and_execute(GameBoy *gb, uint8_t opcode)
{
    switch (opcode)
    {
    /* ==================== 0x00 - 0x0F ==================== */
    case 0x00: /* NOP */
        break;
    case 0x01: /* LD BC, n16 */
        break;
    case 0x02: /* LD (BC), A */
        break;
    case 0x03: /* INC BC */
        break;
    case 0x04: /* INC B */
        break;
    case 0x05: /* DEC B */
        break;
    case 0x06: /* LD B, n8 */
        break;
    case 0x07: /* RLCA */
        break;
    case 0x08: /* LD (a16), SP */
        break;
    case 0x09: /* ADD HL, BC */
        break;
    case 0x0A: /* LD A, (BC) */
        break;
    case 0x0B: /* DEC BC */
        break;
    case 0x0C: /* INC C */
        break;
    case 0x0D: /* DEC C */
        break;
    case 0x0E: /* LD C, n8 */
        break;
    case 0x0F: /* RRCA */
        break;

    /* ==================== 0x10 - 0x1F ==================== */
    case 0x10: /* STOP 0 */
        break;
    case 0x11: /* LD DE, n16 */
        break;
    case 0x12: /* LD (DE), A */
        break;
    case 0x13: /* INC DE */
        break;
    case 0x14: /* INC D */
        break;
    case 0x15: /* DEC D */
        break;
    case 0x16: /* LD D, n8 */
        break;
    case 0x17: /* RLA */
        break;
    case 0x18: /* JR r8 */
        break;
    case 0x19: /* ADD HL, DE */
        break;
    case 0x1A: /* LD A, (DE) */
        break;
    case 0x1B: /* DEC DE */
        break;
    case 0x1C: /* INC E */
        break;
    case 0x1D: /* DEC E */
        break;
    case 0x1E: /* LD E, n8 */
        break;
    case 0x1F: /* RRA */
        break;

    /* ==================== 0x20 - 0x3F ==================== */
    case 0x20: /* JR NZ, r8 */
        break;
    case 0x21: /* LD HL, n16 */
        break;
    case 0x22: /* LD (HL+), A */
        break; // LDI (HL), A
    case 0x23: /* INC HL */
        break;
    case 0x24: /* INC H */
        break;
    case 0x25: /* DEC H */
        break;
    case 0x26: /* LD H, n8 */
        break;
    case 0x27: /* DAA */
        break;
    case 0x28: /* JR Z, r8 */
        break;
    case 0x29: /* ADD HL, HL */
        break;
    case 0x2A: /* LD A, (HL+) */
        break; // LDI A, (HL)
    case 0x2B: /* DEC HL */
        break;
    case 0x2C: /* INC L */
        break;
    case 0x2D: /* DEC L */
        break;
    case 0x2E: /* LD L, n8 */
        break;
    case 0x2F: /* CPL */
        break;

    case 0x30: /* JR NC, r8 */
        break;
    case 0x31: /* LD SP, n16 */
        break;
    case 0x32: /* LD (HL-), A */
        break; // LDD (HL), A
    case 0x33: /* INC SP */
        break;
    case 0x34: /* INC (HL) */
        break;
    case 0x35: /* DEC (HL) */
        break;
    case 0x36: /* LD (HL), n8 */
        break;
    case 0x37: /* SCF */
        break;
    case 0x38: /* JR C, r8 */
        break;
    case 0x39: /* ADD HL, SP */
        break;
    case 0x3A: /* LD A, (HL-) */
        break; // LDD A, (HL)
    case 0x3B: /* DEC SP */
        break;
    case 0x3C: /* INC A */
        break;
    case 0x3D: /* DEC A */
        break;
    case 0x3E: /* LD A, n8 */
        break;
    case 0x3F: /* CCF */
        break;

    /* ==================== 0x40 - 0x7F : LD r, r ==================== */
    /* LD B, r */
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    /* LD C, r */
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:
    /* LD D, r */
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x53:
    case 0x54:
    case 0x55:
    case 0x56:
    case 0x57:
    /* LD E, r */
    case 0x58:
    case 0x59:
    case 0x5A:
    case 0x5B:
    case 0x5C:
    case 0x5D:
    case 0x5E:
    case 0x5F:
    /* LD H, r */
    case 0x60:
    case 0x61:
    case 0x62:
    case 0x63:
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
    /* LD L, r */
    case 0x68:
    case 0x69:
    case 0x6A:
    case 0x6B:
    case 0x6C:
    case 0x6D:
    case 0x6E:
    case 0x6F:
    /* LD (HL), r */
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
    case 0x77:
    /* LD A, r */
    case 0x78:
    case 0x79:
    case 0x7A:
    case 0x7B:
    case 0x7C:
    case 0x7D:
    case 0x7E:
    case 0x7F:
        /* LD Befehle */ break;

    /* ==================== 0x80 - 0xBF : Arithmetic ==================== */
    case 0x80:
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
    case 0x87: /* ADD A, r */
    case 0x88:
    case 0x89:
    case 0x8A:
    case 0x8B:
    case 0x8C:
    case 0x8D:
    case 0x8E:
    case 0x8F: /* ADC A, r */
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97: /* SUB r */
    case 0x98:
    case 0x99:
    case 0x9A:
    case 0x9B:
    case 0x9C:
    case 0x9D:
    case 0x9E:
    case 0x9F: /* SBC A, r */
    case 0xA0:
    case 0xA1:
    case 0xA2:
    case 0xA3:
    case 0xA4:
    case 0xA5:
    case 0xA6:
    case 0xA7: /* AND r */
    case 0xA8:
    case 0xA9:
    case 0xAA:
    case 0xAB:
    case 0xAC:
    case 0xAD:
    case 0xAE:
    case 0xAF: /* XOR r */
    case 0xB0:
    case 0xB1:
    case 0xB2:
    case 0xB3:
    case 0xB4:
    case 0xB5:
    case 0xB6:
    case 0xB7: /* OR r */
    case 0xB8:
    case 0xB9:
    case 0xBA:
    case 0xBB:
    case 0xBC:
    case 0xBD:
    case 0xBE:
    case 0xBF: /* CP r */
        break;

    /* ==================== 0xC0 - 0xFF : Jumps, Calls, Restarts ==================== */
    case 0xC0: /* RET NZ */
        break;
    case 0xC1: /* POP BC */
        break;
    case 0xC2: /* JP NZ, a16 */
        break;
    case 0xC3: /* JP a16 */
        break;
    case 0xC4: /* CALL NZ, a16 */
        break;
    case 0xC5: /* PUSH BC */
        break;
    case 0xC6: /* ADD A, n8 */
        break;
    case 0xC7: /* RST 00H */
        break;
    case 0xC8: /* RET Z */
        break;
    case 0xC9: /* RET */
        break;
    case 0xCA: /* JP Z, a16 */
        break;
    case 0xCB: /* CB Prefix */
        break; // Wichtig!
    case 0xCC: /* CALL Z, a16 */
        break;
    case 0xCD: /* CALL a16 */
        break;
    case 0xCE: /* ADC A, n8 */
        break;
    case 0xCF: /* RST 08H */
        break;

    case 0xD0: /* RET NC */
        break;
    case 0xD1: /* POP DE */
        break;
    case 0xD2: /* JP NC, a16 */
        break;
    case 0xD4: /* CALL NC, a16 */
        break;
    case 0xD5: /* PUSH DE */
        break;
    case 0xD6: /* SUB n8 */
        break;
    case 0xD7: /* RST 10H */
        break;
    case 0xD8: /* RET C */
        break;
    case 0xD9: /* RETI */
        break;
    case 0xDA: /* JP C, a16 */
        break;
    case 0xDC: /* CALL C, a16 */
        break;
    case 0xDE: /* SBC A, n8 */
        break;
    case 0xDF: /* RST 18H */
        break;

    case 0xE0: /* LDH (a8), A */
        break;
    case 0xE1: /* POP HL */
        break;
    case 0xE2: /* LD (C), A */
        break;
    case 0xE5: /* PUSH HL */
        break;
    case 0xE6: /* AND n8 */
        break;
    case 0xE7: /* RST 20H */
        break;
    case 0xE8: /* ADD SP, r8 */
        break;
    case 0xE9: /* JP (HL) */
        break;
    case 0xEA: /* LD (a16), A */
        break;
    case 0xEE: /* XOR n8 */
        break;
    case 0xEF: /* RST 28H */
        break;

    case 0xF0: /* LDH A, (a8) */
        break;
    case 0xF1: /* POP AF */
        break;
    case 0xF2: /* LD A, (C) */
        break;
    case 0xF3: /* DI */
        break;
    case 0xF5: /* PUSH AF */
        break;
    case 0xF6: /* OR n8 */
        break;
    case 0xF7: /* RST 30H */
        break;
    case 0xF8: /* LD HL, SP+r8 */
        break;
    case 0xF9: /* LD SP, HL */
        break;
    case 0xFA: /* LD A, (a16) */
        break;
    case 0xFB: /* EI */
        break;
    case 0xFE: /* CP n8 */
        break;
    case 0xFF: /* RST 38H */
        break;
    }
}

// ========= Operations
void op_nop(GameBoy* gb)
{
    
}