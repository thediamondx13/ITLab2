#include <ITLab2.h>

#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    ITLab2 window;
    window.show();
    return app.exec();
}
