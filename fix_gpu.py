import re

with open("px_vm.comp", "r") as f:
    text = f.read()

# Fix the pop order in GLSL
orig = """                for (int j = int(n)-1; j >= 0; j--) {
                    values[j] = pop();
                }"""
new = """                for (int j = 0; j < int(n); j++) {
                    values[j] = pop();
                }"""
text = text.replace(orig, new)

with open("px_vm.comp", "w") as f:
    f.write(text)
