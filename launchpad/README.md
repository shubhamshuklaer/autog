To generate the launchpad package we need to run:

* `make_package.sh` (Example arguments can be found in the file).
* `publish_package.sh` (Will submit the package generated by the previous command)

For compile time dependencies you can check the control file in the dir `Build-Depends:`.
Apart from that you would need `build-essential gnupg pbuilder ubuntu-dev-tools apt-file dh-make`.

You can follow https://packaging.ubuntu.com/html/getting-set-up.html for launchpad setup.

