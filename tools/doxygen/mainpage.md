# UTF8 - Simple Library for Internationalization # {#mainpage}

Functions to facilitate Windows handling of I18N problems using
the strategy advocated by [UTF-8 Everywhere Manifesto](http://utf8everywhere.org/).

## Purpose
This is a library designed to simplify the usage of UTF-8 strings under Win32.
While most of the (computing) world has standardized on using UTF-8 encoding,
Win32 has remained stuck with wide character strings (also called UTF-16 encoding).

The library uses the principles outlined in the [UTF-8 Everywhere Manifesto](http://utf8everywhere.org/) to make it easy to switch between the different character encodings.

## Content
The main function groups are:
- \ref basecvt "Narrowing/widening functions"
- \ref charclass "Character classification functions"
- \ref folding  "Case folding and case-insensitive comparison" 
- \ref inifile "INI file replacement API"
- \ref reg "Registry functions"

There are also functions for:
- character counting - \ref utf8::length() "length"
- string traversal - \ref utf8::next() "next"
- validity checking - \ref utf8::valid_str() "valid_str"

In addition to those, there are wrappings for commonly used C and C++ functions:
- file access operations: \ref utf8::fopen() "fopen", \ref utf8::access() "access", 
\ref utf8::remove() "remove", \ref utf8::chmod() "chmod", \ref utf8::rename() "rename"
- directory operations: \ref utf8::mkdir() "mkdir", \ref utf8::rmdir() "rmdir", 
\ref utf8::chdir() "chdir", \ref utf8::getcwd() "getcwd"
- symbolic link creation: \ref utf8::symlink() "symlink"
- path management: \ref utf8::splitpath() "splitpath", \ref utf8::makepath() "makepath"
- environment functions: \ref utf8::getenv() "getenv", \ref utf8::putenv() "putenv"
- conversion of command-line arguments: \ref utf8::get_argv() "getargv" and \ref utf8::free_argv() "freeargv"
- C++ I/O streams: \ref utf8::ifstream "ifstream", \ref utf8::ofstream "ofstream", \ref utf8::fstream "fstream"
- File enumerating functions: \ref utf8::find_first() "find_first", \ref utf8::find_next() "find_next"
- A \ref utf8::file_enumerator "file enumerator" object wrapping find_first/find_next functions.
- A simple \ref utf8::buffer "buffer class" for handling Windows API parameters. 
- \ref utf8::IniFile "IniFile" - a class for handling Windows "profile files" API.
- Some frequently used Windows functions like \ref utf8::MessageBox() "MessageBox", 
\ref utf8::LoadString() "LoadString", \ref utf8::ShellExecute "ShellExecute", 
\ref utf8::GetTempPath "GetTempPath", \ref utf8::GetTempFileName "GetTempFileName", 
\ref utf8::GetFullPathName "GetFullPathName", \ref utf8::CopyFile "CopyFile"

## Usage
### General
Before using this library you might want to review the guidelines from the
[UTF-8 Everywhere Manifesto](http://utf8everywhere.org/). In particular:
- define UNICODE or _UNICODE in your program
- for Visual Studio users, make sure "Use Unicode Character Set" option is defined
  under "General" > "Project Defaults" tab.
- for Visual Studio users, add [`/utf-8`](https://docs.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8) option under "C/C++" > "All Options" >
  "Additional Options".
- use only `std::string` and `char\*` variables. Assume they all contain UTF-8
  encoded strings.
- use UTF-16 strings only in arguments to Windows API calls.

All functions and classes in this library are included in the *utf8* namespace.
It is a good idea not to have a using directive for this namespace. That makes it
more evident in the code where UTF8-aware functions are used.

This is an example of a function call:
```cpp
  std::string dirname = "ελληνικό";
  utf8::mkdir (dirname);   //create a directory with a UTF8-encoded name
```
Most functions mimic the behavior of standard C functions. A notable exception is
the access() function. The utf8::access() function returns a bool value while the
standard library function returns 0 if the file can be accessed.
 
### I/O Streams
Wrappers for C++ I/O streams can be used for file streams with UTF-8 encoded filenames.
This is an example of a C++ stream with a weird name and content:
```cpp
  std::string filename = "ελληνικό";
  std::string filetext{ "😃😎😛" };

  utf8::ofstream u8strm(filename);

  u8strm << filetext << endl;
  u8strm.close ();
```
Translation from UTF-16 to UTF-8 applies only to file names. C++ streams are agnostic
about their content.

Calling Windows API functions can be handled using the generic widening and
narrowing functions like this example:
```cpp
  std::string filename = "ελληνικό";
  HANDLE f = CreateFile (utf8::widen (filename).c_str (), GENERIC_READ, 0,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
```

### INI File Handling
INI files, also called "profile files" in Microsoft parlance are still widely used for storing application settings used either for compatibility reasons or because they are simple to work with.

The problem is that the basic Windows API calls for reading and writing INI files, `GetPrivateProfileString` and `PutPrivateProfileString`, combine both the file name and the information to be read or written in one API call. As an example, here is the signature of the `GetPrivateProfileStringW` function:
```cpp
DWORD GetPrivateProfileStringW(
  LPCWSTR lpAppName,
  LPCWSTR lpKeyName,
  LPCWSTR lpDefault,
  LPWSTR  lpReturnedString,
  DWORD   nSize,
  LPCWSTR lpFileName
);
```
Using the `utf8::widen` function to convert all UTF-8 strings would produce an INI file that contains UTF-16 characters.

The solution is to completely forget about the Windows API functions and roll a new implementation for accessing INI files. This is by far not the only implementation of INI files that you can find out there. For a list of implementations you can check the [Wikipedia page](https://en.wikipedia.org/wiki/INI_file). This implementation struggles to be as compatible as possible with the original Windows API.

The only changes compared to the Windows API are:
 - line length defaults to 1024 (the INI_BUFFER_SIZE value) while Windows limits it to 256 characters
 - files without a path are in current directory while Windows places them in Windows folder

### Case Conversion
Case conversion in Unicode is a much more complicated issue than ASCII case conversion.
This library uses standard tables published by Unicode Consortium to perform upper case
to lower case conversions. There is also a function `icompare()` that performs string
comparison ignoring the case.


## Building
The UTF8 library doesn't have any dependencies. The test programs however uses the [UTTP library](https://github.com/neacsum/utpp).

You can use the [CPM - C/C++ Package Manager](https://github.com/neacsum/cpm) to fetch all dependent packages and build them. Just issue the `cpm` command.

The Visual C++ projects are set to compile under C++14 rules and they also compiles fine under C++17 rules. If you are using C++20 rules, you have to add the [`Zc:char8_t-`](https://learn.microsoft.com/en-us/cpp/build/reference/zc-char8-t?view=msvc-170) option.

Alternatively, `BUILD.bat` script will build the libraries and test programs.

## License

The MIT License (MIT)
 
Copyright (c) 2014-2023 Mircea Neacsu

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



