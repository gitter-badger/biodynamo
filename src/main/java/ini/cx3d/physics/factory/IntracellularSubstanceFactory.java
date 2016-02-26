package ini.cx3d.physics.factory;

import ini.cx3d.physics.interfaces.IntracellularSubstance;
import ini.cx3d.swig.physics.physics;


/**
 * Factory that generates Substance objects
 */
public class IntracellularSubstanceFactory {

    private static final boolean NATIVE = physics.useNativeIntracellularSubstance;
    public static final boolean DEBUG = physics.debugIntracellularSubstance;

    public static IntracellularSubstance create() {
        if (NATIVE) {
            return ini.cx3d.swig.physics.IntracellularSubstance.create();
        } else if(!DEBUG) {
            return new ini.cx3d.physics.IntracellularSubstance();
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static IntracellularSubstance create(String id, double diffusionConstant, double degradationConstant) {
        if (NATIVE) {
            return ini.cx3d.swig.physics.IntracellularSubstance.create(id, diffusionConstant, degradationConstant);
        } else if(!DEBUG) {
            return new ini.cx3d.physics.IntracellularSubstance(id, diffusionConstant, degradationConstant);
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static IntracellularSubstance create(IntracellularSubstance templateSubstance) {
        if (NATIVE) {
            return ini.cx3d.swig.physics.IntracellularSubstance.create(templateSubstance);
        } else if(!DEBUG) {
            return new ini.cx3d.physics.IntracellularSubstance((ini.cx3d.physics.IntracellularSubstance) templateSubstance);
        } else {
            throw new UnsupportedOperationException();
        }
    }
}