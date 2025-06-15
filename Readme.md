# TypeBelow

This program reads lines from a specified text source and expects that you will follow-along below it by typing the same text.

## Features
- Accepts an optional starting line number to begin practicing from any line.
- Accepts negative starting line number to offset from the end of the source text.
- Reports ending line number to let you easily start where you left off next time.
- Padded bottom for comfort.  Typing at the very edge of the terminal is no fun.

## Anti-Features
- Does not halt your progress if you type something wrong.
- Does not track your typing speed or mistakes.
- Does not store any data (no profiles, saves, etc).

## Usage
```bash
typebelow [options] filename    # Source text from an existing file
typebelow [options] -           # Source from piping/redirection
typebelow [options]             # Source from OS copy/paste clipboard

Options:
  -s <positive_number>          # Start from the specified line number
  -s <negative_number>          # Start a number of lines from the end
  -h, --help                    # Show this help message and exit
  -v, --version                 # Show program version and exit

Examples:
  typebelow -s 10 myfile.txt   # Start at line 10 from file
  typebelow -s -3 -            # Start 3 lines from end, read from stdin
  typebelow                    # Read from clipboard, start at line 1
```
