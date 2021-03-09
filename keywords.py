# a symbol is an interned string
# a keyword is a symbol of known significance
# need the list of keywords in two formats
# that must correspond to each other
# so generate both lists

# this cannot just be an X macro
# like some of the other cases where enum and array must correspond to each other
# because some keywords contain punctuation characters
# therefore cannot be C++ identifiers


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


def write_lines(filename, lines):
    with open(filename, "w") as f:
        for s in lines:
            f.write(s + "\n")


lines = read_lines("keywords.txt")
old = lines.copy()
lines.sort()
if lines != old:
    write_lines("keywords.txt", lines)

# header
with open("keywords.h", "w") as f:
    f.write("// AUTO GENERATED FILE - DO NOT MODIFY\n")
    f.write("enum {\n")
    for s in lines:
        f.write("k_" + s + ",\n")
    f.write("};\n")
    f.write(f"extern sym keywords[{len(lines)}];\n")

# data
with open("keywords.cc", "w") as f:
    f.write("// AUTO GENERATED FILE - DO NOT MODIFY\n")
    f.write('#include "stdafx.h"\n')
    f.write("// stdafx.h must be first\n")
    f.write('#include "main.h"\n')
    f.write("sym keywords [] = {\n")
    for s in lines:
        f.write('{type::none, type::none,"' + s + '"},\n')
    f.write("};\n")
