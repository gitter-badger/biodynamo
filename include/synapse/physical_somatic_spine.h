#ifndef SYNAPSE_PHYSICAL_SOMATIC_SPINE_H_
#define SYNAPSE_PHYSICAL_SOMATIC_SPINE_H_

#include <exception>
#include <string>

#include "synapse/excrescence.h"
#include "synapse/biological_somatic_spine.h"

namespace cx3d {
namespace synapse {


class PhysicalSomaticSpine : public Excrescence {
 public:
  using UPtr = std::unique_ptr<PhysicalSomaticSpine>;

  PhysicalSomaticSpine();

  PhysicalSomaticSpine(const std::shared_ptr<physics::PhysicalObject>& po, const std::array<double, 2>& origin, double length);

  virtual ~PhysicalSomaticSpine();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual bool synapseWith(Excrescence* other, bool create_physical_bond) override;

  virtual bool synapseWithSoma(Excrescence* other_excrescence, bool create_phyiscal_bond) override;

  virtual bool synapseWithShaft(const std::shared_ptr<local_biology::NeuriteElement>& other_ne, double max_dis,
                                int nr_segments, bool create_phyiscal_bond) override;

  virtual BiologicalSomaticSpine* getBiologicalSomaticSpine() const;

  virtual void setBiologicalSomaticSpine(BiologicalSomaticSpine::UPtr spine);

 private:
  PhysicalSomaticSpine(const PhysicalSomaticSpine&) = delete;
  PhysicalSomaticSpine& operator=(const PhysicalSomaticSpine&) = delete;

  BiologicalSomaticSpine::UPtr biological_spine_;
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_PHYSICAL_SOMATIC_SPINE_H_