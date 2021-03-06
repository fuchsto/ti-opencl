******************************************************
Device Memory 
******************************************************

.. Note::
    The amount of Local, MSMC and DDR that is available for OpenCL use
    may change from release to release.  The amount available for OpenCL use
    can be queried from the OpenCL runtime.  This is illustrated in the
    platform example shipped with the product:
    ``/usr/share/ti/examples/opencl/platforms``.

The following device components are relevant to the memory discussion for
OpenCL.

================================= =========== =========== =========== =========== =======
Attribute                         66AK2H      66AK2L      66AK2E      66AK2G      AM572x
================================= =========== =========== =========== =========== =======
ARM A15 CPU cores                 4           2           4           1           2
C66 DSP cores                     8           4           1           1           2
L1P per C66 core                  32KB        32KB        32KB        32KB        32KB
L1D per C66 core                  32KB        32KB        32KB        32KB        32KB
L2 cache shared across ARM cores  4MB         1MB         4MB         512KB       2MB
L2 memory per C66 core            1MB         1MB         512KB       1MB         288KB
DDR3 available                    up to 8GB   up to 8GB   up to 8GB   2GB         2GB
On-chip shared memory             6MB         2MB         2MB         1MB         1MB
================================= =========== =========== =========== =========== =======

The L1 and L2 memory areas in the C66x cores can be configured as all cache, all
scratchpad or partitioned with both. For OpenCL applications, this partition defaults 
to the following values for each C66x core:

====================================== ======= ======= ======= ======= ========
Attribute                              66AK2H  66AK2L  66AK2E  66AK2G  AM57x
====================================== ======= ======= ======= ======= ========
L1P cache                              32KB    32KB    32KB    32KB    32KB
L1D cache                              32KB    32KB    32KB    32KB    32KB
L2 cache                               128KB   128KB   128KB   128KB   128KB
L2 reserved                            128KB   128KB   128KB   64KB    32KB
L2 available for OpenCL local buffers  768KB   768KB   256KB   832KB   128KB
====================================== ======= ======= ======= ======= ========

Caching
==============================================================================

The below image illustrates the memory regions in which OpenCL buffers may
reside.  Those regions are highlighted in blue.  It also shows the paths from
those regions through the caches to/from the C66x DSP cores.  There are other busses for
data movement on the device, for example to move data from ddr to L2 SRAM.
This image, however, is focused on automatic data movement through cache memory.

As the image illustrates, OpenCL buffers residing in L2 SRAM or MSMC will
bypass the L2 cache, but are still cached in the L1D cache.  OpenCL buffers
residing in off-chip DDR3 will be cached in both L2 and L1D.

OpenCL global buffers may reside in either DDR or MSMC. OpenCL local buffers reside in L2 SRAM

.. Image:: ../images/opencl_memory_cache.png

