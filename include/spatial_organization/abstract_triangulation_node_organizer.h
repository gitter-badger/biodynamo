#ifndef SPATIAL_ORGANIZATION_ABSTRACT_TRIANGULATION_NODE_ORGANIZER_H_
#define SPATIAL_ORGANIZATION_ABSTRACT_TRIANGULATION_NODE_ORGANIZER_H_

#include <vector>
#include <memory>
#include <string>

namespace bdm {
namespace spatial_organization {

template<class T> class SpaceNode;
template<class T> class Triangle3D;

/**
 * Instances of child classes of this class are used to keep track of
 * nodes in an incomplete triangulation which might possibly become neighbors of open triangles.
 *
 * @param <T> The type of user objects associated with nodes in the current triangulation.
 */
template<class T>
class AbstractTriangulationNodeOrganizer {
 public:
  AbstractTriangulationNodeOrganizer();

  virtual ~AbstractTriangulationNodeOrganizer();

  virtual std::vector<SpaceNode<T>*> getNodes(SpaceNode<T>* reference_point) = 0;

  void addTriangleNodes(const std::shared_ptr<Triangle3D<T>>& triangle);

  virtual void removeNode(SpaceNode<T>* node) = 0;

  virtual void addNode(SpaceNode<T>* node) = 0;

  virtual SpaceNode<T>* getFirstNode() const = 0;

  virtual std::string toString() const = 0;

 private:
  AbstractTriangulationNodeOrganizer(const AbstractTriangulationNodeOrganizer<T>&) = delete;
  AbstractTriangulationNodeOrganizer<T>& operator=(const AbstractTriangulationNodeOrganizer<T>&) = delete;
};

}  // namespace spatial_organization
}  // namespace bdm

#endif // SPATIAL_ORGANIZATION_ABSTRACT_TRIANGULATION_NODE_ORGANIZER_H_
