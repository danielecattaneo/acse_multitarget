#!/usr/bin/env python3

#
# MACE instruction verification program test data generator
# 2020 Daniele Cattaneo, Politecnico di Milano
#

from numpy import uint32
from numpy import int32
from itertools import *


INDENT = '      '


def main():
    print("TestTernOrBinData:")
    gen_bin_ter_tests()
    print()
    print("TestBranchAndCondData:")
    gen_branch_cond_tests()


def gen_bin_ter_tests():
    sets = [
        (
            [
                (10, 20),
                (10, -20),
                (-10, 20),
                (-10, -20),
                (0x40000000, 0x40000000),
                (-0x40000000, 0x40000000),
                (0x40000000, -0x40000000),
                (-0x40000000, -0x40000000),
                (0x7FFFFFFF, 0),
                (0x7FFFFFFF, 1),
                (0x7FFFFFFF, -1),
                (0x7FFFFFFF, 0x7FFFFFFF),
                (0x7FFFFFFF, -0x80000000),
                (-0x80000000, 0),
                (-0x80000000, 1),
                (-0x80000000, -1),
                (-0x80000000, -0x80000000),
                (-0x80000000, 0x7FFFFFFF),
            ],
            [
                fun_add,
                fun_sub,
            ],
            6
        ),
        (
            [
                (0, 0),
                (0, 1),
                (1, 0),
                (1, 1),
                (0, 12345678),
                (12345678, 0),
                (12345678, 12345678),
                (0, -12345678),
                (-12345678, 0),
                (-12345678, -12345678)
            ],
            [
                fun_andl,
                fun_orl,
                fun_eorl,
            ],
            6
        ),
        (
            [
                (0x55555555, 0x55555555),
                (0x55555555, 0xAAAAAAAA),
                (0xAAAAAAAA, 0x55555555),
                (0xAAAAAAAA, 0xAAAAAAAA)
            ],
            [
                fun_andb,
                fun_orb,
                fun_eorb,
            ],
            6
        ),
        (
            [
                (0, 0),
                (0, -1),
                (-1, 0),
                (111111111, 8),
                (-111111111, 8),
                (0x7FFFFFFF, 1),
                (-0x80000000, 1),
                (-0x80000000, -1),
                (1, 0x7FFFFFFF),
                (1, -0x80000000),
                (0x8001, 0x7FFF),
                (0x20001, 0x7FFF),
            ],
            [
                fun_mul,
            ],
            6
        ),
        (
            [
                (0, -1),
                (888888888, 8),
                (-888888888, 8),
                (-0x80000000, 2),
                (0x40000000, 2),
                (-0x80000000, -1),
                (1234567890, 5654),
                (-1234567890, 5654),
                (1234567890, -5654),
                (-1234567890, -5654),
                (-0x80000000, 0x7FFFFFFF),
            ],
            [
                fun_div,
            ],
            6
        ),
        (
            [
                (0, 0),
                (0x55555555, 0),
                (-0x55555556, 0),  # 0xAAAAAAAA
                (-1, 0),
                (0, 1),
                (0x55555555, 1),
                (-0x55555556, 1),
                (-1, 1),
                (0, 31),
                (0x55555555, 31),
                (-0x55555556, 31),
                (-1, 31),
            ],
            [
                fun_shr, fun_shl
            ],
            6
        ),
        (
            [
                (0, 0),
                (0xABCDEF97, 0),
                (0x79FEDCBA, 0),
                (-1, 0),
                (0, 4),
                (0xABCDEF97, 4),
                (0x79FEDCBA, 4),
                (-1, 4),
                (0, 16),
                (0xABCDEF97, 16),
                (0x79FEDCBA, 16),
                (-1, 16),
                (0, 31),
                (0xABCDEF97, 31),
                (0x79FEDCBA, 31),
                (-1, 31),
            ],
            [
                fun_rotl, fun_rotr
            ],
            6
        ),
        (
            [
                (0, 0),
                (0, 1),
                (0, 0x7FFFFFFF),
                (0, -0x80000000),
                (0, -0x7FFFFFFF),
                (0, -1),
            ],
            [
                fun_neg
            ],
            5
        ),
        (
            [
                (0, 0),
                (1, 0),
                (0x7FFFFFFF, 0),
                (-0x80000000, 0),
                (-0x7FFFFFFF, 0),
                (-1, 0),
            ],
            [
                fun_notl
            ],
            2
        ),
        (
            [
                (0, 0),
                (1, 0),
                (0x55555555, 0),
                (0x7FFFFFFF, 0),
                (-0x80000000, 0),
                (-0x7FFFFFFF, 0),
                (-0x55555556, 0),
                (-1, 0),
            ],
            [
                fun_notb
            ],
            2
        )
    ]

    expected_behavior = []

    for set_regs, set_funcs, num_variants in sets:
        header = len(set_regs) + (num_variants << 8) + (len(set_funcs) << 16)
        print_words([header])
        flags = []
        for (rs1, rs2), i in zip(set_regs, count()):
            words = [rs1, rs2]
            print_words(words, 'input ' + str(i))
        for fun in set_funcs:
            words = []
            for rs1, rs2 in set_regs:
                rd, n, z, v, c = fun(rs1, rs2)
                expected_behavior += \
                    [(mnemonic(fun), hex(rs1), hex(rs2), '-', '-', '-', '-', 
                    hex(rd), int(n), int(z), int(v), int(c))]
                words += [rd]
                flags += [enc_flags(n, z, v, c)]
            print_words(words, 'outputs of ' + mnemonic(fun))
            
        packed_flags = pack_bytes(flags, 4)
        print_words(packed_flags, 'expected flags')
        print()
        
    print_words([0])
    print()
    print_expectations(expected_behavior)
    
    
