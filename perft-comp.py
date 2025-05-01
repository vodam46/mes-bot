import sys


with open(str(sys.argv[1])) as f:
    our = {}
    for line in f.read().rstrip().split("\n"):
        m, n = line.split(": ")
        if m in our:
            print("double:",m)
        our[m] = n

with open(str(sys.argv[2])) as f:
    their = {}
    for line in f.read().rstrip().split("\n"):
        m, n = line.split(": ")
        if m in their:
            print("double:",m)
        their[m] = n

for m in set(our.keys()) | set(their.keys()):
    if m not in our.keys() or m not in their.keys() or our[m] != their[m]:
        print(m, our.get(m, None), their.get(m, None))
