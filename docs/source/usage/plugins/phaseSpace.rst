.. _usage-plugins-phaseSpace:

Phase Space
-----------

This plugin creates a 2D phase space image for a user-given spatial and momentum coordinate.

External Dependencies
^^^^^^^^^^^^^^^^^^^^^

The plugin is available as soon as the :ref:`libSplash and HDF5 libraries <install-dependencies>` are compiled in.

.cfg file
^^^^^^^^^

Example for *y-pz* phase space for the *electron* species (``.cfg`` file macro):

.. code:: bash

   # Calculate a 2D phase space
   # - momentum range in m_e c
   TGB_ePSypz="--e_phaseSpace.period 10 --e_phaseSpace.filter all --e_phaseSpace.space y --e_phaseSpace.momentum pz --e_phaseSpace.min -1.0 --e_phaseSpace.max 1.0"


The distinct options are (assuming a species ``e`` for electrons):

====================================== ======================================================== ============================
Option                                 Usage                                     Unit
====================================== ======================================================== ============================
``--e_phaseSpace.period <N>``          calculate each N steps                                   *none*
``--e_phaseSpace.filter``              Use filtered particles. Available filters are set up in  *none*
                                       :ref:`particleFilters.param <usage-params-core>`.
``--e_phaseSpace.space <x/y/z>``       spatial coordinate of the 2D phase space                 *none*
``--e_phaseSpace.momentum <px/py/pz>`` momentum coordinate of the 2D phase space                *none*
``--e_phaseSpace.min <ValL>``          minimum of the momentum range                            :math:`m_\mathrm{species} c`
``--e_phaseSpace.max <ValR>``          maximum of the momentum range                            :math:`m_\mathrm{species} c`
====================================== ======================================================== ============================

Memory Complexity
^^^^^^^^^^^^^^^^^

Accelerator
"""""""""""

locally, a counter matrix of the size local-cells of ``space`` direction times ``1024`` (for momentum bins) is permanently allocated.

Host
""""

negligible.

Output
^^^^^^

The 2D histograms are stored in ``.hdf5`` files in the ``simOutput/phaseSpace/`` directory.
A file is created per species, phasespace selection and time step.

Values are given as *charge density* per phase space bin.
In order to scale to a simpler *charge of particles* per :math:`\mathrm{d}r_i` and :math:`\mathrm{d}p_i` -bin multiply by the cell volume ``dV``.

Analysis Tools
^^^^^^^^^^^^^^

Data Reader
"""""""""""
You can quickly load and interact with the data in Python with:

.. code:: python

   from picongpu.plugins.data import PhaseSpaceData
   import numpy as np


   # load data
   ps_data = PhaseSpaceData('/home/axel/runs/lwfa_001')
   ps, meta = ps_data.get(species='e', species_filter='all', ps='ypy', iteration=2000)

   # unit conversion from SI
   mu = 1.e6  # meters to microns
   e_mc_r = 1. / (9.109e-31 * 2.9979e8)  # electrons: kg * m / s to beta * gamma

   Q_dr_dp = np.abs(e_ps) * e_ps_meta.dV  # C s kg^-1 m^-2
   extent = e_ps_meta.extent * [mu, mu, e_mc_r, e_mc_r]  # spatial: microns, momentum: beta*gamma

Note that the spatial extent of the output over time might change when running a moving window simulation.

Matplotlib Visualizer
"""""""""""""""""""""

You can quickly plot the data in Python with:

.. code:: python

   from picongpu.plugins.plot_mpl import PhaseSpaceMPL
   import matplotlib.pyplot as plt


   # create a figure and axes
   fig, ax = plt.subplots(1, 1)

   # create the visualizer
   ps_vis = PhaseSpaceMPL('path/to/run_dir', ax)

   # plot
   ps_vis.visualize(iteration=200, species='e')

   plt.show()

The visualizer can also be used from the command line by writing

 .. code:: bash

    python phase_space_visualizer.py

with the following command line options

================================     =======================================================
Options                              Value
================================     =======================================================
-p                                   Path and filename to the run directory of a simulation.
-i                                   An iteration number
-s (optional, defaults to 'e')       Particle species abbreviation (e.g. 'e' for electrons)
-f (optional, defaults to 'all')     Species filter string
-m (optional, defaults to 'ypy')     Momentum string to specify the phase space
================================     =======================================================

Out-of-Range Behavior
^^^^^^^^^^^^^^^^^^^^^

Particles that are *not* in the range of ``<ValL>``/``<ValR>`` get automatically mapped to the lowest/highest bin respectively.
Take care about that when setting your range and during analysis of the results.

Known Limitations
^^^^^^^^^^^^^^^^^

- only one range per selected space-momentum-pair possible right now (naming collisions)
- charge deposition uses the counter shape for now (would need one more write to neighbours to get it correct to the shape)
- the user has to define the momentum range in advance
- the resolution is fixed to ``1024 bins`` in momentum and the number of cells in the selected spatial dimension
- this plugin does not yet use :ref:`openPMD markup <pp-openPMD>`.

References
^^^^^^^^^^

The internal algorithm is explained in `pull request #347 <https://github.com/ComputationalRadiationPhysics/picongpu/pull/347>`_ and in [Huebl2014]_.

.. [Huebl2014]
        A. Huebl.
        *Injection Control for Electrons in Laser-Driven Plasma Wakes on the Femtosecond Time Scale*,
        chapter 3.2,
        Diploma Thesis at TU Dresden & Helmholtz-Zentrum Dresden - Rossendorf for the German Degree "Diplom-Physiker" (2014),
        https://doi.org/10.5281/zenodo.15924