def gen_branch_cond_tests():
    branches_cc = [
        cc_eq, cc_ge, cc_t, cc_f, cc_hi, cc_ls, cc_gt, cc_le, cc_lt, cc_ne,
        cc_cc, cc_cs, cc_vc, cc_vs, cc_pl, cc_mi ]
    set_cc = [
        cc_eq, cc_ge, cc_gt, cc_le, cc_lt, cc_ne ]
    flags_out = []
    rd_out = []
    expected_behavior = []
    
    print_words([(len(branches_cc) + len(set_cc)) * 16], "number of tests")
    
    for fun in branches_cc:
        for psw in range(16):
            n, z, v, c = decode_flags(psw)
            out = fun(n, z, v, c)
            flags_out += [psw]
            rd_out += [1 if out else 0]
            expected_behavior += [
                ("b"+mnemonic(fun), "-", "-", int(n), int(z), int(v), int(c), 
                hex(1 if out else 0), int(n), int(z), int(v), int(c))]
    
    for fun in set_cc:
        for psw in range(16):
            n, z, v, c = decode_flags(psw)
            out = fun(n, z, v, c)
            flags_out += [enc_flags(0, out == 0, 0, 0)]
            rd_out += [1 if out else 0]
            expected_behavior += [
                ("s"+mnemonic(fun), "-", "-", int(n), int(z), int(v), int(c), 
                hex(1 if out else 0), 0, int(out == 0), 0, 0)]
            
    print_words(pack_bytes(flags_out, 4), "flags")
    print_words(pack_bytes(rd_out, 1), "rd or branch taken")
    
    print()
    print_expectations(expected_behavior)


def pack_bytes(v, bit_width):
    res = []
    for i, shift in zip(v, map(lambda x: x % 32, count(0, bit_width))):
        if shift == 0:
            res += [0]
        res[-1] |= i << shift
    return res


def print_expectations(table):
    print(INDENT + '/*   Expected Behavior:')
    new_table = [ \
        ("Instr.", "Inputs:", "", "", "", "", "", "Outputs:", "", "", "", ""),
        ("", "RS1", "RS2", "N", "Z", "V", "C", "RD", "N", "Z", "V", "C")] \
        + table
    for func, rs1, rs2, in_n, in_z, in_v, in_c, rd, n, z, v, c in new_table:
        print(INDENT + ' * %8s %11s %11s  %1s %1s %1s %1s  %11s  %1s %1s %1s %1s' %
              (func, rs1, rs2, in_n, in_z, in_v, in_c, rd, n, z, v, c))
    print(INDENT + ' */')


def print_words(it, comment=None):
    if len(it) == 0:
        return
    tmp = INDENT + '.WORD'
    for i, ii in zip(it, count()):
        if ii % 4 == 0 and ii != 0:
            tmp += '\n' + INDENT + '.WORD'
        tmp += ' %11s' % hex(i)
    if comment:
        if len(tmp) + len(comment) + len(' /*  */') > 80:
            tmp = INDENT + '/* ' + comment + ' */\n' + tmp
        else:
            tmp += ' /* ' + comment + ' */'
    print(tmp)


def enc_flags(n, z, v, c):
    return (0b1000 if n else 0) + (0b0100 if z else 0) + (0b0010 if v else 0) + (0b0001 if c else 0)
    

def decode_flags(flags):
    return bool(flags & 0b1000), bool(flags & 0b0100), bool(flags & 0b0010), bool(flags & 0b0001)


