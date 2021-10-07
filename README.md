# LemonDB Recovered Files

## Introduction

- ./src
   Contains the source code for an old version of lemnonDB.
   The code was recovered from crash site. As far as we know the
   original developer used CMake as their building system.

- ./bin
   Contains the lastest binary that survived the crash.

- ./db
   Contains sample database files.

- ./sample
   Sample inputs and outputs

## Developer Quick Start

See INSTALL.md for instructions on building from source.

`ClangFormat` and `EditorConfig` are used to format codes.

Hint to using `ClangFormat`:
`find . -name "*.m" -o -name "*.h" | sed 's| |\\ |g' | xargs clang-format -i`

And make sure your code editor has `EditorConfig` support.

## Copyright

Lemonion Inc. 2018
