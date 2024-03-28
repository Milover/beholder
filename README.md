# Industrial OCR project

OCR testing with [tesseract][tesseract] and [OpenCV][opencv].
The goal is to build a dot matrix printout reader for industrial quality
control purposes.

Exploratory phase currently, we're interested in the processing speed,
OCR accuracy, OCR retraining possibilities and the like.

The end product should be able to process an image in under 30ms with
an accuracy of ~99%.

## Research stuff

This is our general idea of how the basic algo should look like at this point:
1. do initialization stuff
2. read image
3. find text boxes
    - these should be either lines or words
    - exact process to be determined (OpenCV EAST model?)
4. for each text box:
    1. pre-process the image
        - exact process to be determined
    2. run OCR


Tesseract notes:
- deep learning-based OCR
- uses a [long short-term memory (LSTM) network][lstm], which is a kind of
[recurrent neural network (RNN)][rnn]
- lexicons and [patterns][tesseract-patterns] are handy for improving accuracy,
but might be an issue from the user side

Notes on improving Tesseract OCR results:
- black text and white background
- DPI >= 300 (x-height should be between 20-30px), there also seems
to be an optimal capital letter size of about 30px
- should try and secure a small border (~10px)
- the following pre-processing steps should be handled by OpenCV:
    - alpha channel removal
    - binarization
    - noise removal
    - rotation/deskewing
    - dilation/erosion (morphological transformations)

We should also determine the optimal order of preprocessing steps

Helpful links:
- [How to use OpenCV With Tesseract for Real-Time Text Detection][encord]
    - this one is not that useful, since they use OpenCV just for the video feed
- [OCR Unlocked: A Guide to Tesseract in Python with Pytesseract and OpenCV][nanonets]
    - mention the following pre-processing techniques:
        - gray scaling
        - noise removal
        - thresholding
        - dilation
        - erosion
        - opening (erosion followed by dilation)
        - canny edge detection (this one seems bad)
        - deskewing
        - template matching
- [OpenCR OCR and text recognition with Tesseract][pyimg-ocr]
- [OpenCV Text Detection (EAST text detector)][pyimg-east]
- [Adaptive Vision OCR][av-ocr]
- [Synthetic OCR data generator][git-ocr-gen]
- [How to prepare training files for Tesseract OCR and improve character recognition][pretius]



[tesseract]: https://github.com/tesseract-ocr/tesseract
[tesseract-patterns]: https://tesseract-ocr.github.io/tessdoc/APIExample-user_patterns.html
[opencv]: https://opencv.org
[encord]: https://encord.com/blog/realtime-text-recognition-with-tesseract-using-opencv/
[nanonets]: https://nanonets.com/blog/ocr-with-tesseract/
[pyimg-ocr]: https://pyimagesearch.com/2018/09/17/opencv-ocr-and-text-recognition-with-tesseract/
[pyimg-east]: https://pyimagesearch.com/2018/08/20/opencv-text-detection-east-text-detector/
[lstm]: https://en.wikipedia.org/wiki/Long_short-term_memory
[rnn]: https://en.wikipedia.org/wiki/Recurrent_neural_network
[av-ocr]: https://docs.adaptive-vision.com/current/studio/machine_vision_guide/OpticalCharacterRecognition.html
[git-ocr-gen]: https://github.com/Belval/TextRecognitionDataGenerator?tab=readme-ov-file
[pretius]: https://pretius.com/blog/ocr-tesseract-training-data/
