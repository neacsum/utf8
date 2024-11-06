/*
  Copyright (c) Mircea Neacsu (2014-2024) Licensed under MIT License.
  This is part of UTF8 project. See LICENSE file for full license terms.
*/

/*
  Generate case mapping tables (lowertab.h and uppertab.h) from
  UnicodeData.txt file.

  Latest version of case mapping table can be downloaded from:
  https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt
*/


#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;

struct codept {
  int uc, lc;
  string descr;
};

/*
  Fields in input file. For a description of input file format see:
  https://www.unicode.org/reports/tr44/#Property_Definitions
*/
#define CODE_FIELD 0  //character code
#define DESCR_FIELD 1 //description
#define UC_FIELD 12   //upper case equivalent
#define LC_FIELD 13   //lower case equivalent
#define NUM_FIELDS 14 //number of fields

// Parse fields in one line of input data file
bool parse (const char* line, vector<string>& arr)
{
  int nf = 0;
  arr.clear ();
  while (line)
  {
    const char* pend = strchr (line, ';');
    if (!pend)
      break;
    size_t len = pend - line;
    arr.push_back (len ? string (line, len) : string ());
    line = pend + 1;
    ++nf;
  }
  return (nf == 14);
}

int main (int argc, char **argv)
{
  char line[1024];
  vector<codept> tab;

  codept code;
  printf ("gen_casetab running. Command line is:\n");
  for (int i = 0; i < argc; i++)
  {
    printf ("`%s` ", argv[i]);
  }
  printf ("\n");

  if (argc < 2)
  {
    fprintf (stderr, "Usage: gen_casetab <input table> <output folder>\n");
    exit (1);
  }
  ifstream in (argv[1]);

  if (!in.is_open ())
  {
    fprintf (stderr, "gen_casetab: cannot open input table: %s\n", argv[1]);
    exit (1);
  }
  printf ("Starting...\n");

  //Generate upper case -> lower case table
  while (in)
  {
    vector<string> fields;
    in.getline (line, sizeof (line));
    // printf ("%s\n", line);
    if (!strlen(line) || line[0] == '#' || line[0] == '\r')
      continue; //ignore empty and comment lines
    if (!parse (line, fields))
    {
      printf ("Cannot parse line:\n%s\n", line);
      continue;
    }
    if (fields[LC_FIELD].empty ())
      continue;

    code.uc = strtol (fields[CODE_FIELD].c_str(), nullptr, 16);
    code.lc = strtol (fields[LC_FIELD].c_str(), nullptr, 16);
    code.descr = fields[DESCR_FIELD];
    tab.push_back (code);
  }

  ofstream out (string(argv[2]) + "/uppertab.h");
  out << "//Upper case table" << endl
    << "static const char32_t u2l [" << tab.size() << "] = { " << endl;
  out << hex;
  for (size_t i=0; i<tab.size(); i++)
  {
    out << "  0x" << std::setfill ('0') << std::setw (5) << tab[i].uc;
    if (i == tab.size () - 1)
      out << "};";
    else
      out << ", ";
    out << "// " << tab[i].descr.c_str() << endl;
  }
  out << dec << endl;
  out << "//Lower case equivalents" << endl
    << "static const char32_t lc [" << tab.size () << "] = { ";
  out << hex;
  for (size_t i = 0; i < tab.size (); i++)
  {
    if (i % 8 == 0)
      out << endl << "  ";
    out << "0x" << std::setfill ('0') << std::setw (5) << tab[i].lc;
    if (i == tab.size () - 1)
      out << "};";
    else
      out << ", ";
  }
  out.close ();

  in.clear ();
  in.seekg (0); //rewind
  tab.clear ();

  //Generate lower case -> upper case table
  while (in)
  {
    vector<string> fields;
    in.getline (line, sizeof (line));
    // printf ("%s\n", line);
    if (!strlen (line) || line[0] == '#' || line[0] == '\r')
      continue; //ignore empty and comment lines
    parse (line, fields);
    if (fields[UC_FIELD].empty ())
      continue;

    code.uc = strtol (fields[UC_FIELD].c_str (), nullptr, 16);
    code.lc = strtol (fields[CODE_FIELD].c_str (), nullptr, 16);
    code.descr = fields[DESCR_FIELD];
    tab.push_back (code);
  }

  out.open (string(argv[2]) + "/lowertab.h");
  out << "//Lower case table" << dec << endl
    << "static const char32_t l2u [" << tab.size () << "] = { " << endl;
  out << hex;
  for (size_t i = 0; i < tab.size (); i++)
  {
    out << "  0x" << std::setfill ('0') << std::setw (5) << tab[i].lc;
    if (i == tab.size () - 1)
      out << "};";
    else
      out << ", ";
    out << "// " << tab[i].descr.c_str () << endl;
  }
  out << dec << endl;
  out << "//Upper case equivalents" << endl
    << "static const char32_t uc [" << tab.size () << "] = { " << endl;
  out << hex;
  for (size_t i = 0; i < tab.size (); i++)
  {
    if (i % 8 == 0)
      out << endl << "  ";
    out << "0x" << std::setfill ('0') << std::setw (5) << tab[i].uc;
    if (i == tab.size () - 1)
      out << "};";
    else
      out << ", ";
  }
  out.close ();

  in.close ();
  return 0;
}
