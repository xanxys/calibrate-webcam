## Compilation

### PNaCl SDK and libraries
1. Install PNaCl SDK (and download latest stable pepper). The pepper directory (e.g. `/home/xyx/nack_sdk/pepper_33`) will be your `NACL_SDK_ROOT`.
2. `git clone https://chromium.googlesource.com/external/naclports/` somewhere else.
3. Compile opencv using the `NACK_SDK_ROOT`. The libraries and include files will be automatically put under `NACK_SDK_ROOT`.

### .pexe and web-deployable bundle
1. `git clone https://github.com/xanxys/calibrate-webcamt`
2. Modify variables in `Makefile`
3. `./make_deploy`
4. If all goes well, `builld/` will contain bunch of files that works.
5. Don't forget to modify Analytics code and such if you plan to deploy it to your own site.
