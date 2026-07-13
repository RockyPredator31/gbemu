#include "cpu.h"
#include "../gb.h"

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
        cpu->halt_bug = false;
        cpu->stopped = false;
        cpu->cycles = 0;
        cpu->ime = 0;
        cpu->ei_delay = 0;
        break;

    default:
        break;
    }
}

// ============ Flag SET ===========================
void cpu_set_z(CPU *cpu) { cpu->f = cpu->f | 0x80; }
void cpu_set_n(CPU *cpu) { cpu->f = cpu->f | 0x40; }
void cpu_set_h(CPU *cpu) { cpu->f = cpu->f | 0x20; }
void cpu_set_c(CPU *cpu) { cpu->f = cpu->f | 0x10; }

// =============== Flag CLEAR =======================
void cpu_clear_z(CPU *cpu) { cpu->f = cpu->f & (~0x80u); }
void cpu_clear_n(CPU *cpu) { cpu->f = cpu->f & (~0x40u); }
void cpu_clear_h(CPU *cpu) { cpu->f = cpu->f & (~0x20u); }
void cpu_clear_c(CPU *cpu) { cpu->f = cpu->f & (~0x10u); }

// =============== GET Flag ==========================
uint8_t cpu_get_z(const CPU* cpu) { return (cpu->f & 0x80 ? 1 : 0); }
uint8_t cpu_get_n(const CPU* cpu) { return (cpu->f & 0x40 ? 1 : 0); }
uint8_t cpu_get_h(const CPU* cpu) { return (cpu->f & 0x20 ? 1 : 0); }
uint8_t cpu_get_c(const CPU* cpu) { return (cpu->f & 0x10 ? 1 : 0); }

// ========= Get Registers ==========================
uint16_t cpu_get_af(const CPU* cpu) { return ( (cpu->a << 8) | cpu->f); }
uint16_t cpu_get_bc(const CPU* cpu) { return ( (cpu->b << 8) | cpu->c); }
uint16_t cpu_get_de(const CPU* cpu) { return ( (cpu->d << 8) | cpu->e); }
uint16_t cpu_get_hl(const CPU* cpu) { return ( (cpu->h << 8) | cpu->l); }

// ========= Set Registers ===========================
void cpu_set_af(CPU* cpu, uint16_t value) { 
    cpu->a = value >> 8; 
    cpu->f = value & 0xF0; 
}

void cpu_set_bc(CPU* cpu, uint16_t value) { 
    cpu->b = value >> 8; 
    cpu->c = value & 0xFF; 
}

void cpu_set_de(CPU* cpu, uint16_t value) { 
    cpu->d = value >> 8; 
    cpu->e = value & 0xFF; 
}

void cpu_set_hl(CPU* cpu, uint16_t value) { 
    cpu->h = value >> 8; 
    cpu->l = value & 0xFF; 
}

// ============ Hilfsfunktionen ============
static inline void cpu_check_z(CPU* cpu, uint8_t value)
{
    if (value == 0)
       cpu_set_z(cpu);
    else
        cpu_clear_z(cpu);
}

// 8-bit
static inline void cpu_check_h_inc(CPU* cpu, uint8_t value)
{
    if ((value & 0x0F) == 0)
         cpu_set_h(cpu);
    else
        cpu_clear_h(cpu);
}
// 8-bit
static inline void cpu_check_h_dec(CPU* cpu, uint8_t value)
{
    if ((value & 0x0F) == 0x0F)
        cpu_set_h(cpu);
    else
        cpu_clear_h(cpu);
}

static inline void cpu_add_hl_r16(CPU* cpu, uint16_t value)
{
    uint32_t result = cpu_get_hl(cpu) + value;

    cpu_clear_n(cpu);
    // Half Carry (Bit 11)
    if (((cpu_get_hl(cpu) & 0x0FFF) + (value & 0x0FFF)) > 0x0FFF)
        cpu_set_h(cpu);
    else
        cpu_clear_h(cpu);
    
    // Carry (Bit 15)
    if (result > 0xFFFF)
        cpu_set_c(cpu);
    else
        cpu_clear_c(cpu);
        
    cpu_set_hl(cpu, (uint16_t)result);
        
    cpu->cycles += 8;
}

static inline void cpu_inc_r8(GameBoy* gb, uint8_t reg)
{
    uint8_t result;

    switch (reg)
    {
        case 0: result = ++gb->cpu.b; break;   // B
        case 1: result = ++gb->cpu.c; break;   // C
        case 2: result = ++gb->cpu.d; break;   // D
        case 3: result = ++gb->cpu.e; break;   // E
        case 4: result = ++gb->cpu.h; break;   // H
        case 5: result = ++gb->cpu.l; break;   // L
        case 6:                                 // (HL)
        {
            uint16_t addr = (gb->cpu.h << 8) | gb->cpu.l;
            result = memory_read(gb, addr) + 1;
            memory_write(gb, addr, result);
            break;
        }
        case 7: result = ++gb->cpu.a; break;   // A
        default: return;
    }

    cpu_clear_n(&gb->cpu);
    cpu_check_z(&gb->cpu, result);
    cpu_check_h_inc(&gb->cpu, result);
    gb->cpu.cycles += 4;
}

static inline void cpu_dec_r8(GameBoy* gb, uint8_t reg)
{
    uint8_t result;

    switch (reg)
    {
        case 0: result = --gb->cpu.b; break;   // B
        case 1: result = --gb->cpu.c; break;   // C
        case 2: result = --gb->cpu.d; break;   // D
        case 3: result = --gb->cpu.e; break;   // E
        case 4: result = --gb->cpu.h; break;   // H
        case 5: result = --gb->cpu.l; break;   // L
        case 6:                                 // (HL)
        {
            uint16_t addr = (gb->cpu.h << 8) | gb->cpu.l;
            result = memory_read(gb, addr) - 1;
            memory_write(gb, addr, result);
            break;
        }
        case 7: result = --gb->cpu.a; break;   // A
        default: return;
    }

    cpu_set_n(&gb->cpu);
    cpu_check_z(&gb->cpu, result);
    cpu_check_h_dec(&gb->cpu, result);
    gb->cpu.cycles += 4;
}

void cpu_push16(GameBoy* gb, uint16_t value) {
    // High Byte
    gb->cpu.sp--;
    memory_write(gb, gb->cpu.sp, (value >> 8) & 0xFF);
    // Low Byte
    gb->cpu.sp--;
    memory_write(gb, gb->cpu.sp, value & 0xFF);
}

uint16_t cpu_pop16(GameBoy* gb)
{
    uint16_t result = (memory_read(gb, gb->cpu.sp + 1)  << 8) | memory_read(gb, gb->cpu.sp);
    gb->cpu.sp += 2;
    return result;
}

