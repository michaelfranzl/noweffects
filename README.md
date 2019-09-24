# noweffects

Proof-of-concept of HTML5 + JavaScript + CSS3 RGBA video overlays on top of live GStreamer video pipelines.

See related [Blog post](https://blog.michael.franzl.name/2018/04/14/html5-javascript-css3-alpha-aware-video-overlays-on-top-of-live-gstreamer-video-pipelines) and [demonstration](https://www.youtube.com/watch?v=jAtEJdnY0Mc).

GStreamer comes with a number of plugins that allow rendering of text and/or graphics overlays on top of video: rsvgoverlay, subtitleoverlay, textoverlay, cairooverlay, gdkpixbufoverlay, opencvtextoverlay, etc. However, some of these plugins often allow only static graphics and text, and often do not approach the flexibility and power of dedicated video post-processing software products.

"noweffects" (a play on the name of a popular video post-processing software) is a proof-of-concept of leveraging the power of a modern HTML5 + JavaScript + CSS3 web browser engine to render high-quality, programmable, alpha-aware, animated, vector- and bitmap based content, which is then rendered into an RGBA raw video stream, which can then be transferred via some kind of IPC method to separate GStreamer processeses, where it can be composited with other content via GStreamers regular `compositor` or `videomixer` plugins.

Qt was chosen for its ease of integration of modern WebKit (QtWebKit) and GStreamer (qt-gstreamer), and its ability to render widgets to RGBA images. The QMainWindow widget is rendered in regular intervals to QImages in RGBA format, then inserted into a GStreamer pipeline via `appsrc` plugin. This pipeline simply uses `udpsink` to multicast the raw video RTP packets on localhost to allow for multiple 'subscribers'. A second GStreamer pipleline can then use `udpsrc` and apply the overlay.

## Usage

Open `noweffects.pro` in QtCreator, or build and run from a terminal:

    qmake
    make
    ./noweffects

The video size is `#define`d in `mainwindow.cpp` as `WIDTH` and `HEIGHT` to 1280 x 720.

To view the generated video stream, a `gst-launch-1.0 ...` pipeline is printed to STDOUT when running the application.

The generated video requires 1280 x 720 x 4colors x 30fps = 110 megabytes/second of UDP bandwidth on localhost. You will likely need to increase the Linux kernel's UDP buffer size, otherwise you may experience UDP packet loss (visible as flickering):

    sysctl -w net.core.wmem_max=2000000000
    sysctl -w net.core.rmem_max=2000000000
    
Alternatively, reduce `WIDTH` and `HEIGHT` in `mainwindow.cpp`.


## Dependencies

* Qt 5.11 commit 3ba94092
* [QtWebkit](https://github.com/qt/qtwebkit) commit 72cfbd7 (20. Jan. 2018)
* [qt-gstreamer](https://cgit.freedesktop.org/gstreamer/qt-gstreamer/) commit 0cf247bf

