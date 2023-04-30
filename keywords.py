words = [
    "ax",
    "bool",
    "ceiling",
    "cnf",
    "conjecture",
    "difference",
    "distinct",
    "false",
    "floor",
    "fof",
    "greater",
    "greatereq",
    "i",
    "include",
    "int",
    "is_int",
    "is_rat",
    "less",
    "lesseq",
    "o",
    "p",
    "product",
    "quotient",
    "quotient_e",
    "quotient_f",
    "quotient_t",
    "rat",
    "real",
    "remainder_e",
    "remainder_f",
    "remainder_t",
    "round",
    "sum",
    "tType",
    "tff",
    "to_int",
    "to_rat",
    "to_real",
    "true",
    "truncate",
    "type",
    "uminus",
]


def find(x):
    for i in range(len(xs)):
        if xs[i].startswith(x):
            return i
    raise Exception(x)


def end(i):
    for j in range(i, len(xs)):
        if xs[j].startswith("}"):
            return j
    raise Exception()


def san(s):
    return s


xs = open("strings.h").readlines()
i = find("enum") + 1
j = end(i)
ys = []
for y in words:
    ys.append(f"\ts_{san(y)},\n")
ys.append("\tend_s\n")
xs[i:j] = ys
open("strings.h", "w", newline="\n").writelines(xs)

xs = open("strings.cc").readlines()
i = find("string keywords") + 1
j = end(i)
ys = []
ys.append("// clang-format off\n")
for y in words:
    ys.append('\t{0, 0, 0, "%s"},\n' % y)
ys.append("// clang-format on\n")
xs[i:j] = ys
open("strings.cc", "w", newline="\n").writelines(xs)
