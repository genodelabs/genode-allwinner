# Genode support for Allwinner-based boards

This repository complements the [Genode OS framework](https://genode.org) with
the board support needed to use Genode on devices based on Allwinner SoCs.

To use it, you first need to obtain a clone of Genode:

```sh
$ git clone https://github.com/genodelabs/genode.git genode
```

Now, clone the `genode-allwinner.git` repository to `genode/repos/allwinner`:

```sh
$ git clone https://github.com/nfeske/genode-allwinner.git genode/repos/allwinner
```

For enabling the board support, adjust the build-directory configuration
`etc/build.conf` by adding the following line before the other `REPOSITORIES`
definitions so that the `allwinner` repository is prioritized over the others.

```sh
$ REPOSITORIES += $(GENODE_DIR)/repos/allwinner
```

# Allwinner-specific tool dependencies

The Allwinner A64 SoC features a dedicated system-control processor (SCP) in
addition the the ARM application processor. This repository contains a custom
firmware for the SCP (at `src/scp/a64/`), which is incorporated into the
U-Boot boot loader (`src/u-boot/pine/`). As the SCP (aka AR100) is based on the
OpenRISC instruction-set architecture, the build process of the SCP firmware
requires the installation of binutils for this architecture. Please refer
to `src/scp/a64/README` for further instructions.


# License

Genode-specific code is provided under Genode's regular open-source license,
which is AGPLv3 + open-source linking exception. This code is also available
under a commercial license offered by Genode Labs.

For code ported from other projects - e.g., device drivers ported from the
Linux kernel - the license of the originating upstream project applies.

Please refer to the individual file headers for detailed information.
