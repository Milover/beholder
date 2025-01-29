# beholder

> [!Warning]
> Repo under construction.

beholder is a program for building industrial machine vision solutions.

The following features are currently implemented:
- configuration and control of [GenICam][genicam] compliant cameras
  integration with [pylon][pylon]
- loading and running DNN inferencing (object detection/classification,
  text detection, text recognition) models as part of the image processing
  pipeline through integration with [OpenCV's DNN module][opencv-dnn] and
  [Tesseract][tesseract]
- defining traditional image processing pipelines through integration
  with [OpenCV][opencv]
- a _very_ basic web front-end for streaming acquired images and controlling
  cameras

More stuff on the way.

### Usage

Run `beholder -h` to see a list of available commands. At this point the
commands presuppose a processing pipeline in the form of:

    image pre-processing->obj. detection->OCR->image post-processing

however, this will change in the future.

See the [configuration files](cmd/testdata) for examples of configuring cameras
and defining processing pipelines.

**TODO**
- [ ] describe usage in more detail
- [ ] describe web front-end usage

### Building

The following are needed to build the project locally:
- CMake 3.25 or newer
- GCC 12 or newer (or Clang 16 or newer)
- ninja
- pkg-config
- make (optional, but handy)
- git
- protobuf compiler

On Debian/Ubuntu use the following to install the dependencies:

```sh
apt-get install cmake g++ git make ninja-build pkg-config protobuf-compiler
```

To build the project locally run:

```sh
$ ./scripts/dev -p release make all
```

Note that when the project is built into a local staging directory, the final
binary will also have to be run through the `scripts/dev` script so that
shared libraries are in the loader search path:

```sh
$ ./scripts/dev ./bin/beholder
```

To build a [Docker][docker] runtime image run:

```sh
$ ./scripts/build_docker_image.sh
```

**TODO**
- [ ] provide a compose file so the Docker image is easier to use/deploy
- [ ] move local build into dev-details, regular users should use the Docker image

### Licensing

The project is published under [Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0),
however the repository also contains DNN model weights from other projects
which are used for testing, and may be published under different licences.
If you intend on using some of them, check [their licensing terms](test/assets/models/licenses).

**TODO**
- [ ] add licences for third-party C++/Go dependencies.


[docker]: https://docker.com
[genicam]: https://www.emva.org/standards-technology/genicam/
[pylon]: https://www.baslerweb.com/en/software/pylon/
[tesseract]: https://tesseract-ocr.github.io/
[opencv]: https://opencv.org
[opencv-dnn]: https://docs.opencv.org/4.10.0/d2/d58/tutorial_table_of_content_dnn.html
