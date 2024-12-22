/*
  Copyright (c) Mircea Neacsu (2014-2024) Licensed under MIT License.
  This is part of UTF8 project. See LICENSE file for full license terms.
*/
#include <utpp/utpp.h>
#include <utf8/utf8.h>

#if UTF8_USE_WINDOWS_API
#include <windows.h>
#include "resource.h"

using namespace std;

// Windows specific tests
SUITE (MS_Windows)
{
  TEST (symlink)
  {
    /* Make a folder using Greek alphabet, and another one using Armenian.
    enter the 2nd directory and create a link to the first. It verifies a file
    created in the first directory is visible through the symlink.*/

    //make first directory
    CHECK (utf8::mkdir ("ελληνικό"));
    utf8::chdir ("ελληνικό");
    //and a file in it
    utf8::ofstream out ("f.txt");
    out << "text" << endl;
    out.close ();
    utf8::chdir ("..");
    //make 2nd directory
    CHECK (utf8::mkdir ("Հայերեն"));
    utf8::chdir ("Հայերեն");
    //and symlink to first
    CHECK (utf8::symlink (u8"..\\ελληνικό", "पंजाबी", true));
    //change into symlink
    utf8::chdir ("पंजाबी");
    utf8::ifstream in ("f.txt");
    string str;
    in >> str;
    CHECK_EQUAL ("text", str);
    in.close ();

    //cleanup
    utf8::remove ("f.txt");
    utf8::chdir ("..");
    utf8::rmdir ("पंजाबी");
    utf8::chdir ("..");
    utf8::rmdir ("Հայերեն");
    utf8::rmdir ("ελληνικό");
  }

  TEST (fopen_write)
  {
    /* Write some text in a file with a UTF8 encoded filename. Verifies using
    standard Windows file reading that content was written. */

    string filename = "ελληνικό";
    string filetext{ "😃😎😛" };
    FILE* u8file = utf8::fopen (filename, "w");
    ABORT_EX (!u8file, "Failed to create output file");

    fwrite (filetext.c_str (), sizeof (char), filetext.length (), u8file);
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
    CHECK (utf8::remove (filename));
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
    wstring wpath = utf8::widen (path);
    CHECK_EQUAL ("C:ελληνικό αλφάβητο\\😃😎😛.txt", path);
    string drv1, dir1, fname1, ext1;
    CHECK (utf8::splitpath (path, drv1, dir1, fname1, ext1));

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

#if 0
  TEST (msgbox)
  {
    //requires user's intervention
    utf8::MessageBox (NULL, "ελληνικό", "😃😎😛", MB_ICONINFORMATION);
  }
#endif

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
    utf8::buffer buf1 (buf);
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

    UINT ret = ::GetTempFileNameW (wpath.c_str (), L"ÄñΩ",
      1, const_cast<wchar_t*>(wfname.data ()));
    CHECK (ret > 0);


    //Do the same thing with utf8::GetTempPath and utf8::GetTempFileName
    auto fname = utf8::GetTempFileName (utf8::GetTempPath (), "ÄñΩ", 1);

    //see that we get the same result
    CHECK_EQUAL (utf8::narrow (wfname.c_str ()), fname);
  }

  static std::string mydir ()
  {
    auto fname = utf8::GetModuleFileName ();
    string drive, dir, name, ext;
    utf8::splitpath (fname, drive, dir, name, ext);
    return drive + dir;
  }

  // find files named "test*" using find_first/find_next functions
  TEST (find)
  {
    utf8::find_data fd;
    bool ret = find_first (mydir () + "test*", fd);
    cout << "find_first: " << fd.filename << " - " << fd.size / 1024 << "kb" << endl;
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
    utf8::file_enumerator f (mydir () + "test*");
    CHECK (f.ok ());
    while (f.ok ())
    {
      cout << "finder: " << f.filename << " - " << f.size / 1024 << "kb" << endl;
      f.next ();
    }
  }

  TEST (find_missing_file)
  {
    utf8::file_enumerator f ("no such file");
    CHECK (!f.ok ());
  }

  TEST (bool_op_missing_file)
  {
    utf8::file_enumerator found ("no such file");
    CHECK (!found);
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
    exe_name.erase (exe_name.begin (), std::find_if (exe_name.rbegin (), exe_name.rend (),
      [](char ch) {return ch == '\\'; }).base ());
    CHECK_EQUAL ("tests.exe", exe_name);
  }

  TEST (out_stream)
  {
    /* Write some text in a file with a UTF8 encoded filename. Verifies using
    standard Windows file reading that content was written. */

    string filename = "ελληνικό";
    string filetext{ "😃😎😛" };

    utf8::ofstream u8strm (filename);

    u8strm << filetext << endl;
    u8strm.close ();

    HANDLE f = CreateFile (utf8::widen (filename).c_str (), GENERIC_READ, 0,
      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    CHECK (f);

    char read_back[80];
    memset (read_back, 0, sizeof (read_back));

    size_t len = filetext.size ();
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
    CHECK (utf8::remove (filename));
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
    CHECK (utf8::remove (filename));
  }
}

SUITE (Registry)
{
  
const string key_name{ u8"αρχείο" };//Greek for "registry" according to Google

TEST (create_open)
{
  HKEY key;
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegCreateKey (HKEY_CURRENT_USER, key_name, key));
  CHECK(!RegCloseKey (key));

  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegOpenKey (HKEY_CURRENT_USER, key_name, key));
  CHECK(!RegCloseKey (key));
  CHECK(!utf8::RegDeleteKey (HKEY_CURRENT_USER, key_name));
}

