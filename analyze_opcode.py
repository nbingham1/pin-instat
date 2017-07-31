#!/usr/bin/python

categories = {
	'Control Jump': [
		'jz', 'jp', 'js', 'jb',
		'jnle', 'jns', 'jl', 'jnp',
		'jnz', 'jnb', 'jnl', 'jmp',
		'jle', 'jbe', 'jnbe', 
	],

	'Control Other': [		
		'call_near', 'syscall', 'ret_near',	# call/ret

		'fwait', 'fnclex', # floating point exceptions
		'leave', 'sfence', 'prefetchnta', 'cpuid', 'ud2', 'prefetcht0', 'rdtsc', 'cld',
		'vldmxcsr', 'vstmxcsr', 'ldmxcsr', 'stmxcsr', # load/store flag register 
		'pmovmskb', 'vpmovmskb', # move mask
		'xgetbv', # get the value of an extended control register
		'lea', # load effective address 
	],

	'Routing Real': [
		'movapd', 'movhpd', 'movlpd', 'vmovapd', 'vmovsd', 'movsd_xmm', # double precision move
		'movss', 'movaps', 'movups', # single precision move
		'fxch', # swap
		'fnstcw', # store fpu control word
		'fst', 'fstp', 'fld', 'fldz', 'fld1', 'fldcw', # load/store floating point value
		'cdq', 'cdqe', 'cwde', 'cqo', 'fild', 'cvtss2sd', 'vcvtsi2sd', 'cvtpd2ps', 'cvttss2si', 'cvttsd2si', 'vcvttsd2si', 'cvtsi2ss', 'cvtsd2ss', 'cvtsi2sd', # floating point/integer conversion
		'vunpcklpd', 'unpcklpd', # unpack and interleave
		'shufps', # shuffle 
		'movmskpd', # move mask
	],

	'Routing Integer': [
		'rep_movsd', 'rep_movsq', # repeated move
		'rep_stosb', 'rep_stosd', 'rep_stosq', # repeated store 
		'cmovs', 'cmovz', 'cmovl', 'cmovb', 'cmovbe', 'cmovle', 'cmovnl', 'cmovnb', 'cmovnz', 'cmovns', 'cmovnbe', 'cmovnle', # conditional move
		'cmpxchg_lock', 'cmpxchg', # conditional swap
		'nop',  # no-op
		'movzx', 'movsx', 'movsxd', # move with zero/sign extend 
		'mov', 'movd', 'movq', 'vmovd', 'vmovq', 
		'movdqu', 'movdqa', 'vmovdqa', 'vmovdqu', # mov
		'movnti', # store using non-temporal hit
		'lddqu',  # load/store integer value
		'punpcklqdq', 'punpcklwd', 'punpcklbw', # unpack and interleave
		'pshufb', 'pshufd', # shuffle bytes
		'xchg', 'bswap', # swap
		'palignr', 'vpalignr', 
		'push', 'pop', # push to and pop from the stack 
	],

	'Arithmetic Real': [
		'minss', 'minsd', # min
		'maxss', 'maxsd', # max 
		'cmpss', 'vcmpsd', 'cmpsd_xmm',
		'fucomi','fucomip', 
		'ucomiss', 'ucomisd', 'vucomisd', # unordered compare

		'addss', 'addpd', 'vaddsd', 'addsd',  # add 
		'fsub', 'fsubp', 'fsubr', 'fsubrp', 'subss', 'vsubsd', 'subsd',  # subtract
		'fmul', 'fmulp', 'mulsd', 'mulss', 'vmulsd', 'mulpd', # multiply
		'fdiv', 'fdivp', 'fdivrp', 'vdivsd', 'divsd', 'divss',  # divide
		'roundss', 'roundsd', # round 
		'sqrtss', 'sqrtsd', # square root 
		'andps', 'andnps', 'andpd', 'vandpd', 'vandnpd', 'andnpd', # and
		'orps', 'orpd', 'vorpd', # or 
		'xorps', 'xorpd', 'vxorpd', # xor
		'fchs', # neg
	],

	'Arithmetic Integer': [
		'pminub', # min/max
		'cmp', 
		'vpcmpistri', 'pcmpistri', # compare strings 
		'pcmpgtb', 'vpcmpgtb', # compare greater than 
		'vpcmpeqb', 'pcmpeqd', 'pcmpeqb', # compare equal
		'pmaxub', # simd compare 
		'repne_scasb', 'repe_cmpsb', # repeated compare
		'bsr', 'bsf', 'tzcnt', # bit scan 
		'setz', 'setle', 'setnle', 'setb', 'setl', 'setp', 'setnl', 'setnb', 'setnp', 'setnbe', 'setbe', 'setnz', # set bit
		'bt', 'bts', # test bit
		'ptest', 'test', # boolean and then check 0

		'shl', 'shr', 'shld', 'shrd', # shift single double word
		'vpslldq', 'psrldq', 'pslldq', # shift quadword
		'rol', 'ror', 'sar',  # rotate
		'add', 'adc', 'add_lock', 'xadd_lock', # add
		'sub', 'sbb', 'psubb', 'vpsubb', 'sub_lock', # subtract
		'mul', 'imul', # multiply
		'idiv', 'div', # divide
		'and', 'pand', 'vpand', 'vpandn', # binary and 
		'xor', 'pxor', 'vpxor', # binary xor
		'or', 'por', 'vpor', 'or_lock',  # binary or
		'inc', 'dec', 'dec_lock', # increment and decrement
		'not', 'neg', # not and neg
	]
}

cat_col = {
	'Arithmetic Integer': 0,
	'Arithmetic Real' : 1,
	'Routing Integer': 0,
	'Routing Real': 1,
	'Control Jump': 2,
	'Control Other': 3
}

