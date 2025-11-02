import sys

h = open("symbols")
syms = []
for i in h.readlines():
	data = i.split(" ")
	syms.append([int(data[0], base=16), data[2].strip("\n")])
h.close()

for i in sys.argv[1:]:
	thingy = int(i, base=16)
	nearest = [0, "null"]
	for i in syms:
		if (i[0] < thingy and i[0] > nearest[0]) or i[0] == thingy:
			nearest = i

	print(nearest[1])