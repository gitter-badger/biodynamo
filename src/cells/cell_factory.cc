#include "cells/cell_factory.h"

namespace bdm {
namespace cells {

using std::vector;
using physics::PhysicalSphere;

Cell* CellFactory::getCellInstance(const std::array<double, 3>& cell_origin) {
  // Create new cell
  auto cell = new Cell();
  auto soma = SomaElement::UPtr(new SomaElement());
  auto ps = PhysicalSphere::UPtr(new PhysicalSphere());
  auto son = ECM::getInstance()->getSpatialOrganizationNodeInstance(cell_origin, ps.get());
  ps->setSoNode(std::move(son));

  // Add cell to ECM instance
  ECM::getInstance()->addPhysicalSphere(ps.get());

  // Set cell properties
  ps->setMassLocation(cell_origin);
  ps->setColor(ECM::getInstance()->cellTypeColor(cell->getType()));

  soma->setPhysical(std::move(ps));
  cell->setSomaElement(std::move(soma));
  return cell;
}

vector<Cell*> CellFactory::get2DCellGrid(double x_min, double x_max, double y_min, double y_max, double z_pos, int n_x,
                                         int n_y, double noise_std) {

  // Insert all generated cells in a vector
  vector<Cell*> cell_list;
  double dx = (x_max - x_min) / (1 + n_x);
  double dy = (y_max - y_min) / (1 + n_y);

  // Generate cells
  for (int i = 1; i < n_x + 1; i++) {
    for (int j = 1; j < n_y + 1; j++) {
      std::array<double, 3> new_location { x_min + i * dx + Random::nextGaussian(0, noise_std), y_min + j * dy
          + Random::nextGaussian(0, noise_std), z_pos + Random::nextGaussian(0, noise_std) };
      auto cell = getCellInstance(new_location);
      cell_list.push_back(cell);
    }
  }
  return cell_list;
}

vector<Cell*> CellFactory::get3DCellGrid(double x_min, double x_max, double y_min, double y_max, double z_min,
                                         double z_max, int n_x, int n_y, int n_z, double noise_xy_std,
                                         double noise_z_std) {

  // Insert all generated cells in a vector
  vector<Cell*> cell_list;
  double dx = (x_max - x_min) / (1 + n_x);
  double dy = (y_max - y_min) / (1 + n_y);
  double dz = (z_max - z_min) / (1 + n_z);

  // Generate cells
  for (int i = 1; i < n_x + 1; i++) {
    for (int j = 1; j < n_y + 1; j++) {
      for (int k = 1; k < n_z + 1; k++) {
        std::array<double, 3> new_location { x_min + i * dx + Random::nextGaussian(0, noise_xy_std), y_min + j * dy
            + Random::nextGaussian(0, noise_xy_std), z_min + k * dz + Random::nextGaussian(0, noise_z_std) };
        auto cell = getCellInstance(new_location);
        cell_list.push_back(cell);
      }
    }
  }
  return cell_list;
}

}  // namespace cells
}  // namespace bdm
