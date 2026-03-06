import re
import sys
import os

# Definitions matching px_vm.h
MAX_TOKENS = 1024

class TokenType:
    NUMBER = 'NUMBER'
    VARIABLE = 'VARIABLE'
    FREQUENCY = 'FREQUENCY'
    CONSTANT = 'CONSTANT'
    FUNCTION = 'FUNCTION'
    OPERATOR = 'OPERATOR'
    UNARY_OP = 'UNARY_OP'
    COMPARISON = 'COMPARISON'
    LOGICAL_AND = 'LOGICAL_AND'
    LOGICAL_OR = 'LOGICAL_OR'
    LOGICAL_XOR = 'LOGICAL_XOR'
    TERNARY_QM = 'TERNARY_QM'
    TERNARY_CL = 'TERNARY_CL'
    LPAREN = 'LPAREN'
    RPAREN = 'RPAREN'
    COMMA = 'COMMA'
    END = 'END'
    RAND_OFFSET = 'RAND_OFFSET'
    MOD_A = 'MOD_A'
    MOD_B = 'MOD_B'
    MOD_C = 'MOD_C'

class Token:
    def __init__(self, type, value):
        self.type = type
        self.value = value
    def __repr__(self):
        return f"Token({self.type}, {self.value})"

# Functions map: name -> (arity, c_func_name)
FUNCTIONS = {
    "sin": (1, "sinf"), "cos": (1, "cosf"), "tan": (1, "tanf"),
    "asin": (1, "asinf"), "acos": (1, "acosf"), "atan": (1, "atanf"),
    "abs": (1, "fabsf"), "tanh": (1, "vm_fast_tanh"), # Use fast tanh from vm
    "exp": (1, "expf"), "log": (1, "logf"), "log10": (1, "log10f"),
    "floor": (1, "floorf"), "ceil": (1, "ceilf"),
    "min": (2, "fminf"), "max": (2, "fmaxf"), "fma": (3, "fmaf"),
    "sqrt": (1, "sqrtf"), "pow": (2, "powf"),
    "rand": (0, "vm_rand"),
    "sigma": (5, "sigma"), # Special handling
    "lfsr_val": (3, "vm_lfsr_val"),
    "lfsr_noise": (2, "vm_lfsr_noise"),
    "lfsr_clock": (2, "vm_lfsr_clock")
}

CONSTANTS = {
    "PI": "3.14159265358979323846f",
    "TWO_PI": "(2.0f * 3.14159265358979323846f)",
    "PI_OVER_2": "(3.14159265358979323846f / 2.0f)",
    "THREE_PI_OVER_2": "(3.0f * 3.14159265358979323846f / 2.0f)",
    "E": "2.71828182845904523536f",
    # LFSR Constants
    "LFSR_4BIT": "0", "LFSR_5BIT": "1", "LFSR_6BIT": "2", "LFSR_7BIT": "3",
    "LFSR_8BIT": "4", "LFSR_9BIT": "5", "LFSR_10BIT": "6", "LFSR_11BIT": "7",
    "LFSR_12BIT": "8", "LFSR_13BIT": "9", "LFSR_14BIT": "10", "LFSR_15BIT": "11",
    "LFSR_16BIT": "12", "LFSR_17BIT": "13", "LFSR_GALOIS": "14", "LFSR_FIBONACCI": "15"
}

