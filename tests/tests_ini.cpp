#define _CRT_SECURE_NO_WARNINGS

#include <utf8/utf8.h>
#include <utpp/utpp.h>

using namespace std;
SUITE (IniTests)
{
  /*
    Write some keys and verify their retrieval.
  */
  TEST (PutGetString)
  {
    char val[80];
    utf8::remove ("test.ini");
    Sleep (200);
    utf8::IniFile test ("test.ini");

    // new file
    CHECK (test.PutString ("key0", "value00", "section0  "));

    // new key in existing section
    CHECK (test.PutString ("key1", "value01", "section0"));

    // same key with same value
    CHECK (test.PutString ("key0", "value00", "section0"));

    // same key different value
    CHECK (test.PutString ("key0", "value1", "section0"));

    // new section
    CHECK (test.PutString ("key1", "value11", "section1"));

    CHECK_EQUAL (strlen ("value11"), test.GetString (val, sizeof (val), "key1", "section1", "default"));
    CHECK_EQUAL ("value11", val);
    utf8::remove ("test.ini");
  }


  TEST (Compatibility)
  {
    char val[80];
    FILE *wfp = fopen ("test.ini", "w");

    fputs (" \t[section]\n", wfp);  //section entries can be preceded by whitespace
    fputs ("key01=value01\n", wfp);
    fputs (";key02=value02\n", wfp);    //semicolon is comment
    fputs ("#key03=value03\n", wfp);    //hash is NOT comment
    fputs (" \tkey04=value04\n", wfp);  //keys can be preceded by spaces
    fputs ("key05=  value05\n", wfp);   //values are stripped of leading spaces
    fputs ("key06 =value06\n", wfp);    //trailing key spaces are removed
    fputs ("key07=value07  \n", wfp);   //trailing value spaces are removed
    fputs ("key08=value08  ;\n", wfp);  //no trailing comment removal
    fputs (" \t[  spaced section  ]\n", wfp);  //section names can be preceded by whitespace
    fputs ("key01=value11\n", wfp);

    fclose (wfp);
    utf8::IniFile ini ("test.ini");

    GetPrivateProfileStringA ("section", "key01", "inexistent", val, sizeof (val), ".\\test.ini");
    CHECK_EQUAL ("value01", val);
    CHECK_EQUAL (val, ini.GetString ("key01", "section", "inexistent"));

    GetPrivateProfileStringA ("section", "key02", "inexistent", val, sizeof (val), ".\\test.ini");
    CHECK_EQUAL ("inexistent", val);
    CHECK_EQUAL (val, ini.GetString ("key02", "section", "inexistent"));

    GetPrivateProfileStringA ("section", ";key02", "inexistent", val, sizeof (val), ".\\test.ini");
    CHECK_EQUAL ("inexistent", val);
    CHECK_EQUAL (val, ini.GetString (";key02", "section", "inexistent"));

    GetPrivateProfileStringA ("section", "#key03", "inexistent", val, sizeof (val), ".\\test.ini");
    CHECK_EQUAL ("value03", val);
    CHECK_EQUAL (val, ini.GetString ("#key03", "section", "inexistent"));

    GetPrivateProfileStringA ("section", "key04", "inexistent", val, sizeof (val), ".\\test.ini");
    CHECK_EQUAL ("value04", val);
    CHECK_EQUAL (val, ini.GetString ("key04", "section", "inexistent"));

    GetPrivateProfileStringA ("section", "key05", "inexistent", val, sizeof (val), ".\\test.ini");
    CHECK_EQUAL ("value05", val);
    CHECK_EQUAL (val, ini.GetString ("key05", "section", "inexistent"));

    GetPrivateProfileStringA ("section", "key06 ", "inexistent", val, sizeof (val), ".\\test.ini");
    CHECK_EQUAL ("value06", val);
    CHECK_EQUAL (val, ini.GetString ("key06 ", "section", "inexistent"));

    GetPrivateProfileStringA ("section", "key07", "inexistent", val, sizeof (val), ".\\test.ini");
    CHECK_EQUAL ("value07", val);
    CHECK_EQUAL (val, ini.GetString ("key07", "section", "inexistent"));

    GetPrivateProfileStringA ("section", "key08", "inexistent", val, sizeof (val), ".\\test.ini");
    CHECK_EQUAL ("value08  ;", val);
    CHECK_EQUAL (val, ini.GetString ("key08", "section", "inexistent"));

    //Leading and trailing spaces are removed from parameters
    const char *key7 = " key07   ";
    const char *sect = "  section  ";
    GetPrivateProfileStringA (sect, key7, "inexistent", val, sizeof (val), ".\\test.ini");
    CHECK_EQUAL ("value07", val);
    CHECK_EQUAL (val, ini.GetString (key7, sect, "inexistent"));

    GetPrivateProfileStringA ("spaced section", "key01", "inexistent", val, sizeof (val), ".\\test.ini");
    CHECK_EQUAL ("value11", val);
    CHECK_EQUAL (val, ini.GetString ("key01", "spaced section", "inexistent"));

    remove ("test.ini");
    //
  }

  // Check malformed section line
  TEST (Malformed_section)
  {
    FILE *wfp = fopen ("test.ini", "w");

    fputs ("[section0]\n", wfp);
    fputs ("key01=value01\n", wfp);
    fputs ("key02=value02\n", wfp);
    fputs ("[sectionxx\n", wfp);
    fputs ("key11=value11\n", wfp);
    fputs ("[section2]", wfp);
    fclose (wfp);

    utf8::IniFile ini{ "test.ini" };
    deque<string> sects;
    CHECK_EQUAL (2, ini.GetSections (sects));
    CHECK_EQUAL ("section0", sects[0]);
    CHECK_EQUAL ("section2", sects[1]);
    CHECK_EQUAL ("inexistent", ini.GetString ("key11", "sectionxx", "inexistent"));
    remove ("test.ini");
  }

  // Check malformed key entry
  TEST (Malformed_key)
  {
    FILE *wfp = fopen ("test.ini", "w");

    fputs ("[section0]\n", wfp);
    fputs ("key01=value01\n", wfp);
    fputs ("key02=value02\n", wfp);
    fputs ("keyxx\n", wfp);
    fputs ("key11=value11\n", wfp);
    fclose (wfp);

    utf8::IniFile ini{ "test.ini" };
    deque<string> keys;
    CHECK_EQUAL (3, ini.GetKeys (keys, "section0"));
    CHECK_EQUAL ("inexistent", ini.GetString ("keyxx", "section0", "inexistent"));
    remove ("test.ini");
  }

  TEST (Put_Spaced_Params)
  {
    utf8::IniFile ini{ "test.ini" };
    char val[80];
    ini.PutString (" key00  ", "  value00  ", "  section0  ");
    GetPrivateProfileStringA ("section0", "key00", "bad", val, 
      sizeof(val), ".\\test.ini");
    CHECK_EQUAL ("value00", val);
    remove ("test.ini");
  }

  // Check GetString that returns a C++ string
  TEST (GetStringString)
  {
    utf8::remove ("test.ini");
    Sleep (200);
    utf8::IniFile test ("test.ini");
    CHECK (test.PutString ("key0", "value00", "section0"));
    CHECK (test.PutString ("key1", "value01", "section0"));
    CHECK (test.PutString ("key0", "value10", "section1"));
    CHECK (test.PutString ("key1", "value11", "section1"));
    CHECK_EQUAL ("value11", test.GetString ("key1", "section1", "default"));
    CHECK_EQUAL ("default", test.GetString ("missing_key", "section1", "default"));
    utf8::remove ("test.ini");
  }

  //Check PutString, PutDouble, PutInt, PutBool fail when ini file cannot be created
  TEST (IniNotCreated)
  {
    utf8::IniFile test ("inexistent folder\\test.ini");
    CHECK (!test.PutString ("key0", "value00", "section0"));
    CHECK (!test.PutDouble ("key1", 123.45, "section0"));
    CHECK (!test.PutInt ("key1", 123, "section0"));
    CHECK (!test.PutBool ("key1", true, "section0"));
  }

  //Modify a key value and verify it has changed.
  TEST (PutReplaceGet)
  {
    char val[80];
    utf8::remove ("test.ini");
    Sleep (200);
    utf8::IniFile test ("test.ini");
    CHECK (test.PutString ("key0", "value00", "section0"));
    CHECK (test.PutString ("key0", "newval", "section0"));
    size_t sz = test.GetString (val, sizeof (val), "key0", "section0");
    CHECK_EQUAL ("newval", val);
    CHECK_EQUAL (strlen ("newval"), sz);
    utf8::remove ("test.ini");
  }

  /*
    Test key removal. Write 3 keys and delete the 2nd one. Verify that first and
    3rd are unchanged. Verify that GET for deleted key returns default value.
  */
  TEST (PutDelete)
  {
    char val[80];
    utf8::remove ("test.ini");
    Sleep (200);
    utf8::IniFile test ("test.ini");
    test.PutString ("key0", "value00", "section0");
    test.PutString ("key1", "value01", "section0");
    test.PutString ("key2", "value02", "section0");
    test.DeleteKey ("key1", "section0");
    CHECK_EQUAL (1234, test.GetInt ("key1", "section0", 1234));
    test.GetString (val, sizeof (val), "key2", "section0");
    CHECK_EQUAL ("value02", val);
    test.GetString (val, sizeof (val), "key0", "section0");
    CHECK_EQUAL ("value00", val);
    utf8::remove ("test.ini");
  }

  // Write 2 sections in and verify that GetSections retrieves them.
  TEST (GetSections)
  {
    char sections[80];
    utf8::remove ("test.ini");
    Sleep (200);
    utf8::IniFile test ("test.ini");
    test.PutString ("key0", "value00", "section0");
    test.PutString ("key1", "value01", "section0");
    test.PutString ("key0", "value10", "section1");
    test.PutString ("key1", "value11", "section1");

    test.GetSections (sections, sizeof (sections));
    CHECK_EQUAL ("section0", sections);
    CHECK_EQUAL ("section1", sections + strlen ("section0") + 1);
    utf8::remove ("test.ini");
  }

  TEST (GetSections_SmallBuffer)
  {
    char sections[20];
    utf8::remove ("test.ini");
    Sleep (200);
    utf8::IniFile test ("test.ini");
    test.PutString ("key0", "value00", "Asection12345678901234567890");
    test.PutString ("key1", "value01", "Bsection12345678901234567890");

    size_t ret = test.GetSections (sections, sizeof (sections));
    CHECK_EQUAL ("Asection1234567890", sections);
    CHECK_EQUAL (2, ret); //returns all sections not only the ones copied
    utf8::remove ("test.ini");
  }

  TEST (GetKeys)
  {
    char buffer[256];
    utf8::remove ("test.ini");
    Sleep (200);
    utf8::IniFile test ("test.ini");
    test.PutString ("key0", "value00", "section0");
    test.PutString ("key1", "value01", "section0");
    test.PutString ("key2", "value02", "section0");
    int i = test.GetKeys (buffer, sizeof (buffer), "section0");
    CHECK_EQUAL (3, i);
    char *p = buffer;
    CHECK_EQUAL ("key0", p);
    p += strlen (p) + 1;
    CHECK_EQUAL ("key1", p);
    p += strlen (p) + 1;
    CHECK_EQUAL ("key2", p);
    p += strlen (p) + 1;
    CHECK_EQUAL ("", p);
    utf8::remove ("test.ini");
  }

  TEST (GetKeys_deque)
  {
    utf8::remove ("test.ini");
    Sleep (200);
    utf8::IniFile test ("test.ini");
    test.PutString ("key0", "value00", "section0");
    test.PutString ("key1", "value01", "section0");
    test.PutString ("key2", "value02", "section0");
    deque<string> keys;
    size_t i = test.GetKeys (keys, "section0");
    CHECK_EQUAL (3, i);
    CHECK_EQUAL (3, keys.size ());
    CHECK_EQUAL ("key0", keys[0]);
    CHECK_EQUAL ("key1", keys[1]);
    CHECK_EQUAL ("key2", keys[2]);
    utf8::remove ("test.ini");
  }

  TEST (GetKeys_SmallBuffer)
  {
    char buffer[20];
    utf8::remove ("test.ini");
    Sleep (200);
    utf8::IniFile test ("test.ini");
    test.PutString ("key0", "value00", "section0");
    test.PutString ("key1", "value01", "section0");
    test.PutString ("key2_01234567890123456789", "value02", "section0");
    int i = test.GetKeys (buffer, sizeof (buffer), "section0");
    CHECK_EQUAL (3, i);
    char *p = buffer;
    CHECK_EQUAL ("key0", p);
    p += strlen (p) + 1;
    CHECK_EQUAL ("key1", p);
    utf8::remove ("test.ini");
  }

  TEST (HasSection)
  {
    utf8::remove ("test.ini");
    Sleep (200);
    utf8::IniFile test ("test.ini");
    test.PutString ("key0", "value00", "section0");
    test.PutString ("key1", "value01", "section0");
    bool t = test.HasSection ("section0");
    CHECK_EQUAL (true, t);
    t = test.HasSection ("no_section");
    CHECK_EQUAL (false, t);
    utf8::remove ("test.ini");
  }

  // Write a non-ANSI value in an INI file with non-ANSI name.
  TEST (Greek_filename)
  {
    const char *filename = u8"αλφάβητο.ini";
    const char *greek_text = u8"αλφάβητο";
    char strval[80];

    utf8::remove (filename);
    Sleep (200);
    utf8::IniFile greek (filename);
    greek.PutInt ("Integer", 1, "Keys");
    CHECK_EQUAL (1, greek.GetInt ("Integer", "Keys", 2));
    greek.PutString ("GreekAlphabet", greek_text, "Keys");
    greek.GetString (strval, sizeof (strval), "GreekAlphabet", "Keys");
    CHECK_EQUAL (strval, greek_text);
    greek.PutString (greek_text, "This is how you spell alphabet in Greek", "Keys");
    utf8::remove (filename);
  }

  TEST (Quoted_strings)
  {
    const char *quoted = "\"Quoted String with \" in the middle\"";
    char buffer[256];
    utf8::remove ("test.ini");
    Sleep (200);
    utf8::IniFile test ("test.ini");
    test.PutString ("key0", quoted, "section");
    test.GetString (buffer, sizeof (buffer), "key0", "section");
    CHECK_EQUAL (quoted, buffer);
    utf8::remove ("test.ini");
  }

  TEST (CopySection_new_file)
  {
    utf8::IniFile f1 ("test1.ini");
    f1.PutString ("key0", "value00", "section0");
    f1.PutString ("key1", "value01", "section0");

    utf8::IniFile f2 ("test2.ini");
    f2.CopySection (f1, "section0", "section1");
    
    CHECK_EQUAL ("value00", f2.GetString ("key0", "section1"));
    CHECK_EQUAL ("value01", f2.GetString ("key1", "section1"));

    //same file but different name
    utf8::IniFile f3{ ".\\test1.ini" };
    CHECK (f3.CopySection (f1, "section0"));
    CHECK_EQUAL ("value00", f3.GetString ("key0", "section0"));

    //change destination section name
    f3.CopySection (f1, "section0", "section0-copy");
    CHECK_EQUAL ("value00", f3.GetString ("key0", "section0-copy"));

    //copy inexistent section
    CHECK (f3.CopySection (f1, "No such section"));

    utf8::remove ("test1.ini");
    utf8::remove ("test2.ini");
  }

  TEST (CopySection_existing_file)
  {
    utf8::IniFile f1 ("test1.ini");
    f1.PutString ("key0", "f1_val00", "section0");
    f1.PutString ("key1", "f1_val01", "section0");
    f1.PutString ("key0", "f1_val10", "section1");
    f1.PutString ("key1", "f1_val11", "section1");

    utf8::IniFile f2 ("test2.ini");
    f2.PutString ("key0", "f2_val00", "section0");
    f2.PutString ("key1", "f2_val01", "section0");
    f2.PutString ("key0", "f2_val10", "section1");
    f2.PutString ("key1", "f2_val11", "section1");
    f2.PutString ("key0", "f2_val20", "section2");
    f2.PutString ("key1", "f2_val21", "section2");

    f2.CopySection (f1, "section1");
    deque<string> keys;
    
    //previous content of section1 was erased
    CHECK_EQUAL (2, f2.GetKeys (keys, "section1"));

    //key was copied from test1 file
    CHECK_EQUAL ("f1_val10", f2.GetString ("key0", "section1"));

    //other sections of test2 file are not changed
    CHECK_EQUAL ("f2_val21", f2.GetString ("key1", "section2"));

    utf8::remove ("test1.ini");
    utf8::remove ("test2.ini");
  }

  TEST (Delete_section)
  {
    utf8::IniFile f1 ("test1.ini");
    f1.PutString ("key0", "f1_val00", "section0");
    f1.PutString ("key1", "f1_val01", "section0");
    f1.PutString ("key0", "f1_val10", "section1");
    f1.PutString ("key1", "f1_val11", "section1");
    f1.PutString ("key0", "f1_val20", "section2");
    f1.PutString ("key1", "f1_val21", "section2");

    f1.DeleteSection ("section1"); //delete section in the middle

    //other sections of file are not changed
    CHECK_EQUAL ("f1_val01", f1.GetString ("key1", "section0"));
    CHECK_EQUAL ("f1_val21", f1.GetString ("key1", "section2"));
    utf8::remove ("test1.ini");
  }

  TEST (Delete_last_section)
  {
    utf8::IniFile f1 ("test1.ini");
    f1.PutString ("key0", "f1_val00", "section0");
    f1.PutString ("key1", "f1_val01", "section0");
    f1.PutString ("key0", "f1_val10", "section1");
    f1.PutString ("key1", "f1_val11", "section1");
    f1.PutString ("key0", "f1_val20", "section2");
    f1.PutString ("key1", "f1_val21", "section2");

    f1.DeleteSection ("section2"); //delete last section

    //other sections of file are not changed
    CHECK_EQUAL ("f1_val01", f1.GetString ("key1", "section0"));
    CHECK_EQUAL ("f1_val11", f1.GetString ("key1", "section1"));
    utf8::remove ("test1.ini");
  }
}
