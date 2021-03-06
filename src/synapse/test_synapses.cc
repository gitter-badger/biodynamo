#include "synapse/test_synapses.h"

namespace bdm {
namespace synapse {

void TestSynapses::extendExcressencesAndSynapseOnEveryNeuriteElement() {
  extendExcressencesAndSynapseOnEveryNeuriteElement(1.0);
}

void TestSynapses::extendExcressencesAndSynapseOnEveryNeuriteElement(double probability_to_synapse) {
  for (auto ne : ECM::getInstance()->getNeuriteElementList()) {
    if (ne->isAxon()) {
      ne->makeBoutons(2);
    } else {
      ne->makeSpines(5);
    }
  }
  for (auto ne : ECM::getInstance()->getNeuriteElementList()) {
    if (ne->isAxon()) {
      ne->synapseBetweenExistingBS(probability_to_synapse);
    }
  }
}

}  // namespace synapse
}  // namespace bdm
