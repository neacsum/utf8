#include <utpp/utpp.h>
#include <windows.h>
#include <utf8/utf8.h>
#include <iostream>
#include <filesystem>
#include <tuple>

#include "resource.h"

#pragma warning (disable : 4566)

using namespace std;
using namespace utf8;

TEST_MAIN (int argc, char **argv)
{
  const char* suite_under_test = nullptr;
  std::cerr << "Running " << *argv++ << endl
    << "working directory is: " << getcwd () << endl;
  --argc;
  if (argc && (*argv)[0] == '-')
  {
    if (!strcmp (*argv, "-s") && argc > 1)
    {
      ++argv;
      return UnitTest::RunSuite (argv[1]);
    }
    else
    {
      std::cerr << "Invalid syntax." << endl;
      exit (-1);
    }
  }
  if (argc)
  {
    std::filesystem::path xml_filename(*argv);
    std::ofstream xml_stream (xml_filename);
    UnitTest::ReporterXml xml(xml_stream);
    std::cerr << "Output sent to " 
              << std::filesystem::absolute (xml_filename) << endl;
    return RunAllTests (xml);
  }
  else
    return UnitTest::RunAllTests ();
}

TEST (narrow_with_null)
{
  wstring w1 (L"ABC");
  w1 += L'\0';
  w1 += L"DEF";
  string n1 = utf8::narrow (w1);
  size_t szw = w1.size (), szn = size (n1);

  CHECK_EQUAL (szw, szn);
}

TEST (widen_with_null)
{
  string n1 ("ABC");
  n1 += '\0';
  n1 += "DEF";
  wstring w1 = utf8::widen (n1);
  size_t szw = w1.size (), szn = size (n1);

  CHECK_EQUAL (szn, szw);
}

TEST (widen_string)
{
  string s1 ("ABCD");
  wstring l1(L"ABCD");

  wstring l2 = widen (s1);

  CHECK (l1 == l2);
}

TEST (widen_ptr)
{
  const char *s1 = "ABCD";
  wstring l1(L"ABCD");

  wstring l2 = widen (s1);

  CHECK (l1 == l2);
}

TEST (widen_count)
{
  const char *s1 = "ABCDEFGH";
  wstring l1 (L"ABCD");

  wstring l2 = widen (s1, 4);

  CHECK (l1 == l2);

}

TEST (narrow_string)
{
  wstring l1(L"ABCD");
  string s1 = narrow(l1);

  CHECK ("ABCD" == s1);
}

TEST (narrow_ptr)
{
  const wchar_t *l1 = L"ABCD";
  string s1 = narrow(l1);

  CHECK ("ABCD" == s1);
}

TEST (narrow_count)
{
  const wchar_t *l1 = L"ABCDEFGH";
  string s1 = narrow (l1, 4);

  CHECK ("ABCD" == s1);
}

TEST (narrow_char32)
{
  const char32_t* u1 = U"A😀BCDEFGH";
  string s1 = narrow (u1, 4);
  CHECK_EQUAL (u8"A😀BC", s1);
}

TEST (widen_narrow)
{
  const char *ptr = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  CHECK_EQUAL (ptr, narrow(widen(ptr)).c_str());
}

TEST (surrogates)
{
  //example from RFC8259
  const wchar_t* gclef{ L"\xd834\xdd1e" };
  const char32_t rune_gclef = 0x1d11e;
  string u8_1, u8_2;
  u8_1 = utf8::narrow (gclef);
  u8_2 = utf8::narrow (&rune_gclef, 1);
  CHECK_EQUAL (u8_1, u8_2);
}

TEST (greek_letters)
{
  const wchar_t *greek = L"ελληνικό αλφάβητο";
  string s = narrow (greek);
  CHECK (widen (s) == greek);
}

TEST (string_len)
{
  const wchar_t *greek = L"ελληνικό αλφάβητο";
  string s = narrow (greek);
  size_t l = length (s);
  CHECK_EQUAL (wcslen (greek), l);
}

TEST (wemoji)
{
  const wchar_t *wsmiley = L"😄";
  size_t wlen = wcslen (wsmiley);
  CHECK_EQUAL (2, wlen);
  string smiley = narrow (wsmiley);
  CHECK_EQUAL ("\xF0\x9f\x98\x84", smiley);
}

