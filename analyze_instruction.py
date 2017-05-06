#!/usr/bin/python

counts = dict()

with open("instruction.tsv", "r") as fptr:
	for line in fptr:
		splt = line.strip().split('\t')
		addr = splt[0].lower()
		count = splt[1].lower()

		if count not in counts:
			counts[count] = 0

		counts[count] += 1
		
with open("block.tsv", "w") as fptr:
	for key,count in counts.items():
		print >>fptr, key + "\t" + str(count)
