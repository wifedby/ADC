#!/usr/bin/env python3
"""
Clean all CubeMX-generated comments from source files.
Removes: copyright headers, section separators, doxygen blocks,
USER CODE BEGIN/END markers, and inline CubeMX comments.
"""

import re
import os

BASE = os.path.dirname(os.path.abspath(__file__))

FILES = [
    "Core/Src/main.c",
    "Core/Src/gpio.c",
    "Core/Src/adc.c",
    "Core/Src/stm32f1xx_it.c",
    "Core/Src/stm32f1xx_hal_msp.c",
    "Core/Inc/main.h",
    "Core/Inc/adc.h",
    "Core/Inc/gpio.h",
    "Core/Inc/stm32f1xx_it.h",
    "Core/Inc/stm32f1xx_hal_conf.h",
]


def read_file(path):
    with open(path, "r", encoding="utf-8") as f:
        return f.read()


def write_file(path, content):
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)


def is_section_separator(line):
    """Lines like '/* Includes ------------------------------------------------------------------*/'
       or    '/*----------------------------------------------------------------------------*/'
       or    '/* Configure GPIO */' (a section title without dashes)"""
    s = line.strip()
    # Pattern 1: /* text ---- */
    if re.match(r'^/\* .*?-{2,}\*/$', s):
        return True
    # Pattern 2: /*---- text ----*/
    if re.match(r'^/-{2,}.*?-{2,}\*/$', s):
        return True
    # These specific section headers
    if s in (
        '/* Configure GPIO                                                             */',
        '/*----------------------------------------------------------------------------*/',
    ):
        return True
    return False


def is_cubemx_inline_comment(line):
    """Check for inline CubeMX comments."""
    s = line.strip()
    triggers = [
        'MCU Configuration',
        'Reset of all peripherals',
        'Configure the system clock',
        'Initialize all configured peripherals',
        'Infinite loop',
        'GPIO Ports Clock Enable',
        'Configure GPIO pin Output Level',
        'Configure GPIO pin :',
        'ADC1 clock enable',
        'Peripheral clock disable',
        'ADC1 init function',
        'Configure pins as',
        'Configure Regular Channel',
        'Common config',
    ]
    for t in triggers:
        if t in s:
            return True
    return False


def is_user_code_marker(line):
    """Check if line is a USER CODE BEGIN/END marker."""
    s = line.strip()
    if 'USER CODE BEGIN' in s or 'USER CODE END' in s:
        return True
    return False


def is_user_leftover_comment(line):
    """Check if line is part of a user-code comment left from USER CODE blocks."""
    s = line.strip()
    if 'User can add' in s:
        return True
    if s == '*/' and 'User can add' in line:
        return True
    return False


def clean_file(filepath):
    print(f"Processing: {filepath}")
    text = read_file(filepath)
    lines = text.split('\n')
    output_lines = []
    in_doxygen = False
    in_header_block = False
    in_user_comment = False

    for line in lines:
        stripped = line.strip()

        # --- State: header block ---
        if 'USER CODE BEGIN Header' in stripped:
            in_header_block = True
            continue
        if in_header_block:
            if 'USER CODE END Header' in stripped:
                in_header_block = False
            continue

        # --- State: doxygen block /** ... */ ---
        if stripped.startswith('/**') and not stripped.startswith('/**/'):
            # Could be multi-line or single-line
            if stripped.endswith('*/'):
                # Single-line doxygen: skip it entirely
                continue
            in_doxygen = True
            continue
        if in_doxygen:
            if stripped == '*/':
                in_doxygen = False
            continue

        # --- State: user leftover multi-line comment ---
        if 'User can add' in stripped and not stripped.endswith('*/'):
            in_user_comment = True
            continue
        if in_user_comment:
            if stripped.endswith('*/'):
                in_user_comment = False
            continue
        if stripped.startswith('* User can add'):
            continue
        if stripped == '*/' and in_user_comment:
            in_user_comment = False
            continue

        # --- Single-line patterns ---
        if is_section_separator(line):
            continue
        if is_cubemx_inline_comment(line):
            continue
        if is_user_code_marker(line):
            continue
        if is_user_leftover_comment(line):
            continue

        output_lines.append(line)

    text = '\n'.join(output_lines)

    # Clean up excess blank lines
    text = re.sub(r'\n{3,}', '\n\n', text)
    text = re.sub(r'^\n+', '', text)
    text = text.rstrip('\n') + '\n'

    write_file(filepath, text)
    print(f"  Done.")


def main():
    for rel_path in FILES:
        full_path = os.path.join(BASE, rel_path)
        if os.path.exists(full_path):
            clean_file(full_path)
        else:
            print(f"Skipping (not found): {full_path}")


if __name__ == "__main__":
    main()