TEST (rune)
{
  string smiley{ u8"😀" };
  int rune_smiley = rune (smiley.begin ());
  CHECK_EQUAL (0x1f600, rune_smiley);
}

TEST (rune2)
{
  const char* smiley{ u8"😀" };
  char32_t rune_smiley = rune (smiley);
  CHECK_EQUAL ((int)U'😀', (int)rune_smiley);
}


//check that next function advances with one code point
TEST (next)
{
  string emojis{ u8"😃😎😛" };
  int i = 0;
  auto ptr = emojis.begin ();
  while (ptr != emojis.end ())
  {
    next (ptr, emojis.end ());
    i++;
  }
  CHECK_EQUAL (3, i);
}

// same test but using a character pointer instead of a string iterator
TEST (next_ptr)
{
  string emojis{ u8"😃😎😛" };
  int i = 0;
  const char *ptr = emojis.c_str ();
  while (utf8::next (ptr))
  {
    i++;
  }
  CHECK_EQUAL (3, i);
}

TEST (next_non_const)
{
  char emojis[20];
  strcpy (emojis, u8"😃😎😛" );
  int i = 0;
  char* ptr = emojis;
  while (utf8::next (ptr))
  {
    i++;
  }

  CHECK (*ptr == 0);
  CHECK_EQUAL (3, i);
}

TEST (overlong)
{
  const char* o1{ "\xf0\x82\x82\xac" }; //overlong euro sign
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, utf8::rune (o1));
}

TEST (encoding_limits)
{
  CHECK_EQUAL ("\x0", narrow (U"\x0"));
  CHECK_EQUAL ("\x7f", narrow (U"\x7f"));
  CHECK_EQUAL ("\xc2\x80", narrow (U"\x80"));
  CHECK_EQUAL ("\xdf\xbf", narrow (U"\x7ff"));
  CHECK_EQUAL ("\xe0\xa0\x80", narrow (U"\x800"));
  CHECK_EQUAL ("\xef\xbf\xbf", narrow (U"\xffff"));
  CHECK_EQUAL ("\xf0\x90\x80\x80", narrow (U"\x10000"));
  CHECK_EQUAL ("\xf4\x8f\xbf\xbf", narrow (U"\x10ffff"));

  CHECK_EQUAL ("\xf0\x90\x80\x80", narrow (L"\xd800\xdc00"));
  CHECK_EQUAL ("\xf4\x8f\xbf\xbf", narrow (L"\xdbff\xdfff"));

  CHECK_EQUAL ("\xf0\x90\x80\x80", narrow (wstring(L"\xd800\xdc00")));
  CHECK_EQUAL ("\xf4\x8f\xbf\xbf", narrow (wstring(L"\xdbff\xdfff")));
}

TEST (decoding_limits)
{
  CHECK_EQUAL (0, rune ("\x0"));
  CHECK_EQUAL (0x7f, rune ("\x7f"));
  CHECK_EQUAL (0x80, rune ("\xc2\x80"));
  CHECK_EQUAL (0x7ff, rune ("\xdf\xbf"));
  CHECK_EQUAL (0x800, rune ("\xe0\xa0\x80"));
  CHECK_EQUAL (0xffff, rune ("\xef\xbf\xbf"));
  CHECK_EQUAL (0x10000, rune ("\xf0\x90\x80\x80"));
  CHECK_EQUAL (0x10ffff, rune ("\xf4\x8f\xbf\xbf"));

  CHECK_EQUAL (wstring (L"\xd800\xdc00"), widen ("\xf0\x90\x80\x80"));
  CHECK_EQUAL (wstring (L"\xdbff\xdfff"), widen ("\xf4\x8f\xbf\xbf"));
}

