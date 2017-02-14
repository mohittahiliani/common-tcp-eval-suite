#ifndef TMIX_VARIABLES_H
#define TMIX_VARIABLES_H

#include <stdint.h>
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/tmix-topology.h"

namespace ns3 {

class TmixVariables : public Object
{
public:

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   */
  TmixVariables (void);

  /**
   * \brief Destructor
   */
  ~TmixVariables (void);

  void SetNBottlenecks(uint32_t nBottlenecks);
  uint32_t GetNBottlenecks(void);
  void SetTwoSided(bool twoSided);
  void SetCvecPortion(double cvecPortion);
  void SetNumPairs(int numPairs);
  void SetCvecsPerPair(uint32_t cvecsPerPair);

  void AddCvecsToPairs (Ptr<TmixTopology> tmix);

private:
  uint32_t m_nBottlenecks;
  bool m_twosided;
  double m_cvecPortion;
  uint32_t m_numPairs;
  uint32_t m_cvecsPerPair;
};

}
#endif
