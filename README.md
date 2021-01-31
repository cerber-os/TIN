# myNFS - Simple NFS stack
Stateless NFS server, C user library and example Cpp client prepared for Internet Techniques subject at Warsaw University of Technology

## Building
Install development headers for Linux PAM. Example for Ubuntu:
```sh
sudo apt install libpam0g-dev
```

After installing neccesary dependencies, create `out/` directory and run `make` command with one of the following goals:

```sh
make all        # Build release version of project

make asan_lsan  # Build with AddressSanitizer enabled

make debug      # Build with debug symbols

make tests      # Build unit tests
```

Compiled files should be available in `out/` directory.

## Example usage

Starting server with default config file:
```sh
out/server server/example.cfg
```

Starting client:
```sh
out/client
```
