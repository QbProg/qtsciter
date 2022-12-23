#include <QApplication>
#include "QSciter.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QSciter sciter;
  sciter.resize(800, 600);
  sciter.load(QUrl(QString(_CMAKE_SOURCE_DIR) + "page.html"));
  sciter.show();

  return app.exec();
}
