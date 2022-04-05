float3_dtype = [('x', '<f4'), ('y', '<f4'), ('z', '<f4')]

Vertex_dtype = [('p', float3_dtype), ('n', float3_dtype)]
Face_dtype = [
	('v0', '<i4'),
	('v1', '<i4'),
	('v2', '<i4'),

	('normal', float3_dtype),
	('smooth', 'i1'),
	('material', '<i4')
]

Hit_dtype = [
	('p', float3_dtype),
	('n', float3_dtype),
	('ng', float3_dtype),
	('wo', float3_dtype),
	('mtlID', '<u4'),
	('pixel', '<u4'),
	('R', '<f4'),
	('N', '<i4'),
	('tau', float3_dtype),
	('weight', float3_dtype),
	('iteration', '<i4'),
	('depth', '<i4')
]