TEST (next_invalid_replace)
{
  string s2 = u8"°";
  string s3 = u8"€";
  string s4 = u8"😃";
  auto prev_mode = utf8::error_mode(utf8::action::replace);

  auto p = s2.begin () + 1;
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, next (p, s2.end ()));
  p = s3.begin () + 1;
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, next (p, s3.end ()));
  p = s4.begin () + 1;
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, next (p, s4.end ()));

  p = s2.begin ();
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, next (p, s2.end () - 1));
  p = s3.begin ();
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, next (p, s3.end () - 1));
  p = s4.begin ();
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, next (p, s4.end () - 1));

  utf8::error_mode (prev_mode);
}

TEST (next_invalid_throw)
{
  string s2 = u8"°";
  string s3 = u8"€";
  string s4 = u8"😃";
  auto prev_mode = utf8::error_mode (utf8::action::except);

  auto p = s2.begin () + 1;
  CHECK_THROW_EQUAL (next (p, s2.end ()), utf8::exception(utf8::exception::invalid_utf8), utf8::exception);
  p = s3.begin () + 1;
  CHECK_THROW_EQUAL (next (p, s3.end ()), utf8::exception (utf8::exception::invalid_utf8), utf8::exception);
  p = s4.begin () + 1;
  CHECK_THROW_EQUAL (next (p, s4.end ()), utf8::exception (utf8::exception::invalid_utf8), utf8::exception);

  p = s2.begin ();
  CHECK_THROW_EQUAL (next (p, s2.end () - 1), utf8::exception (utf8::exception::invalid_utf8), utf8::exception);
  p = s3.begin ();
  CHECK_THROW_EQUAL (next (p, s3.end () - 1), utf8::exception (utf8::exception::invalid_utf8), utf8::exception);
  p = s4.begin ();
  CHECK_THROW_EQUAL (next (p, s4.end () - 1), utf8::exception (utf8::exception::invalid_utf8), utf8::exception);

  utf8::error_mode (prev_mode);
}

TEST (valid_str_funcs)
{
  string emojis{ u8"😃😎😛" };
  CHECK (utf8::valid_str (emojis));
  CHECK (utf8::valid_str (emojis.c_str ()));
  CHECK (utf8::valid_str (emojis.c_str (), emojis.length()));

  CHECK (!utf8::valid_str (emojis.c_str (), emojis.length () - 1));
  emojis[8] = 0;
  CHECK (utf8::valid_str (emojis.c_str ()));
  emojis[6] = 0;
  CHECK (!utf8::valid_str (emojis.c_str (), emojis.length ()));
}

TEST (is_valid_yes)
{
  string s1 = u8"a";
  string s2 = u8"°";
  string s3 = u8"€";
  string s4 = u8"😃";

  CHECK (is_valid (s1.c_str()));
  CHECK (is_valid (s2.c_str ()));
  CHECK (is_valid (s3.c_str ()));
  CHECK (is_valid (s4.c_str ()));

  // same tests with string iterators
  CHECK (is_valid (s1.begin (), s1.end ()));
  CHECK (is_valid (s2.begin (), s2.end ()));
  CHECK (is_valid (s3.begin (), s3.end ()));
  CHECK (is_valid (s4.begin (), s4.end ()));

  CHECK (is_valid ("\xEF\xBB\xBF")); //BOM
  CHECK (is_valid ("")); //empty string
}

TEST (is_valid_no)
{
  CHECK (!is_valid ("\xC1\xA1")); //overlong 'a'
  CHECK (!is_valid ("\xE0\x82\xB0")); //overlong '°'
  CHECK (!is_valid ("\xF0\x82\x82\xAC")); //overlong '€'
  CHECK (!is_valid ("\xFE\xFF")); //UTF-16 BOM BE
  CHECK (!is_valid ("\xFF\xFE")); //UTF-16 BOM LE
  CHECK (!is_valid ("\xED\xA0\x80")); // 0xD800 surrogate code point
}

TEST (prev_ptr)
{
  const char* emojis{ u8"😃😎😛" };
  auto ptr = emojis + strlen(emojis);

  int count = 0;
  while (ptr > emojis)
  {
  std::ignore =  prev (ptr);
    count++;
  }
  CHECK_EQUAL (3, count);
}

