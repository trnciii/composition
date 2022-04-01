float3_dtype = [('x', '<f4'), ('y', '<f4'), ('z', '<f4')]
uint3_dtype = [('x', '<i4'), ('y', '<i4'), ('z', '<i4')]

Vertex_dtype = [('p', float3_dtype), ('n', float3_dtype)]
Face_dtype = [
	('v0', '<i4'),
	('v1', '<i4'),
	('v2', '<i4'),

	('normal', float3_dtype),
	('smooth', 'i1'),
	('material', '<i4')
]
