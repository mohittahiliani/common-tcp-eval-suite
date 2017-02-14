/*
Written in C++ by Ujjwal Pasupulety
October 2016
Inspiration from Perl Cookbook pg. 299
 cout.cpp - convert from Felix's connection vector format to a format more
              suitable for reading by tmix-ns
            - replace t, >, < lines with line with format:
               * {I,A} - initator or acceptor
               * time to wait after sending (usecs)
               * time to wait after receiving (usecs)
               * bytes to send (if 0, send a FIN)
 EXAMPLES:
  > 826             I   0    0    826
  t 534             A   0    534  1213
  < 1213            A   872  0    0
  t 872

  c< 190            A   0    0    190
  t< 505            I   0    0    396
  c> 396            A   505  0    46
  t> 6250           I   6250 0    0
  c< 46

*/

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <stdint.h>
using std::fstream;

//Function to remove leading and trailing whitespaces
std::string trim (std::string &str)
{
  size_t first = str.find_first_not_of (" ");
  size_t last = str.find_last_not_of (" ");
  return str.substr (first, (last - first + 1));
}

int main (int argc, char *argv[])
{
  std::string s,a,b,t,infile (argv[1]),outfile (argv[2]);
  std::string dir (""),lastDir,sym,x,y,sym2;
  uint64_t initTime = 0,accTime = 0,time = 0,sendTime = 0,recvTime = 0,size = 0,val = 0,val2 = 0;
  bool header = false;

  std::fstream fin;
  std::fstream fout;

  fin.open (infile.c_str()); // Taking input from a Felix connection vector file
  fout.open (outfile.c_str(),fstream::out); // Output modified headers and remaining lines into a new file

  while (!fin.eof ())
    {
      std::getline (fin,s); // Read file line by line
      if (!s.empty ()) //Skip empty lines
        {
          t = trim (s); //Remove leading and trailing whitespace

          a = t.substr (0,3); // Extract first 3 characters from current line(for SEQ)
          b = t.substr (0,4); // Extract first 4 characters from current line(for CONC)

          if (!(a.compare ("SEQ"))) // Check for 'SEQ' and replace with 'S'
            {
              t.replace (0,3,"S");
            }
          if (!(b.compare ("CONC"))) // Check for 'CONC' and replace with 'C'
            {
              t.replace (0,4,"C");
            }

          //For SEQ sequences
          a = t.substr (0,1);
          if (!(a.compare (">"))||!(a.compare ("<"))||!(a.compare ("t")))
            {
              x = t.substr (2); //Extract value after symbol

            }
          //For CONC sequences
          b = t.substr (0,2);
          if (!(b.compare ("c>"))||!(b.compare ("c<"))||!(b.compare ("t>"))||!(b.compare ("t<")))
            {
              y = t.substr (3); // Extract value after symbol

            }
          //For lines starting with S,m,w,r,l(headers)
          if (!(a.compare ("S"))||!(a.compare ("C"))||!(a.compare ("w"))||!(a.compare ("r"))||!(a.compare ("l"))||!(a.compare ("m")))
            {
              if (!header)
                {

                  //this is the first line in header, finish up last entry

                  //print SEQ FIN, if necessary
                  if (time > 0)
                    {
                      fout << lastDir << " " << time << " 0 0\n";
                    }

                  //print CONC FIN, if necessary
                  if (initTime > 0)
                    {
                      fout << "I " << initTime << " 0 0\n";
                    }
                  if (accTime > 0)
                    {
                      fout << "A " << accTime << " 0 0\n";
                    }
                  //reset vars
                  time = 0;
                  initTime = 0;
                  accTime = 0;
                  lastDir = "";
                  header = true;
                }
              fout << t << "\n"; //print header line
            }
          else if (!(a.compare ("#"))) // Copy comment lines
            {
              fout << t << "\n";
            }
          else
            {
              //For any other line that's not a header or comment
              sym = a; //Extract first character (for 't')
              sym2 = b; //Extract first 2 characters (for 't>'/'t<'/'c<'/'c>')

              std::stringstream v (x);
              v >> val; //convert extracted substring into integer(for 't')
              std::stringstream v2 (y);
              v2 >> val2; //convert extracted substring into integer (for 't>'/'t<'/'c<'/'c>')

              //SEQ initiator or acceptor sending
              if (!(sym.compare (">"))||!(sym.compare ("<")))
                {
                  size = val;

                  if (!(sym.compare (">")))
                    {
                      dir = "I"; //initiator
                    }
                  if (!(sym.compare ("<")))
                    {
                      dir = "A"; //acceptor

                    }
                  if (!(lastDir.compare (dir)))
                    { //the last data was sent in the same dir, so send next data after $time
                      sendTime = time;
                      recvTime = 0;
                    }
                  else
                    { //wait to receive before sending new data
                      sendTime = 0;
                      recvTime = time;
                    }

                  time = 0; //reset time
                  lastDir = dir; // save this dir
                  fout << dir << " " << sendTime << " " << recvTime << " " << size << "\n";
                }

              else if (!(sym.compare ("t"))&&(sym2.compare ("t>"))&&(sym2.compare ("t<")))
                { //SEQ time
                  time = val;
                  //if time is 0, then we can't distinguish between sending immediately and not waiting for a receive
                  if (time == 0)
                    {
                      time = 1;
                    }
                }

              if (!(sym2.compare ("c>"))||!(sym2.compare ("c<")))
                {
                  //CONC direction and size
                  size = val2;

                  if (!(sym2.compare ("c>")))
                    {
                      //initiator
                      dir = "I";
                      fout << dir << " " << initTime << " 0 " << size << "\n";
                      initTime = 0;
                    }
                  else if (!(sym2.compare ("c<")))
                    {
                      //acceptor
                      dir = "A";
                      fout << dir << " " << accTime << " 0 " << size << "\n";
                      accTime = 0;
                    }
                }
              else if (!(sym2.compare ("t<")))
                {
                  //CONC acceptor time
                  accTime = val2;
                  //if time is 0, then we can't distinguish between sending immediately and not waiting for a receive
                  if (accTime == 0)
                    {
                      accTime = 1;
                    }

                }
              else if (!(sym2.compare ("t>")))
                {
                  //CONC initiator time
                  initTime = val2;
                  //if time is 0, then we can't distinguish between sending immediately and not waiting for a receive
                  if (initTime == 0)
                    {
                      initTime = 1;
                    }

                }

            }
          header = false;
        }
    }
//catch very last FIN

//print SEQ FIN, if necessary
  if (time > 0)
    {
      fout << lastDir << " " << time << " 0 0\n";
    }

//print CONC FIN, if necessary
  if (initTime > 0)
    {
      fout << "I " << initTime << " 0 0\n";
    }
  if (accTime > 0)
    {
      fout << "A " << accTime << " 0 0\n";
    }

//print FIN if no time delay given
  if (time == 0 && initTime == 0 && accTime == 0 && dir.compare (""))
    {
      fout << dir << " 1 0 0\n";
    }

// Close file I/O streams
  fin.close ();
  fout.close ();

  return 0;
}

