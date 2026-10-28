// SPDX-License-Identifier: MIT
#include "mainwindow.h"
#include "androidfont.h"
#include <QFileInfo>
int main(int argc,char**argv){
 QApplication app(argc,argv);
 QCoreApplication::setApplicationName("EMCICalc");
 QCoreApplication::setApplicationVersion("1.0.0");
 QCoreApplication::setOrganizationName("davoda1");
 app.setStyle("Fusion");
 AndroidFont::initialize(app);
 const auto args=app.arguments();
 bool verify=args.contains("--verify-settings");
 bool smoke=args.contains("--smoke-test");
 if(smoke||verify) {
  QStandardPaths::setTestModeEnabled(true);
  int logIndex=args.indexOf("--test-log");
  if(logIndex>=0 && logIndex+1<args.size()){
   static QFile testLog; testLog.setFileName(args[logIndex+1]);
   if(testLog.open(QIODevice::WriteOnly|QIODevice::Truncate))qInstallMessageHandler([](QtMsgType,const QMessageLogContext&,const QString&message){testLog.write(message.toUtf8()+"\n");testLog.flush();});
  }
 }
 MainWindow window(smoke||verify);
 window.show();
 if(verify)QTimer::singleShot(0,&window,[&]{app.exit(window.verifyLoadedSettings()?0:1);});
 if(smoke) QTimer::singleShot(200,&window,[&]{
  QString output; int index=args.indexOf("--screenshots"); if(index>=0 && index+1<args.size())output=args[index+1];
  bool ok=window.smokeTest(output); app.exit(ok?0:1);
 });
 return app.exec();
}
