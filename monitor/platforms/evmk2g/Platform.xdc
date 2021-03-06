metaonly module Platform inherits xdc.platform.IPlatform
{
config ti.platforms.generic.Platform.Instance CPU =
  ti.platforms.generic.Platform.create("CPU", {
    clockRate:      1000,
    catalogName:    "ti.catalog.c6000",
    deviceName:     "TCI66AK2G02",

    customMemoryMap:
    [
      /*-----------------------------------------------------------------------
      * L1D regions, both l1d cache and l1d scratchpad will reside within this
      *----------------------------------------------------------------------*/
      [ "L1DSRAM",       { name: "L1DSRAM",
                          base: 0x00F00000,
                          len:  0x00008000,
                          space: "code/data",
                          access: "RWX", } ],

      /*-----------------------------------------------------------------------
      * L2 Related regions
      *----------------------------------------------------------------------*/
      [ "L2SRAM",    { name: "L2SRAM",
                       base: 0x00800000,
                       len:     0x10000,
                       space: "code/data",
                       access: "RWX", } ],

      [ "OCL_LOCAL", { name: "OCL_LOCAL",
                       base: 0x00810000,
                       len:     0xd0000,
                       space: "code/data",
                       access: "RWX", } ],

      /*-----------------------------------------------------------------------
      * DDR Releated regions in MPM memory: 0x9D00_0000 - 0x9F80_0000
      *----------------------------------------------------------------------*/
      [ "DDR3",         { name: "DDR3",
                          base: 0x9D000000,
                          len:  0x003E0000,
                          space: "code/data",
                          access: "RWX", } ],

      /* Each DSP core uses its own framework components data structure
         via remoteproc memory allocation for carveout */
      [ "DDR3_FC",     { name: "DDR3_FC",
                          base: 0x9D3E0000,
			  len:  0x00020000,
                          space: "code/data",
			  access: "RWX", } ],

      /* 8MB DDR3_NC memory in MPM and 8MB VRING memory (0x9F8 - 0x9FF) share
         the same 16MB non-cached MAR setting: 0x9F00_0000 - 0x9FFF_FFFF */
      /* Non-cached DDR */
      [ "DDR3_NC",   { name: "DDR3_NC",
                          base: 0x9F000000,
                          len:  0x00800000,
                          space: "code/data",
                          access: "RWX", } ],

      /* Cached DDR */
      /* Stack for ocl_service_omp task - 0x10000 for each core */
      [ "DDR3_STACK", { name: "DDR3_STACK",
                          base: 0x9E000000,
                          len:  0x00010000,
                          space: "code/data",
                          access: "RWX", } ],

      [ "DDR3_HEAP", { name: "DDR3_HEAP",
                          base: 0x9E010000,
                          len:  0x00FF0000,
                          space: "code/data",
                          access: "RWX", } ],
    ],

    l2Mode:"128k",
    l1PMode:"32k",
    l1DMode:"32k",
});

instance :
  override config string codeMemory  = "DDR3";
  override config string stackMemory = "L2SRAM";
  override config string dataMemory  = "L2SRAM";
}
