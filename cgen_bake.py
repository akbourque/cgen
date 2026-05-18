#!/usr/bin/env python3
import sys

def bake_template(filename, key_placeholder="BKey", val_placeholder="BVal"):
    try:
        with open(filename, 'r') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"Error reading file: {e}")
        return

    baked_lines = []
    for line in lines:
        # Strip trailing newlines to handle manually
        line_str = line.rstrip('\r\n')
        
        # Token Replacement Rules for Keys
        line_str = line_str.replace(f"{key_placeholder}_t_U", "{{KEY_BU}}")
        line_str = line_str.replace(f"{key_placeholder}_t", "{{KEY}}")
        line_str = line_str.replace(f"{key_placeholder}_U", "{{KEY_U}}")
        line_str = line_str.replace(key_placeholder, "{{KEY_B}}")
        
        # Token Replacement Rules for Values
        line_str = line_str.replace(f"{val_placeholder}_t_U", "{{VAL_BU}}")
        line_str = line_str.replace(f"{val_placeholder}_t", "{{VAL}}")
        line_str = line_str.replace(f"{val_placeholder}_U", "{{VAL_U}}")
        line_str = line_str.replace(val_placeholder, "{{VAL_B}}")
        
        # Escape double quotes and backslashes for safe insertion into a C string
        escaped = line_str.replace('\\', '\\\\').replace('"', '\\"')
        
        # Format as a clean C string literal line
        baked_lines.append(f'    "{escaped}\\n"')

    print(f"\n// --- Baked Template Block From {filename} ---")
    print("const char *YOUR_TEMPLATE_NAME = ")
    print("\n".join(baked_lines) + ";\n")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 cgen_bake.py <source_file.c/.h> [key_placeholder] [val_placeholder]")
        sys.exit(1)
    
    kp = sys.argv[2] if len(sys.argv) > 2 else "BKey"
    vp = sys.argv[3] if len(sys.argv) > 3 else "BVal"
    bake_template(sys.argv[1], kp, vp)
