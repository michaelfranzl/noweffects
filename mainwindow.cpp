/*
 * noweffects - Proof-of-concept of HTML5 + JavaScript + CSS3
 * RGBA video overlays on top of live GStreamer video pipelines.
 *
 * Copyright © 2018 Michael Karl Franzl
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>

#include "mainwindow.h"

#include <QTimer>

#include <Qt5GStreamer/QGst/Utils/ApplicationSource>
#include <Qt5GStreamer/QGst/Buffer>
#include <Qt5GStreamer/QGst/Parse>
#include <Qt5GStreamer/QGst/Pipeline>
#include <Qt5GStreamer/QGst/Bus>
#include <Qt5GStreamer/QGlib/Connect>

#define WIDTH 1280
#define HEIGHT 720


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    QDebug debug = qDebug();
    debug.noquote();

    m_webview = new QWebView(this);
    m_webview->setAttribute(Qt::WA_TranslucentBackground);
    m_webview->setStyleSheet("background:transparent");

    this->setGeometry(0, 0, WIDTH, HEIGHT);
    m_webview->setGeometry(0, 0, WIDTH, HEIGHT);

    QWebPage *page = m_webview->page();
    page->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true); // enable web inspector

    m_webview->setHtml(""
    "<html>"
    "  <head>"
    "    <style>"
    "      @keyframes slidein {"
    "        from { transform: scaleX(0); }"
    "        to   { transform: scaleX(1); }"
    "      }"
    "      body { background-color: rgba(0,0,0,0.3) }"
    "      h1 { background-color: rgba(0,0,0,0.3); animation: slidein 3s ease-in 1s infinite reverse both running; }"
    "    </style>"
    "  </head>"
    "  <body><h1 style='color:red'>Test</h1></body>"
    "</html>"
    "", QUrl("http://localhost"));

    m_frameshot = new QImage(this->size(), QImage::Format_RGBA8888);
    m_painter = new QPainter(m_frameshot);

    QString pipdescr = QString("appsrc name=mysrc is-live=true format=time do-timestamp=true caps=\"video/x-raw,format=RGBA,width=%1,height=%2\" ! rtpvrawpay mtu=65000 ! udpsink host=225.0.0.37 auto-multicast=true multicast-iface=lo ttl-mc=0 port=50000 bind-address=127.0.0.1 sync=false async=false buffer-size=200000000").arg(this->width()).arg(this->height());
    QString viewer_cmd = QString("VIEWER COMMAND: gst-launch-1.0 compositor name=comp ! videoconvert ! autovideosink udpsrc address=225.0.0.37 auto-multicast=true multicast-iface=lo buffer-size=200000000 port=50000 caps=\"application/x-rtp,clock-rate=90000,encoding-name=RAW,sampling=(string)RGBA,width=(string)%1,height=(string)%2\" ! rtpvrawdepay ! comp.\n").arg(this->width()).arg(this->height());
    debug << "\n";
    debug << viewer_cmd;
    debug << "\n";

    m_pip = QGst::Parse::launch(pipdescr).dynamicCast<QGst::Pipeline>();

    QGlib::connect(m_pip->bus(), "message", this, &MainWindow::onBusMessage);
    m_pip->bus()->addSignalWatch();
    m_pip->setState(QGst::StatePlaying);

    this->m_src.setElement(m_pip->getElementByName("mysrc"));

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::screenshot);
    timer->start(30); // appox. 33 fps
}

void MainWindow::screenshot()
{
    m_frameshot->fill(QColor(0,0,0,0));
    this->render(m_painter, QPoint(), QRegion(), QWidget::DrawChildren);

    QGst::BufferPtr buf;
    buf = QGst::Buffer::create(WIDTH * HEIGHT * 4);

    QGst::MapInfo info;
    buf->map(info, QGst::MapFlag::MapWrite);
    quint8 *dataptr = info.data();
    memcpy(dataptr, m_frameshot->bits(), WIDTH * HEIGHT * 4);
    buf->unmap(info);

    QGst::FlowReturn ret = this->m_src.pushBuffer(buf);
    // TODO: properly shut down if ret is not success
}

void MainWindow::onBusMessage(const QGst::MessagePtr &msg)
{
    switch (msg->type()) {
    case QGst::MessageEos:
        qDebug() << "Received EOS";
        // TODO: properly shut down
        break;
    case QGst::MessageError:
        qCritical() << msg.staticCast<QGst::ErrorMessage>()->error();
        break;
    case QGst::MessageWarning:
        qDebug() << "Warning" << msg.staticCast<QGst::WarningMessage>()->error();
        break;
    case QGst::MessageStateChanged:
        //qDebug() << "state changed" << msg.staticCast<QGst::StateChangedMessage>()->newState();
        break;
    default:
        qDebug() << "Received message type" << msg.staticCast<QGst::Message>()->typeName();
        break;
    }
}

MainWindow::~MainWindow()
{
    m_pip->setState(QGst::StateNull);
}
