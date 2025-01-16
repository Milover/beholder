# beholder

> [!Warning]
> Repo under construction.

beholder is a program for building industrial machine vision solutions.

The following features are currently implemented:
- configuration and control of [GenICam][genicam] compliant cameras, with
  support for various transport protocols (GigE, USB) through integration
  with [pylon][pylon]
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

To build the project into a local staging directory run the
[`build_local.sh`](scripts/build_local.sh) script.

Note that when building locally, to run the final binary, `LD_LIBRARY_PATH` needs
to be updated so that the shared libraries become available:

```sh
LD_LIBRARY_PATH=/<beholder-path>/c-deps/build/staging bin/beholder
```
or

```sh
export LD_LIBRARY_PATH=/<beholder-path>/c-deps/build/staging bin/beholder
bin/beholder
```

To build a Docker image run the [`beholder_image.sh`](scripts/beholder_image.sh)
script.

**TODO**
- [ ] describe manual building

### Licensing

The project is published under [Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0),
however the repository also contains DNN model weights from other projects
which are used for testing, and may be published under different licences.
If you intend on using some of them, check [their licensing terms](test/assets/models/licenses).

**TODO**
- [ ] add licences for third-party C++/Go dependencies.


[genicam]: https://www.emva.org/standards-technology/genicam/
[pylon]: https://www.baslerweb.com/en/software/pylon/
[tesseract]: https://tesseract-ocr.github.io/
[opencv]: https://opencv.org
[opencv-dnn]: https://docs.opencv.org/4.10.0/d2/d58/tutorial_table_of_content_dnn.html
