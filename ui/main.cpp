#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    slate::ui::MainWindow window;
    window.setWindowTitle("Slate - Understand your codebase.");
    window.resize(1200, 800);
    window.show();
    return app.exec();
}