def fun_add(rs1, rs2):
    dest = int32(rs1 + rs2)
    return dest, dest < 0, dest == 0, rs1+rs2 != dest, (int(uint32(rs1))+int(uint32(rs2)) & 0x100000000) != 0


def fun_sub(rs1, rs2):
    dest = int32(rs1 - rs2)
    return dest, dest < 0, dest == 0, rs1-rs2 != dest, (int(uint32(rs1))-int(uint32(rs2)) & 0x100000000) != 0


def fun_andl(rs1, rs2):
    dest = int32((rs1 != 0) and (rs2 != 0))
    return dest, dest < 0, dest == 0, 0, 0


def fun_orl(rs1, rs2):
    dest = int32((rs1 != 0) or (rs2 != 0))
    return dest, dest < 0, dest == 0, 0, 0


def fun_eorl(rs1, rs2):
    dest = int32((rs1 != 0) != (rs2 != 0))
    return dest, dest < 0, dest == 0, 0, 0


def fun_andb(rs1, rs2):
    dest = int32(rs1) & int32(rs2)
    return dest, dest < 0, dest == 0, 0, 0


def fun_orb(rs1, rs2):
    dest = int32(rs1) | int32(rs2)
    return dest, dest < 0, dest == 0, 0, 0


def fun_eorb(rs1, rs2):
    dest = int32(rs1) ^ int32(rs2)
    return dest, dest < 0, dest == 0, 0, 0


def fun_mul(rs1, rs2):
    dest = int32(int32(rs1) * int32(rs2))
    return dest, dest < 0, dest == 0, dest != rs1*rs2, 0


def fun_div(rs1, rs2):
    # round towards zero!
    if (rs1 < 0) == (rs2 < 0):
        arbdest = rs1 // rs2
    else:
        arbdest = -(-rs1 // rs2) if rs1 < 0 else -(rs1 // -rs2)
    dest = int32(arbdest)
    return dest, dest < 0, dest == 0, dest != arbdest, 0


def fun_shr(rs1, rs2):
    dest = int32(int32(rs1) >> int32(rs2))
    return dest, dest < 0, dest == 0, 0, (int32(rs1) & (1 << rs2-1) != 0) if rs2 > 0 else 0


def fun_shl(rs1, rs2):
    dest = int32(int32(rs1) << int32(rs2))
    return dest, dest < 0, dest == 0, 0, (int32(rs1) & (1 << 32-rs2) != 0) if rs2 > 0 else 0


def fun_rotl(rs1, rs2):
    dest = uint32(uint32(rs1) << uint32(rs2)) + \
        uint32(uint32(rs1) >> uint32(32-rs2))
    return int32(dest), int32(dest) < 0, dest == 0, 0, dest & 1 if rs2 > 0 else 0


def fun_rotr(rs1, rs2):
    dest = uint32(uint32(rs1) << uint32(32-rs2)) + \
        uint32(uint32(rs1) >> uint32(rs2))
    return int32(dest), int32(dest) < 0, dest == 0, 0, 1 if (dest & 0x80000000) and (rs2 > 0) else 0


def fun_neg(rs1, rs2):
    return fun_sub(0, rs2)


def fun_notl(rs1, rs2):
    dest = 1 if rs1 == 0 else 0
    return dest, 0, dest == 0, 0, 0


def fun_notb(rs1, rs2):
    dest = ~uint32(rs1)
    return dest, int32(dest) < 0, dest == 0, 0, 0
    
    
def cc_eq(n, z, v, c):
    return z
    

def cc_ge(n, z, v, c):
    return (n and v) or ((not n) and (not v))
    

def cc_t(n, z, v, c):
    return True
    
    
def cc_f(n, z, v, c):
    return False
    
    
def cc_hi(n, z, v, c):
    return (not c) and (not z)
    

def cc_ls(n, z, v, c):
    return c or z       # ACSE Toolchain Manual (2008-12-23 ver.) is incorrect
    

def cc_gt(n, z, v, c):
    return (n and v and (not z)) or ((not n) and (not v) and (not z))
    
    
def cc_le(n, z, v, c):
    return z or (n and (not v)) or ((not n) and v)
    
    
def cc_lt(n, z, v, c):
    return ((not n) and v) or (n and (not v))
    

def cc_ne(n, z, v, c):
    return not z
    

def cc_cc(n, z, v, c):
    return not c
    
    
def cc_cs(n, z, v, c):
    return c
    
    
def cc_vc(n, z, v, c):
    return not v
    

def cc_vs(n, z, v, c):
    return v
    

def cc_pl(n, z, v, c):
    return not n
    
    
def cc_mi(n, z, v, c):
    return n


def mnemonic(fun):
    return fun.__name__.split('_')[1]


if __name__ == "__main__":
    main()
