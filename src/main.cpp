
#include "MyViewer.h"
#include <QApplication>

int main( int argc , char** argv )
{
    QApplication app( argc , argv );

    MyViewer * viewer = new MyViewer;
    viewer->show();

    return app.exec();
}
