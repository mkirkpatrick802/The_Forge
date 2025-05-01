import os
import re

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

    # Filter to lines that look like member declarations
    member_lines = [line.strip() for line in class_body.splitlines()
                    if line.strip().endswith(';') and '(' not in line and ')' not in line]

    members = []
    for line in member_lines:
        if line in ('public:', 'private:', 'protected:') or not line:
            continue

        # Match optional REPLICATE(), then type, then variable name
        match = re.match(r'(REPLICATE\(\)\s*)?([\w:<>\s*&]+?)\s+(\w+)(\s*=\s*[^;]*)?;', line)
        if match:
            replicate = bool(match.group(1))
            var_type = match.group(2).strip()
            var_name = match.group(3).strip()

            # Skip pointer members (check if the type ends with *)
            if var_type.endswith('*'):
                print(f"[-] Skipped pointer member: {var_type} {var_name}")
                continue

            members.append((var_type, var_name, replicate))
            print(f"  [+] Member: type={var_type}, name={var_name}, replicate={replicate}")
        else:
            print(f"  [-] Skipped line: {line}")

    if not members:
        print(f"[!] No members parsed from class {class_name}")
        return

    # Generate .reflected.cpp
    reflected_path = file_path.replace('.h', '.reflected.cpp')
    with open(reflected_path, 'w') as f:
        f.write(f'#include "{os.path.basename(file_path)}"\n\n')
        f.write(f'ReflectionInfo* {class_name}::GetReflectionInfo() {{\n')
        f.write('    static ReflectionInfo info = {\n')
        f.write(f'        "{class_name}", {{\n')
        for var_type, var_name, replicate in members:
            f.write(f'            MemberInfo{{"{var_type}", offsetof({class_name}, {var_name}), "{var_name}", {"true" if replicate else "false"}}},\n')
        f.write('        }\n    };\n    return &info;\n}\n')

    print(f"[OK] Generated reflection: {reflected_path}")

    # Inject include at bottom if not already there
    include_line = f'#include "{os.path.basename(reflected_path)}"'
    if include_line not in content:
        with open(file_path, 'a') as f:
            f.write(f'\n{include_line}\n')
        print(f"[OK] Appended include to header: {include_line}")
    else:
        print(f"[=] Header already includes: {include_line}")

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