// =============== Hilfsfunktionen für 0x40 - 0x7F ====================
// Hilfsfunktion zum LESEN der Quelle (src)
uint8_t cpu_get_reg_by_index(GameBoy* gb, uint8_t index) {
    switch (index) {
        case 0: return gb->cpu.b;
        case 1: return gb->cpu.c;
        case 2: return gb->cpu.d;
        case 3: return gb->cpu.e;
        case 4: return gb->cpu.h;
        case 5: return gb->cpu.l;
        case 6: return memory_read(gb, cpu_get_hl(&gb->cpu)); // (HL) Speicher lesen
        case 7: return gb->cpu.a;
    }
    return 0; // Sollte nie erreicht werden
}

// Hilfsfunktion zum SCHREIBEN in das Ziel (dest)
void cpu_set_reg_by_index(GameBoy* gb, uint8_t index, uint8_t value) {
    switch (index) {
        case 0: gb->cpu.b = value; break;
        case 1: gb->cpu.c = value; break;
        case 2: gb->cpu.d = value; break;
        case 3: gb->cpu.e = value; break;
        case 4: gb->cpu.h = value; break;
        case 5: gb->cpu.l = value; break;
        case 6: memory_write(gb, cpu_get_hl(&gb->cpu), value); break; // (HL) Speicher schreiben
        case 7: gb->cpu.a = value; break;
    }
}

// ========= step ========
void cpu_step(GameBoy *gb)
{
    // CPU schläft wenn halted != 0
    if (gb->cpu.halted) {
        gb->cpu.cycles += 4;
        return;
    }

    uint8_t opcode = cpu_fetch(gb);
    cpu_decode_and_execute(gb, opcode);
    
    if (gb->cpu.cycles >= 70224)        // 70224 Zyklen = 1 Frame (DMG)
    {
        // Hier kommt später:
        // - Bild rendern (PPU)
        // - Sound updaten
        // - Input updaten

        gb->cpu.cycles -= 70224;        // WICHTIG: subtrahieren, nicht auf 0 setzen!
    }
}

// ======== fetch =========
uint8_t cpu_fetch(GameBoy *gb)
{
    uint8_t opcode = memory_read(gb, gb->cpu.pc);
    // ist der HALT bug aktiv, wird der pc nicht erhöht.
    if (gb->cpu.halt_bug) {
        gb->cpu.halt_bug = false; 
    } else {
        gb->cpu.pc++;
    }
    return opcode;
}


