UTF8 - Simple Library for Internationalization
=============================================

While most of the (computing) world has standardized on using UTF-8 encoding,
Win32 has remained stuck with wide character strings (also called UTF-16 encoding).

This library simplifies usage of UTF-8 encoded strings under Win32 using principles outlined in the [UTF-8 Everywhere Manifesto](http://utf8everywhere.org/).

Here is an example of a function call:
````
  utf8::mkdir (u8"ελληνικό");   //create a directory with a UTF8-encoded name
````
and another example of a C++ stream with a name and content that are not ASCII characters:
````
  utf8::ofstream u8strm(u8"😃😎😛");

  u8strm << u8"Some Cree ᓀᐦᐃᔭᐍᐏᐣ text" << endl;
  u8strm.close ();
````

A call to Windows API functions can be written as:
````
  HANDLE f = CreateFile (utf8::widen (u8"ελληνικό").c_str (), GENERIC_READ, 0,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
````

# Usage #
Before using this library, please review the guidelines from the
[UTF-8 Everywhere Manifesto](http://utf8everywhere.org/). In particular:
- define UNICODE or _UNICODE in your program
- for Visual Studio users, make sure "Use Unicode Character Set" option is defined
  (under "Configuration Properties" > "General" > "Project Defaults" page).
- for Visual Studio users, add "/utf-8" option under "C/C++" > "All Options" >
  "Additional Options" (see https://docs.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8)
- use only `std::string` and `char*` variables. Assume they all contain UTF-8
  encoded strings.
- use UTF-16 strings **only** in arguments to Windows API calls.

All functions and classes in this library are included in the `utf8` namespace.
It is a good idea **not** to have a using directive for this namespace. That makes it
more evident in the code where UTF8-aware functions are used.

## Narrowing and Widening Functions ##
The basic conversion functions change the encoding between UTF-8, UTF-16 and UTF-32.

`narrow()` function converts strings from UTF-16 or UTF-32 encoding to UTF-8:
````
std::string utf8::narrow (const wchar_t* s, size_t nch=0);
std::string utf8::narrow (const std::wstring & s);
std::string utf8::narrow (const char32_t* s, size_t nch=0);
std::string utf8::narrow (const std::u32string& s);	
````

The `widen()` function converts from UTF-16:
````
std::wstring utf8::widen (const char* s, size_t nch);
std::wstring utf8::widen (const std::string& s);
````
The `runes()` function converts from UTF-32:
````
std::u32string runes (const char* s, size_t nch = 0);
std::u32string utf8::runes (const std::string& s);
````

There are also functions for:
- character counting
- string traversal
- validity checking

## Case Folding Functions ##
Case folding (conversion between upper case and lower case) in Unicode is more complicated than traditional
ASCII case conversion. This library uses standard tables published by Unicode Consortium to perform upper
case to lower case conversions and case-insensitive string comparison.

- case folding - `toupper()` and `tolower()`
- case-insensitive string comparison - `icompare()`

## Common "C" Functions Wrappers
The library provides UTF-8 wrappings most frequently used C functions. Function name and arguments match their
traditional C counterparts.
- Common file access operations: `utf8::fopen`, `utf8::access`, `utf8::remove`, `utf8::chmod`, `utf8::rename`
- Directory operations: `utf8::mkdir`, `utf8::rmdir`, `utf8::chdir`, `utf8::getcwd`
- Environment functions: `utf8::getenv`, `utf8::putenv`
- Character classification functions *is...* (`isalnum`, `isdigit`, etc.)

## C++ File I/O Streams ##
C++ I/O streams (`utf8::ifstream`, `utf8::ofstream`, `utf8::fstream`) provide and easy way to create files
with names that are encoded using UTF-8. Because UTF-8 strings are character strings, reading and writing from these files can be done with standard insertion and extraction operators.

## Windows-Specific Functions ##
- path management: `splitpath`, `makepath`
- conversion of command-line arguments: `get_argv` and `free_argv`
- popular Windows API functions: `MessageBox`, `LoadString`

The API for Windows profile files (also called INI files) was replaced with an object `utf8::IniFile`.

# Documentation #
[Doxygen](http://www.doxygen.nl/) documentation can be found at https://neacsum.github.io/utf8/

## License ##

The MIT License (MIT)
 
Copyright (c) 2014-2021 Mircea Neacsu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 
# Building #
The UTF8 library doesn't have any dependencies. The test program however uses
the [UTTP library](https://github.com/neacsum/utpp).



