from . import core
from . import color

from importlib import util
if util.find_spec("bpy"):
	print('import blender interface')
	from .bi import *

del util