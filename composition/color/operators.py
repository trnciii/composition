import numpy as np

mul = lambda f1, f2: lambda hit: f1(hit) * f2(hit)
add = lambda f1, f2: lambda hit: f1(hit) + f2(hit)
sub = lambda f1, f2: lambda hit: f1(hit) - f2(hit)
div = lambda f1, f2: lambda hit: f1(hit) / f2(hit)
mix = lambda f1, f2, t: lambda hit: (1-t)*f1(hit) + t*f2(hit)
pow = lambda f1, f2: lambda hit: f1(hit) ** f2(hit)
max = lambda f1, f2: lambda hit: np.maximum(f1(hit), f2(hit))
min = lambda f1, f2: lambda hit: np.mimimum(f1(hit), f2(hit))
