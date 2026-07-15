#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("YtDlpGui");
    QApplication::setOrganizationName("Necto");

    MainWindow window;
    window.resize(720, 640);
    window.show();

    return app.exec();
}