// ========= decode and execute ========
void cpu_decode_and_execute(GameBoy *gb, uint8_t opcode)
{
    uint8_t result = 0;
    uint8_t op_value = 0;
    uint8_t u8Val = 0;
    uint16_t u16Val = 0;
    int32_t result32 = 0; 
    uint8_t carry = 0;
    uint16_t address = 0;
    int8_t offset = 0;
    switch (opcode)
    {
    /* ==================== 0x00 - 0x0F ==================== */
    case 0x00:
        gb->cpu.cycles += 4;
        break;
    case 0x01: /* LD BC, n16 */
        cpu_set_bc(&gb->cpu, memory_read16(gb, gb->cpu.pc));
        gb->cpu.pc += 2;
        gb->cpu.cycles += 12;
        break;
    case 0x02: /* LD (BC), A */
        memory_write(gb, cpu_get_bc(&gb->cpu), gb->cpu.a);
        gb->cpu.cycles += 8;
        break;
    case 0x03: /* INC BC */
        cpu_set_bc(&gb->cpu, cpu_get_bc(&gb->cpu) + 1);
        gb->cpu.cycles += 8;
        break;
    case 0x04: /* INC B */
        cpu_inc_r8(gb, 0);
        break;
    case 0x05: /* DEC B */
        cpu_dec_r8(gb, 0);
        break;
    case 0x06: /* LD B, n8 */
        gb->cpu.b = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc++;
        gb->cpu.cycles += 8;
        break;
    case 0x07: /* RLCA */
        carry = (gb->cpu.a & 0x80) >> 7;
        gb->cpu.a = (gb->cpu.a << 1) | carry;
        cpu_clear_h(&gb->cpu);
        cpu_clear_n(&gb->cpu);
        cpu_clear_z(&gb->cpu);
        
        if(carry)
            cpu_set_c(&gb->cpu);
        else
            cpu_clear_c(&gb->cpu);

        gb->cpu.cycles += 4;
        break;
    case 0x08: /* LD (a16), SP */
        address = memory_read16(gb, gb->cpu.pc); 
        memory_write16(gb, address, gb->cpu.sp);
        gb->cpu.pc += 2;
        gb->cpu.cycles += 20;
        break;
    case 0x09: /* ADD HL, BC */
        cpu_add_hl_r16(&gb->cpu, cpu_get_bc(&gb->cpu));
        break;
    case 0x0A: /* LD A, (BC) */
        gb->cpu.a = memory_read(gb, cpu_get_bc(&gb->cpu));
        gb->cpu.cycles += 8;
        break;
    case 0x0B: /* DEC BC */
        cpu_set_bc(&gb->cpu, cpu_get_bc(&gb->cpu) - 1);
        gb->cpu.cycles += 8;
        break;
    case 0x0C: /* INC C */
        cpu_inc_r8(gb, 1);
        break;
    case 0x0D: /* DEC C */
        cpu_dec_r8(gb, 1);
        break;
    case 0x0E: /* LD C, n8 */
        gb->cpu.c = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc++;
        gb->cpu.cycles += 8;
        break;
    case 0x0F: /* RRCA */
        carry = gb->cpu.a & 0x01;
        gb->cpu.a = (gb->cpu.a >> 1) | (carry << 7);
        cpu_clear_z(&gb->cpu);
        cpu_clear_n(&gb->cpu);
        cpu_clear_h(&gb->cpu);

        if (carry)
            cpu_set_c(&gb->cpu);
        else
            cpu_clear_c(&gb->cpu);
    
        gb->cpu.cycles += 4;
        break;

    /* ==================== 0x10 - 0x1F ==================== */
    case 0x10: /* STOP 0 */
        gb->cpu.stopped = true;
        gb->cpu.cycles += 4;
        break;
    case 0x11: /* LD DE, n16 */
        cpu_set_de(&gb->cpu, memory_read16(gb, gb->cpu.pc));
        gb->cpu.pc += 2;
        gb->cpu.cycles += 12;
        break;
    case 0x12: /* LD (DE), A */
        memory_write(gb, cpu_get_de(&gb->cpu), gb->cpu.a);
        gb->cpu.cycles += 8;
        break;
    case 0x13: /* INC DE */
        cpu_set_bc(&gb->cpu, cpu_get_de(&gb->cpu) + 1);
        gb->cpu.cycles += 8;
        break;
    case 0x14: /* INC D */
        cpu_inc_r8(gb, 2);
        break;
    case 0x15: /* DEC D */
        cpu_dec_r8(gb, 2);
        break;
    case 0x16: /* LD D, n8 */
        gb->cpu.d = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc++;
        gb->cpu.cycles += 8;
        break;
    case 0x17: /* RLA */
        carry = (gb->cpu.a & 0x80)>> 7;
        gb->cpu.a = (gb->cpu.a << 1) | cpu_get_c(&gb->cpu);

        if(carry)
            cpu_set_c(&gb->cpu);
        else
            cpu_clear_c(&gb->cpu);

        cpu_clear_z(&gb->cpu);
        cpu_clear_n(&gb->cpu);
        cpu_clear_h(&gb->cpu);
        gb->cpu.cycles += 4;
        break;
    case 0x18: /* JR r8 */
        offset = (int8_t)memory_read(gb, gb->cpu.pc);    // signed 8-Bit Wert
        gb->cpu.pc++;                                        // PC auf nächsten Opcode
        gb->cpu.pc += offset;                                   // Relativer Sprung
        gb->cpu.cycles += 12;
        break;
    case 0x19: /* ADD HL, DE */
        cpu_add_hl_r16(&gb->cpu, cpu_get_de(&gb->cpu));
        break;
    case 0x1A: /* LD A, (DE) */
        gb->cpu.a = memory_read(gb, cpu_get_de(&gb->cpu));
        gb->cpu.cycles += 8;
        break;
    case 0x1B: /* DEC DE */
        cpu_set_de(&gb->cpu, cpu_get_de(&gb->cpu) - 1);
        gb->cpu.cycles += 8;
        break;
    case 0x1C: /* INC E */
        cpu_inc_r8(gb, 3);
        break;
    case 0x1D: /* DEC E */
        cpu_dec_r8(gb, 3);
        break;
    case 0x1E: /* LD E, n8 */
        gb->cpu.e = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc++;
        gb->cpu.cycles += 8;
        break;
    case 0x1F: /* RRA */
        carry = gb->cpu.a & 0x01;
        gb->cpu.a = (gb->cpu.a >> 1) | (cpu_get_c(&gb->cpu) << 7);

        if(carry)
            cpu_set_c(&gb->cpu);
        else
            cpu_clear_c(&gb->cpu);

        cpu_clear_z(&gb->cpu);
        cpu_clear_n(&gb->cpu);
        cpu_clear_h(&gb->cpu);
        gb->cpu.cycles += 4;
        break;

    /* ==================== 0x20 - 0x3F ==================== */
    case 0x20: /* JR NZ, r8 */
        offset = (int8_t)memory_read(gb, gb->cpu.pc);    // signed 8-Bit Wert
        gb->cpu.pc++;                                        // PC auf nächsten Opcode

        if (cpu_get_z(&gb->cpu) == 0){
            gb->cpu.pc += offset;
            gb->cpu.cycles += 12;
        } else {
            gb->cpu.cycles += 8;
        }
        break;
    case 0x21: /* LD HL, n16 */
        cpu_set_hl(&gb->cpu, memory_read16(gb, gb->cpu.pc));
        gb->cpu.pc += 2;
        gb->cpu.cycles += 12;
        break;
    case 0x22: /* LD (HL+), A */
        address = cpu_get_hl(&gb->cpu);
        memory_write(gb, address, gb->cpu.a);
        cpu_set_hl(&gb->cpu, address + 1);
        gb->cpu.cycles += 8;
        break; // LDI (HL), A
    case 0x23: /* INC HL */
        cpu_set_hl(&gb->cpu, cpu_get_hl(&gb->cpu) + 1);
        gb->cpu.cycles += 8;
        break;
    case 0x24: /* INC H */
        cpu_inc_r8(gb, 4);
        break;
    case 0x25: /* DEC H */
        cpu_dec_r8(gb, 4);
        break;
    case 0x26: /* LD H, n8 */
        gb->cpu.h = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc++;
        gb->cpu.cycles += 8;
        break;
    case 0x27: /* DAA */
        result = 0;
        carry = 0;
    
        if (cpu_get_h(&gb->cpu) || (!cpu_get_n(&gb->cpu) && (gb->cpu.a & 0x0F) > 9)) {
            result |= 0x06;
        }
        if (cpu_get_c(&gb->cpu) || (!cpu_get_n(&gb->cpu) && gb->cpu.a > 0x99)) {
            result |= 0x60;
            carry = 1;
        }
    
        if (cpu_get_n(&gb->cpu)) {
            gb->cpu.a -= result;
        } else {
            gb->cpu.a += result;
        }
    
        cpu_check_z(&gb->cpu, gb->cpu.a);
        cpu_clear_h(&gb->cpu);
        if (carry) {
            cpu_set_c(&gb->cpu);
        } else {
            cpu_clear_c(&gb->cpu);
        }
        gb->cpu.cycles += 4;
        break;
    case 0x28: /* JR Z, r8 */
        offset = (int8_t)memory_read(gb, gb->cpu.pc);    // signed 8-Bit Wert
        gb->cpu.pc++;                                        // PC auf nächsten Opcode
        if (cpu_get_z(&gb->cpu) == 1){
            gb->cpu.pc += offset;
            gb->cpu.cycles += 12;
        } else {
            gb->cpu.cycles += 8;
        }
        break;
    case 0x29: /* ADD HL, HL */
        cpu_add_hl_r16(&gb->cpu, cpu_get_hl(&gb->cpu));
        break;
    case 0x2A: /* LD A, (HL+) */
        gb->cpu.a = memory_read(gb, cpu_get_hl(&gb->cpu));
        cpu_set_hl(&gb->cpu, cpu_get_hl(&gb->cpu) + 1);
        gb->cpu.cycles += 8;
        break; // LDI A, (HL)
    case 0x2B: /* DEC HL */
        cpu_set_hl(&gb->cpu, cpu_get_hl(&gb->cpu) - 1);
        gb->cpu.cycles += 8;
        break;
    case 0x2C: /* INC L */
        cpu_inc_r8(gb, 5);
        break;
    case 0x2D: /* DEC L */
        cpu_dec_r8(gb, 5);
        break;
    case 0x2E: /* LD L, n8 */
        gb->cpu.l = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc++;
        gb->cpu.cycles += 8;
        break;
    case 0x2F: /* CPL */
        gb->cpu.a = ~(gb->cpu.a);
        cpu_set_c(&gb->cpu);
        cpu_set_h(&gb->cpu);
        gb->cpu.cycles += 4;
        break;
    case 0x30: /* JR NC, r8 */
        offset = (int8_t)memory_read(gb, gb->cpu.pc);    // signed 8-Bit Wert
        gb->cpu.pc++;                                        // PC auf nächsten Opcode

        if (cpu_get_c(&gb->cpu) == 0){
            gb->cpu.pc += offset;
            gb->cpu.cycles += 12;
        } else {
            gb->cpu.cycles += 8;
        }
        break;
    case 0x31: /* LD SP, n16 */
        gb->cpu.sp = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc += 2;
        gb->cpu.cycles += 12;
        break;
    case 0x32: /* LD (HL-), A */
        address = cpu_get_hl(&gb->cpu);
        memory_write(gb, address, gb->cpu.a);
        cpu_set_hl(&gb->cpu, address - 1);
        gb->cpu.cycles += 8;
        break; // LDD (HL), A
    case 0x33: /* INC SP */
        gb->cpu.sp++;
        gb->cpu.cycles += 8;
        break;
    case 0x34: /* INC (HL) */
        address = cpu_get_hl(&gb->cpu);
        result = memory_read(gb, address);
        result++;
        memory_write(gb, address, result);
        cpu_clear_n(&gb->cpu);
        cpu_check_z(&gb->cpu, result);
        cpu_check_h_inc(&gb->cpu, result);
        gb->cpu.cycles += 12;
        break;
    case 0x35: /* DEC (HL) */
        address = cpu_get_hl(&gb->cpu);
        result = memory_read(gb, address);
        result--;
        memory_write(gb, address, result);
        cpu_set_n(&gb->cpu);
        cpu_check_z(&gb->cpu, result);
        cpu_check_h_dec(&gb->cpu, result);
        gb->cpu.cycles += 12;
        break;
    case 0x36: /* LD (HL), n8 */
        result = memory_read(gb, gb->cpu.pc);
        memory_write(gb, cpu_get_hl(&gb->cpu), result);
        gb->cpu.pc++;
        gb->cpu.cycles += 12;
        break;
    case 0x37: /* SCF */
        cpu_set_c(&gb->cpu);
        cpu_clear_h(&gb->cpu);
        cpu_clear_n(&gb->cpu);
        gb->cpu.cycles += 4;
        break;
    case 0x38: /* JR C, r8 */
        offset = (int8_t)memory_read(gb, gb->cpu.pc);    // signed 8-Bit Wert
        gb->cpu.pc++;                                        // PC auf nächsten Opcode
        if (cpu_get_c(&gb->cpu) == 1){
            gb->cpu.pc += offset;
            gb->cpu.cycles += 12;
        } else {
            gb->cpu.cycles += 8;
        }
        break;
    case 0x39: /* ADD HL, SP */
        cpu_add_hl_r16(&gb->cpu, gb->cpu.sp);
        break;
    case 0x3A: /* LD A, (HL-) */
        gb->cpu.a = memory_read(gb, cpu_get_hl(&gb->cpu));
        cpu_set_hl(&gb->cpu, cpu_get_hl(&gb->cpu) - 1);
        gb->cpu.cycles += 8;
        break; // LDD A, (HL)
    case 0x3B: /* DEC SP */
        gb->cpu.sp--;
        gb->cpu.cycles += 8;
        break;
    case 0x3C: /* INC A */
        cpu_inc_r8(gb, 7);
        break;
    case 0x3D: /* DEC A */
        cpu_dec_r8(gb, 7);
        break;
    case 0x3E: /* LD A, n8 */
         gb->cpu.a = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc++;
        gb->cpu.cycles += 8;
        break;
    case 0x3F: /* CCF */
        if(cpu_get_c(&gb->cpu) == 1)
        {
            cpu_clear_c(&gb->cpu);
        }else
        {
            cpu_set_c(&gb->cpu);
        }
        cpu_clear_h(&gb->cpu);
        cpu_clear_n(&gb->cpu);
        gb->cpu.cycles += 4;
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
        result = cpu_get_reg_by_index(gb, opcode & 0x07);
        cpu_set_reg_by_index(gb, (opcode >> 3) & 0x07, result);
        
        if (((opcode >> 3) & 0x07) == 6 || (opcode & 0x07) == 6) {
            gb->cpu.cycles += 8;
        } else {
            gb->cpu.cycles += 4;
        }
     break;

     case 0x76: /* HALT */
    // IME = Interrupt Master Enable (durch EI/DI-Befehle gesteuert)
    // IE  = Interrupt Enable Register (0xFFFF im Speicher)
    // IF  = Interrupt Flag Register   (0xFF0F im Speicher)
    
    uint8_t ie = memory_read(gb, 0xFFFF);
    uint8_t if_flag = memory_read(gb, 0xFF0F);

    if (gb->cpu.ime == 1) {
        // --- Normales Verhalten ---
        // CPU schläft, bis ein Interrupt kommt
        gb->cpu.halted = true;
    } else {
        // --- IME ist 0 (Interrupts global aus) ---
        if ((if_flag & ie & 0x1F) != 0) {
            // HALT bug triggered
            gb->cpu.halt_bug = true;
        } else {
            gb->cpu.halted = true;
        }
    }

    gb->cpu.cycles += 4;
        break;

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
        // result wird hier als temporäre variable genutzt.
        result = cpu_get_reg_by_index(gb, opcode & 0x07);
        switch ((opcode >> 3) & 0x07) 
        {
        case 0:  
            result32 = gb->cpu.a + result;

            if((uint8_t)result32 == 0)
                cpu_set_z(&gb->cpu);
            else
                cpu_clear_z(&gb->cpu);
            
            cpu_clear_n(&gb->cpu);
            
            if(((gb->cpu.a & 0x0F) + (result & 0x0F)) > 0x0F)
                cpu_set_h(&gb->cpu);
            else
                cpu_clear_h(&gb->cpu);
            
            if (result32 > 0xFF) 
                cpu_set_c(&gb->cpu); 
            else 
                cpu_clear_c(&gb->cpu);

            gb->cpu.a = (uint8_t)result32;
            break;
        case 1: 
            carry = cpu_get_c(&gb->cpu);
            result32 = gb->cpu.a + result + carry;

            if ((uint8_t)result32 == 0) 
                cpu_set_z(&gb->cpu); 
            else 
                cpu_clear_z(&gb->cpu);

            cpu_clear_n(&gb->cpu);

            if (((gb->cpu.a & 0x0F) + (result & 0x0F) + carry) > 0x0F) 
                cpu_set_h(&gb->cpu); 
            else 
                cpu_clear_h(&gb->cpu);
            
            if (result32 > 0xFF) 
                cpu_set_c(&gb->cpu); 
            else 
                cpu_clear_c(&gb->cpu);
            
            gb->cpu.a = (uint8_t)result32;
            break;
        case 2:
            result32 = gb->cpu.a - result;

            if ((uint8_t)result32 == 0) 
                cpu_set_z(&gb->cpu); 
            else 
                cpu_clear_z(&gb->cpu);
            
            cpu_set_n(&gb->cpu); // 1 bei Subtraktion
            
            if (((gb->cpu.a & 0x0F) - (result & 0x0F)) < 0) 
                cpu_set_h(&gb->cpu); 
            else 
                cpu_clear_h(&gb->cpu);

            if (result32 < 0) 
                cpu_set_c(&gb->cpu); 
            else 
                cpu_clear_c(&gb->cpu);
            
            gb->cpu.a = (uint8_t)result32;

            break;
        case 3: 
            carry = cpu_get_c(&gb->cpu);
            result32 = gb->cpu.a - result - carry;

            if ((uint8_t)result32 == 0) 
                cpu_set_z(&gb->cpu); 
            else 
                cpu_clear_z(&gb->cpu);
            
            cpu_set_n(&gb->cpu);
            
            if (((gb->cpu.a & 0x0F) - (result & 0x0F) - carry) < 0) 
                cpu_set_h(&gb->cpu); 
            else 
                cpu_clear_h(&gb->cpu);
            
            if (result32 < 0) 
                cpu_set_c(&gb->cpu); 
            else 
                cpu_clear_c(&gb->cpu);

            gb->cpu.a = (uint8_t)result32;
            break;
        case 4:
            gb->cpu.a &= result;

            if (gb->cpu.a == 0) 
                cpu_set_z(&gb->cpu); 
            else 
                cpu_clear_z(&gb->cpu);
            
            cpu_clear_n(&gb->cpu);
            cpu_set_h(&gb->cpu);
            cpu_clear_c(&gb->cpu);
            break;
        case 5:
            gb->cpu.a ^= result;

            if (gb->cpu.a == 0) 
                cpu_set_z(&gb->cpu); 
            else 
                cpu_clear_z(&gb->cpu);
            
            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);
            cpu_clear_c(&gb->cpu);
            break;
        case 6:
            gb->cpu.a |= result;

            if (gb->cpu.a == 0) 
                cpu_set_z(&gb->cpu); 
            else 
                cpu_clear_z(&gb->cpu);
            
            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);
            cpu_clear_c(&gb->cpu);
            break;
        case 7:
            result32 = gb->cpu.a - result;

            if ((uint8_t)result32 == 0) 
                cpu_set_z(&gb->cpu); 
            else 
                cpu_clear_z(&gb->cpu);
            
            cpu_set_n(&gb->cpu); // 1 bei Subtraktion
            
            if (((gb->cpu.a & 0x0F) - (result & 0x0F)) < 0) 
                cpu_set_h(&gb->cpu); 
            else 
                cpu_clear_h(&gb->cpu);

            if (result32 < 0) 
                cpu_set_c(&gb->cpu); 
            else 
                cpu_clear_c(&gb->cpu);
            
            break;
        }

        if ((opcode & 0x07) == 6)
            gb->cpu.cycles += 8;
        else 
             gb->cpu.cycles += 4;  

        break;

    /* ==================== 0xC0 - 0xFF : Jumps, Calls, Restarts ==================== */
    case 0xC0: /* RET NZ */
        if(cpu_get_z(&gb->cpu) == 0)
        {
            gb->cpu.pc = cpu_pop16(gb);
            gb->cpu.cycles += 20;
        }else
        {
            gb->cpu.cycles += 8;
        }
        break;
    case 0xC1: /* POP BC */
        cpu_set_bc(&gb->cpu, cpu_pop16(gb));
        gb->cpu.cycles += 12;
        break;
    case 0xC2: /* JP NZ, a16 */
        address = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc += 2;
        if(cpu_get_z(&gb->cpu) == 0)
        {
            gb->cpu.pc = address;
            gb->cpu.cycles += 16;
        }else
        {
            gb->cpu.cycles += 12;
        }
        break;
    case 0xC3: /* JP a16 */
        address = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc = address;
        gb->cpu.cycles += 16;
        break;
    case 0xC4: /* CALL NZ, a16 */
        address = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc += 2;
        if(cpu_get_z(&gb->cpu) == 0)
        {
            cpu_push16(gb, gb->cpu.pc);
            gb->cpu.pc = address;
            gb->cpu.cycles += 24;
        }else
        {
            gb->cpu.cycles += 12;
        }
        break;
    case 0xC5: /* PUSH BC */
        cpu_push16(gb, cpu_get_bc(&gb->cpu));
        gb->cpu.cycles += 16;
        break;
    case 0xC6: /* ADD A, n8 */
            result = memory_read(gb, gb->cpu.pc);        
            gb->cpu.pc++;

            result32 = gb->cpu.a + result;

            if((uint8_t)result32 == 0)
                cpu_set_z(&gb->cpu);
            else
                cpu_clear_z(&gb->cpu);
            
            cpu_clear_n(&gb->cpu);
            
            if(((gb->cpu.a & 0x0F) + (result & 0x0F)) > 0x0F)
                cpu_set_h(&gb->cpu);
            else
                cpu_clear_h(&gb->cpu);
            
            if (result32 > 0xFF) 
                cpu_set_c(&gb->cpu); 
            else 
                cpu_clear_c(&gb->cpu);

            gb->cpu.a = (uint8_t)result32;

            gb->cpu.cycles += 8;
        break;
    case 0xC7: /* RST 00H */
        cpu_push16(gb, gb->cpu.pc);
        gb->cpu.pc = 0x0000;
        gb->cpu.cycles += 16;
        break;
    case 0xC8: /* RET Z */
        if(cpu_get_z(&gb->cpu) == 1)
        {
            gb->cpu.pc = cpu_pop16(gb);
            gb->cpu.cycles += 20;
        }else
        {
            gb->cpu.cycles += 8;
        }
        break;
    case 0xC9: /* RET */
        gb->cpu.pc = cpu_pop16(gb);
        gb->cpu.cycles += 16;
        break;
    case 0xCA: /* JP Z, a16 */
        address = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc += 2;
        if(cpu_get_z(&gb->cpu) == 1)
        {
            gb->cpu.pc = address;
            gb->cpu.cycles += 16;
        }else
        {
            gb->cpu.cycles += 12;
        }
        break;
    case 0xCB: /* CB Prefix */
        opcode = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc++;
        cpu_decode_and_execute_cp(gb, opcode);
        break;
    case 0xCC: /* CALL Z, a16 */
        address = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc += 2;
        if(cpu_get_z(&gb->cpu) == 1)
        {
            cpu_push16(gb, gb->cpu.pc);
            gb->cpu.pc = address;
            gb->cpu.cycles += 24;
        }else
        {
            gb->cpu.cycles += 12;
        }
        break;
    case 0xCD: /* CALL a16 */
        address = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc += 2;
        cpu_push16(gb, gb->cpu.pc);
        gb->cpu.pc = address;
        gb->cpu.cycles += 24;
        break;
    case 0xCE: /* ADC A, n8 */
        result = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc++;

        carry = cpu_get_c(&gb->cpu);
        result32 = gb->cpu.a + result + carry;

        if ((uint8_t)result32 == 0) 
            cpu_set_z(&gb->cpu); 
        else 
            cpu_clear_z(&gb->cpu);

        cpu_clear_n(&gb->cpu);

        if (((gb->cpu.a & 0x0F) + (result & 0x0F) + carry) > 0x0F) 
            cpu_set_h(&gb->cpu); 
        else 
            cpu_clear_h(&gb->cpu);
            
        if (result32 > 0xFF) 
            cpu_set_c(&gb->cpu); 
        else 
            cpu_clear_c(&gb->cpu);
            
        gb->cpu.a = (uint8_t)result32;
        gb->cpu.cycles += 8;
        break;
    case 0xCF: /* RST 08H */
        cpu_push16(gb, gb->cpu.pc);
        gb->cpu.pc = 0x0008;
        gb->cpu.cycles += 16;
        break;

    case 0xD0: /* RET NC */
        if(cpu_get_c(&gb->cpu) == 0)
        {
            gb->cpu.pc = cpu_pop16(gb);
            gb->cpu.cycles += 20;
        }else
        {
            gb->cpu.cycles += 8;
        }
        break;
    case 0xD1: /* POP DE */
        cpu_set_de(&gb->cpu, cpu_pop16(gb));
        gb->cpu.cycles += 12;
        break;
    case 0xD2: /* JP NC, a16 */
        address = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc += 2;
        if(cpu_get_c(&gb->cpu) == 0)
        {
            gb->cpu.pc = address;
            gb->cpu.cycles += 16;
        }else
        {
            gb->cpu.cycles += 12;
        }
        break;
    case 0xD4: /* CALL NC, a16 */
        address = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc += 2;
        if(cpu_get_c(&gb->cpu) == 0)
        {
            cpu_push16(gb, gb->cpu.pc);
            gb->cpu.pc = address;
            gb->cpu.cycles += 24;
        }else
        {
            gb->cpu.cycles += 12;
        }
        break;
    case 0xD5: /* PUSH DE */
        cpu_push16(gb, cpu_get_de(&gb->cpu));
        gb->cpu.cycles += 16;
        break;
    case 0xD6: /* SUB n8 */
        result = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc++;
        result32 = gb->cpu.a - result;

        if ((uint8_t)result32 == 0) 
            cpu_set_z(&gb->cpu); 
        else 
            cpu_clear_z(&gb->cpu);
        
        cpu_set_n(&gb->cpu); // 1 bei Subtraktion
            
        if (((gb->cpu.a & 0x0F) - (result & 0x0F)) < 0) 
            cpu_set_h(&gb->cpu); 
        else 
            cpu_clear_h(&gb->cpu);

        if (result32 < 0) 
            cpu_set_c(&gb->cpu); 
        else 
            cpu_clear_c(&gb->cpu);
            
        gb->cpu.a = (uint8_t)result32;
        gb->cpu.cycles += 8;
        break;
    case 0xD7: /* RST 10H */
        cpu_push16(gb, gb->cpu.pc);
        gb->cpu.pc = 0x0010;
        gb->cpu.cycles += 16;
        break;
    case 0xD8: /* RET C */
        if(cpu_get_c(&gb->cpu) == 1)
        {
            gb->cpu.pc = cpu_pop16(gb);
            gb->cpu.cycles += 20;
        }else
        {
            gb->cpu.cycles += 8;
        }
        break;
    case 0xD9: /* RETI */
        gb->cpu.pc = cpu_pop16(gb);
        // enable Interrupt Flag
        gb->cpu.ime = 1;
        gb->cpu.cycles += 16;
        break;
    case 0xDA: /* JP C, a16 */
        address = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc += 2;
        if(cpu_get_c(&gb->cpu) == 1)
        {
            gb->cpu.pc = address;
            gb->cpu.cycles += 16;
        }else
        {
            gb->cpu.cycles += 12;
        }
        break;
    case 0xDC: /* CALL C, a16 */
        address = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc += 2;
        if(cpu_get_c(&gb->cpu) == 1)
        {
            cpu_push16(gb, gb->cpu.pc);
            gb->cpu.pc = address;
            gb->cpu.cycles += 24;
        }else
        {
            gb->cpu.cycles += 12;
        }
        break;
    case 0xDE: /* SBC A, n8 */
        op_value = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc += 1;

        carry = cpu_get_c(&gb->cpu);

        result32 = ((int)gb->cpu.a - (int)op_value - (int)carry);

        if(result32 < 0)
        {
            cpu_set_c(&gb->cpu);
        }else
        {
            cpu_clear_c(&gb->cpu);
        }

        cpu_set_n(&gb->cpu);

        result = (uint8_t)result32;

        cpu_check_z(&gb->cpu, result);

        if (((int)(gb->cpu.a & 0x0F) - (int)(op_value & 0x0F) - (int)carry) < 0) {
            cpu_set_h(&gb->cpu);
        } else {
            cpu_clear_h(&gb->cpu);
        }

        gb->cpu.a = result;

        gb->cpu.cycles += 8;
        break;
    case 0xDF: /* RST 18H */
        cpu_push16(gb, gb->cpu.pc);
        gb->cpu.pc = 0x0018;
        gb->cpu.cycles += 16;
        break;
    case 0xE0: /* LDH (a8), A */
        result = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc += 1;
        address = 0xff00 | result;
        memory_write(gb, address, gb->cpu.a);
        gb->cpu.cycles += 12;
        break;
    case 0xE1: /* POP HL */
        cpu_set_hl(&gb->cpu, cpu_pop16(gb));
        gb->cpu.cycles += 12;
        break;
    case 0xE2: /* LD (C), A */
        address = 0xff00 + gb->cpu.c;
        memory_write(gb, address, gb->cpu.a);
        gb->cpu.cycles += 8;
        break;
    case 0xE5: /* PUSH HL */
        cpu_push16(gb, cpu_get_hl(&gb->cpu));
        gb->cpu.cycles += 16;
        break;
    case 0xE6: /* AND n8 */
        result =  memory_read(gb, gb->cpu.pc);
        gb->cpu.pc += 1;
        result = (uint8_t)(gb->cpu.a & result);
        gb->cpu.a = result;
        if (result == 0)
        {
            cpu_set_z(&gb->cpu);
        } else
        {
            cpu_clear_z(&gb->cpu);
        }
        cpu_clear_n(&gb->cpu);
        cpu_set_h(&gb->cpu);
        cpu_clear_c(&gb->cpu);
        gb->cpu.cycles += 8;
        break;
    case 0xE7: /* RST 20H */
        cpu_push16(gb, gb->cpu.pc);
        gb->cpu.pc = 0x0020;
        gb->cpu.cycles += 16;
        break;
    case 0xE8: /* ADD SP, r8 */
        offset = (int8_t)memory_read(gb, gb->cpu.pc);
        gb->cpu.pc += 1;

        op_value = (uint8_t)(gb->cpu.sp & 0xFF);
        u8Val = (uint8_t)offset;

        //Half Carry
        if (((op_value & 0x0F) + (u8Val & 0x0F)) > 0x0F) {
            cpu_set_h(&gb->cpu);
        } else {
            cpu_clear_h(&gb->cpu);
        }

        //Carry
        if (((int)op_value + (int)u8Val) > 0xFF) {
            cpu_set_c(&gb->cpu);
        } else {
            cpu_clear_c(&gb->cpu);
        }

        cpu_clear_z(&gb->cpu);
        cpu_clear_n(&gb->cpu);

        gb->cpu.sp = (uint16_t)((int32_t)gb->cpu.sp + offset);

        gb->cpu.cycles += 16;
        break;
    case 0xE9: /* JP HL */
        address = cpu_get_hl(&gb->cpu);
        gb->cpu.pc = address;
        gb->cpu.cycles += 4;
        break;
    case 0xEA: /* LD (a16), A */
        address = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc += 2;
        memory_write(gb, address, gb->cpu.a);
        gb->cpu.cycles += 16;
        break;
    case 0xEE: /* XOR n8 */
        result =  memory_read(gb, gb->cpu.pc);
        gb->cpu.pc += 1;
        result = (uint8_t)(gb->cpu.a ^ result);
        gb->cpu.a = result;
        if (result == 0)
        {
            cpu_set_z(&gb->cpu);
        } else
        {
            cpu_clear_z(&gb->cpu);
        }
        cpu_clear_n(&gb->cpu);
        cpu_clear_h(&gb->cpu);
        cpu_clear_c(&gb->cpu);
        gb->cpu.cycles += 8;
        break;
    case 0xEF: /* RST 28H */
        cpu_push16(gb, gb->cpu.pc);
        gb->cpu.pc = 0x0028;
        gb->cpu.cycles += 16;
        break;
    case 0xF0: /* LDH A, (a8) */
        result = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc += 1;
        address = 0xff00 | result;
        gb->cpu.a = memory_read(gb, address);
        gb->cpu.cycles += 12;
        break;
    case 0xF1: /* POP AF */
        cpu_set_af(&gb->cpu, cpu_pop16(gb));
        gb->cpu.cycles += 12;
        break;
    case 0xF2: /* LD A, (C) */
        address = 0xff00 + gb->cpu.c;
        gb->cpu.a = memory_read(gb, address);
        gb->cpu.cycles += 8;
        break;
    case 0xF3: /* DI */
        gb->cpu.ime = 0;
        gb->cpu.cycles += 4;
        break;
    case 0xF5: /* PUSH AF */
        cpu_push16(gb, cpu_get_af(&gb->cpu));
        gb->cpu.cycles += 16;
        break;
    case 0xF6: /* OR n8 */
        result =  memory_read(gb, gb->cpu.pc);
        gb->cpu.pc += 1;
        result = (uint8_t)(gb->cpu.a | result);
        gb->cpu.a = result;
        if (result == 0)
        {
            cpu_set_z(&gb->cpu);
        } else
        {
            cpu_clear_z(&gb->cpu);
        }
        cpu_clear_n(&gb->cpu);
        cpu_clear_h(&gb->cpu);
        cpu_clear_c(&gb->cpu);
        gb->cpu.cycles += 8;
        break;
    case 0xF7: /* RST 30H */
        cpu_push16(gb, gb->cpu.pc);
        gb->cpu.pc = 0x0030;
        gb->cpu.cycles += 16;
        break;
    case 0xF8: /* LD HL, SP+r8 */
        offset = (int8_t)memory_read(gb, gb->cpu.pc);
        gb->cpu.pc += 1;
        u16Val = (uint16_t)(gb->cpu.sp + offset);
        cpu_set_hl(&gb->cpu, u16Val);

        //Half Carry
        if ((((uint8_t)offset & 0x0F) + ((uint8_t)gb->cpu.sp & 0x0F)) > 0x0F) {
            cpu_set_h(&gb->cpu);
        } else {
            cpu_clear_h(&gb->cpu);
        }

        //Carry
        if (((uint8_t)offset + (uint8_t)gb->cpu.sp) > 0xFF) {
            cpu_set_c(&gb->cpu);
        } else {
            cpu_clear_c(&gb->cpu);
        }

        cpu_clear_z(&gb->cpu);
        cpu_clear_n(&gb->cpu);

        gb->cpu.cycles += 12;
        break;
    case 0xF9: /* LD SP, HL */
        gb->cpu.sp = cpu_get_hl(&gb->cpu);
        gb->cpu.cycles += 8;
        break;
    case 0xFA: /* LD A, (a16) */
        address = memory_read16(gb, gb->cpu.pc);
        gb->cpu.pc += 2;
        gb->cpu.a = memory_read(gb, address);
        gb->cpu.cycles += 16;
        break;
    case 0xFB: /* EI */
        // Interupt Enables after the following instruction.
        gb->cpu.ei_delay = 2;
        gb->cpu.cycles += 4;
        break;
    case 0xFE: /* CP n8 */
        op_value = memory_read(gb, gb->cpu.pc);
        gb->cpu.pc += 1;
        result = gb->cpu.a - op_value;

        if (result == 0)
        {
            cpu_set_z(&gb->cpu);
        }else
        {
            cpu_clear_z(&gb->cpu);
        }
        
        cpu_set_n(&gb->cpu);

        if ((gb->cpu.a & 0x0F) < (op_value & 0x0F)) {
            cpu_set_h(&gb->cpu);
        } else {
            cpu_clear_h(&gb->cpu);
        }
        
        if(op_value > gb->cpu.a)
        {
            cpu_set_c(&gb->cpu);
        }else
        {
            cpu_clear_c(&gb->cpu);
        }
        gb->cpu.cycles += 8;
        break;
    case 0xFF: /* RST 38H */
        cpu_push16(gb, gb->cpu.pc);
        gb->cpu.pc = 0x0038;
        gb->cpu.cycles += 16;
        break;
    
    }
    
    // delay for EI
    if(gb->cpu.ei_delay > 0)
    {
        if(gb->cpu.ei_delay == 1)
        {
            gb->cpu.ime = 1;
        }
        gb->cpu.ei_delay--;
    }

}