def tokenize(expr):
    tokens = []
    i = 0
    length = len(expr)
    while i < length:
        c = expr[i]
        if c.isspace():
            i += 1
            continue

        # Numbers
        if c.isdigit() or (c == '.' and i+1 < length and expr[i+1].isdigit()):
            start = i
            dot_seen = False
            while i < length and (expr[i].isdigit() or expr[i] == '.'):
                if expr[i] == '.':
                    if dot_seen: break
                    dot_seen = True
                i += 1
            val = expr[start:i]
            if not '.' in val: val += ".0" # Ensure float literal
            val += "f"
            tokens.append(Token(TokenType.NUMBER, val))
            continue

        # Identifiers
        if c.isalpha() or c == '_':
            start = i
            while i < length and (expr[i].isalnum() or expr[i] == '_'):
                i += 1
            val = expr[start:i]

            if val == "FREQUENCY": tokens.append(Token(TokenType.FREQUENCY, val))
            elif val == "RAND_OFFSET": tokens.append(Token(TokenType.RAND_OFFSET, val))
            elif val == "MOD_A": tokens.append(Token(TokenType.MOD_A, val))
            elif val == "MOD_B": tokens.append(Token(TokenType.MOD_B, val))
            elif val == "MOD_C": tokens.append(Token(TokenType.MOD_C, val))
            elif val in CONSTANTS: tokens.append(Token(TokenType.CONSTANT, val))
            elif val in FUNCTIONS: tokens.append(Token(TokenType.FUNCTION, val))
            elif val == "x": tokens.append(Token(TokenType.VARIABLE, val))
            else: tokens.append(Token(TokenType.VARIABLE, val)) # Loop variable k, etc.
            continue

        # Operators
        if expr[i:i+2] == '&&':
            tokens.append(Token(TokenType.LOGICAL_AND, '&&'))
            i += 2
        elif expr[i:i+2] == '||':
            tokens.append(Token(TokenType.LOGICAL_OR, '||'))
            i += 2
        elif c == '^':
            tokens.append(Token(TokenType.LOGICAL_XOR, '^'))
            i += 1
        elif expr[i:i+2] in ['<=', '>=', '==', '!=']:
            tokens.append(Token(TokenType.COMPARISON, expr[i:i+2]))
            i += 2
        elif c in ['<', '>']:
            tokens.append(Token(TokenType.COMPARISON, c))
            i += 1
        elif c in "+-*/%":
            tokens.append(Token(TokenType.OPERATOR, c))
            i += 1
        elif c == '!':
            tokens.append(Token(TokenType.UNARY_OP, '!'))
            i += 1
        elif c == '(':
            tokens.append(Token(TokenType.LPAREN, '('))
            i += 1
        elif c == ')':
            tokens.append(Token(TokenType.RPAREN, ')'))
            i += 1
        elif c == ',':
            tokens.append(Token(TokenType.COMMA, ','))
            i += 1
        elif c == '?':
            tokens.append(Token(TokenType.TERNARY_QM, '?'))
            i += 1
        elif c == ':':
            tokens.append(Token(TokenType.TERNARY_CL, ':'))
            i += 1
        else:
            print(f"Unknown char: {c}")
            return None
    tokens.append(Token(TokenType.END, ""))
    return tokens

# --- AST ---
class Node:
    def to_c(self, context):
        raise NotImplementedError

class NumberNode(Node):
    def __init__(self, val): self.val = val
    def to_c(self, ctx): return self.val

class VarNode(Node):
    def __init__(self, name): self.name = name
    def to_c(self, ctx):
        if self.name == 'x': return "params->x"
        if self.name == 'FREQUENCY': return "params->frequency"
        if self.name == 'RAND_OFFSET': return "params->rand_offset"
        if self.name == 'MOD_A': return "params->modA"
        if self.name == 'MOD_B': return "params->modB"
        if self.name == 'MOD_C': return "params->modC"
        return self.name # Loop variable k

class ConstNode(Node):
    def __init__(self, name): self.name = name
    def to_c(self, ctx): return CONSTANTS[self.name]

class UnaryNode(Node):
    def __init__(self, op, expr): self.op = op; self.expr = expr
    def to_c(self, ctx):
        val = self.expr.to_c(ctx)
        if self.op == '!': return f"(fabsf({val}) < 1e-6f ? 1.0f : 0.0f)"
        return f"({self.op}{val})"

class BinaryNode(Node):
    def __init__(self, op, left, right): self.op = op; self.left = left; self.right = right
    def to_c(self, ctx):
        l = self.left.to_c(ctx)
        r = self.right.to_c(ctx)
        if self.op == '%': return f"fmodf({l}, {r})"
        if self.op == '^': # Logical XOR
             return f"((fabsf({l}) > 1e-6f) != (fabsf({r}) > 1e-6f) ? 1.0f : 0.0f)"
        return f"({l} {self.op} {r})"

class ComparisonNode(Node):
    def __init__(self, op, left, right): self.op = op; self.left = left; self.right = right
    def to_c(self, ctx):
        l = self.left.to_c(ctx)
        r = self.right.to_c(ctx)
        # C comparisons return int 0 or 1. We want float 0.0f or 1.0f.
        # But in expression context (e.g. conditional), int is fine?
        # Wait, the VM pushes 1.0f or 0.0f.
        # So we should cast.
        return f"(({l} {self.op} {r}) ? 1.0f : 0.0f)"

