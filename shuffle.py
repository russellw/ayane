import sys
import random

fname = sys.argv[1]
with open(fname) as f:
    content = f.readlines()
random.shuffle(content)
with open(fname, "w") as f:
    f.writelines(content)
print("shuffled " + str(len(content)) + " lines")
