# AwesomePascal

Another PL/0 language (https://en.wikipedia.org/wiki/PL/0) compiler and debugger that is written in C++.   

## Introduction

This code came from my undergrad course project for compiler principles in 2015. The code quality is beyond horrible but it's some of my earliest big projects.

## Architecture
- The compiler uses **SLR(1)** parsing. The grammar is hard-coded in the form of an LR parsing table (`states`) and the FOLLOW set (`follows`) which is initialized in `initStates()` function.
- The compiler was written from the ground up **without any third-party libraries** except the standard C++ library.
- It contains:
  - A lexer (`GETSYM()` function) that processes the input into tokens and literals while constructing a symbol table
  - A parser (`BLOCK()` function) that uses shift/reduce-parsing to generate the target code (pseudo-assembly)
  - An interpreter (`INTERPRET()` function) that executes the pseudo-assembly code.

##  Build 

Supported Environments:   
- MSVC project: AwesomePascal.vcxproj 
- GNU C++/Clang: compile AwesomePascal.cpp directly  

## Examples
- InsertionSort.pas: A simple program that takes 5 integers and sorts them in descending order.
- NumericExpr.pas: A simple program that contains numerical expressions and control flow. 
- 
## Usage

>`$ g++ -O3 -DNDEBUG AwesomePascal.cpp -o BP`  
>`$ ./BP InsersionSort.pas`  