TEST (rename_delete_tree)
{
  HKEY key;
  utf8::RegCreateKey (HKEY_CURRENT_USER, key_name, key);
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegRenameKey (HKEY_CURRENT_USER, key_name, u8"पंजाबी"));
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegDeleteTree (HKEY_CURRENT_USER, u8"पंजाबी"));
}

TEST (set_get_query_value)
{
  HKEY key;
  DWORD sz = sizeof(DWORD);
  DWORD data = 123, data1 = 0, data2 = 0;
  utf8::RegCreateKey (HKEY_CURRENT_USER, key_name, key);

  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegSetValue (key, "dword_value", REG_DWORD, &data, sizeof (DWORD)));
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegGetValue (key, "", "dword_value", RRF_RT_ANY, &data1, &sz));
  CHECK_EQUAL (data, data1);

  string sval = "0123456789", sval1;
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegSetValue (key, "string_value", sval));
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegGetValue (key, "", "string_value", sval1));
  CHECK_EQUAL (sval, sval1);

  DWORD type;
  sz = sizeof (DWORD);
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegQueryValue (key, "dword_value", &type, &data2, &sz));
  CHECK_EQUAL (REG_DWORD, type);

  CHECK(!RegCloseKey (key));
  CHECK(!utf8::RegDeleteKey (HKEY_CURRENT_USER, key_name));
}

TEST (set_get_vector)
{
  HKEY key;
  utf8::RegCreateKey (HKEY_CURRENT_USER, key_name, key);

  vector<string> in_vect = { "string 1", "string 2", "string 3" };
  vector<string> out_vect;

  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegSetValue (key, "string_vector", in_vect));
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegGetValue (key, "", "string_vector", out_vect));
  CHECK_EQUAL (in_vect, out_vect);

  CHECK(!utf8::RegDeleteKey (HKEY_CURRENT_USER, key_name));
}

TEST (enum_keys)
{
  HKEY key;
  utf8::RegCreateKey (HKEY_CURRENT_USER, key_name, key);
  
  vector<string>in_name{ u8"α1", u8"β2", u8"γ3", u8"😃😎😛"};
  vector<string>out_name(4);

  for (int i = 0; i < 4; i++)
  {
    HKEY subkey;
    CHECK(!utf8::RegCreateKey (key, in_name[i], subkey));
  }

  for (int i = 0; i < 4; i++)
    CHECK_EQUAL (ERROR_SUCCESS, utf8::RegEnumKey (key, i, out_name[i]));

  CHECK_EQUAL (in_name, out_name);

  //2nd function
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegEnumKey (key, out_name));
  CHECK_EQUAL (in_name, out_name);

  CHECK(!utf8::RegDeleteTree (HKEY_CURRENT_USER, key_name));
}

TEST (enum_values)
{
  HKEY key;
  utf8::RegCreateKey (HKEY_CURRENT_USER, key_name, key);

  vector<string>in_vals{ u8"α1", u8"β2", u8"γ3", u8"😃😎😛" };
  vector<string>out_vals (4);

  for (int i = 0; i < 4; i++)
    CHECK(!utf8::RegSetValue (key, in_vals[i], in_vals[3 - i]));

  for (int i = 0; i < 4; i++)
    CHECK_EQUAL (ERROR_SUCCESS, utf8::RegEnumValue (key, i, out_vals[i]));

  CHECK_EQUAL (in_vals, out_vals);

  //2nd function
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegEnumValue (key, out_vals));
  CHECK_EQUAL (in_vals, out_vals);

  CHECK(!utf8::RegDeleteKey (HKEY_CURRENT_USER, key_name));
}

} //end suite
#endif