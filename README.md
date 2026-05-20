**This repository has been migrated to
[codeberg.org](https://codeberg.org/genodelabs/genode-allwinner) and it will no
longer be updated at GitHub.**

Adapting to this change requires little adaptation on your side. Please adapt
any existing clone of this repository. Note that we also changed the name of
the default branch from "master" to "main". The instructions below
also demonstrate how to create a local "main" branch in your cloned repository.

```
allwinner$ git remote set-url origin https://codeberg.org/genodelabs/genode-allwinner
allwinner$ git fetch origin
allwinner$ git checkout origin/main
allwinner$ git switch -c main
```

Alternatively, you may also create a fresh clone:

```
$ git clone https://codeberg.org/genodelabs/genode-allwinner allwinner
```
