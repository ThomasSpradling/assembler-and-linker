regs = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
        "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
        "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
        "t8", "t9",
        "k0", "k1", "gp", "sp", "fp", "ra"]

f = open("regs.c", "w")
f.write("if (strcmp(str, \"$zero\") == 0 || strcmp(str, \"$0\") == 0)  return 0;\n")
for i, r in enumerate(regs):
    if i > 0:
        f.write(
            f"else if (strcmp(str, \"${r}\") == 0 || strcmp(str, \"${i}\") == 0) return {i};\n")
