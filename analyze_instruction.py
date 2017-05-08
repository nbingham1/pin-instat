#!/usr/bin/python

from collections import Counter

counts = dict()
exe_count = 0
first = True

with open("instruction.tsv", "r") as fptr:
	for line in fptr:
		if first:
			first = False
		else:
			splt = line.strip().split('\t')
			addr = splt[0].lower()
			total = int(splt[1])
			reads = Counter()
			writes = Counter()
			for mem in splt[3:]:
				key,read,write = (int(x) for x in mem.strip().split(','))
				reads[key] = read
				writes[key] = write

			if total not in counts:
				counts[total] = [0, Counter(), Counter()]

			counts[total][0] += 1
			counts[total][1] += reads
			counts[total][2] += writes
			exe_count += total

mems = dict()
mem_count = 0.0

with open("block.tsv", "w") as fptr:
	for key,count in sorted(counts.items(), reverse=True, key=lambda x: x[0]*x[1][0]):
		size = count[0]
		total = float(key*size)/float(exe_count)
		print >>fptr, "{total}\t{size}".format(
			total = total,
			size = size)

		total_read = sum(count[1].values(), 0)
		total_write = sum(count[2].values(), 0)
		total_mem = total_read + total_write
		combined = sorted([(count[1][k] if k in count[1] else 0, count[2][k] if k in count[2] else 0) for k in (count[1].viewkeys() | count[2].keys())], reverse=True, key=lambda x: (x[0] + x[1]))
		if total_mem > 0:
			mem_count += 1.0
			for i,values in enumerate(combined):
				if i in mems:
					mems[i][0] += float(values[0])/float(total_mem)
					mems[i][1] += float(values[1])/float(total_mem)
					mems[i][2] += float(max(values))/float(values[0] + values[1])
					mems[i][3] += 1.0
				else:
					mems[i] = [
						float(values[0])/float(total_mem),
						float(values[1])/float(total_mem),
						float(max(values))/float(values[0] + values[1]),
						1.0
					]

with open("blockmem.tsv", "w") as fptr:
	for x in mems.values():
		print >>fptr, str(x[0]/mem_count) + "\t" + str(x[1]/mem_count) + "\t" + str(x[2]/x[3])

