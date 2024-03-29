import re

import common


# SORT
def block(v, dent, i):
    assert common.indent(v, i) == dent
    assert re.match(r"\s*(case .*|default):", v[i])

    # block starts with one or more case labels
    while re.match(r"\s*(case .*|default):", v[i]):
        i += 1

    # braced block
    if v[i - 1].endswith("{"):
        while not (common.indent(v, i) == dent and re.match(r"\s*}$", v[i])):
            i += 1
        return i + 1

    # blocks may be chained with [[fallthrough]]
    while 1:
        while common.indent(v, i) != dent:
            i += 1
        assert re.match(r"\s*(case .*|default):", v[i]) or re.match(r"\s*}", v[i])
        if not re.match(r"\s*\[\[fallthrough\]\];", v[i - 1]):
            return i
        while re.match(r"\s*(case .*|default):", v[i]):
            i += 1


def f(v):
    i = 0
    while i < len(v):
        if not re.match(r"(\s*)switch \(.*\) {", v[i]):
            i += 1
            continue

        dent = common.indent(v, i)
        i += 1

        j = i
        r = []
        while not re.match(r"\s*}", v[j]):
            k = block(v, dent, j)
            r.append(v[j:k])
            j = k
        assert common.indent(v, i) == dent
        assert r
        r.sort()
        v[i:j] = common.cat(r)

        i = j + 1


common.modify_files(f, common.args_c_files())
