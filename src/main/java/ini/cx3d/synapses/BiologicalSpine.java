/*
Copyright (C) 2009 Frédéric Zubler, Rodney J. Douglas,
Dennis Göhlsdorf, Toby Weston, Andreas Hauri, Roman Bauer,
Sabina Pfister & Adrian M. Whatley.

This file is part of CX3D.

CX3D is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CX3D is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CX3D.  If not, see <http://www.gnu.org/licenses/>.
*/

package ini.cx3d.synapses;

import static ini.cx3d.SimStateSerializationUtil.keyValue;

public class BiologicalSpine implements ini.cx3d.synapses.interfaces.BiologicalSpine {
	ini.cx3d.synapses.interfaces.PhysicalSpine physicalSpine;

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		sb.append("{");

		//physical spine is circular reference
//		keyValue(sb, "physicalSpine", physicalSpine);

//		removeLastChar(sb);
		sb.append("}");
		return sb;
	}

	@Override
	public ini.cx3d.synapses.interfaces.PhysicalSpine getPhysicalSpine() {
		return physicalSpine;
	}

	@Override
	public void setPhysicalSpine(ini.cx3d.synapses.interfaces.PhysicalSpine physicalSpine) {
		this.physicalSpine = physicalSpine;
	}
	
}