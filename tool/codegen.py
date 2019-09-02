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

class WpdErrorCategory():
    def __init__(self, name, index, errors):
        self.name = name
        self.index = index
        self.errors = errors

    def full_name(self):
        return "WPD_ERROR_CATEGORY_{}".format(self.name)


class WpdError():
    def __init__(self, name, index, category):
        self.name = name
        self.index = index
        self.category = category

    def full_name(self):
        return "WPD_ERROR_{}_{}".format(self.category.name, self.name)


def generate_error_header(categories, path):
    writer = Writer(path)

    # File header comment
    writer.write_comment_header()
    writer.write_blank_line()

    # Header guard
    writer.write_line('#ifndef ERROR_G_H')
    writer.write_line('#define ERROR_G_H')
    writer.write_blank_line()

    # Includes
    writer.write_line("#include <stdint.h>")
    writer.write_blank_line()

    # Error category typedef
    writer.write_line("typedef uint32_t wpd_error_category_t;")
    writer.write_blank_line()

    # Error category declarations
    for category in categories:
        writer.write_line("static const wpd_error_category_t {} = {};".format(
            category.full_name(), category.index))
    writer.write_blank_line()

    # Error category string conversion function
    writer.write_line("const char *")
    writer.write_line("wpd_error_category_str(wpd_error_category_t error_category);")
    writer.write_blank_line()

    # Error typedef
    writer.write_line("typedef uint32_t wpd_error_t;")
    writer.write_blank_line()

    # Error declarations
    for category in categories:
        for error in category.errors:
            writer.write_line("static const wpd_error_t {} = {};".format(
                error.full_name(), error.index))
    writer.write_blank_line()
    
    # Error string conversion function
    writer.write_line("const char *")
    writer.write_line("wpd_error_str(wpd_error_t error);")
    writer.write_blank_line()

    # Close header guard
    writer.write_line('#endif // ERROR_G_H')
    writer.write_blank_line()

    writer.complete()

def generate_error_implementation(categories, path):
    writer = Writer(path)

    # File header comment
    writer.write_comment_header()
    writer.write_blank_line()

    # Includes
    writer.write_line("#include \"error.g.h\"")
    writer.write_line("#include <assert.h>")
    writer.write_blank_line()

    # Error category struct definition
    writer.write_line("struct wpd_error_category_definition_t")
    writer.write_line("{")
    writer.increase_indent()
    writer.write_line("const char *str;")
    writer.decrease_indent()
    writer.write_line("};")
    writer.write_blank_line()

    # Error category definition array
    writer.write_line("static struct wpd_error_category_definition_t error_category_definitions[] = {")
    writer.increase_indent()
    for category in categories:
        writer.write_line('{ "' + category.full_name() + '" },')
    writer.decrease_indent()
    writer.write_line("};")
    writer.write_blank_line()

    # Error category string conversion function implementation
    writer.write_line("const char *")
    writer.write_line("wpd_error_category_str(wpd_error_category_t error_category)")
    writer.write_line("{")
    writer.increase_indent()
    writer.write_line("return error_category_definitions[error_category].str;")
    writer.decrease_indent()
    writer.write_line("}")
    writer.write_blank_line()

    # Error struct definition
    writer.write_line("struct wpd_error_definition_t")
    writer.write_line("{")
    writer.increase_indent()
    writer.write_line("wpd_error_category_t category;")
    writer.write_line("const char *str;")
    writer.decrease_indent()
    writer.write_line("};")
    writer.write_blank_line()

    # Error definition array
    writer.write_line("static struct wpd_error_definition_t error_definitions[] = {")
    writer.increase_indent()
    for category in categories:
        for error in category.errors:
            writer.write_line('{ ' + category.full_name() + ', "' + error.full_name() + '" },')
    writer.decrease_indent()
    writer.write_line("};")
    writer.write_blank_line()

    # Error string conversion function implementation
    writer.write_line("const char *")
    writer.write_line("wpd_error_str(wpd_error_t error)")
    writer.write_line("{")
    writer.increase_indent()
    writer.write_line("return error_definitions[error].str;")
    writer.decrease_indent()
    writer.write_line("}")
    writer.write_blank_line()

    writer.complete()

def generate_error_files():
    tree = ET.parse(os.path.join(TOOL_DIR, 'error.xml'))
    root = tree.getroot()

    error_index = 0
    categories = []
    for error_category in root.findall('ErrorCategory'):
        category_name = error_category.attrib['name']
        category_index = len(categories)

        category = WpdErrorCategory(category_name, category_index, [])
        
        errors = []
        for error in error_category.findall('Error'):
            error_name = error.attrib['name']
            errors.append(WpdError(error_name, error_index, category))
            error_index += 1

        category.errors = errors
        categories.append(category)
        
    generate_error_header(categories, os.path.join(SRC_DIR, 'error.g.h'))
    generate_error_implementation(categories, os.path.join(SRC_DIR, 'error.g.c'))
        
    return root

def main():
    generate_error_files()

if __name__ == '__main__':
    main()