class LogicalNode(Node):
    def __init__(self, op, left, right): self.op = op; self.left = left; self.right = right
    def to_c(self, ctx):
        l = self.left.to_c(ctx)
        r = self.right.to_c(ctx)
        # Logic in C uses ints.
        # Convert inputs to bool-ish check?
        # (a && b) -> (val(a) && val(b) ? 1.0f : 0.0f)
        op_c = self.op
        return f"(( (fabsf({l}) > 1e-6f) {op_c} (fabsf({r}) > 1e-6f) ) ? 1.0f : 0.0f)"

class TernaryNode(Node):
    def __init__(self, cond, true_br, false_br): self.cond=cond; self.true_br=true_br; self.false_br=false_br
    def to_c(self, ctx):
        c = self.cond.to_c(ctx)
        t = self.true_br.to_c(ctx)
        f = self.false_br.to_c(ctx)
        # Check truthiness of c
        return f"((fabsf({c}) > 1e-6f) ? ({t}) : ({f}))"

class FuncNode(Node):
    def __init__(self, name, args): self.name=name; self.args=args
    def to_c(self, ctx):
        c_func = FUNCTIONS[self.name][1]
        arg_strs = [a.to_c(ctx) for a in self.args]

        if self.name == "sigma":
            # Extract sigma
            # sigma(k, start, end, step, body)
            k_name = self.args[0].name # Assume VarNode
            start = arg_strs[1]
            end = arg_strs[2]
            step = arg_strs[3]

            # For the body, we need to generate it contextually
            # We add a pre-calculation block to context
            temp_var = f"sigma_{ctx['sigma_count']}"
            ctx['sigma_count'] += 1

            # Generate body C code.
            # Note: body expression might use k.
            body_code = self.args[4].to_c(ctx)

            loop_code = f"""
    float {temp_var} = 0.0f;
    {{
        float _start = {start};
        float _end = {end};
        float _step = {step};
        float _limit = (_step > 0) ? (_end + fabsf(_step)*0.5f) : (_end - fabsf(_step)*0.5f);
        for(float {k_name} = _start; (_step > 0) ? ({k_name} <= _limit) : ({k_name} >= _limit); {k_name} += _step) {{
            {temp_var} += {body_code};
        }}
    }}
"""
            ctx['pre_calcs'].append(loop_code)
            return temp_var

        if self.name == "rand":
            return f"vm_rand(params)"

        if self.name in ["lfsr_val", "lfsr_noise", "lfsr_clock"]:
            # Need to pass params first
            return f"{c_func}(params, {', '.join(arg_strs)})"

        return f"{c_func}({', '.join(arg_strs)})"

# --- Parser ---
class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def peek(self): return self.tokens[self.pos]
    def consume(self): t = self.tokens[self.pos]; self.pos+=1; return t
    def match(self, type):
        if self.peek().type == type: return self.consume()
        return None

    def parse(self): return self.parse_conditional()

    def parse_conditional(self):
        node = self.parse_or()
        if self.match(TokenType.TERNARY_QM):
            true_br = self.parse() # Recurse top level? Or conditional? Precedence says expr.
            if not self.match(TokenType.TERNARY_CL): raise Exception("Expected :")
            false_br = self.parse_conditional()
            return TernaryNode(node, true_br, false_br)
        return node

    def parse_or(self):
        node = self.parse_xor()
        while self.peek().type == TokenType.LOGICAL_OR:
            op = self.consume().value
            right = self.parse_xor()
            node = LogicalNode(op, node, right)
        return node

    def parse_xor(self):
        node = self.parse_and()
        while self.peek().type == TokenType.LOGICAL_XOR:
            op = self.consume().value
            right = self.parse_and()
            node = BinaryNode(op, node, right)
        return node

    def parse_and(self):
        node = self.parse_comp()
        while self.peek().type == TokenType.LOGICAL_AND:
            op = self.consume().value
            right = self.parse_comp()
            node = LogicalNode(op, node, right)
        return node

    def parse_comp(self):
        node = self.parse_term()
        while self.peek().type == TokenType.COMPARISON:
            op = self.consume().value
            right = self.parse_term()
            node = ComparisonNode(op, node, right)
        return node

    def parse_term(self):
        node = self.parse_factor()
        while self.peek().type == TokenType.OPERATOR and self.peek().value in "+-":
            op = self.consume().value
            right = self.parse_factor()
            node = BinaryNode(op, node, right)
        return node

    def parse_factor(self):
        node = self.parse_unary()
        while self.peek().type == TokenType.OPERATOR and self.peek().value in "*/%":
            op = self.consume().value
            right = self.parse_unary()
            node = BinaryNode(op, node, right)
        return node

    def parse_unary(self):
        if self.peek().type == TokenType.UNARY_OP or (self.peek().type == TokenType.OPERATOR and self.peek().value == '-'):
            op = self.consume().value
            expr = self.parse_unary()
            return UnaryNode(op, expr)
        return self.parse_primary()

    def parse_primary(self):
        t = self.consume()
        if t.type == TokenType.NUMBER: return NumberNode(t.value)
        if t.type == TokenType.VARIABLE: return VarNode(t.value)
        if t.type == TokenType.FREQUENCY: return VarNode("FREQUENCY")
        if t.type == TokenType.RAND_OFFSET: return VarNode("RAND_OFFSET")
        if t.type == TokenType.MOD_A: return VarNode("MOD_A")
        if t.type == TokenType.MOD_B: return VarNode("MOD_B")
        if t.type == TokenType.MOD_C: return VarNode("MOD_C")
        if t.type == TokenType.CONSTANT: return ConstNode(t.value)
        if t.type == TokenType.LPAREN:
            node = self.parse()
            if not self.match(TokenType.RPAREN): raise Exception("Expected )")
            return node
        if t.type == TokenType.FUNCTION:
            name = t.value
            args = []
            if self.match(TokenType.LPAREN):
                if self.peek().type != TokenType.RPAREN:
                    while True:
                        if name == "sigma" and len(args) == 0:
                            # Sigma first arg is variable name
                            vt = self.consume()
                            if vt.type != TokenType.VARIABLE: raise Exception("Expected var for sigma")
                            args.append(VarNode(vt.value))
                        else:
                            args.append(self.parse())

                        if not self.match(TokenType.COMMA): break
                if not self.match(TokenType.RPAREN): raise Exception("Expected ) after args")
            return FuncNode(name, args)

        raise Exception(f"Unexpected token {t}")

