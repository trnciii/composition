from . import core
from . import color

import importlib
if importlib.util.find_spec("bpy"):
	print('import blender interface')
	from .bi import *

del importlib