TEST (prev_string)
{
  const string emojis{ u8"😃😎😛" };
  u32string runes;
  auto ptr = emojis.end ();
  int count = 0;
  while (ptr > emojis.begin())
  {
    runes.insert (runes.begin(), prev (ptr, emojis.begin()));
    count++;
  }
  CHECK_EQUAL (3, count);
  CHECK_EQUAL (emojis, narrow (runes));
}

TEST (prev_invalid)
{
  const char *invalid_1 = "\xC1\xA1"; //overlong 'a'
  const char *invalid_2 = "\xE0\x82\xB0"; //overlong '°'
  const char *invalid_3 = "\xF0\x82\x82\xAC"; //overlong '€'
  const char *invalid_4 = "\xFE\xFF"; //UTF-16 BOM BE
  const char *invalid_5 = "\xFF\xFE"; //UTF-16 BOM LE
  const char *invalid_6 = "\xED\xA0\x80"; // 0xD800 surrogate code point

  const char* ptr = invalid_1 + strlen (invalid_1);
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (ptr));
  ptr = invalid_2 + strlen (invalid_2);
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (ptr));
  ptr = invalid_3 + strlen (invalid_3);
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (ptr));
  ptr = invalid_4 + strlen (invalid_4);
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (ptr));
  ptr = invalid_5 + strlen (invalid_5);
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (ptr));
  ptr = invalid_6 + strlen (invalid_6);
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (ptr));
}

TEST (prev_invalid_str)
{
  std::string invalid_1{ "\xC1\xA1" }; //overlong 'a'
  std::string invalid_2{ "\xE0\x82\xB0" }; //overlong '°'
  std::string invalid_3{ "\xF0\x82\x82\xAC" }; //overlong '€'
  std::string invalid_4{ "\xFE\xFF" }; //UTF-16 BOM BE
  std::string invalid_5{ "\xFF\xFE" }; //UTF-16 BOM LE
  std::string invalid_6{ "\xED\xA0\x80" }; // 0xD800 surrogate code point

  auto p = end (invalid_1);
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (p, begin (invalid_1)));
  p = end (invalid_2);
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (p, begin (invalid_2)));
  p = end (invalid_3);
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (p, begin (invalid_3)));
  p = end (invalid_4);
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (p, begin (invalid_4)));
  p = end (invalid_5);
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (p, begin (invalid_5)));
  p = end (invalid_6);
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (p, begin (invalid_6)));

  std::string s2 = "x°";
  std::string s3 = "x€";
  std::string s4 = "x😃";
  p = s2.end ()-1;
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (p, begin (s2)));
  p = end (s3) - 1;
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (p, begin (s3)));
  p = end (s4) - 1;
  CHECK_EQUAL (utf8::REPLACEMENT_CHARACTER, prev (p, begin (s4)));
}

TEST (invalid_utf8)
{
  const char invalid[] { "\xFE\xFF\xFF\xFE" }; //UTF-16 BOM markers
  bool thrown = false;
  auto prev_mode = utf8::error_mode (action::except);
  try {
    auto s = runes(invalid);
  }
  catch (utf8::exception& e) {
    CHECK_EQUAL (utf8::exception::invalid_utf8, e.code);
    thrown = true;
    cout << "Exception caught: " << e.what () << endl;
  }
  CHECK (thrown);
  utf8::error_mode (prev_mode);
}

TEST (throw_invalid_char32)
{
  auto prev_mode = utf8::error_mode (action::except);
  bool thrown = false;
  try {
    narrow (0xd800);
  }
  catch (utf8::exception& e) {
    CHECK_EQUAL (utf8::exception::invalid_char32, e.code);
    thrown = true;
    cout << "Exception caught: " << e.what () << endl;
  }
  CHECK (thrown);
  utf8::error_mode (prev_mode);
}

// test for runes function (conversion from UTF8 to UTF32)
TEST (runes)
{
  string emojis{ "😃😎😛" };
  u32string emojis32 = runes (emojis);
  CHECK_EQUAL (3, emojis32.size ());
  CHECK_EQUAL (0x1f603, (int)emojis32[0]);
}


