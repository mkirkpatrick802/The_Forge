import os
import re

# Declarations that look like data members but cannot be reflected with offsetof()
NON_MEMBER_KEYWORDS = (
    'using', 'typedef', 'friend', 'static', 'enum', 'class', 'struct',
    'template', 'virtual', 'constexpr', 'inline',
)

# A declarator, once its initializer has been split off: "<type...> <name>[array]"
DECLARATOR_RE = re.compile(r'^(?P<type>.+?)\s+(?P<name>\w+)\s*(?:\[[^\]]*\])?$')

# Strip // and /* */ comments so they can't be mistaken for declarations
def strip_comments(text):
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.DOTALL)
    return re.sub(r'//[^\n]*', '', text)

# Split a class-body statement (no trailing ';') into its declarator and whether
# it is a function declaration. A '(' reached before any initializer means a
# parameter list: C++ forbids paren-initialising a non-static data member, so
# parens in that position are unambiguously a function. Angle-bracket depth is
# tracked so template arguments aren't mistaken for an initializer.
def split_declarator(statement):
    angle = 0
    for i, ch in enumerate(statement):
        if ch == '<':
            angle += 1
        elif ch == '>':
            angle = max(0, angle - 1)
        elif angle == 0:
            if ch in '={':
                return statement[:i].strip(), False
            if ch == '(':
                return statement[:i].strip(), True
    return statement.strip(), False

# Extract the content inside a class body by matching curly brace depth
def extract_class_body(content, start_pos):
    depth = 0
    for i in range(start_pos, len(content)):
        if content[i] == '{':
            depth += 1
        elif content[i] == '}':
            depth -= 1
            if depth == 0:
                return content[start_pos + 1:i]
    return ""

# Walk directory tree, yielding all .h/.hpp files except in "Vendors"
def find_h_files(root_dir):
    for dirpath, dirnames, filenames in os.walk(root_dir):
        # Skip "Vendors" folders (case-insensitive)
        dirnames[:] = [d for d in dirnames if d.lower() != "vendors"]

        for filename in filenames:
            if filename.endswith('.h') or filename.endswith('.hpp'):
                full_path = os.path.join(dirpath, filename)
                yield full_path

# Process each header to find REFLECT() classes and generate reflection
def parse_class_and_generate(file_path):
    print(f"\n=== Parsing: {file_path} ===")
    with open(file_path, 'r') as f:
        content = f.read()

    # Match class with REFLECT() macro inside body
    class_match = re.search(r'class\s+(\w+)(\s*:\s*[^{]+)?\s*{[^}]*?REFLECT\(\)', content, re.DOTALL)
    if not class_match:
        print(f"[!] No REFLECT() class found in {file_path}")
        return

    class_name = class_match.group(1)
    class_start = content.find('{', class_match.start())
    if class_start == -1:
        print(f"[!] Could not find opening brace for class {class_name}")
        return

    class_body = extract_class_body(content, class_start)
    print(f"[OK] Found REFLECT() class: {class_name}")

    members = []
    pending_replicate = False

    for raw_line in strip_comments(class_body).splitlines():
        line = raw_line.strip()
        if not line:
            continue

        # REPLICATE() marks the *next* data member as network-replicated. It is
        # accepted both on its own line (the documented style) and inline ahead
        # of the declaration.
        if line.startswith('REPLICATE()'):
            pending_replicate = True
            line = line[len('REPLICATE()'):].strip()
            if not line:
                continue

        if line in ('public:', 'private:', 'protected:'):
            continue

        if not line.endswith(';'):
            continue

        declarator, is_function = split_declarator(line[:-1])
        if is_function:
            print(f"  [-] Skipped function: {line}")
            continue

        match = DECLARATOR_RE.match(declarator)
        if not match:
            print(f"  [-] Skipped line: {line}")
            continue

        var_type = ' '.join(match.group('type').split())
        var_name = match.group('name')

        # offsetof() is only valid on non-static data members.
        if any(re.match(rf'(^|\s){kw}(\s|$)', f' {var_type} ') for kw in NON_MEMBER_KEYWORDS):
            print(f"  [-] Skipped non-member declaration: {line}")
            continue

        # Skip pointer members (check if the type ends with * or &)
        if var_type.endswith('*') or var_type.endswith('&'):
            print(f"[-] Skipped pointer member: {var_type} {var_name}")
            pending_replicate = False
            continue

        members.append((var_type, var_name, pending_replicate))
        print(f"  [+] Member: type={var_type}, name={var_name}, replicate={pending_replicate}")
        pending_replicate = False

    if not members:
        print(f"[!] No members parsed from class {class_name}")
        return

    # Generate .reflected.cpp
    # This is compiled as its own translation unit -- every module's premake
    # config globs "Source/**.cpp", so it must NOT also be #included by the
    # header, or the definition lands in every TU that pulls the header in.
    reflected_path = file_path.replace('.h', '.reflected.cpp')
    with open(reflected_path, 'w') as f:
        f.write(f'#include "{os.path.basename(file_path)}"\n')
        f.write('#include <cstddef>\n\n')
        f.write(f'ReflectionInfo* {class_name}::GetReflectionInfo() {{\n')
        f.write('    static ReflectionInfo info = {\n')
        f.write(f'        "{class_name}", {{\n')
        for var_type, var_name, replicate in members:
            # Field order must match MemberInfo in ReflectionUtils.h:
            # {name, offset, type, replicate}
            f.write(f'            MemberInfo{{"{var_name}", offsetof({class_name}, {var_name}), "{var_type}", {"true" if replicate else "false"}}},\n')
        f.write('        }\n    };\n    return &info;\n}\n')

    print(f"[OK] Generated reflection: {reflected_path}")

# Run over all headers in directory
def generate_reflection_for_dir(directory):
    for file_path in find_h_files(directory):
        parse_class_and_generate(file_path)

# === ENTRY POINT ===
if __name__ == "__main__":
    import sys
    target_dir = sys.argv[1] if len(sys.argv) > 1 else '.'
    print("=== Reflection Generator Starting ===")
    print(f"Target directory: {target_dir}")
    generate_reflection_for_dir(target_dir)