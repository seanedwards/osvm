#!/usr/bin/python
import argparse, string

parser = argparse.ArgumentParser(description="Assemble PBRAIN programs.")
parser.add_argument('input', nargs='+', help='The file to assemble')
args = parser.parse_args()

labels = {}
labellines = {}

def int_name(x):
    return {
        "print": 0,
        "getpid": 1,
        "send_msg": 2,
        "recv_msg": 3,
        "recv_any": 4,
    }[x.lower()]

def resolve(x):
    if x[:1] == ':':
        return labels[x]
    return int(x)

def op_lpi(line): return "00P%01d%02d" % (resolve(line[1]), resolve(line[2]))
def op_api(line): return "01P%01d%02d" % (resolve(line[1]), resolve(line[2]))
def op_api(line): return "02P%01d%02d" % (resolve(line[1]), resolve(line[2]))

def op_acloadi(line): return "03%04d" % resolve(line[1])
def op_acloadr(line): return "04P%01d99" % resolve(line[1])
def op_acloadd(line): return "05%02d99" % resolve(line[1])
def op_acstorr(line): return "06P%01d99" % resolve(line[1])
def op_acstord(line): return "07%02d99" % resolve(line[1])

def op_rstorr(line): return "08R%01dP%01d" % (resolve(line[1]), resolve(line[2]))
def op_rstord(line): return "09R%01d%02d" % (resolve(line[1]), resolve(line[2]))
def op_rloadr(line): return "10R%01dP%01d" % (resolve(line[1]), resolve(line[2]))
def op_rloadd(line): return "11R%01d%02d" % (resolve(line[1]), resolve(line[2]))

def op_addi(line): return "12%04d" % resolve(line[1])
def op_addr(line): return "14R%01d99" % resolve(line[1])
def op_addmr(line): return "16P%01d99" % resolve(line[1])
def op_addmd(line): return "17%02d99" % resolve(line[1])

def op_subi(line): return "13%04d" % resolve(line[1])
def op_subr(line): return "15R%01d99" % resolve(line[1])
def op_submr(line): return "18P%01d99" % resolve(line[1])
def op_submd(line): return "19%02d99" % resolve(line[1])

def op_muli(line): return "50%04d" % resolve(line[1])
def op_mulr(line): return "51R%01d99" % resolve(line[1])
def op_mulmr(line): return "52P%01d99" % resolve(line[1])
def op_mulmd(line): return "53%02d99" % resolve(line[1])

def op_divi(line): return "54%04d" % resolve(line[1])
def op_divr(line): return "55R%01d99" % resolve(line[1])
def op_divmr(line): return "56P%01d99" % resolve(line[1])
def op_divmd(line): return "57%02d99" % resolve(line[1])

def op_cmpeqr(line): return "20R%01d99" % resolve(line[1])
def op_cmpeqi(line): return "22%04d" % resolve(line[1])
def op_cmpltr(line): return "21R%01d99" % resolve(line[1])
def op_cmplti(line): return "23%04d" % resolve(line[1])

def op_brc(line): return "24%02d99" % resolve(line[1])
def op_brf(line): return "25%02d99" % resolve(line[1])
def op_jmp(line): return "26%02d99" % resolve(line[1])

def op_halt(line): return "989999"
def op_int(line): return "27%02d99" % int_name(line[1])

def op_push(line): return "28%02d99" % resolve(line[1])
def op_pop(line): return "29%02d99" % resolve(line[1])

def op_lar(line): return "30R%01d99" % resolve(line[1])
def op_sar(line): return "31R%01d99" % resolve(line[1])

def op_setmr(line): return "609999"
def op_readmr(line): return "619999"

# Extensions
def op_dbgbrk(line): return "909999"
def op_printchr(line): return "919999"
def op_printnum(line): return "929999"
def op_noop(line): return "999999"
def op_dat(line): return "99%04d" % resolve(line[1])

for fname in args.input:
    file = open(fname)
    contents = file.read()
    file.close()

    instructions = []
    addr = 0

    lines = contents.split('\n');
    for line in lines: # Build labels
        line = line.strip()
        if not line or line[:1] == '#': # Skip empty lines and comments
            continue
        if (line[:1] == ':'): # Lines that start with : are lables
            labels[line] = addr
            labellines[addr] = line
            continue
        
        # Every other line is an instruction.
        addr = addr + 1

    addr = 0
    for line in lines:
        line = line.strip()
        if not line or line[:1] == ':' or line[:1] == '#': # Skip, label and comment lines
            continue

        linesegs = line.split(" ")
        func = locals()["op_%s" % linesegs[0].lower()]
        
        labeldef = labellines.get(addr, "")
    #   if labeldef:
    #    print "# %s" % labeldef
    #    print "%s # %s" % (func(linesegs), line)
        print func(linesegs)

        addr = addr + 1

