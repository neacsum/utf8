UTF8 - Simple Library for Internationalization
=============================================

While most of the (computing) world has standardized on using UTF-8 encoding,
Win32 has remained stuck with wide character strings (also called UTF-16 encoding).

This library simplifies usage of UTF-8 encoded strings under Win32 using principles outlined in the [UTF-8 Everywhere Manifesto](http://utf8everywhere.org/).

Here is an example of a function call:
```C++
  utf8::mkdir ("ελληνικό");   //create a directory with a UTF8-encoded name
```
and another example of a C++ stream with a name and content that are not ASCII characters:
```C++
  utf8::ofstream u8strm("😃😎😛");

  u8strm << "Some Cree ᓀᐦᐃᔭᐍᐏᐣ text" << endl;
  u8strm.close ();
```

A call to Windows API functions can be written as:
```C++
  HANDLE f = CreateFile (utf8::widen ("ελληνικό").c_str (), GENERIC_READ, 0,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
```

## Usage
Before using this library, please review the guidelines from the
[UTF-8 Everywhere Manifesto](http://utf8everywhere.org/). In particular:
- define UNICODE or _UNICODE in your program
  
- for Visual C++ users, make sure "Use Unicode Character Set" option is defined (under "Configuration Properties" > "General" > "Project Defaults" page).

- for Visual C++ users, add [`/utf-8`](https://docs.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8) option under "C/C++" > "All Options" > "Additional Options".

- use only `std::string` and `char*` variables. Assume they all contain UTF-8 encoded strings.

- for Visual C++ users, if compiling under C++20 language standard, add the [`Zc:char8_t-`](https://learn.microsoft.com/en-us/cpp/build/reference/zc-char8-t?view=msvc-170) option under "C/C++" > "All Options" >"Additional Options" (see discussion below.)
  
- use UTF-16 strings **only** in arguments to Windows API calls.

All functions and classes in this library are included in the `utf8` namespace. It is a good idea **not** to have a using directive for this namespace. That makes it more evident in the code where UTF8-aware functions are used.

### Narrowing and Widening Functions
The basic conversion functions change the encoding between UTF-8, UTF-16 and UTF-32.

`narrow()` function converts strings from UTF-16 or UTF-32 encoding to UTF-8:
```C++
std::string utf8::narrow (const wchar_t* s, size_t nch=0);
std::string utf8::narrow (const std::wstring & s);
std::string utf8::narrow (const char32_t* s, size_t nch=0);
std::string utf8::narrow (const std::u32string& s);	
```

The `widen()` function converts from UTF-16:
```C++
std::wstring utf8::widen (const char* s, size_t nch);
std::wstring utf8::widen (const std::string& s);
```
The `runes()` function converts from UTF-32:
```C++
std::u32string runes (const char* s, size_t nch = 0);
std::u32string utf8::runes (const std::string& s);
```

There are also functions for:
- character counting
- string traversal
- validity checking

### Case Folding Functions
Case folding (conversion between upper case and lower case) in Unicode is more complicated than traditional ASCII case conversion. This library uses standard tables published by Unicode Consortium to perform upper case to lower case conversions and case-insensitive string comparison.

- case folding - `toupper()`, `tolower()`, `make_upper()`, `make_lower()`
- case-insensitive string comparison - `icompare()`

### Common "C" Functions Wrappers
The library provides UTF-8 wrappings most frequently used C functions. Function name and arguments match their traditional C counterparts.
- Common file access operations: `utf8::fopen`, `utf8::access`, `utf8::remove`, `utf8::chmod`, `utf8::rename`
- Directory operations: `utf8::mkdir`, `utf8::rmdir`, `utf8::chdir`, `utf8::getcwd`
- Environment functions: `utf8::getenv`, `utf8::putenv`
- Program execution: `utf8::system`
- Character classification functions *is...* (`isalnum`, `isdigit`, etc.)

### C++ File I/O Streams
C++ I/O streams (`utf8::ifstream`, `utf8::ofstream`, `utf8::fstream`) provide and easy way to create files
with names that are encoded using UTF-8. Because UTF-8 strings are character strings, reading and writing from these files can be done with standard insertion and extraction operators.

### Windows-Specific Functions
- path management: `splitpath`, `makepath`
- conversion of command-line arguments: `get_argv` and `free_argv`
- popular Windows API functions: `MessageBox`, `LoadString`, `ShellExecute`, `CopyFile`, etc.
- Registry API (`RegCreateKey`, `RegOpenKey`, `RegSetValue`, `RegGetValue`, etc.)

The API for Windows profile files (also called INI files) was replaced with an object `utf8::IniFile`.

### Error Handling
Invalid characters or sequences can be handled in tow different ways:
- the invalid character/sequence is replaced by a `REPLACEMENT_CHARACTER` (0xFFFD)
- the functions throw an exception `utf8::exception`. The member `utf8::exception::code` indicates what has triggered the exception.

The function `error_mode()` selects the error handling strategy. The error handling strategy is thread-safe.

## Using the library under C++20 standard
The C++20 standard has [added an additional type `char8_t`](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0482r6.html), designed to keep UTF-8 encoded characters, and a string type `std::u8string`. By making it a separate type from `char` and `unsigned char`, the committee has also created a number of incompatibilities. For instance the following fragment will produce an error:
```C++
std::string s {"English text"}; //this is ok
s = {u8"日本語テキスト"}; //"Japaneese text" - error
```
You would have to change it to something like:
```C++
std::u8string s {u8"English text"}; 
s = {u8"日本語テキスト"}; 
```
Recently (June, 2022) the committee seems to have changed position and introduced a [compatibility and portability fix - DR2513R3](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2513r3.html) allowing initialization of arrays of `char` or `unsigned char` with UTF-8 string literals. Until the defect report makes its way into the next standard edition, the solution for Visual C++ users who compile under C++20 standard rules is to use the [`Zc:char8_t-`](https://learn.microsoft.com/en-us/cpp/build/reference/zc-char8-t?view=msvc-170) compiler option.

In my opinion, by introducing the `char8_t` type, the committee went against the very principles of UTF-8 encoding. The purpose of the encoding was to extend usage of the `char` type to additional Unicode code points. It has been so successful that it is now the de-facto standard used all across the Internet. Even Windows, that has been a bastion of UTF-16 encoding, is now slowly [moving toward UTF-8](https://learn.microsoft.com/en-us/windows/apps/design/globalizing/use-utf8-code-page).

In this context, the use of `char` data type for anything other than holding encodings of strings, seems out of place. In particular arithmetic computations with `char` or `unsigned char` entities are just a small fraction of the use cases. The standard should try to simplify usage in the most common cases leaving the infrequent ones to bear the burden of complexity.

Following this principle, you would want to write:
```C++
std::string s {"English text"};
s += " and ";
s += "日本語テキスト";
```
with the implied assumption that all `char` strings are UTF-8 encoded character strings.

## Using the library under Linux
While the library was specifically built for Windows environment, a reduced version can be compiled and used under Linux. It has been tested under Ubuntu 22.04 with GCC. Obviously, functions that are specific to the Windows environment are not available.

## Documentation
[Doxygen](http://www.doxygen.nl/) documentation can be found at https://neacsum.github.io/utf8/
 
## Building
The UTF8 library doesn't have any dependencies. The test program however uses the [UTTP library](https://github.com/neacsum/utpp).

The preferred method is to use the [CPM - C/C++ Package Manager](https://github.com/neacsum/cpm) to fetch all dependent packages and build them. Download the [CPM program](https://github.com/neacsum/cpm/releases/latest/download/cpm.exe) and, from the root of the development tree, issue the `cpm` command:
```
  cpm -u https://github.com/neacsum/utf8.git utf8
```

The Visual C++ projects are set to compile under C++17 rules and can also be compiled under C++20 rules. If you are using C++20 rules, you have to add the [`Zc:char8_t-`](https://learn.microsoft.com/en-us/cpp/build/reference/zc-char8-t?view=msvc-170) option as discussed above.

You can build the library using CMake. From the _utf8_ directory:
```
  cmake -S . -B build
  cmake --build build
```
Alternatively, `BUILD.bat` script will build the libraries and test programs.

While the library has been designed for Windows, some of the functions may be useful in a Linux environment. Under Linux, the library can be build using `CPM` as explained before, or with `cmake` using the same commands shown above.


## License
[The MIT License](https://github.com/neacsum/utf8/blob/master/LICENSE)
