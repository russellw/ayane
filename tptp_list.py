import os

r = []
for root, dirs, files in os.walk(r"\TPTP"):
    for fname in files:
        filename = os.path.join(root, fname)
        if os.path.splitext(filename)[1] != ".p":
            continue
        if "^" in filename:
            continue
        r.append(filename + "\n")
open("tptp.lst", "w").writelines(r)
