import os
import sys

out = open("a.p", "w")


def read_file(filename):
    lines = open(filename).readlines()
    for s in lines:
        if s.startswith("include("):
            tptp = os.getenv("TPTP")
            if not tptp:
                raise ValueError("TPTP environment variable not set")
            read_file(tptp + "/" + s.split("'")[1])
            continue
        out.write(s)


read_file(sys.argv[1])
