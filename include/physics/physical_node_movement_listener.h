#ifndef PHYSICS_PHYSICAL_NODE_MOVEMENT_LISTENER_H_
#define PHYSICS_PHYSICAL_NODE_MOVEMENT_LISTENER_H_

#include <vector>
#include <array>
#include <memory>

#include "physics/physical_node.h"
#include "spatial_organization/spatial_organization_node_movement_listener.h"

namespace bdm {

namespace spatial_organization {
template<class T> class SpaceNode;
}  // namespace spatial_organization

namespace physics {

class Substance;
using spatial_organization::SpaceNode;
using spatial_organization::SpatialOrganizationNodeMovementListener;

/**
 * The role of this class is a) to determinate the concentration of different chemicals
 * inside a <code>PhysicalNode</code> when it is moved or added and b) to make sure that the total quantity
 * is conserved when <code>PhysicalNode</code> are added, moved or removed.
 * To be efficient, the methods have to be executed right before or after the node operation.
 * Therefore the class implements the SpatialOrganizationNodeMovementListener interface.
 */
class PhysicalNodeMovementListener : public SpatialOrganizationNodeMovementListener<PhysicalNode> {
 public:
  using UPtr = std::unique_ptr<PhysicalNodeMovementListener>;

  static UPtr create();

  static void setMovementOperationId(int i);

  PhysicalNodeMovementListener();

  virtual ~PhysicalNodeMovementListener();

  SpatialOrganizationNodeMovementListener<PhysicalNode>::UPtr getCopy() const override;

  /**
   * MASS CONSERVATION WHEN A POINT IS MOVED :
   * We compute the total of the quantity in the node to be moved and in all
   * the nodes that will be affected (it means neighbors before and neighbors
   * after the move, that are not always the same). The total quantity before
   * and after the move defines a ratio. We then multiply the concentration
   * in all the participants by this ration -> total mass by definition is the same.
   * (Implementation note : the neighbors after the movement are only known after
   * the move, i.e. when the second method is called).
   */
  void nodeAboutToMove(const SpatialOrganizationNode<PhysicalNode>* node, const std::array<double, 3>& planned_movement)
      override;

  void nodeMoved(const SpatialOrganizationNode<PhysicalNode>* node) override;

  /**
   * MASS CONSERVATION WHEN A POINT IS REMOVED :
   * We sum the total quantity in the node that we remove and
   * its neighbors before the movement. This is compared to the sum
   * of what the neighbors have after the removal (note that their volume
   * is then bigger). The concentration in the ex-neighbors is multiplied
   * by the ratio of the two sums.
   */
  void nodeAboutToBeRemoved(const SpatialOrganizationNode<PhysicalNode>* node) override;

  void nodeRemoved(const SpatialOrganizationNode<PhysicalNode>* node) override;

  /**
   * MASS CONSERVATION WHEN A NEW POINT IS ADDED :
   * Before the point is added, we compute the concentrations at its future location
   * (if it is inside a tetrahedron, otherwise we don't add any substances and the
   * concentration will be zero !!!!). After the node is added, we know its neighbors.
   *  We then compute the summed quantity of the neighbors before volume update, we
   * then update quantities in everyones and compute the sum again (this time in
   * including the new node). The concentration in the ex-neighbors is multiplied
   *  by the ratio of the two sums.
   */
  void nodeAboutToBeAdded(const SpatialOrganizationNode<PhysicalNode>* node,
                          const std::array<double, 3>& planned_position,
                          const std::array<PhysicalNode*, 4>& vertices_of_the_tetrahedron_containing_the_position)
                              override;

  void nodeAdded(const SpatialOrganizationNode<PhysicalNode>* node) override;

  /**
   * Returns a String representation of this PhysicalNodeMovementListener
   */
  std::string toString() const override;

 private:
  PhysicalNodeMovementListener(const PhysicalNodeMovementListener&) = delete;
  PhysicalNodeMovementListener& operator=(const PhysicalNodeMovementListener&) = delete;

  /**
   * flag we put into each neighboring PhysicalNode before the move
   * so we recognize a new neighbor after the movement has occurred.
   */
  static int movement_operation_id_;

  /**
   * all extracellularSubstances present in this PhysicalNode.
   */
  std::vector<Substance*> substances_in_n_;

  /**
   * respective quantity of the extracellularSubstances before the move.
   */
  std::vector<double> q_;

  /**
   * all the neighbors of the PhysicalNode.
   */
  std::vector<PhysicalNode*> neighbors_before_;
};

}  // namespace physics
}  // namespace bdm

#endif  // PHYSICS_PHYSICAL_NODE_MOVEMENT_LISTENER_H_