TEST (dir)
{
  /* Make a folder using Greek alphabet, change current directory into it,
  obtain the current working directory and verify that it matches the name
  of the newly created folder */

  string dirname = "ελληνικό";
  CHECK (mkdir (dirname));   //mkdir returns true  for success

  //enter newly created directory
  CHECK (chdir (dirname));   //chdir returns true for success

  //Path returned by getcwd should end in our Greek string
  string cwd = getcwd ();
  size_t idx = cwd.rfind ("\\");      //last backslash
  string last = cwd.substr (idx+1);
  CHECK_EQUAL (dirname, last);

  //Move out of directory and remove it
  chdir ("..");
  CHECK (rmdir (dirname));    //rmdir returrs true for success
}

TEST (symlink)
{
  /* Make a folder using Greek alphabet, and another one using Armenian.
  enter the 2nd directory and create a link to the first. It verifies a file
  created in the first directory is visible through the symlink.*/

  //make first directory
  CHECK (mkdir ("ελληνικό"));
  chdir ("ελληνικό");
  //and a file in it
  utf8::ofstream out ("f.txt");
  out << "text" << endl;
  out.close ();
  chdir ("..");
  //make 2nd directory
  CHECK (mkdir ("Հայերեն"));
  chdir ("Հայերեն");
  //and symlink to first
  CHECK (symlink (u8"..\\ελληνικό", "पंजाबी", true));
  //change into symlink
  chdir ("पंजाबी");
  utf8::ifstream in ("f.txt");
  string str;
  in >> str;
  CHECK_EQUAL ("text", str);
  in.close ();

  //cleanup
  utf8::remove ("f.txt");
  chdir ("..");
  rmdir ("पंजाबी");
  chdir ("..");
  rmdir ("Հայերեն");
  rmdir ("ελληνικό");
}

