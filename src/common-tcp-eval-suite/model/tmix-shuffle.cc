#include "tmix-shuffle.h"
#include "ns3/log.h"
#include <vector>
#include <math.h> 
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TmixShuffle");

TmixShuffle::TmixShuffle ()
{
  m_burstTEstI = 0;
  m_burstTEstA = 0;
  m_numSbinsI = 0;
  m_numSbinsA = 0;
  m_binSizeus = 10000000.0;
  m_connDataI = 0;
  m_connDataA = 0;
  m_preConnDataI = 0;
  m_preConnDataA = 0;
  m_preConnOverheadI = 0;
  m_preConnOverheadA = 0;
  m_lastidleI = 0;
  m_lastidleA = 0;
  m_mssI = 0;
  m_mssA = 0;
  m_rStartStream = 0;
}

TmixShuffle::~TmixShuffle ()
{
}


void TmixShuffle::SetStartStream (long int rss)
{
  m_rStartStream = rss;
}


void TmixShuffle::AssignStreams ()
{
  //m_srng->SetStream (m_rStartStream);
}

void TmixShuffle::NextStream ()
{
  //m_srng->SetStream (m_srng->GetStream()+1);
}

std::vector<int> TmixShuffle::FisherYatesShuffle (std::vector<int> binlist, int reqlen)
{
  int len = binlist.size ();
  int len2 = len;
  std::vector<int> retlist;
  for (int i = 0; i < len - 1; i++)
    {
      int n = i + len2 / 0x7fffffff;
      len2--;
      int temp = binlist[n];
      binlist[n] = binlist[i];
      binlist[i] = temp;
    }
  retlist.assign (binlist.begin (), binlist.begin () + reqlen);
  return retlist;
}

double TmixShuffle::AddBurstStats (double data, double overhead, direction dir)
{
  direction oppdir;
  int binidx;
  double tdata = data + overhead;
  double ackoh = overhead;
  double data_dur = tdata / m_bpus;
  double bin_data;
  double bin_oh;
  binidx = (int) ((gluedir (m_burstTEst,dir) - m_prefillus) / m_binSizeus);
  if (data_dur > m_binSizeus)
    {
      bin_data = m_binSizeus / data_dur * tdata;
      bin_oh = m_binSizeus / data_dur * ackoh;
    }
  else
    {
      bin_data = tdata;
      bin_oh = ackoh;
    }
  if (dir == INITIATOR)
    {
      oppdir = ACCEPTOR;
    }
  else
    {
      oppdir = INITIATOR;

    }
  for (int idx = binidx; tdata > 0.0; idx++)
    {
      if (idx >= 0)
        {
          while ( idx > gluedir (m_numSbins,dir))
            {
              gluedir (m_binConnDataList,dir).push_back (0.0);
              gluedir (m_numSbins,dir)++;
            }
          while ( idx > gluedir (m_numSbins,oppdir))
            {
              gluedir (m_binConnDataList,oppdir).push_back (0.0);
              gluedir (m_numSbins,oppdir)++;
            }
          if (gluedir (m_numSbins,dir))
            {
              gluedir (m_binConnDataList,dir)[idx] += bin_data;
            }
          else
            {
              gluedir (m_binConnDataList,dir).push_back (bin_data);
            }
          if (gluedir (m_numSbins,oppdir))
            {
              gluedir (m_binConnDataList,oppdir)[idx] += bin_oh;
            }
          else
            {
              gluedir (m_binConnDataList,oppdir).push_back (bin_oh);
            }
        }
      tdata = tdata - bin_data;
      ackoh = ackoh - bin_oh;
      if (tdata < bin_data)
        {
          bin_data = tdata;
          if (ackoh > 0.0 )
            {
              bin_oh = ackoh;
            }
          else
            {
              bin_oh = 0.0;
            }
        }

    }

  return data_dur;
}