def ins_type(opcode):
	for key, value in categories.iteritems():
		if opcode in value:
			return key

	print "unrecognized opcode \"" + opcode + "\""
	return opcode

counts = dict()
widths = dict()
icounts = dict()
iwidths = dict()
total = 0
mems = 0
first = 1

with open("opcode.tsv", "r") as fptr:
	for line in fptr:
		if first:
			first = 0
		else:
			splt = line.strip().split('\t')
			category = ins_type(splt[1].lower())
			opcode = splt[1].lower()
			count = int(splt[2])
			mem = int(splt[3])
			width = [int(x) for x in splt[4:]]

			total += count
			mems += mem

			if category not in counts:
				counts[category] = [count, mem]
			else:
				counts[category][0] += count
				counts[category][1] += mem
			
			if category not in widths:
				widths[category] = [0 for i in range(0, 65+53+53+17)]

			widths[category] = [sum(x) for x in zip(widths[category], width)]

			if opcode not in icounts:
				icounts[opcode] = [category, count, mem]
			else:
				icounts[opcode][1] += count
				icounts[opcode][2] += mem

			if opcode not in iwidths:
				iwidths[opcode] = [0 for i in range(0, 65+53+53+17)]

			iwidths[opcode] = [sum(x) for x in zip(iwidths[opcode], width)]

with open("catcount.tsv", "w") as fptr:
	print >>fptr, "Category\tInteger\tInteger (Mem)\tReal\tReal (Mem)\tJump\tJump (Mem)\tOther\tOther (Mem)"

	for row in ["Arithmetic", "Routing", "Control"]:
		rowstr = row
		if row != "Control":
			count = counts[row + ' Integer']
			nomem = float(count[0]-count[1])/float(total)
			mem = float(count[1])/float(total)
			rowstr += '\t' + str(nomem) + '\t' + str(mem)

			count = counts[row + ' Real']
			nomem = float(count[0]-count[1])/float(total)
			mem = float(count[1])/float(total)
			rowstr += '\t' + str(nomem) + '\t' + str(mem)
		else:
			rowstr += '\t\t\t\t'
			count = counts[row + ' Jump']
			nomem = float(count[0]-count[1])/float(total)
			mem = float(count[1])/float(total)
			rowstr += '\t' + str(nomem) + '\t' + str(mem)

			count = counts[row + ' Other']
			nomem = float(count[0]-count[1])/float(total)
			mem = float(count[1])/float(total)
			rowstr += '\t' + str(nomem) + '\t' + str(mem)
		print >>fptr, rowstr

with open("incount.tsv", "w") as fptr:
	print >>fptr, "Instruction\tInteger\tInteger (Mem)\tReal\tReal (Mem)\tJump\tJump (Mem)\tOther\tOther (Mem)"
	for key,count in sorted(icounts.items(), reverse=True, key=lambda x: x[1][1]):
		nomem = float(count[1]-count[2])/float(total)
		mem = float(count[2])/float(total)

		print >>fptr, "{key}{col}\t{nomem}\t{mem}".format(key=key, col='\t\t'*cat_col[count[0]], nomem=nomem, mem=mem)

with open("width.tsv", "w") as fptr:
	print >>fptr, "Integer\t\tLSB Significand\t\tMSB Significand\t\tExponent"
	print >>fptr, "Arithmetic\tRouting\t"*4
	lengths = [65, 53, 53, 17]
	for i in range(0, max(lengths)):
		width = widths["Arithmetic Integer"]
		print >>fptr, str(width[i]),
		width = widths["Routing Integer"]
		print >>fptr, "\t" + str(width[i]),
		if i < 53:
			width = widths["Arithmetic Real"]
			print >>fptr, "\t" + str(width[65+i]),
			width = widths["Routing Real"]
			print >>fptr, "\t" + str(width[65+i]),
		else:
			print >>fptr, "\t\t",
		if i < 53:
			width = widths["Arithmetic Real"]
			print >>fptr, "\t" + str(width[65+53+i]),
			width = widths["Routing Real"]
			print >>fptr, "\t" + str(width[65+53+i]),
		else:
			print >>fptr, "\t\t",
		if i < 17:
			width = widths["Arithmetic Real"]
			print >>fptr, "\t" + str(width[65+53+53+i]),
			width = widths["Routing Real"]
			print >>fptr, "\t" + str(width[65+53+53+i]),
		else:
			print >>fptr, "\t\t",
		print >>fptr, ""

with open("integer_widths.tsv", "w") as fptr:
	print >>fptr, "\t".join(key for key,width in iwidths.items())
	for i in range(0, 65):
		print >>fptr, "\t".join(str(width[i]) for key,width in iwidths.items())

with open("significand_lsb_widths.tsv", "w") as fptr:
	print >>fptr, "\t".join(key for key,width in iwidths.items())
	for i in range(65, 65+53):
		print >>fptr, "\t".join(str(width[i]) for key,width in iwidths.items())

with open("significand_msb_widths.tsv", "w") as fptr:
	print >>fptr, "\t".join(key for key,width in iwidths.items())
	for i in range(65+53, 65+53+53):
		print >>fptr, "\t".join(str(width[i]) for key,width in iwidths.items())

with open("exponent_widths.tsv", "w") as fptr:
	print >>fptr, "\t".join(key for key,width in iwidths.items())
	for i in range(65+53+53, 65+53+53+17):
		print >>fptr, "\t".join(str(width[i]) for key,width in iwidths.items())

with open("mem.tsv", "w") as fptr:
	print >>fptr, "{total}\t{mems}".format(total=total,mems=mems)

#for key, value in categories.iteritems():
#	print key + '\t' + '\t'.join(value)
