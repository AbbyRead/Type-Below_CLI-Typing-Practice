# TypeBelow: A Simple, Terminal-Based Typing Practice Program

## Overview

This program reads lines from a specified text file and shows them one by one in the terminal.  
You follow along by typing each line and pressing Enter before continuing.
My focus here is to make something that fits my own use case.

## Features
- Shows one line at a time from the input file or from piped-in text.
- Accepts an optional starting line number to begin practicing from any line.
- Accepts negative starting line number to offset from the end of the source text.
- Reports ending line number to let you easily start where you left off next time.

## Anti-Features
- Does not halt your progress if you type something wrong.
- Does not track your typing speed or mistakes.
- Does not store any data (no profiles, saves, etc).

## Usage
```bash
./typebelow <text_file> [starting_line_number]
