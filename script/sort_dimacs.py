import argparse

parser = argparse.ArgumentParser(
    description="sort list of DIMACS files by number of variables and clauses"
)
parser.add_argument("list_file")
args = parser.parse_args()


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


def write_lines(filename, lines):
    with open(filename, "w") as f:
        for s in lines:
            f.write(s + "\n")


class Dimacs:
    def __init__(self, filename):
        self.filename = filename
        with open(filename) as f:
            for s in f:
                if s[0] == "p":
                    s = s.split()
                    self.variables = int(s[2])
                    self.clauses = int(s[3])
                    break
        if not hasattr(self, "variables"):
            print(filename + ": no problem definition")
            exit(1)

    def __repr__(self):
        return self.filename + "," + str(self.variables) + "," + str(self.clauses)


files = read_lines(args.list_file)
for i in range(len(files)):
    if "/" not in files[i] and "\\" not in files[i]:
        files[i] = "C:/satlib/" + files[i]
problems = [Dimacs(s) for s in files]
problems.sort(key=lambda p: p.variables * 100 + p.clauses)
files = [p.filename for p in problems]
write_lines(args.list_file, files)
print(str(len(files)) + " files sorted")
