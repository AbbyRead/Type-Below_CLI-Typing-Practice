# TypeBelow

This program reads lines from a specified text source and expects that you will follow-along below it by typing the same text.

## Features
- Accepts an optional starting line number to begin practicing from any line.
- Accepts negative starting line number to offset from the end of the source text.
- Reports ending line number to let you easily start where you left off next time.

## Anti-Features
- Does not halt your progress if you type something wrong.
- Does not track your typing speed or mistakes.
- Does not store any data (no profiles, saves, etc).

## Usage
```bash
typebelow [options] filename    # Source text from an existing file
typebelow [options] -           # Source text from stdin via piping/redirection
typebelow [options]             # Source text from the OS copy/paste clipboard
```
