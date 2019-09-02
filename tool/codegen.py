#!/usr/bin/env python3

import xml.etree.ElementTree as ET
import os

REPO_DIR = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
TOOL_DIR = os.path.join(REPO_DIR, 'tool')
SRC_DIR = os.path.join(REPO_DIR, 'src')

class Writer():
    def __init__(self, path):
        self.path = path
        self.indent_level = 0
        self.indent = "    "
        self.lines = []

    def increase_indent(self):
        self.indent_level += 1

    def decrease_indent(self):
        self.indent_level -= 1

    def write_comment_header(self):
        self.lines.append("//")
        self.lines.append("// !!! WARNING !!!")
        self.lines.append("// ")
        self.lines.append("// This file was auto-generated using the tool/codegen.py script. To make")
        self.lines.append("// modifications, edit tool/error.xml and re-run that script.")
        self.lines.append("//")

    def write_blank_line(self):
        self.lines.append("")

    def write_line(self, line):
        indent = ""
        for i in range(self.indent_level):
            indent += self.indent
        self.lines.append("{}{}".format(indent, line))

    def complete(self):
        with open(self.path, 'w') as f:
            f.write('\n'.join(self.lines))

class WpdError():
    def __init__(self, name, index):
        self.name = name
        self.index = index

    def full_name(self):
        return "WPD_ERROR_{}".format(self.name)


def generate_error_header(errors, path):
    writer = Writer(path)

    writer.write_comment_header()
    writer.write_blank_line()

    writer.write_line('#ifndef ERROR_G_H')
    writer.write_line('#define ERROR_G_H')

    writer.write_blank_line()

    writer.write_line("#include <stdint.h>")

    writer.write_blank_line()

    writer.write_line("typedef uint32_t wpd_error_t;")

    writer.write_blank_line()

    for error in errors:
        writer.write_line("static const wpd_error_t {} = {};".format(
            error.full_name(), error.index))

    writer.write_blank_line()
    
    writer.write_line("const char *")
    writer.write_line("wpd_error_str(wpd_error_t error);")

    writer.write_blank_line()

    writer.write_line('#endif // ERROR_G_H')

    writer.write_blank_line()

    writer.complete()

def generate_error_implementation(errors, path):
    writer = Writer(path)

    writer.write_comment_header()

    writer.write_blank_line()

    writer.write_line("#include \"error.g.h\"")
    writer.write_line("#include <assert.h>")

    writer.write_blank_line()

    writer.write_line("static const char * error_strings[] = {")
    writer.increase_indent()
    for error in errors:
        writer.write_line('"{}",'.format(error.full_name()))
    writer.decrease_indent()
    writer.write_line("};")

    writer.write_blank_line()

    writer.write_line("const char *")
    writer.write_line("wpd_error_str(wpd_error_t error)")
    writer.write_line("{")
    writer.increase_indent()
    writer.write_line("return error_strings[error];")
    writer.decrease_indent()
    writer.write_line("}")

    writer.write_blank_line()

    writer.complete()

def generate_error_files():
    tree = ET.parse(os.path.join(TOOL_DIR, 'error.xml'))
    root = tree.getroot()

    index = 0
    errors = []
    for error in root.findall('Error'):
        errors.append(WpdError(error.attrib['name'], index))
        index += 1

    generate_error_header(errors, os.path.join(SRC_DIR, 'error.g.h'))
    generate_error_implementation(errors, os.path.join(SRC_DIR, 'error.g.c'))
        
    return root

def main():
    generate_error_files()

if __name__ == '__main__':
    main()