TEST (out_stream)
{
  /* Write some text in a file with a UTF8 encoded filename. Verifies using
  standard Windows file reading that content was written. */

  string filename = "ελληνικό";
  string filetext{ "😃😎😛" };

  utf8::ofstream u8strm(filename);

  u8strm << filetext << endl;
  u8strm.close ();

  HANDLE f = CreateFile (utf8::widen (filename).c_str (), GENERIC_READ, 0,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  CHECK (f);

  char read_back[80];
  memset (read_back, 0, sizeof (read_back));

  size_t len = filetext.size();
  DWORD nr;
  CHECK (ReadFile (f, read_back, (DWORD)len, &nr, NULL));
  CloseHandle (f);
  CHECK_EQUAL (len, nr);
  CHECK_EQUAL (filetext, read_back);

  //Read back using utf8::fopen API
  auto fil = utf8::fopen (filename, "r");
  CHECK (fil);
  if (fil)
  {
    nr = (DWORD)fread (read_back, sizeof (char), len, fil);
    CHECK_EQUAL (len, nr);
    CHECK_EQUAL (filetext, read_back);
    fclose (fil);
  }

  //cleanup
  CHECK (remove (filename));
}

TEST (in_stream)
{
  /* write some stuff in file using utf8::ofstream object and read it
  back using utf8::ifstream. Verify read back matches original.*/

  string filetext{ "ελληνικό" };
  string filename{ "😃😎😛" };

  utf8::ofstream u8out (filename);

  u8out << filetext << endl;
  u8out.close ();

  utf8::ifstream u8in (filename);

  char read_back[80];
  memset (read_back, 0, sizeof (read_back));
  u8in.getline (read_back, sizeof (read_back));

  CHECK_EQUAL (filetext, read_back);

  u8in.close ();
  CHECK (remove (filename));
}

TEST (fopen_write)
{
  /* Write some text in a file with a UTF8 encoded filename. Verifies using
  standard Windows file reading that content was written. */

  string filename = "ελληνικό";
  string filetext{ "😃😎😛" };
  FILE *u8file = utf8::fopen (filename, "w");
  ABORT_EX (!u8file, "Failed to create output file");

  fwrite (filetext.c_str(), sizeof(char), filetext.length(), u8file);
  fclose (u8file);

  HANDLE f = CreateFile (utf8::widen (filename).c_str (), GENERIC_READ, 0,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  CHECK (f);

  char read_back[80];
  memset (read_back, 0, sizeof (read_back));

  size_t len = filetext.size ();
  DWORD nr;
  CHECK (ReadFile (f, read_back, (DWORD)len, &nr, NULL));
  CloseHandle (f);
  CHECK (remove (filename));
  CHECK_EQUAL (len, nr);
  CHECK_EQUAL (filetext, read_back);
}

TEST (full_path)
{
  const char* fname = "file.txt";
  FILE* f = ::fopen (fname, "w");
  fclose (f);

  char full[_MAX_PATH];
  _fullpath (full, fname, sizeof (full));
  CHECK_EQUAL (full, utf8::fullpath (fname));
  ::remove (fname);
}


TEST (make_splitpath)
{
  const string dir{ "ελληνικό αλφάβητο" },
    fname{ "😃😎😛" };
  string path;
  CHECK (utf8::makepath (path, "C", dir, fname, ".txt"));
  wstring wpath = widen (path);
  CHECK_EQUAL ("C:ελληνικό αλφάβητο\\😃😎😛.txt", path);
  string drv1, dir1, fname1, ext1;
  CHECK (splitpath (path, drv1, dir1, fname1, ext1));

  CHECK_EQUAL ("C:", drv1);
  CHECK_EQUAL (dir + "\\", dir1);
  CHECK_EQUAL (fname, fname1);
  CHECK_EQUAL (".txt", ext1);
}

TEST (get_putenv)
{
  //a long variable
  string path = utf8::getenv ("PATH");
  CHECK (!path.empty ());

  utf8::putenv ("ελληνικό=😃😎😛");
  CHECK_EQUAL ("😃😎😛", utf8::getenv ("ελληνικό"));

  utf8::putenv ("ελληνικό", string ());
  CHECK (utf8::getenv ("ελληνικό").empty ());
}

TEST (msgbox)
{
#if 0
  //requires user's intervention
  utf8::MessageBox (NULL, "ελληνικό", "😃😎😛", MB_ICONINFORMATION);
#endif
}

TEST (buffer_test)
{
  utf8::buffer buf (_MAX_PATH);

  // You can set a buffer to a an initial value through the string assignment
  // operator
  string tmp{ "Some initial value" };
  buf = tmp;

  CHECK_EQUAL (tmp, (string)buf);
  // size doesn't shrink when assigning a string 
  CHECK_EQUAL (_MAX_PATH, buf.size ());

  //Copy ctor
  utf8::buffer buf1(buf);
  CHECK_EQUAL (tmp, (string)buf1);
  CHECK_EQUAL (_MAX_PATH, buf1.size ());

  //Principal assignment operator
  utf8::buffer buf2 (50);
  buf2 = buf1;
  CHECK_EQUAL (tmp, (string)buf2);
  //After buffer assignment, size matches the right-hand side size
  CHECK_EQUAL (_MAX_PATH, buf2.size ());
}

TEST (Temp_FileName)
{
  wstring wpath (_MAX_PATH, L'\0');
  wstring wfname (_MAX_PATH, L'\0');

  GetTempPathW ((DWORD)wpath.size (), const_cast<wchar_t*>(wpath.data ()));

  UINT ret = ::GetTempFileNameW (wpath.c_str(), L"ÄñΩ",
    1, const_cast<wchar_t*>(wfname.data ()));
  CHECK (ret > 0);


  //Do the same thing with utf8::GetTempPath and utf8::GetTempFileName
  auto fname =  utf8::GetTempFileName (utf8::GetTempPath (), "ÄñΩ", 1);

  //see that we get the same result
  CHECK_EQUAL (narrow (wfname.c_str()), fname);
}

//check in-place versions of case folding functions
TEST (case_conversion_inplace)
{
  string lc{ "mircea neacșu ăâățî" };
  string uc{ "MIRCEA NEACȘU ĂÂĂȚÎ" };
  string t = lc;
  utf8::make_upper (t);
  CHECK_EQUAL (uc, t);
  t = uc;
  utf8::make_lower (t);
  CHECK_EQUAL (lc, t);
}

//check string-returning versions of case folding functions
TEST (case_conversion_ret)
{
  string uc = utf8::toupper ("αλφάβητο");
  CHECK_EQUAL ("ΑΛΦΆΒΗΤΟ", uc);
  CHECK_EQUAL ("αλφάβητο", utf8::tolower ("ΑΛΦΆΒΗΤΟ"));
}

//check case-insensitive comparison
TEST (icompare_equal)
{
  string lc{ "mircea neacșu ăâățî" };
  string uc{ "MIRCEA NEACȘU ĂÂĂȚÎ" };
  CHECK (utf8::icompare (lc, uc) == 0);
}

TEST (icompare_less)
{
  string lc{ "mircea neacșu ăâățî" };
  string uc{ "MIRCEA NEACȘU ĂÂĂȚÎ " };
  CHECK (utf8::icompare (lc, uc) < 0);
}

TEST (icompare_greater)
{
  string lc{ "mircea neacșu ăâățî" };
  string uc{ "MIRCEA NEACȘU ĂÂ2ȚÎ" };
  CHECK (utf8::icompare (lc, uc) > 0);
}

static std::string mydir ()
{
  auto fname = GetModuleFileName ();
  std::string drive, dir, name, ext;
  splitpath (fname, drive, dir, name, ext);
  return drive+dir;
}

// find files named "test*" using find_first/find_next functions
TEST (find)
{
  find_data fd;
  bool ret = find_first (mydir()+"test*", fd);
  cout << "find_first: " << fd.filename << " - " << fd.size/1024 << "kb" << endl;
  CHECK (ret);
  while (ret)
  {
    ret = find_next (fd);
    if (ret)
      cout << "find_next: " << fd.filename << " - " << fd.size / 1024 << "kb" << endl;
  }
  cout << endl;
  CHECK_EQUAL (ERROR_NO_MORE_FILES, GetLastError ());
  find_close (fd);
}

// same thing as above using the file_enumerator class
TEST (find_with_finder)
{
  file_enumerator f(mydir()+"test*");
  CHECK (f.ok());
  while (f.ok ())
  {
    cout << "finder: " << f.filename << " - " << f.size / 1024 << "kb" << endl;
    f.next ();
  }
}

TEST (find_missing_file)
{
  file_enumerator f ("no such file");
  CHECK (!f.ok ());
}

TEST (bool_op_missing_file)
{
  file_enumerator found ("no such file");
  CHECK (!found);
}

// test character classes in 0-127 range match standard functions
TEST (char_class)
{
  char chartab[128];
  for (int i = 0; i < 128; i++)
    chartab[i] = (char)i;

  for (int i = 0; i < 128; i++)
  {
    char temp[2];
    temp[0] = chartab[i];
    temp[1] = 0;
    char tst[80];
    sprintf_s (tst, "testing char %d", i);
    CHECK_EQUAL_EX ((bool)isalpha (chartab[i]), utf8::isalpha (temp), tst);
    CHECK_EQUAL_EX ((bool)isalnum (chartab[i]), utf8::isalnum (temp), tst);
    CHECK_EQUAL_EX ((bool)(isdigit) (chartab[i]), utf8::isdigit (temp), tst);
    CHECK_EQUAL_EX ((bool)(isspace) (chartab[i]), utf8::isspace (temp), tst);
    CHECK_EQUAL_EX ((bool)(isblank)(chartab[i]), utf8::isblank (temp), tst);
    CHECK_EQUAL_EX ((bool)(isxdigit) (chartab[i]), utf8::isxdigit (temp), tst);
    CHECK_EQUAL_EX ((bool)isupper (chartab[i]), utf8::isupper (temp), tst);
    CHECK_EQUAL_EX ((bool)islower (chartab[i]), utf8::islower (temp), tst);
  }
}

//skip spaces in UTF-8 string
TEST (skip_spaces)
{
  string s{ " \xC2\xA0日本語" };
  auto p = s.begin ();
  int blanks = 0;
  while (p != s.end () && utf8::isspace (p))
  {
    blanks++;
    CHECK (utf8::next (p, s.end ()));
  }

  CHECK_EQUAL (2, blanks); //both space and "no-break space" are space characters
}


// test character classes outside the 0-127 range
TEST (is_upper_lower)
{
  const char* uc{ "MIRCEANEACȘUĂÂȚÎ" };
  const char* lc{ "mirceaneacșuăâțî" };

  for (auto p = uc; *p; next (p))
    CHECK (isupper (p));

  for (auto p = lc; *p; next (p))
    CHECK (islower (p));
}

// test character classes outside the 0-127 range using string iterators
TEST (is_upper_lower_str)
{
  const std::string uc{ "MIRCEANEACȘUĂÂȚÎ" };
  const std::string lc{ "mirceaneacșuăâțî" };

  auto it = uc.cbegin ();
  while (it != uc.cend())
  {
    CHECK (isupper (it));
    utf8::next (it, uc.cend ());
  }

  for (it = lc.cbegin (); it != lc.cend();)
  {
    CHECK (islower (it));
    utf8::next (it, lc.cend ());
  }
}

TEST (lower_substring)
{
  const string uc{ "ȚEPUȘ nicolae" };
  const string lc{ "Țepuș nicolae" };

  auto p = uc.begin ();
  string s = utf8::narrow (utf8::rune (p));
  utf8::next (p, uc.end());
  s += utf8::tolower (string (p, uc.end ()));

  CHECK_EQUAL (lc, s);
}

TEST (func_load_string)
{
  auto uc = utf8::LoadString (IDS_UC);
  auto lc = utf8::LoadString (IDS_LC);

  CHECK_EQUAL (lc, utf8::tolower (uc));
}

TEST (func_get_module_filename)
{
  string exe_name;
  CHECK (utf8::GetModuleFileName (nullptr, exe_name));
  CHECK_EQUAL (exe_name, utf8::GetModuleFileName ());
  exe_name.erase (exe_name.begin(), std::find_if (exe_name.rbegin (), exe_name.rend (),
    [](char ch) {return ch == '\\'; }).base());
  CHECK_EQUAL ("tests.exe", exe_name);
}

/*
  Trim white spaces at beginning and end of a string.
  Similar to Excel function TRIM.
*/
std::string trim (const std::string& str)
{
  auto pb = cbegin (str);

  //trim leading blanks
  while (pb != cend (str) && utf8::isblank (pb))
  {
    auto c = utf8::next (pb, cend (str));
    if (c == utf8::REPLACEMENT_CHARACTER)
      throw utf8::exception (utf8::exception::cause::invalid_utf8);
  }

  //now trim trailing blanks
  auto pe = cend (str);
  auto pe1 = pe;
  while (pe != pb)
  {
    auto c = utf8::prev (pe1, pb);
    if (c == utf8::REPLACEMENT_CHARACTER)
      throw utf8::exception (utf8::exception::cause::invalid_utf8);
    if (!utf8::isblank (c))
      break;
    else
      pe = pe1;
  }
  size_t pos = pb - begin (str);
  size_t cnt = pe - pb;
  return str.substr (pos, cnt);
}

TEST (trim)
{
  auto t = trim (u8" MIRCEA NEACȘU ĂÂȚÎ   ");
  CHECK_EQUAL (t, u8"MIRCEA NEACȘU ĂÂȚÎ");

  t = trim (u8"MIRCEA NEACȘU ĂÂȚÎ   ");
  CHECK_EQUAL (t, u8"MIRCEA NEACȘU ĂÂȚÎ");

  t = trim (u8"  MIRCEA NEACȘU ĂÂȚÎ");
  CHECK_EQUAL (t, u8"MIRCEA NEACȘU ĂÂȚÎ");

  t = trim ("");
  CHECK_EQUAL (t, "");

  t = trim (" ");
  CHECK_EQUAL (t, "");

  //these are all spaces
  t = trim(u8"\u0009\u0020\u00A0\u1680\u2000\u2001\u2002\u2003\u2004\u2005"
           u8"\u2006\u2007\u2008\u2009\u200A\u202f\u205f\u3000");
  CHECK_EQUAL (t, "");

  t = trim (u8"  MIRCEA NEACȘU ĂÂȚÎ"
            u8"\u2006\u2007\u2008\u2009\u200A\u202f\u205f\u3000");
  CHECK_EQUAL (t, u8"MIRCEA NEACȘU ĂÂȚÎ");
}

