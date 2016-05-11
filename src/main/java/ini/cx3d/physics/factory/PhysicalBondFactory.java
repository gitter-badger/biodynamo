package ini.cx3d.physics.factory;

import ini.cx3d.physics.interfaces.PhysicalBond;
import ini.cx3d.swig.simulation.simulation;

import static ini.cx3d.utilities.StringUtilities.toStr;

/**
 * Factory that generates PhysicalBond objects
 */
public class PhysicalBondFactory {

    private static final boolean NATIVE = simulation.useNativePhysicalBond;
    public static final boolean DEBUG = simulation.debugPhysicalBond;

    public static PhysicalBond create() {
        if (NATIVE) {
            return ini.cx3d.swig.simulation.PhysicalBond.create();
        } else if(!DEBUG) {
            return new ini.cx3d.physics.PhysicalBond();
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static PhysicalBond create(ini.cx3d.physics.interfaces.PhysicalObject a, ini.cx3d.physics.interfaces.PhysicalObject b) {
        if (NATIVE) {
            return ini.cx3d.swig.simulation.PhysicalBond.create(a, b);
        } else if(!DEBUG) {
            return new ini.cx3d.physics.PhysicalBond(a, b);
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static PhysicalBond create(ini.cx3d.physics.interfaces.PhysicalObject a, double[] positionOnA, ini.cx3d.physics.interfaces.PhysicalObject b , double[] positionOnB, double restingLength, double springConstant) {
        if (NATIVE) {
            return ini.cx3d.swig.simulation.PhysicalBond.create(a, positionOnA, b, positionOnB, restingLength, springConstant);
        } else if(!DEBUG) {
            return new ini.cx3d.physics.PhysicalBond(a, positionOnA, b, positionOnB, restingLength, springConstant);
        } else {
            throw new UnsupportedOperationException();
        }
    }
}