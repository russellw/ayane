import os

here = os.path.dirname(os.path.realpath(__file__))
src = os.path.join(here, "..", "src")

words = (
    # SORT
    "Bool",
    "Float32",
    "Float64",
    "Int",
    "Real",
    "RoundingMode",
    "String",
    "and",
    "assert",
    "ax",
    "bool",
    "bvand",
    "bvnot",
    "bvor",
    "bvsdiv",
    "bvsmod",
    "bvxor",
    "ceiling",
    "check-sat",
    "cnf",
    "concat",
    "conjecture",
    "declare-const",
    "declare-datatype",
    "declare-datatypes",
    "declare-fun",
    "declare-sort",
    "define-fun",
    "define-sort",
    "difference",
    "distinct",
    "div",
    "exists",
    "false",
    "floor",
    "fof",
    "forall",
    "fp.eq",
    "fp.lt",
    "greater",
    "greatereq",
    "i",
    "include",
    "int",
    "is_int",
    "is_rat",
    "ite",
    "less",
    "lesseq",
    "let",
    "mod",
    "not",
    "o",
    "or",
    "p",
    "product",
    "push",
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
    "set-info",
    "set-logic",
    "smt2",
    "sum",
    "tType",
    "tcf",
    "tff",
    "thf",
    "to_int",
    "to_rat",
    "to_real",
    "true",
    "truncate",
    "type",
    "uminus",
    "xor",
    ("!", "bang"),
    ("*", "star"),
    ("+", "plus"),
    ("-", "minus"),
    ("/", "slash"),
    ("<", "lt"),
    ("<=", "le"),
    ("=", "eq"),
    ("=>", "imp"),
    (">", "gt"),
    (">=", "ge"),
    ("_", "underscore"),
)


def find(s):
    for i in range(len(u)):
        if u[i].startswith(s):
            assert "{" in u[i]
            return i + 1
    raise Exception(s)


def end(i):
    for j in range(i, len(u)):
        if u[j].startswith("}"):
            return j
    raise Exception()


def san(s):
    s = s.replace("-", "_")
    s = s.replace(".", "_")
    return s


# declare
u = open(os.path.join(src, "str.h")).readlines()
i = find("enum")
j = end(i)

v = []
for s in words:
    if isinstance(s, tuple):
        s = s[1]
    v.append(f"s_%s,\n" % san(s))

u[i:j] = v
open(os.path.join(src, "str.h"), "w", newline="\n").writelines(u)

# define
u = open(os.path.join(src, "str.cc")).readlines()
i = find("Str keywords")
j = end(i)

v = []
v.append("// clang-format off\n")
for s in words:
    if isinstance(s, tuple):
        s = s[0]
    v.append('\t{0, 0, "%s"},\n' % s)
v.append("// clang-format on\n")

u[i:j] = v
open(os.path.join(src, "str.cc"), "w", newline="\n").writelines(u)
