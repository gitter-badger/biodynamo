#include "simulation/ecm.h"

#include "matrix.h"
#include "stl_util.h"
#include "sim_state_serialization_util.h"

#include "cells/cell.h"
#include "local_biology/soma_element.h"
#include "local_biology/neurite_element.h"

namespace cx3d {
namespace simulation {

using physics::Substance;
using physics::SubstanceHash;
using physics::SubstanceEqual;
using physics::PhysicalNode;
using physics::IntracellularSubstance;
using physics::PhysicalNodeMovementListener;
using spatial_organization::SpaceNode;

std::shared_ptr<cx3d::JavaUtil2> ECM::java_ = std::shared_ptr<cx3d::JavaUtil2> { nullptr };

std::shared_ptr<ECM> ECM::getInstance() {
  static std::shared_ptr<ECM> instance { new ECM() };
  return instance;
}

ECM::~ECM() {
}

StringBuilder& ECM::simStateToJson(StringBuilder& sb) const {
  sb.append("{");

  SimStateSerializationUtil::unorderedCollection(sb, "physicalNodeList", physical_nodes_);
  SimStateSerializationUtil::unorderedCollection(sb, "physicalSphereList", physical_spheres_);
  SimStateSerializationUtil::unorderedCollection(sb, "physicalCylinderList", physical_cylinders_);
  SimStateSerializationUtil::unorderedCollection(sb, "somaElementList", soma_elements_);
  SimStateSerializationUtil::unorderedCollection(sb, "neuriteElementList", neurite_elements_);
  SimStateSerializationUtil::unorderedCollection(sb, "cellList", cells_);
  SimStateSerializationUtil::keyValue(sb, "ecmChemicalReactionList", "[]", false);  // fixme replace with ecmChemicalReactionList once added

  SimStateSerializationUtil::keyValue(sb, "initialNode", initial_node_);
  SimStateSerializationUtil::map(sb, "substancesLibrary", substance_lib_);
  SimStateSerializationUtil::map(sb, "intracellularSubstancesLibrary", intracellular_substance_lib_);
  //FIXME color library

  SimStateSerializationUtil::keyValue(sb, "artificialWallsForSpheres", articicial_walls_for_spheres_);
  SimStateSerializationUtil::keyValue(sb, "artificialWallsForCylinders", articicial_walls_for_cylinders_);

  SimStateSerializationUtil::keyValue(sb, "Xmin", x_min_);
  SimStateSerializationUtil::keyValue(sb, "Xmax", x_max_);
  SimStateSerializationUtil::keyValue(sb, "Ymin", y_min_);
  SimStateSerializationUtil::keyValue(sb, "Ymax", y_max_);
  SimStateSerializationUtil::keyValue(sb, "Zmin", z_min_);
  SimStateSerializationUtil::keyValue(sb, "Zmax", z_max_);

  SimStateSerializationUtil::keyValue(sb, "anyArtificialGradientDefined", any_artificial_gradient_defined_);

  SimStateSerializationUtil::mapOfDoubleArray(sb, "gaussianArtificialConcentrationZ",
                                              gaussian_artificial_concentration_z_);
  SimStateSerializationUtil::mapOfDoubleArray(sb, "linearArtificialConcentrationZ", linear_artificial_concentration_z_);
  SimStateSerializationUtil::mapOfDoubleArray(sb, "gaussianArtificialConcentrationX",
                                              gaussian_artificial_concentration_x_);
  SimStateSerializationUtil::mapOfDoubleArray(sb, "linearArtificialConcentrationX", linear_artificial_concentration_x_);
  SimStateSerializationUtil::map(sb, "allArtificialSubstances", all_artificial_substances_);

  SimStateSerializationUtil::removeLastChar(sb);
  sb.append("}");
  return sb;
}

void ECM::setBoundaries(double x_min, double x_max, double y_min, double y_max, double z_min, double z_max) {
  x_min_ = x_min;
  x_max_ = x_max;
  y_min_ = y_min;
  y_max_ = y_max;
  z_min_ = z_min;
  z_max_ = z_max;
}

void ECM::setArtificialWallsForSpheres(bool wall_for_sphere) {
  articicial_walls_for_spheres_ = wall_for_sphere;
}

bool ECM::getArtificialWallForSpheres() const {
  return articicial_walls_for_spheres_;
}

void ECM::setArtificialWallsForCylinders(bool wall_for_cylinders) {
  articicial_walls_for_cylinders_ = wall_for_cylinders;
}

bool ECM::getArtificialWallForCylinders() const {
  return articicial_walls_for_cylinders_;
}

std::array<double, 3> ECM::forceFromArtificialWall(const std::array<double, 3>& location, double radius) {
  // TODO : take the radius into account
  std::array<double, 3> force { 0.0, 0.0, 0.0 };
  double spring = 2.0;  // that pulls Cells back into boundaries
  if (location[0] < x_min_) {
    force[0] += spring * (x_min_ - location[0]);
  } else if (location[0] > x_max_) {
    force[0] += spring * (x_max_ - location[0]);
  }

  if (location[1] < y_min_) {
    force[1] += spring * (y_min_ - location[1]);
  } else if (location[1] > y_max_) {
    force[1] += spring * (y_max_ - location[1]);
  }

  if (location[2] < z_min_) {
    force[2] += spring * (z_min_ - location[2]);
  } else if (location[2] > z_max_) {
    force[2] += spring * (z_max_ - location[2]);
  }
  return force;
}

std::shared_ptr<spatial_organization::SpaceNode<physics::PhysicalNode> > ECM::getSpatialOrganizationNodeInstance(
    const std::array<double, 3>& position, const std::shared_ptr<physics::PhysicalNode>& userObject) {
  if (initial_node_ == nullptr) {
    auto sn1 = SpaceNode<PhysicalNode>::create(position, userObject);
    sn1->addSpatialOrganizationNodeMovementListener(PhysicalNodeMovementListener::create());
    initial_node_ = sn1;
    return sn1;
  }
  return initial_node_->getNewInstance(position, userObject);  // todo catch PositionNotAllowedException
}

std::shared_ptr<spatial_organization::SpaceNode<physics::PhysicalNode> > ECM::getSpatialOrganizationNodeInstance(
    const std::shared_ptr<spatial_organization::SpaceNode<physics::PhysicalNode> >& n,
    const std::array<double, 3>& position, const std::shared_ptr<physics::PhysicalNode>& userObject) {
  if (initial_node_ == nullptr) {
    auto sn1 = SpaceNode<PhysicalNode>::create(position, userObject);
    sn1->addSpatialOrganizationNodeMovementListener(PhysicalNodeMovementListener::create());
    initial_node_ = sn1;
    return sn1;

  }
  return n->getNewInstance(position, userObject);
}

void ECM::addGridOfPhysicalNodes(double x1, double x2, double y1, double y2, double z1, double z2, double d) {
  // distance outside the boundary where you put your first nodes
  double border_length = 20;

  // finding the number of nodes in each coordinate (total length / internode dist.)
  int x_lim = (int) ((x2 - x1 + 2 * border_length) / d);
  int y_lim = (int) ((y2 - y1 + 2 * border_length) / d);
  int z_lim = (int) ((z2 - z1 + 2 * border_length) / d);

  // the neighbor Node (close to which we will create the new one
  std::shared_ptr<SpaceNode<PhysicalNode>> old_son;
  // loop to put the nodes in 3D space
  for (int kx = 0; kx < x_lim + 1; kx++) {
    for (int ky = 0; ky < y_lim + 1; ky++) {
      for (int kz = 0; kz < z_lim + 1; kz++) {
        // finding exact position
        std::array<double, 3> coord { (x1 - border_length) + d * kx, (y1 - border_length) + d * ky, (z1 - border_length)
            + d * kz };
        // add small jitter
        coord = Matrix::add(coord, matrixRandomNoise3(d * 0.01));
        // create the node
        auto pn = java_->newPhysicalNode();
        // request a delaunay vertex
        std::shared_ptr<SpaceNode<PhysicalNode>> new_son;
        if (old_son != nullptr) {
          new_son = getSpatialOrganizationNodeInstance(old_son, coord, pn);
        } else {
          new_son = getSpatialOrganizationNodeInstance(coord, pn);
        }
        // setting call-back
        pn->setSoNode(new_son);
        // register this node as in ECM
        addPhysicalNode(pn);
        // becomes the neighbor of the next node
        old_son = new_son;
      }
    }
  }
}

std::shared_ptr<physics::PhysicalNode> ECM::getPhysicalNodeInstance(const std::array<double, 3>& position) {
  auto pn = java_->newPhysicalNode();
  auto son = getSpatialOrganizationNodeInstance(position, pn);
  pn->setSoNode(son);
  addPhysicalNode(pn);
  return pn;
}

void ECM::addPhysicalCylinder(const std::shared_ptr<physics::PhysicalCylinder>& cyl) {
  physical_cylinders_.push_back(cyl);
  addPhysicalNode(cyl);
}

void ECM::removePhysicalCylinder(const std::shared_ptr<physics::PhysicalCylinder>& cyl) {
  STLUtil::vectorRemove(physical_cylinders_, cyl);
  removePhysicalNode(cyl);
}

void ECM::addPhysicalSphere(const std::shared_ptr<physics::PhysicalSphere>& sphere) {
  physical_spheres_.push_back(sphere);
  addPhysicalNode(sphere);
}

void ECM::removePhysicalSphere(const std::shared_ptr<physics::PhysicalSphere>& sphere) {
  STLUtil::vectorRemove(physical_spheres_, sphere);
  removePhysicalNode(sphere);
}

void ECM::addPhysicalNode(const std::shared_ptr<physics::PhysicalNode>& node) {
  physical_nodes_.push_back(node);
}

void ECM::removePhysicalNode(const std::shared_ptr<physics::PhysicalNode>& node) {
  STLUtil::vectorRemove(physical_nodes_, node);
}

void ECM::addCell(const std::shared_ptr<cells::Cell>& cell) {
  cells_.push_back(cell);
}

void ECM::removeCell(const std::shared_ptr<cells::Cell>& cell) {
  STLUtil::vectorRemove(cells_, cell);
}

void ECM::addSomaElement(const std::shared_ptr<local_biology::SomaElement>& soma) {
  soma_elements_.push_back(soma);
}

void ECM::removeSomaElement(const std::shared_ptr<local_biology::SomaElement>& soma) {
  STLUtil::vectorRemove(soma_elements_, soma);
}

void ECM::addNeuriteElement(const std::shared_ptr<local_biology::NeuriteElement>& neurite) {
  neurite_elements_.push_back(neurite);
}

void ECM::removeNeuriteElement(const std::shared_ptr<local_biology::NeuriteElement>& neurite) {
  STLUtil::vectorRemove(neurite_elements_, neurite);
}

void ECM::resetTime() {
  time_ = 0.0;
}

void ECM::clearAll() {
  physical_nodes_.clear();
  physical_spheres_.clear();
  physical_cylinders_.clear();
  soma_elements_.clear();
  neurite_elements_.clear();
  cells_.clear();
  time_ = 0;

  initial_node_= std::shared_ptr<SpaceNode<PhysicalNode>> { nullptr };
  substance_lib_.clear();
  intracellular_substance_lib_.clear();
  cell_color_lib_.clear();
  articicial_walls_for_spheres_ = false;
  articicial_walls_for_cylinders_ = false;
  x_min_ = -100;
  x_max_ = 100;
  y_min_ = -100;
  y_max_ = 100;
  z_min_ = -100;
  z_max_ = 300;
  any_artificial_gradient_defined_ = false;
  gaussian_artificial_concentration_z_.clear();
  linear_artificial_concentration_z_.clear();
  gaussian_artificial_concentration_x_.clear();
  linear_artificial_concentration_x_.clear();
  all_artificial_substances_.clear();
}

void ECM::addNewSubstanceTemplate(const std::shared_ptr<physics::Substance>& s) {
  substance_lib_[s->getId()] = s;
}

void ECM::addNewIntracellularSubstanceTemplate(const std::shared_ptr<physics::IntracellularSubstance>& s) {
  intracellular_substance_lib_[s->getId()] = s;
}

std::shared_ptr<physics::Substance> ECM::substanceInstance(const std::string& id) {
  auto s = STLUtil::mapGet(substance_lib_, id);
  if (s == nullptr) {
    s = Substance::create();
    s->setId(id);
    // s will have the default color blue, diff const 1000 and degrad const 0.01
    substance_lib_[id] = s;
  }
  return Substance::create(s);
}

std::shared_ptr<physics::IntracellularSubstance> ECM::intracellularSubstanceInstance(const std::string& id) {
  auto s = STLUtil::mapGet(intracellular_substance_lib_, id);
  if (s == nullptr) {
    s = IntracellularSubstance::create();
    s->setId(id);
    // s will have the default color blue, diff const 1000 and degrad const 0.01
    intracellular_substance_lib_[id] = s;
  }
  return IntracellularSubstance::create(s);
}

void ECM::addNewCellTypeColor(const std::string& cellType, Color color) {
  cell_color_lib_[cellType] = color;
}

Color ECM::cellTypeColor(const std::string& cellType) {
  //Select color from a list of cellsTypes
  if (STLUtil::mapContains(cell_color_lib_, cellType)) {
    auto c = cell_color_lib_[cellType];
    return c;
  } else {
    auto c = java_->getRandomColor();
    cell_color_lib_[cellType] = c;
    return c;
  }
}

bool ECM::thereAreArtificialGradients() const {
  return any_artificial_gradient_defined_;
}

void ECM::addArtificialGaussianConcentrationZ(const std::shared_ptr<physics::Substance>& substance,
                                              double max_concentration, double z_coord, double sigma) {
  auto s = getRegisteredArtificialSubstance(substance);  //todo why?
  // define distribution values for the chemical, and store them together
  std::array<double, 3> value { max_concentration, z_coord, sigma };
  gaussian_artificial_concentration_z_[s] = value;
}

void ECM::addArtificialGaussianConcentrationZ(const std::string& substance_name, double max_concentration, double z_coord,
                                              double sigma) {
  auto s = getRegisteredArtificialSubstance(substance_name);  //todo why?
  // define distribution values for the chemical, and store them together
  std::array<double, 3> value { max_concentration, z_coord, sigma };
  gaussian_artificial_concentration_z_[s] = value;
}

void ECM::addArtificialLinearConcentrationZ(const std::shared_ptr<physics::Substance>& substance,
                                            double max_concentration, double z_coord_max, double z_coord_min) {
  auto s = getRegisteredArtificialSubstance(substance);  //todo why?
  // define distribution values for the chemical, and store them together
  std::array<double, 3> value { max_concentration, z_coord_max, z_coord_min };
  linear_artificial_concentration_z_[s] = value;
}

void ECM::addArtificialLinearConcentrationZ(const std::string& substance_name, double max_concentration, double z_coord_max,
                                            double z_coord_min) {
  auto s = getRegisteredArtificialSubstance(substance_name);  //todo why?
  // define distribution values for the chemical, and store them together
  std::array<double, 3> value { max_concentration, z_coord_max, z_coord_min };
  linear_artificial_concentration_z_[s] = value;
}

void ECM::addArtificialGaussianConcentrationX(const std::shared_ptr<physics::Substance>& substance,
                                              double max_concentration, double x_coord, double sigma) {
  auto s = getRegisteredArtificialSubstance(substance);  //todo why?
  // define distribution values for the chemical, and store them together
  std::array<double, 3> value { max_concentration, x_coord, sigma };
  gaussian_artificial_concentration_x_[s] = value;
}

void ECM::addArtificialGaussianConcentrationX(const std::string& substance_name, double max_concentration, double x_coord,
                                              double sigma) {
  auto s = getRegisteredArtificialSubstance(substance_name);  //todo why?
  // define distribution values for the chemical, and store them together
  std::array<double, 3> value { max_concentration, x_coord, sigma };
  gaussian_artificial_concentration_x_[s] = value;
}

void ECM::addArtificialLinearConcentrationX(const std::shared_ptr<physics::Substance>& substance,
                                            double max_concentration, double x_coord_max, double x_coord_min) {
  auto s = getRegisteredArtificialSubstance(substance);  //todo why?
  // define distribution values for the chemical, and store them together
  std::array<double, 3> value { max_concentration, x_coord_max, x_coord_min };
  linear_artificial_concentration_x_[s] = value;
}

void ECM::addArtificialLinearConcentrationX(const std::string& substanceId, double max_concentration, double x_coord_max,
                                            double x_coord_min) {
  auto s = getRegisteredArtificialSubstance(substanceId);  //todo why?
  // define distribution values for the chemical, and store them together
  std::array<double, 3> value { max_concentration, x_coord_max, x_coord_min };
  linear_artificial_concentration_x_[s] = value;
}

double ECM::getValueArtificialConcentration(const std::string& chemical,
                                            const std::array<double, 3>& position) const {
  double x = position[0];
  double z = position[2];
  // does the substance exist at all ?
  std::shared_ptr<Substance> sub;
  if (STLUtil::mapContains(all_artificial_substances_, chemical)) {
    sub = STLUtil::mapGet(all_artificial_substances_, chemical);
  } else {
    return 0;
  }
  // if yes, we look for every type of gradient it might be implicated in,
  // and we add them up
  double concentration = 0;
  // X Gaussian
  if (STLUtil::mapContains(gaussian_artificial_concentration_x_, sub)) {
    auto val = STLUtil::mapGet(gaussian_artificial_concentration_x_, sub);
    double exposant = (x - val[1]) / val[2];
    exposant = exposant * exposant * 0.5;
    concentration += val[0] * java_->exp(-exposant);
  }
  // Z Gaussian
  if (STLUtil::mapContains(gaussian_artificial_concentration_z_, sub)) {
    auto val = STLUtil::mapGet(gaussian_artificial_concentration_z_, sub);
    double exposant = (z - val[1]) / val[2];
    exposant = exposant * exposant * 0.5;
    concentration += val[0] * java_->exp(-exposant);
  }
  // X linear
  if (STLUtil::mapContains(linear_artificial_concentration_x_, sub)) {
    auto val = STLUtil::mapGet(linear_artificial_concentration_x_, sub);
    // only if between max and min
    if ((x < val[1] && x > val[2]) || (x > val[1] && x < val[2])) {
      double slope = val[0] / (val[1] - val[2]);
      double result = (x - val[2]) * slope;
      concentration += result;
    }
  }
  // Z linear
  if (STLUtil::mapContains(linear_artificial_concentration_z_, sub)) {
    auto val = STLUtil::mapGet(linear_artificial_concentration_z_, sub);
    // only if between max and min
    if ((z < val[1] && z > val[2]) || (z > val[1] && z < val[2])) {
      double slope = val[0] / (val[1] - val[2]);
      double result = (z - val[2]) * slope;
      concentration += result;
    }
  }
  return concentration;
}

double ECM::getValueArtificialConcentration(const std::shared_ptr<physics::Substance>& substance,
                                            const std::array<double, 3>& position) const {
  return getValueArtificialConcentration(substance->getId(), position);
}

std::array<double, 3> ECM::getGradientArtificialConcentration(const std::string& chemical,
                                                              const std::array<double, 3>& position) const {
  // Do we have the substance in stock?
  std::shared_ptr<Substance> sub;
  if (STLUtil::mapContains(all_artificial_substances_, chemical)) {
    sub = STLUtil::mapGet(all_artificial_substances_, chemical);
  } else {
    return {0.0, 0.0, 0.0};
  }
  // if yes, we look for every type of gradient it might be implicated in
  std::array<double, 3> gradient { 0.0, 0.0, 0.0 };
  double x = position[0];
  double z = position[2];
  // Gaussian X
  if (STLUtil::mapContains(gaussian_artificial_concentration_x_, sub)) {
    auto val = STLUtil::mapGet(gaussian_artificial_concentration_x_, sub);
    double exposant = (x - val[1]) / val[2];
    exposant = exposant * exposant * 0.5;
    double xValOfGradient = -((x - val[1]) / (val[2] * val[2])) * val[0] * java_->exp(-exposant);
    gradient[0] += xValOfGradient;
  }
  // Gaussian Z
  if (STLUtil::mapContains(gaussian_artificial_concentration_z_, sub)) {
    auto val = STLUtil::mapGet(gaussian_artificial_concentration_z_, sub);
    double exposant = (z - val[1]) / val[2];
    exposant = exposant * exposant * 0.5;
    double zValOfGradient = -((z - val[1]) / (val[2] * val[2])) * val[0] * java_->exp(-exposant);
    gradient[2] += zValOfGradient;
  }
  // Linear X
  if (STLUtil::mapContains(linear_artificial_concentration_x_, sub)) {
    auto val = STLUtil::mapGet(linear_artificial_concentration_x_, sub);
    // only if x between max and min
    if (val[1] > x && x > val[2]) {  // if up is higher, the gradient points up
      double slope = val[0] / (val[1] - val[2]);
      gradient[0] += slope;
    }
    if (val[1] < x && x < val[2]) {
      double slope = val[0] / (val[1] - val[2]);  // otherwise the gradient points down
      gradient[0] += slope;
    }
  }
  // Linear Z
  if (STLUtil::mapContains(linear_artificial_concentration_z_, sub)) {
    auto val = STLUtil::mapGet(linear_artificial_concentration_z_, sub);
    // only if x between max and min
    if (val[1] > z && z > val[2]) {  // if up is higher, the gradient points up
      double slope = val[0] / (val[1] - val[2]);
      gradient[2] += slope;
    }
    if (val[1] < z && z < val[2]) {
      double slope = val[0] / (val[1] - val[2]);  // otherwise the gradient points down
      gradient[2] += slope;
    }
  }
  return gradient;
}

double ECM::getGradientArtificialConcentration(const std::shared_ptr<physics::Substance>& s,
                                               const std::array<double, 3>& position) const {
  return getValueArtificialConcentration(s->getId(), position);
}

std::vector<std::shared_ptr<cells::Cell>> ECM::getCellList() const {
  return cells_;
}

std::vector<std::shared_ptr<physics::PhysicalNode>> ECM::getPhysicalNodeList() const {
  return physical_nodes_;
}

std::vector<std::shared_ptr<physics::PhysicalSphere>> ECM::getPhysicalSphereList() const {
  return physical_spheres_;
}

std::vector<std::shared_ptr<physics::PhysicalCylinder>> ECM::getPhysicalCylinderList() const {
  return physical_cylinders_;
}

std::list<std::shared_ptr<local_biology::NeuriteElement>> ECM::getNeuriteElementList() const {
  std::list<std::shared_ptr<local_biology::NeuriteElement>> ret;
  for (auto neurite : neurite_elements_) {
    ret.push_back(neurite);
  }
  return ret;
  //fixme change to return neuriteElementList
}

std::vector<std::shared_ptr<local_biology::SomaElement>> ECM::getSomaElementList() const {
  return soma_elements_;
}

bool ECM::isAnyArtificialGradientDefined() const {
  return any_artificial_gradient_defined_;
}

std::unordered_map<std::shared_ptr<physics::Substance>, std::array<double, 3>, SubstanceHash, SubstanceEqual> ECM::getGaussianArtificialConcentrationZ() const {
  return gaussian_artificial_concentration_z_;
}

std::unordered_map<std::shared_ptr<physics::Substance>, std::array<double, 3>, SubstanceHash, SubstanceEqual> ECM::getLinearArtificialConcentrationZ() const {
  return linear_artificial_concentration_z_;
}

std::unordered_map<std::shared_ptr<physics::Substance>, std::array<double, 3>, SubstanceHash, SubstanceEqual> ECM::getGaussianArtificialConcentrationX() const {
  return gaussian_artificial_concentration_x_;
}

std::unordered_map<std::shared_ptr<physics::Substance>, std::array<double, 3>, SubstanceHash, SubstanceEqual> ECM::getLinearArtificialConcentrationX() const {
  return linear_artificial_concentration_x_;
}

double ECM::getECMtime() const {
  return time_;
}

void ECM::setECMtime(double time) {
  time_ = time;
}

void ECM::increaseECMtime(double delta) {
  time_ += delta;
}

void ECM::setCellList(const std::vector<std::shared_ptr<cells::Cell>>& cells) {
  cells_ = cells;
}

std::unordered_map<std::string, std::shared_ptr<physics::IntracellularSubstance>> ECM::getIntracelularSubstanceTemplates() const {
  return intracellular_substance_lib_;
}

std::unordered_map<std::string, std::shared_ptr<physics::Substance>> ECM::getSubstanceTemplates() const {
  return substance_lib_;
}

std::array<double, 3> ECM::getMinBounds() const {
  return {x_min_,y_min_,z_min_};
}

std::array<double, 3> ECM::getMaxBounds() const {
  return {x_max_,y_max_,z_max_};
}

ECM::ECM() {
}

std::shared_ptr<physics::Substance> ECM::getRegisteredArtificialSubstance(
    const std::shared_ptr<physics::Substance>& substance) {
  auto registered_substance = STLUtil::mapGet(all_artificial_substances_, substance->getId());
  if (registered_substance != nullptr) {
    return registered_substance;
  } else {
    registered_substance = substance->getCopy();
    all_artificial_substances_[substance->getId()] = registered_substance;
    any_artificial_gradient_defined_ = true;
    return registered_substance;
  }
}

std::shared_ptr<physics::Substance> ECM::getRegisteredArtificialSubstance(const std::string& substanceId) {
  auto registered_substance = STLUtil::mapGet(all_artificial_substances_, substanceId);
  if (registered_substance != nullptr) {
    return registered_substance;
  } else {
    registered_substance = Substance::create(substanceId, Color(0xFF));
    all_artificial_substances_[substanceId] = registered_substance;
    any_artificial_gradient_defined_ = true;
    return registered_substance;
  }
}

}  // namespace simulation
}  // namespace cx3d