#include <utpp/utpp.h>
#include <windows.h>
#include <utf8/utf8.h>
#include <iostream>

#pragma warning (disable : 4566)

using namespace std;
using namespace utf8;

int main (int argc, char **argv)
{
  if (argc > 1)
    return UnitTest::RunSuite (argv[1]);

  std::ofstream os ("utf8_tests.xml");
  UnitTest::ReporterXml xml (os);
  return UnitTest::RunAllTests (xml);
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
  CHECK_EQUAL (U'😀', rune_smiley);
}


//check that next function advances with one UTF8 character (rune)
TEST (next)
{
  string emojis{ u8"😃😎😛" };
  int i = 0;
  auto ptr = emojis.begin ();
  while (ptr != emojis.end ())
  {
    i++;
    CHECK (next (emojis, ptr));
  }

  CHECK_EQUAL (3, i);
}

// same test but using a character pointer instead of a string iterator
TEST (next_ptr)
{
  string emojis{ u8"😃😎😛" };
  int i = 0;
  const char *ptr = emojis.c_str ();
  while (*ptr)
  {
    i++;
    CHECK (utf8::next (ptr));
  }

  CHECK_EQUAL (3, i);
}

// test for runes function (conversion from UTF8 to UTF32)
TEST (runes)
{
  string emojis{ u8"😃😎😛" };
  u32string emojis32 = runes (emojis);
  CHECK_EQUAL (3, emojis32.size ());
  CHECK_EQUAL (0x1f603, emojis32[0]);
}


TEST (dir)
{
  /* Make a folder using Greek alphabet, change current directory into it,
  obtain the current working directory and verify that it matches the name
  of the newly created folder */

  string dirname = u8"ελληνικό";
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

TEST (out_stream)
{
  /* Write some text in a file with a UTF8 encoded filename. Verifies using
  standard Windows file reading that content was written. */

  string filename = u8"ελληνικό";
  string filetext{ u8"😃😎😛" };

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
  ReadFile (f, read_back, (DWORD)len, &nr, NULL);
  CloseHandle (f);
  CHECK (remove (filename));
  CHECK_EQUAL (len, nr);
  CHECK_EQUAL (filetext, read_back);
}

TEST (in_stream)
{
  /* write some stuff in file using utf8::ofstream object and read it
  back using utf8::ifstream. Verify read back matches original.*/

  string filetext{ u8"ελληνικό" };
  string filename{ u8"😃😎😛" };

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

  string filename = u8"ελληνικό";
  string filetext{ u8"😃😎😛" };

  FILE *u8file = utf8::fopen (filename, "w");
  CHECK (u8file);

  fwrite (filetext.c_str(), sizeof(char), filetext.length(), u8file);
  fclose (u8file);

  HANDLE f = CreateFile (utf8::widen (filename).c_str (), GENERIC_READ, 0,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  CHECK (f);

  char read_back[80];
  memset (read_back, 0, sizeof (read_back));

  size_t len = filetext.size ();
  DWORD nr;
  ReadFile (f, read_back, (DWORD)len, &nr, NULL);
  CloseHandle (f);
  CHECK (remove (filename));
  CHECK_EQUAL (len, nr);
  CHECK_EQUAL (filetext, read_back);
}

TEST (make_splitpath)
{
  const string dir{ u8"ελληνικό αλφάβητο\\" },
    fname{ u8"😃😎😛" };
  string path;
  utf8::makepath (path, "C", dir, fname, ".txt");
  wstring wpath = widen (path);
  CHECK_EQUAL (u8"C:ελληνικό αλφάβητο\\😃😎😛.txt", path);
  string drv1, dir1, fname1, ext1;
  splitpath (path, drv1, dir1, fname1, ext1);

  CHECK_EQUAL ("C:", drv1);
  CHECK_EQUAL (dir, dir1);
  CHECK_EQUAL (fname, fname1);
  CHECK_EQUAL (".txt", ext1);
}

TEST (get_putenv)
{
  //a long variable
  string path = utf8::getenv ("PATH");
  CHECK (!path.empty ());

  utf8::putenv (u8"ελληνικό=😃😎😛");
  CHECK_EQUAL (u8"😃😎😛", utf8::getenv (u8"ελληνικό"));

  utf8::putenv (u8"ελληνικό", string ());
  CHECK (utf8::getenv (u8"ελληνικό").empty ());
}

TEST (msgbox)
{
#if 0
  //requires user's intervention
  utf8::MessageBox (NULL, u8"ελληνικό", u8"😃😎😛", MB_ICONINFORMATION);
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

  GetTempPath ((DWORD)wpath.size (), const_cast<wchar_t*>(wpath.data ()));

  UINT ret = GetTempFileName (wpath.c_str(), L"ÄñΩ",
    1, const_cast<wchar_t*>(wfname.data ()));
  CHECK (ret > 0);


  //Let's do the same thing with utf8::buffer
  utf8::buffer path (_MAX_PATH);
  utf8::buffer fname (_MAX_PATH);

  GetTempPath ((DWORD)path.size (), path);

  ret = GetTempFileName (path, L"ÄñΩ", 1, fname);
  CHECK (ret > 0);

  //see that we get the same result
  string result = fname;
  CHECK_EQUAL (narrow (wfname), result);
}

//check in-place versions of case folding functions
TEST (case_conversion_inplace)
{
  string lc{ u8"mircea neacșu ăâățî" };
  string uc{ u8"MIRCEA NEACȘU ĂÂĂȚÎ" };
  string t = lc;
  utf8::toupper (t);
  CHECK_EQUAL (uc, t);
  t = uc;
  utf8::tolower (t);
  CHECK_EQUAL (lc, t);
}

//check string-returning versions of case folding functions
TEST (case_conversion_ret)
{
  string uc = utf8::toupper (u8"αλφάβητο");
  CHECK_EQUAL (u8"ΑΛΦΆΒΗΤΟ", uc);
  CHECK_EQUAL (u8"αλφάβητο", utf8::tolower (u8"ΑΛΦΆΒΗΤΟ"));
}

//check case-insensitive comparison
TEST (icompare_equal)
{
  string lc{ u8"mircea neacșu ăâățî" };
  string uc{ u8"MIRCEA NEACȘU ĂÂĂȚÎ" };
  CHECK (utf8::icompare (lc, uc) == 0);
}

TEST (icompare_less)
{
  string lc{ u8"mircea neacșu ăâățî" };
  string uc{ u8"MIRCEA NEACȘU ĂÂĂȚÎ " };
  CHECK (utf8::icompare (lc, uc) < 0);
}

TEST (icompare_greater)
{
  string lc{ u8"mircea neacșu ăâățî" };
  string uc{ u8"MIRCEA NEACȘU ĂÂ2ȚÎ" };
  CHECK (utf8::icompare (lc, uc) > 0);
}



// find files named "test*" using find_first/find_next functions
TEST (find)
{
  find_data fd;
  bool ret = find_first ("test*", fd);
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
  file_enumerator f("test*");
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