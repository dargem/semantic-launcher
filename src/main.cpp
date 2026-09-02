#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QWindow>
#include <QtCore/qglobal.h>

#include <LayerShellQt/window.h>

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "wayland");
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "layer-shell");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.loadFromModule("SemanticLauncher", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    auto *window = qobject_cast<QWindow *>(engine.rootObjects().constFirst());
    if (!window) {
        return -1;
    }

    window->setVisible(false);

    auto *layerShellWindow = LayerShellQt::Window::get(window);
    layerShellWindow->setLayer(LayerShellQt::Window::LayerOverlay);
    layerShellWindow->setAnchors(LayerShellQt::Window::AnchorTop);
    layerShellWindow->setMargins(QMargins(0, 100, 0, 0));
    layerShellWindow->setExclusiveEdge(LayerShellQt::Window::AnchorTop);
    layerShellWindow->setDesiredSize(QSize(window->width(), window->height()));
    layerShellWindow->setExclusiveZone(0);
    layerShellWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);

    window->show();
    return app.exec();
}