void TmixShuffle::ProcessBurst (double bdata, direction dir, int first)
{
  gluedir (m_connData,dir) += bdata;
  double oh_est = ceil (1.0 * bdata / gluedir (m_mss,dir)) * m_pktoh + 2.0 * first * m_pktoh;
  gluedir (m_OverheadEst,dir) += oh_est;
  if (gluedir (m_burstTEst,dir) < m_prefillus)
    {
      double tbd = 1.0 * bdata / m_bpus;
      double tpl = m_prefillus - gluedir (m_burstTEst,dir);
      double probdata;
      if (tbd > tpl)
        {
          probdata = ceil ((tbd - tpl) * bdata / tbd);
        }
      else
        {
          probdata = bdata;
        }
      gluedir (m_preConnData,dir) += probdata;
      gluedir (m_preConnOverhead,dir) += (int) ceil (1.0 * probdata / gluedir (m_mss,dir)) * m_pktoh;
    }
  double burst_dur = AddBurstStats (bdata, oh_est, dir);
  gluedir (m_burstTEst,dir) += burst_dur;
  gluedir (m_lastidle, dir) = 0;

}

std::vector<std::string> TmixShuffle::ShuffleTraces (double& scale, double& simtime, double binsecs, std::vector<std::string> tmixBaseCVName, bool findstats, bool findtarget, double prefillT, double bps, int ccTmixSrcs, int maxrtt, double targetload, direction targetdirection, double longflowthresh, int mss, int pktoh, double balancetol, double loadtol)
{ 
  m_pktoh = pktoh;
  m_mssI = mss;
  m_mssA = mss;
  m_prefillus = prefillT * 1000000;
  double longflowthresh_us = longflowthresh * 1000000;
  int minbins = (int)(ceil (1.0 * simtime / binsecs));
  int min_num_balance = 5;
  if (findtarget)
    {
      findstats = true;
    }
  bool looparound = true;
  double highscale = 0.0;
  double lowscale = 0.0;
  double highload = 0.0;
  double lowload = 0.0;
  int iterationtrouble = 0;
  int multiiterationtrouble = 0;
  std::vector<std::string> outfilelist;
  double Sum_prefill_dataI;
  double Sum_prefill_dataA;
  double Sum_prefill_overheadI;
  double Sum_prefill_overheadA;
  std::vector<int>::iterator bs;
  int num_conns;
  int prefill_bins;
  while ( looparound )
    {
      double concur = 1.0 > (ccTmixSrcs * 500.0 / scale * maxrtt) ? 1.0 : 500.0 / scale * maxrtt;
      m_bpus = 1.0 * bps / 8 / concur / (1000000);
      int num_balance = 0;
      outfilelist.clear ();
      std::map<std::string, std::string> shufFid;
      for (std::vector<std::string>::iterator it = tmixBaseCVName.begin (); it != tmixBaseCVName.end (); ++it)
        {
          //int found = (int) it->find_last_of ("/\\");
          char str_scale [200];
          sprintf (str_scale, "%f",scale);
          std::string tempx = std::string (str_scale);
          //std::string ofname = it->substr (found + 1) + tempx + ".shuf";
          std::string ofname = it->substr (0) + tempx + ".shuf";
          outfilelist.push_back (ofname);
          shufFid[*it] = ofname;
          //(shufFid[*it]).open(ofname.c_str(), std::ios::out | std::ios::trunc );
        }
      int tracelength = 3000;
      int totalbins = (int)ceil (1.0 * scale * tracelength / binsecs);
      int maxbins = (int)ceil (1.0 * simtime / binsecs);
      if ( maxbins > totalbins || findtarget)
        {
          maxbins = totalbins;
        }
      m_binSizeus = 1.0 * binsecs * 1000000;
      double scale_binsize_us = m_binSizeus / scale;
      prefill_bins = (int)(ceil (1.0 * prefillT / binsecs));
      std::vector<int> baseseq;
      std::vector<int> totaldataperbin;
      for (int b = 0; b < totalbins; b++)
        {
          baseseq.push_back (b);
        }
      AssignStreams ();
      std::map<std::string, std::vector<int> >newbinseq;
      std::map<std::string, int> lastnbs;
      for (std::vector<std::string>::iterator it = tmixBaseCVName.begin (); it != tmixBaseCVName.end (); ++it)
        {
          NextStream ();
          newbinseq[*it] = FisherYatesShuffle (baseseq, maxbins);
          lastnbs[*it] = 0;
        }
      num_conns = 0;
      Sum_prefill_dataI = 0;
      Sum_prefill_dataA = 0;
      Sum_prefill_overheadI = 0;
      Sum_prefill_overheadA = 0;
      m_binConnDataListI.clear ();
      m_binConnDataListA.clear ();
      m_numSbinsI = 0;
      m_numSbinsA = 0;
      for (bs = baseseq.begin (); bs != baseseq.end (); ++bs)
        {

          if (*bs > maxbins)
            {
              break;
            }
          for (std::vector<std::string>::iterator cvf = tmixBaseCVName.begin (); cvf != tmixBaseCVName.end (); ++cvf)
            {

              if (!findstats)
                {
                  continue;
                }
              int nbs = newbinseq[*cvf][*bs];
              int lower_time = nbs * scale_binsize_us;
              int upper_time = (nbs + 1) * scale_binsize_us;
              int offset_time = (nbs - *bs) * scale_binsize_us;
              char str_lower_time[200];
              char str_upper_time[200];
              sprintf (str_lower_time,"%d", lower_time);
              sprintf (str_upper_time,"%d", upper_time);
              std::string p1 = "awk \'BEGIN { out=0 } {if ($1 ~ /^[SC]/) {if ($2 >=";
              std::string p2 = ") { if ($2 < ";
              std::string p3 = ") {out=1; print $0;} else {out=0; exit;}}} else { if (out) print $0}} END{}\' ";
              std::string p4 = ".orig";
              std::string cmd = p1 + std::string (str_lower_time) + p2 + std::string (str_upper_time) + p3 + *cvf + p4;
              FILE* cvfFid = popen (cmd.c_str (), "r");
              m_connDataI = 0.0;
              m_connDataA = 0.0;
              m_OverheadEstI = 0.0;
              m_OverheadEstA = 0.0;
              m_preConnDataI = 0.0;
              m_preConnDataA = 0.0;
              double idleI = 0.0;
              double idleA = 0.0;
              m_lastidleI = 0.0;
              m_lastidleA = 0.0;
              std::vector<std::string> connout;
              bool writethis = false;
              bool firstburstI = false;
              bool firstburstA = false;
              char *line = NULL;
              size_t len = 0;
              ssize_t read;
              double newConnTime = 0;
              while ((read = getline (&line, &len, cvfFid)) != -1)
                {
                  if (line[0] == 'S' || line[0] == 'C')
                    {
                      firstburstI = true;
                      firstburstA = true;
                      if (connout.size () > 0)
                        {

                          idleI -= m_lastidleI;
                          idleA -= m_lastidleA;
                          double scaled_conntime = scale * newConnTime;
                          if ( scaled_conntime > m_prefillus)
                            {
                              writethis = true;
                            }
                          else
                            {
                              if ((scaled_conntime + idleI + (m_connDataI + m_OverheadEstI) / m_bpus) > longflowthresh_us || (scaled_conntime + idleA + (m_connDataA + m_OverheadEstA) / m_bpus) > longflowthresh_us)
                                {
                                  Sum_prefill_dataI += m_preConnDataI;
                                  Sum_prefill_dataA += m_preConnDataA;
                                  Sum_prefill_overheadI += m_preConnOverheadI;
                                  Sum_prefill_overheadA += m_preConnOverheadA;
                                }
                            }
                          if (writethis)
                            {
                              if (!findtarget)
                                {
                                  for (std::vector<std::string>::iterator co = connout.begin (); co != connout.end (); ++co)
                                    {
                                      std::ofstream x;
                                      x.open(shufFid[*cvf].c_str(), std::ofstream::out | std::ofstream::app);
                                      x << co->c_str () << "\n";
                                      x.close();
                                    }
                                }
                              writethis = false;
                            }
                          connout.clear ();
                        }
                      m_CVtype = line[0];
                      char l1[200], str_l2[200], l3[200], l4[200], l5[200], l6[200];
                      float l2;
                      if (m_CVtype == 'S')
                        {
                          sscanf (line,"%s%f%s%s%s", l1, &l2, l3, l4, l5);
                        }
                      else
                        {
                          sscanf (line,"%s%f%s%s%s%s", l1, &l2, l3, l4, l5, l6);
                        }

                      sprintf (str_l2,"%d",(int)(l2 + (int)offset_time));
                      newConnTime = l2 + (int)offset_time;
                      m_burstTEstI = scale * l2;
                      m_burstTEstA = m_burstTEstI;
                      std::string newline;
                      std::string whites = " ";
                      if (m_CVtype == 'S')
                        {
                          newline = std::string (l1) + whites + std::string (str_l2) + whites + std::string (l3) + whites + std::string (l4) + whites + std::string (l5);
                        }
                      else
                        {
                          newline = std::string (l1) + whites + std::string (str_l2) + whites + std::string (l3) + whites + std::string (l4) + whites + std::string (l5)+ whites + std::string (l6);
                        }
                      connout.push_back (newline);
                      m_connDataI = 0;
                      m_connDataA = 0;
                      m_OverheadEstI = 0;
                      m_OverheadEstA = 0;
                      m_preConnDataI = 0;
                      m_preConnDataA = 0;
                      m_preConnOverheadI = 0;
                      m_preConnOverheadA = 0;
                      idleI = 0;
                      idleA = 0;
                      m_lastidleI = 0;
                      m_lastidleA = 0;
                      num_conns++;
                    }
                  else
                    {
                      if (line[0] == '>' ||(line[0] == 'c' && line[1] == '>'))
                        {
                          char *l1 = NULL;
                          float l2;
                          sscanf (line,"%s%f", l1, &l2);
                          ProcessBurst (l2, INITIATOR, firstburstI);
                          firstburstI = 0;
                        }
                      else if (line[0] == '<' ||(line[0] == 'c' && line[1] == '<'))
                        {
                          char *l1 = NULL;
                          float l2;
                          sscanf (line,"%s%f", l1, &l2);
                          ProcessBurst (l2, ACCEPTOR, firstburstA);
                          firstburstA = 0;
                        }
                      else if (line[0] == 't' && line[1] == '>')
                        {
                          char *l1 = NULL;
                          float l2;
                          sscanf (line,"%s%f", l1, &l2);
                          idleI += l2;
                          m_burstTEstI += l2;
                          m_lastidleI = l2;
                        }
                      else if (line[0] == 't' && line[1] == '<')
                        {
                          char *l1 = NULL;
                          float l2;
                          sscanf (line,"%s%f", l1, &l2);
                          idleA += l2;
                          m_burstTEstA += l2;
                          m_lastidleA = l2;
                        }
                      else if (line[0] == 't')
                        {
                          char *l1 = NULL;
                          float l2;
                          sscanf (line,"%s%f", l1, &l2);
                          idleI += l2;
                          m_burstTEstI += l2;
                          m_lastidleI = l2;
                          idleA += l2;
                          m_burstTEstA += l2;
                          m_lastidleA = l2;
                        }
                      else if (line[0] == 'm')
                        {
                          char *l1 = NULL;
                          float l2;
                          float l3;
                          sscanf (line,"%s%f%f", l1, &l2, &l3);
                          m_mssI = l2;
                          m_mssA = l3;
                        }
                      connout.push_back (line);
                    }
                }
              pclose (cvfFid);
            }

          if (findtarget && targetload > 0 && *bs > minbins + prefill_bins)
            {
              int bcdl_length = *bs - prefill_bins;
              int third = bcdl_length / 3;
              int working_length = bcdl_length - third;
              int mid = working_length / 2 + third;
              std::vector <double> connB_l;
              std::vector <double> connB_u;
              double avB_l = 0;
              double avB_u = 0;
              if (targetdirection == BOTH)
                {
                  connB_l.assign (m_binConnDataListI.begin () + third, m_binConnDataListI.begin () + mid - 1);
                  connB_l.insert (connB_l.end (), m_binConnDataListA.begin () + third, m_binConnDataListA.begin () + mid - 1);
                  connB_u.assign (m_binConnDataListI.begin () + mid, m_binConnDataListI.begin () + bcdl_length);
                  connB_u.insert (connB_u.end (), m_binConnDataListA.begin () + mid, m_binConnDataListA.begin () + bcdl_length);
                  for (std::vector<double>::iterator cbl = connB_l.begin (); cbl != connB_l.end (); ++cbl)
                    {
                      avB_l += *cbl;
                    }
                  for (std::vector<double>::iterator cbu = connB_u.begin (); cbu != connB_u.end (); ++cbu)
                    {
                      avB_u += *cbu;
                    }
                  avB_l = avB_l / (0.5 * connB_l.size ());
                  avB_u = avB_u / (0.5 * connB_u.size ());
                }
              else
                {
                  connB_l.assign (gluedir (m_binConnDataList,targetdirection).begin () + third, gluedir (m_binConnDataList,targetdirection).begin () + mid - 1);
                  connB_u.assign (gluedir (m_binConnDataList,targetdirection).begin () + mid, gluedir (m_binConnDataList,targetdirection).begin () + bcdl_length);
                  for (std::vector<double>::iterator cbl = connB_l.begin (); cbl != connB_l.end (); ++cbl)
                    {
                      avB_l += *cbl;
                    }
                  for (std::vector<double>::iterator cbu = connB_u.begin (); cbu != connB_u.end (); ++cbu)
                    {
                      avB_u += *cbu;
                    }
                  avB_l = avB_l / (0.5 * connB_l.size ());
                  avB_u = avB_u / (0.5 * connB_u.size ());
                }
              double est_load_l = (100.0 * avB_l * 0.8) / binsecs / bps;
              double est_load_u = (100.0 * avB_u * 0.8) / binsecs / bps;
              if (avB_l > 0 )
                {
                  if (abs (est_load_l - est_load_u) / est_load_l <= balancetol)
                    {
                      num_balance++;
                      if (num_balance >= min_num_balance)
                        {
                          simtime = binsecs * *bs;
                        }
                    }
                  else
                    {
                      num_balance = 0;
                    }
                }
            }
        }
      if (!findtarget)
        {
          looparound = false;
        }
      else
        {
          int bl = *bs - prefill_bins;
          int third = bl / 3;
          double est_load;
          std::vector <double> ur;
          double ursum = 0;
          if (targetdirection == BOTH)
            {
              ur.assign (m_binConnDataListI.begin () + third,m_binConnDataListI.begin () + bl);
              ur.insert (ur.end (), m_binConnDataListA.begin () + third, m_binConnDataListA.begin () + bl);
              for (std::vector<double>::iterator uri = ur.begin (); uri != ur.end (); ++uri)
                {
                  ursum += *uri;
                }
              est_load = 100 * ursum / (0.5 * ur.size ()) * 8.0 / binsecs / bps;
            }
          else
            {
              ur.assign (gluedir (m_binConnDataList,targetdirection).begin () + third,gluedir (m_binConnDataList,targetdirection).begin () + bl);
              for (std::vector<double>::iterator uri = ur.begin (); uri != ur.end (); ++uri)
                {
                  ursum += *uri;
                }
              est_load = 100 * ursum / (0.5 * ur.size ()) * 8.0 / binsecs / bps;
            }
          if (abs (est_load - targetload) / targetload <= loadtol)
            {
              findtarget = false;
            }
          else
            {
              if (est_load > targetload)
                {
                  if (highload != 0.0)
                    {
                      if (est_load > highload)
                        {
                          iterationtrouble++;
                        }
                      else if (iterationtrouble > 0)
                        {
                          iterationtrouble = -1;
                        }
                    }
                  highload = est_load;
                  lowscale = scale;
                }
              else
                {
                  if (lowload != 0.0)
                    {
                      if (est_load <= lowload)
                        {
                          iterationtrouble++;
                        }
                      else if (iterationtrouble > 0)
                        {
                          iterationtrouble = -1;
                        }
                    }
                  lowload = est_load;
                  highscale = scale;
                }
              if (iterationtrouble > 5)
                {
                  if (est_load > targetload)
                    {
                      highload = est_load;
                      lowscale = scale;
                      highscale = 0.0;
                    }
                  else
                    {
                      lowload = est_load;
                      highscale = scale;
                      lowscale = 0.0;
                    }
                  iterationtrouble = 0;
                  multiiterationtrouble++;
                }
              if (multiiterationtrouble > 4)
                {
                  looparound = 0;
                }
              double oldscale = scale;
              if (lowload == 0.0 || highload == 0.0)
                {
                  scale = scale + scale * 0.5 * (est_load - targetload) / targetload;
                }
              else
                {
                  if (est_load > targetload)
                    {
                      scale += (highscale - scale) / 2.0;
                    }
                  else
                    {
                      scale -= (scale - lowscale) / 2.0;
                    }
                }
              if (oldscale - scale < 1.0e-10)
                {
                  looparound = false;
                }
            }
        }

    }
  m_connArrRate = 1.0 * num_conns / simtime;
  m_bl =  *bs - prefill_bins;
  m_sumPrefillValI = Sum_prefill_dataI + Sum_prefill_overheadI;
  m_sumPrefillValA = Sum_prefill_dataA + Sum_prefill_overheadA;
  return outfilelist;
}


}

