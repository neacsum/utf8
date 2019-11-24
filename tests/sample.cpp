/*
  SAMPLE.CPP Example program for UTF8 functions

  (c) Mircea Neacsu 2019. Licensed under MIT License.
  See README file for full license terms.
*/
#include <utf8/utf8.h>
#include <iostream>
#include <conio.h>

using namespace std;

/*
  Windows default console fonts are very poor when it comes to exotic
  characters. All messages will be written in this text file.
*/
utf8::ofstream out("sample_log.txt");


void confirm ()
{
  printf ("Press any key to continue...");
  while (!_kbhit ())
    ;
  _getch ();
  printf ("\n");
}


int main (int /*unused*/, char ** /*unused*/)
{
  int argc;

  char **argv = utf8::get_argv (&argc);

  out << "Program name is "  << argv[0] << endl;
  if (argc > 1)
  {
    out << "There are " << argc-1 << "command line arguments:\n";
    for (int i = 1; i < argc; i++)
      out << "Arg " << i << " - " << argv[i] << endl;
  }
  out << endl;

  //create a (sub)folder
  printf ("Creating Arabic folder. ");
  utf8::mkdir (u8"اللغة العربي");
  out << u8"Created Arabic folder اللغة العربي" << endl;
  confirm ();

  //Change working directory to this new folder and create a file using fopen
  utf8::chdir (u8"اللغة العربي");
  out << "Working folder is " << utf8::getcwd () << endl;
  printf ("Creating Aramaic file. ");
  FILE *f = utf8::fopen (u8"ܐܪܡܝܐ.txt", "w");

  /* Basic I/O functions are 'agnostic' to the actual encoding that's why there
  are no special versions for fread and fwrite functions. String I/O functions
  however need to know where the string ends and you have narrow version like
  fputs and wide version fputws. Here we work with UTF-8 strings so we use the
  narrow versions of them.*/
  fputs (u8"This text is in Aramaic ܐܪܡܝܐ", f);
  fclose (f);

  out << u8"Created Aramaic file ܐܪܡܝܐ.txt\n";
  confirm ();

  //How about a file with stream I/O
  printf ("Using streams to create an Armenian file. ");
  utf8::ofstream os (u8"Հայերեն.txt");
  os << u8"Text in Armenian Հայերեն" << std::endl;
  os.close ();
  out << u8"Created Armenian file Հայերեն.txt" << endl;
  confirm ();

  //Let's read some data using streams
  printf ("Reading data from Aramaic file. ");
  utf8::ifstream is (u8"ܐܪܡܝܐ.txt");
  string s;
  getline (is, s);
  out << u8"The first line in file Aramaic ܐܪܡܝܐ.txt is: " << s << endl;
  is.close ();
  confirm ();

  //Renaming a file
  utf8::rename (u8"Հայերեն.txt", u8"Japanese 日本語.txt");
  out << u8"New file name is Japanese 日本語.txt" << endl;

  //Set an environment variable and retrieve its value
  utf8::putenv (u8"Punjabi=पंजाबी");
  out << "The environment variable Punjabi is " 
      << utf8::getenv ("Punjabi") << endl;

  // Cleanup
  printf ("Deleting Aramaic file.\n");
  utf8::remove (u8"ܐܪܡܝܐ.txt");
  printf ("Deleting Japanese file.\n");
  utf8::remove (u8"Japanese 日本語.txt");
  utf8::chdir ("..");
  printf ("Deleting Arabic folder. ");
  utf8::rmdir (u8"اللغة العربي");
  confirm ();

  //Done
  printf ("\nThat's all folks!\n");

  //Don't forget to free argv array
  utf8::free_argv (argc, argv);
  return 0;
}