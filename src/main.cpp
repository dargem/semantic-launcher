#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QWindow>
#include <QtCore/qglobal.h>

#include <LayerShellQt/window.h>

#include "engine/search_engine.hpp"

int main(int argc, char* argv[])
{
    qputenv("QT_QPA_PLATFORM", "wayland");
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "layer-shell");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    SearchEngine search_engine;

    engine.rootContext()->setContextProperty("searchEngine", &search_engine);
    engine.loadFromModule("SemanticLauncher", "Main");

    if (engine.rootObjects().isEmpty())
    {
        return -1;
    }

    auto* window = qobject_cast<QWindow*>(engine.rootObjects().constFirst());
    if (!window)
    {
        return -1;
    }

    window->setVisible(false);

    auto* layer_shell_window = LayerShellQt::Window::get(window);
    layer_shell_window->setLayer(LayerShellQt::Window::LayerOverlay);
    layer_shell_window->setAnchors(LayerShellQt::Window::AnchorTop);
    layer_shell_window->setMargins(QMargins(0, 100, 0, 0));
    layer_shell_window->setExclusiveEdge(LayerShellQt::Window::AnchorTop);
    layer_shell_window->setDesiredSize(window->size());
    QObject::connect(window,
                     &QWindow::widthChanged,
                     window,
                     [window, layer_shell_window]() { layer_shell_window->setDesiredSize(window->size()); });
    QObject::connect(window,
                     &QWindow::heightChanged,
                     window,
                     [window, layer_shell_window]() { layer_shell_window->setDesiredSize(window->size()); });
    layer_shell_window->setExclusiveZone(0);
    layer_shell_window->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);

    window->show();
    return app.exec();
}