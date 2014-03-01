bl_info = {
	"name": "Move X Axis",
	"category": "Object",
}

import bpy
import os
import os.path
import xml.etree.ElementTree as ET
from xml.dom.minidom import parseString
import mathutils
from math import radians, degrees
import iqm_export

def new_prettify(fw,str):
	reparsed = parseString(str)
	fw('\n'.join([line for line in reparsed.toprettyxml(indent=' '*2).split('\n') if line.strip()]))

def save(operator,
		 context,
		 filepath="",
		 global_matrix=None
		 ):

	scene = context.scene
	
	
	file = open(filepath, "w", encoding="utf8", newline="\n")
	fw = file.write
	
	root = ET.Element("scene")
	tree = ET.ElementTree(root) 
	
	for obj in scene.objects:
		if global_matrix is None:
			from mathutils import Matrix
			global_matrix = Matrix()
			print('Global matrix was None')

		if bpy.ops.object.mode_set.poll():
			bpy.ops.object.mode_set(mode='OBJECT')
		
		save_object(context,filepath, root, obj, global_matrix)
		
	new_prettify(fw,ET.tostring(root, 'utf-8'))
	file.close()

	return {'FINISHED'}
	

def save_object(context, filepath, xml_element,
				object,
				global_matrix
				):

	#axis rotation
	old_mat = object.matrix_world;
	object.matrix_world = (global_matrix * object.matrix_world)
	
	el = ET.Element("object",
	{
		"type":str(object.type),
		"name":str(object.name)
	})
	
	pos = ET.Element("position",
	{
		"x":str(object.location.x),
		"y":str(object.location.y),
		"z":str(object.location.z),
	})  
	el.append(pos)

	if object.rotation_mode == 'QUATERNION':
		rot = ET.Element("quat_rotation",
		{
			"x":str(object.rotation_quaternion.x),
			"y":str(object.rotation_quaternion.y),
			"z":str(object.rotation_quaternion.z),
			"w":str(object.rotation_quaternion.w),
		})
	else:
		rot = ET.Element("rotation",
		{
			"x":str(degrees(object.rotation_euler.x)),
			"y":str(degrees(object.rotation_euler.y)),
			"z":str(degrees(object.rotation_euler.z)),
		})
	el.append(rot)
	
	if object.type == "MESH":
		if object.library != None:
			mesh_path = ET.Element("blend_path",
			{
				"file":object.library.filepath,
			})
			el.append(mesh_path)
		elif object.data.library != None:
			mesh_path = ET.Element("blend_file",
			{
				"name":object.data.library.filepath,
			})
			el.append(mesh_path)

		iqm_path = os.path.dirname(filepath)
		iqm_filename =  object.name + ".iqm"
		iqm_fullname = iqm_path + "/" + iqm_filename
		
		iqm_export.exportIQM(context, iqm_fullname, usemesh = True, useskel = False, usebbox = True, usecol = False, scale = 1.0, animspecs = None, matfun = (lambda prefix, image: prefix + "|" + image), derigify = False)
		
		iqm_path = ET.Element("iqm_file",
				{
					"name":iqm_filename,
				})
		el.append(iqm_path)
		
	
	
	xml_element.append(el)
	
	
	object.matrix_world = old_mat
	
	return True