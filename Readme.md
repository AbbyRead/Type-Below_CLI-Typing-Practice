# Terminal Typing Practice

A VERY minimal terminal-based typing practice program written in C.

## Overview

This program reads lines from a specified text file and shows them one by one in the terminal.  
You follow along by typing each line and pressing Enter before continuing.
My focus here is to make something that fits my own use case.

It does NOT:
- Stop you if you get something wrong.
- Track your typing speed or mistakes.
- Store any data (no profiles, saves, etc).

## Features

- Shows one line at a time from the input file.
- Skips blank lines automatically.
- Minimal dependencies — just standard C and terminal input/output.
- (soon) Accepts an optional starting line number to begin practicing from any line.
- (soon) On pressing `Ctrl+D` (EOF), the program reports the line number where you left off.

## Usage
```bash
./type_below <text_file>