def transpile(expr):
    tokens = tokenize(expr)
    if not tokens: return None
    try:
        parser = Parser(tokens)
        ast = parser.parse()
        ctx = {'sigma_count': 0, 'pre_calcs': []}
        c_expr = ast.to_c(ctx)

        lines = []
        lines.extend(ctx['pre_calcs'])
        lines.append(f"    return {c_expr};")
        return "\n".join(lines)
    except Exception as e:
        print(f"Transpile error for '{expr}': {e}")
        return None

def main():
    rom_path = "px_wave_rom.h"
    out_path = "px_wave_native.h"

    with open(rom_path, 'r') as f:
        content = f.read()

    # Extract default_waves array content
    # Look for WaveDefinition default_waves[...] = { ... };
    match = re.search(r'WaveDefinition\s+default_waves\[.*?\]\s*=\s*\{(.*?)\};', content, re.DOTALL)
    if not match:
        print("Could not find default_waves array")
        return

    array_content = match.group(1)
    # Regex to find individual entries: { "Name", "Expr" }
    # Handles comments and whitespace
    pattern = re.compile(r'\{\s*"(.*?)"\s*,\s*"(.*?)"\s*\}')

    waves = []
    for m in pattern.finditer(array_content):
        waves.append((m.group(1), m.group(2)))

    print(f"Found {len(waves)} waves.")

    c_funcs = []
    native_array = []

    for i, (name, expr) in enumerate(waves):
        func_name = f"px_wave_native_{i}"
        print(f"Transpiling {i}: {name}")
        body = transpile(expr)

        if body:
            c_func = f"static float {func_name}(VmParams* params) {{\n{body}\n}}"
            c_funcs.append(c_func)
            native_array.append(func_name)
        else:
            print(f"  FAILED to transpile {i}. Using NULL.")
            native_array.append("NULL")

    # Generate Header
    with open(out_path, 'w') as f:
        f.write("#ifndef PX_WAVE_NATIVE_H\n#define PX_WAVE_NATIVE_H\n\n")
        f.write("// Auto-generated by tools/transpile_waves.py\n")
        f.write("#include <math.h>\n")
        f.write("#include \"px_vm.h\"\n\n")
        f.write("// --- Native Wave Functions ---\n\n")

        for func in c_funcs:
            f.write(func + "\n\n")

        f.write(f"static NativeWaveFunc native_waves[{len(native_array)}] = {{\n")
        for i, func in enumerate(native_array):
            f.write(f"    {func}, // {i}\n")
        f.write("};\n\n")
        f.write("#endif // PX_WAVE_NATIVE_H\n")

if __name__ == "__main__":
    main()
