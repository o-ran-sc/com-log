# Continuous Integration

This directory contains configuration for the Jenkins CI system to
build a docker image with the software in this repository.

Additional files required to build a Docker image, for example a
script for the CMD or ENTRYPOINT, might also be stored here.

Now the image build creates mdclog library RPM and Debian packages
which can be installed by running the Docker image. By default the
pacakges are copied to the /export directory but one can give the
target directory as an argument to the run command.