// Prefix Op-Codes
void cpu_decode_and_execute_cp(GameBoy* gb, uint8_t opcode)
{
    uint8_t u8Val = 0;
    uint8_t u8Result = 0;
    uint16_t u16Val = 0;
    uint16_t address = 0;

    switch(opcode)
    {
        case 0x00: /* RLC B */
            u8Val = gb->cpu.b;
            gb->cpu.b = (uint8_t)(u8Val << 1U);
            
            if(u8Val > 0x7F)
            {
                gb->cpu.b++;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.b == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x01: /* RLC C */
            u8Val = gb->cpu.c;
            gb->cpu.c = (uint8_t)(u8Val << 1U);
            
            if(u8Val > 0x7F)
            {
                gb->cpu.c++;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.c == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x02: /* RLC D */
            u8Val = gb->cpu.d;
            gb->cpu.d = (uint8_t)(u8Val << 1U);
            
            if(u8Val > 0x7F)
            {
                gb->cpu.d++;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.d == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x03: /* RLC E */
            u8Val = gb->cpu.e;
            gb->cpu.e = (uint8_t)(u8Val << 1U);
            
            if(u8Val > 0x7F)
            {
                gb->cpu.e++;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.e == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x04: /* RLC H */
            u8Val = gb->cpu.h;
            gb->cpu.h = (uint8_t)(u8Val << 1U);
            
            if(u8Val > 0x7F)
            {
                gb->cpu.h++;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.h == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x05: /* RLC L */
            u8Val = gb->cpu.l;
            gb->cpu.l = (uint8_t)(u8Val << 1U);
            
            if(u8Val > 0x7F)
            {
                gb->cpu.l++;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.l == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x06: /* RLC (HL) */
            address = cpu_get_hl(&gb->cpu);
            u8Val = memory_read(gb, address);
            u8Result = (uint8_t)(u8Val << 1U);
            
            if(u8Val > 0x7F)
            {
                u8Result++;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            memory_write(gb, address, u8Result);

            gb->cpu.cycles += 16;
            break;
        case 0x07: /* RLC A */
            u8Val = gb->cpu.a;
            gb->cpu.a = (uint8_t)(u8Val << 1U);
            
            if(u8Val > 0x7F)
            {
                gb->cpu.a++;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.a == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x08: /* RRC B */
            u8Val = gb->cpu.b;
            gb->cpu.b = (uint8_t)(u8Val >> 1U);
            
            if(u8Val & 0x01)
            {
                gb->cpu.b |= 0x80;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.b == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x09: /* RRC C */
            u8Val = gb->cpu.c;
            gb->cpu.c = (uint8_t)(u8Val >> 1U);
            
            if(u8Val & 0x01)
            {
                gb->cpu.c |= 0x80;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.c == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x0A: /* RRC D */
            u8Val = gb->cpu.d;
            gb->cpu.d = (uint8_t)(u8Val >> 1U);
            
            if(u8Val & 0x01)
            {
                gb->cpu.d |= 0x80;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.d == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x0B: /* RRC E */
            u8Val = gb->cpu.e;
            gb->cpu.e = (uint8_t)(u8Val >> 1U);
            
            if(u8Val & 0x01)
            {
                gb->cpu.e |= 0x80;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.e == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x0C: /* RRC H */
            u8Val = gb->cpu.h;
            gb->cpu.h = (uint8_t)(u8Val >> 1U);
            
            if(u8Val & 0x01)
            {
                gb->cpu.h |= 0x80;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.h == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x0D: /* RRC L */
            u8Val = gb->cpu.l;
            gb->cpu.l = (uint8_t)(u8Val >> 1U);
            
            if(u8Val & 0x01)
            {
                gb->cpu.l |= 0x80;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.l == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x0E: /* RRC (HL) */
            address = cpu_get_hl(&gb->cpu);
            u8Val = memory_read(gb, address);
            u8Result = (uint8_t)(u8Val >> 1U);
            
            if(u8Val & 0x01)
            {
                u8Result |= 0x80;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            memory_write(gb, address, u8Result);

            gb->cpu.cycles += 16;
            break;
        case 0x0F: /* RRC A */
            u8Val = gb->cpu.a;
            gb->cpu.a = (uint8_t)(u8Val >> 1U);
            
            if(u8Val & 0x01)
            {
                gb->cpu.a |= 0x80;
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            if(gb->cpu.a == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_n(&gb->cpu);
            cpu_clear_h(&gb->cpu);

            gb->cpu.cycles += 8;
            break;
        case 0x10: /* RL B */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.b;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result << 1U) | u8Val);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.b = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x11: /* RL C */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.c;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result << 1U) | u8Val);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.c = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x12: /* RL D */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.d;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result << 1U) | u8Val);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.d = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x13: /* RL E */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.e;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result << 1U) | u8Val);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.e = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x14: /* RL H */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.h;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result << 1U) | u8Val);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.h = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x15: /* RL L */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.l;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result << 1U) | u8Val);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.l = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x16: /* RL (HL) */
            address = cpu_get_hl(&gb->cpu);
            u8Val = cpu_get_c(&gb->cpu);
            
            u8Result = memory_read(gb, address);

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result << 1U) | u8Val);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            memory_write(gb, address, u8Result);
            gb->cpu.cycles += 16;
            break;
        case 0x17: /* RL A */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.a;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result << 1U) | u8Val);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.a = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x18: /* RR B */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.b;

            if(u8Result & 0x01)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result >> 1U) | (u8Val << 7U));

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.b = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x19: /* RR C */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.c;

            if(u8Result & 0x01)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result >> 1U) | (u8Val << 7U));

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.c = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x1A: /* RR D */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.d;

            if(u8Result & 0x01)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result >> 1U) | (u8Val << 7U));

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.d = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x1B: /* RR E */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.e;

            if(u8Result & 0x01)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result >> 1U) | (u8Val << 7U));

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.e = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x1C: /* RR H */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.h;

            if(u8Result & 0x01)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result >> 1U) | (u8Val << 7U));

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.h = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x1D: /* RR L */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.l;

            if(u8Result & 0x01)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result >> 1U) | (u8Val << 7U ));

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.l = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x1E: /* RR (HL) */
            address = cpu_get_hl(&gb->cpu);
            u8Val = cpu_get_c(&gb->cpu);
            
            u8Result = memory_read(gb, address);

            if(u8Result & 0x01)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result >> 1U) | (u8Val << 7U));

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            memory_write(gb, address, u8Result);
            gb->cpu.cycles += 16;
            break;
        case 0x1F: /* RR A */
            u8Val = cpu_get_c(&gb->cpu);
            u8Result = gb->cpu.a;

            if(u8Result & 0x01)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = ((u8Result >> 1U) | (u8Val << 7U));

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.a = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x20: /* SLA B */
            u8Result = gb->cpu.b;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = (u8Result << 1U);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.b = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x21: /* SLA C */
            u8Result = gb->cpu.c;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = (u8Result << 1U);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.c = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x22: /* SLA D */
            u8Result = gb->cpu.d;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = (u8Result << 1U);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.d = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x23: /* SLA E */
            u8Result = gb->cpu.e;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = (u8Result << 1U);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.e = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x24: /* SLA H */
            u8Result = gb->cpu.h;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = (u8Result << 1U);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.h = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x25: /* SLA L */
            u8Result = gb->cpu.l;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = (u8Result << 1U);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.l = u8Result;
            gb->cpu.cycles += 8;
            break;
        case 0x26: /* SLA (HL) */
            address = cpu_get_hl(&gb->cpu);
            
            u8Result = memory_read(gb, address);

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = (u8Result << 1U);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            memory_write(gb, address, u8Result);
            gb->cpu.cycles += 16;
            break;
        case 0x27: /* SLA A */
            u8Result = gb->cpu.a;

            if(u8Result & 0x80)
            {
                cpu_set_c(&gb->cpu);
            }else
            {
                cpu_clear_c(&gb->cpu);
            }

            u8Result = (u8Result << 1U);

            if(u8Result == 0)
            {
                cpu_set_z(&gb->cpu);
            }else
            {
                cpu_clear_z(&gb->cpu);
            }

            cpu_clear_h(&gb->cpu);
            cpu_clear_n(&gb->cpu);

            gb->cpu.a = u8Result;
            gb->cpu.cycles += 8;
            break;
    }
}