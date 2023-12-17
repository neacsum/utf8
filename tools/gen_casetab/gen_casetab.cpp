/*
  Generate case folding tables (UCASETAB.C and LCASETAB.C) from
  CASEFOLDING.TXT file.
  
  Latest version of case folding table can be downloaded from:
  https://www.unicode.org/Public/UCD/latest/ucd/CaseFolding.txt
*/


#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace std;

struct codept {
  int uc, lc;
  string descr;
};

vector<codept> tab;

int main (int argc, char **argv)
{
  char line[1024];
  char *ptr;
  codept code;
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

  while (in)
  {
    in.getline (line, sizeof (line));
    if (line[0] == '#')
      continue; //ignore comment lines
    if (!(ptr = strtok (line, "; ")))
      continue;
    code.uc = strtol (ptr, nullptr, 16);
    ptr = strtok (nullptr, "; ");
    if (*ptr != 'C' && *ptr != 'S')
      continue;
    ptr = strtok (nullptr, "; ");
    code.lc = strtol (ptr, nullptr, 16);
    ptr = strtok (nullptr, ";");
    ptr = strchr (ptr, '#') + 1;
    code.descr = ptr;
    tab.push_back (code);
  }

  ofstream out (string(argv[2]) + "/uppertab.c");
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


  //Sort table by lower case codes
  sort (tab.begin (), tab.end (),
    [](codept& p1, codept& p2)->bool {return p1.lc < p2.lc; });

  //check for duplicates
  for (size_t i = 0; i < tab.size () - 1; i++)
  {
    if (tab[i].lc == tab[i + 1].lc)
    {
      printf ("Duplicate lowercase: %05x:\n Upper case %05x - %s\n and  %05x %s\n\n",
        (int)tab[i].lc, (int)tab[i].uc, tab[i].descr.c_str (), (int)tab[i + 1].uc,
        tab[i + 1].descr.c_str ());
      tab.erase (tab.begin () + i+1);
    }
  }

  out.open (string(argv[2]) + "/lowertab.c");
  out << "//Lower case table" << dec << endl
    << "static const char32_t l2u [" << tab.size () << "] = { ";
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
  out << dec << endl;
  out << "//Upper case equivalents" << endl
    << "static const char32_t uc [" << tab.size () << "] = { " << endl;
  out << hex;
  for (size_t i = 0; i < tab.size (); i++)
  {
    out << "  0x" << std::setfill ('0') << std::setw (5) << tab[i].uc;
    if (i == tab.size () - 1)
      out << "};";
    else
      out << ", ";
    out << "// " << tab[i].descr.c_str () << endl;
  }
  out.close ();

  return 0;
}
