/*
  Copyright (c) Mircea Neacsu (2014-2024) Licensed under MIT License.
  This is part of UTF8 project. See LICENSE file for full license terms.
*/

///  SAMPLE.CPP Example program for UTF8 functions

#include <utf8/utf8.h>
#include <utf8/ini.h>
#include <iostream>
#include <tuple>
#include <climits>

using namespace std;

// All messages will be written in this text file if ACP is not UTF-8
const std::string FNAME{ "sample_output.txt" };


void confirm ()
{
  cout << "Press ENTER to continue...";
  cin.ignore ((streamsize)UINT_MAX , '\n');
  cout << '\n';
}


int main (int /*unused*/, char ** /*unused*/)
{
  int argc;
#ifdef _WIN32
  ofstream fout;
  if (GetACP () != 65001)
  {
    cout << "Windows ACP is not UTF-8. Output will be sent to " << FNAME 
      << endl << endl;
    fout.open (FNAME);
  }
  ostream& out{ (GetACP () == 65001) ? cout : fout };

  char **argv = utf8::get_argv (&argc);

  out << "Program name is "  << argv[0] << endl;
  if (argc > 1)
  {
    out << "There are " << argc-1 << "command line arguments:\n";
    for (int i = 1; i < argc; i++)
      out << "Arg " << i << " - " << argv[i] << endl;
  }
  out << endl;
#else
  ostream& out = cout;
#endif
  //Create an INI file where we can store settings
  utf8::IniFile ini("Ελληνικός.ini");

  //create a (sub)folder
  cout << "Creating Arabic folder. ";
  utf8::mkdir ("اللغة العربي");
  out << "Created Arabic folder اللغة العربي" << endl;
  confirm ();
  ini.PutString ("Folder", "اللغة العربي", "Settings");

  //Change working directory to this new folder and create a file using fopen
  utf8::chdir ("اللغة العربي");
  out << "Working folder is " << utf8::getcwd () << endl;
  cout << "Creating Aramaic file. ";
  FILE *f = utf8::fopen ("ܐܪܡܝܐ.txt", "w");

  /* Basic I/O functions are 'agnostic' to the actual encoding that's why there
  are no special versions for fread and fwrite functions. String I/O functions
  however need to know where the string ends and you have narrow version like
  fputs and wide version fputws. Here we work with UTF-8 strings so we use the
  narrow versions of them.*/
  fputs ("This text is in Aramaic ܐܪܡܝܐ", f);
  fclose (f);
  ini.PutString ("Aramaic", "ܐܪܡܝܐ", "Settings");

  out << "Created Aramaic file ܐܪܡܝܐ.txt\n";
  confirm ();

  //How about a file with stream I/O
  cout << "Using streams to create an Armenian file. ";
  utf8::ofstream os ("Հայերեն.txt");
  os << "Text in Armenian Հայերեն" << std::endl;
  os.close ();
  out << "Created Armenian file Հայերեն.txt" << endl;
  confirm ();
  ini.PutString ("Հայերեն", "Armenian", "Settings");

  //Let's read some data using streams
  cout << "Reading data from Aramaic file. ";
  utf8::ifstream is ("ܐܪܡܝܐ.txt");
  string s;
  getline (is, s);
  out << "The first line in file Aramaic ܐܪܡܝܐ.txt is: " << s << endl;
  is.close ();
  confirm ();

  //Renaming a file
  utf8::rename ("Հայերեն.txt", u8"Japanese 日本語.txt");
  out << "New file name is Japanese 日本語.txt" << endl;

#ifdef _WIN32
  //Set an environment variable and retrieve its value
  utf8::putenv ("Punjabi=पंजाबी");
  out << "The environment variable Punjabi is " 
      << utf8::getenv ("Punjabi") << endl;
#endif

  //convert a string to uppercase
  string all_caps = utf8::toupper ("Neacșu"); // all_caps should be "NEACȘU"
  out << "The author's name is " << all_caps << endl;

  //convert a string to lowercase
  string greek{ "ΑΛΦΆΒΗΤΟ"};
  utf8::tolower (greek); //string should be  u8"αλφάβητο"
  out << "This is how Greeks say 'alphabet': " << greek << endl;

  // Cleanup
  cout << "Deleting Aramaic file.\n";
  utf8::remove (u8"ܐܪܡܝܐ.txt");
  cout << "Deleting Japanese file.\n";
  utf8::remove (u8"Japanese 日本語.txt");
  utf8::chdir ("..");
  cout << "Deleting Arabic folder. ";
  utf8::rmdir (u8"اللغة العربي");
  confirm ();

  //retrieve settings from INI file
  out << "INI setting: Folder= " << ini.GetString ("Folder", "Settings") << endl;
  out << "INI setting: Aramaic(ܐܪܡܝܐ)= " << ini.GetString ("Aramaic", "Settings") << endl;
  out << "INI setting: Armenian(Հայերեն)= " << ini.GetString ("Հայերեն", "Settings") << endl;

  //delete INI file
  utf8::remove (u8"Ελληνικός.ini");

  //Done
  cout << "\nThat's all folks!\n";

#ifdef _WIN32
  //Don't forget to free argv array
  utf8::free_argv (argc, argv);
#endif
  return 0;
}