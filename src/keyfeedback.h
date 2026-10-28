// SPDX-License-Identifier: MIT
#pragma once
#include <QtCore>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QtCore/qnativeinterface.h>
#endif

inline void initializeCalculatorKeyFeedback(bool sound) {
#ifdef Q_OS_ANDROID
 const auto activity=QNativeInterface::QAndroidApplication::context();
 if(!activity.isValid())return;
 QJniObject::callStaticMethod<void>("io/github/davoda1/emcicalc/KeyFeedback","initialize",
  "(Landroid/app/Activity;Z)V",activity.object<jobject>(),jboolean(sound));
 static bool shutdownConnected=false;
 if(!shutdownConnected && qApp){
  shutdownConnected=true;
  QObject::connect(qApp,&QCoreApplication::aboutToQuit,qApp,[]{
   QJniObject::callStaticMethod<void>("io/github/davoda1/emcicalc/KeyFeedback","shutdown","()V");
  });
 }
#else
 Q_UNUSED(sound);
#endif
}

inline void setCalculatorKeySoundEnabled(bool sound) {
#ifdef Q_OS_ANDROID
 QJniObject::callStaticMethod<void>("io/github/davoda1/emcicalc/KeyFeedback","setSoundEnabled",
  "(Z)V",jboolean(sound));
#else
 Q_UNUSED(sound);
#endif
}

inline void playCalculatorKeyFeedback(bool sound,bool vibration) {
#ifdef Q_OS_ANDROID
 if(!sound&&!vibration)return;
 const auto activity=QNativeInterface::QAndroidApplication::context();
 if(!activity.isValid())return;
 QJniObject::callStaticMethod<void>("io/github/davoda1/emcicalc/KeyFeedback","perform",
  "(Landroid/app/Activity;ZZ)V",activity.object<jobject>(),jboolean(sound),jboolean(vibration));
#else
 Q_UNUSED(sound);Q_UNUSED(vibration);
#endif
}
