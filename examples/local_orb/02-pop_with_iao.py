#!/usr/bin/env python

'''
Mulliken population analysis with IAO orbitals
'''

import numpy
from pyscf import gto, scf, lo

x = .63
mol = gto.M(atom=[['C', (0, 0, 0)],
                  ['H', (x ,  x,  x)],
                  ['H', (-x, -x,  x)],
                  ['H', (-x,  x, -x)],
                  ['H', ( x, -x, -x)]],
            basis='ccpvtz')
mf = scf.RHF(mol).run()

mo_occ = mf.mo_coeff[:,mf.mo_occ>0]
a = lo.iao.iao(mol, mo_occ)

# Orthogonalize IAO
a = numpy.dot(a, lo.orth.lowdin(reduce(numpy.dot, (a.T, mf.get_ovlp(), a))))

# transform mo_occ to IAO representation. Note the AO dimension is reduced
mo_occ = reduce(numpy.dot, (a.T, mf.get_ovlp(), mo_occ))

dm = numpy.dot(mo_occ, mo_occ.T) * 2
pmol = mol.copy()
pmol.build(False, False, basis='minao')
mf.mulliken_pop(pmol, dm, s=numpy.eye(pmol.nao_nr()))
