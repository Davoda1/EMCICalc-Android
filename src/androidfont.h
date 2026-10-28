// SPDX-License-Identifier: MIT
#pragma once
#include <QtWidgets>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

// Android 14+ has a size-dependent SP curve: never multiply fontScale or
// scaledDensity. Values stored here are nominal SP, not previously scaled fonts.
namespace AndroidFont {
inline qreal deviceRatio(){auto *s=QGuiApplication::primaryScreen();return s?qMax(qreal(.01),s->devicePixelRatio()):1.;}
inline QHash<int,qreal>& sizes(){static QHash<int,qreal> value;return value;}
// The stable stored names also support the R8/R9 fixed-size preference.
inline QString normalizeSizeMode(const QString &value){
    if(value=="Fixed size"||value=="fixed")return QStringLiteral("Medium");
    if(value=="Small"||value=="Medium"||value=="Large")return value;
    return QStringLiteral("System");
}
inline QString sizeMode(){return qApp?normalizeSizeMode(qApp->property("androidFontSizeMode").toString()):QStringLiteral("System");}
inline bool fixedSize(){return sizeMode()!=QStringLiteral("System");}
inline qreal presetScale(){const auto mode=sizeMode();return mode=="Small"?.9:mode=="Large"?1.2:1.;}
inline qreal logicalPixels(qreal sp){
#ifdef Q_OS_ANDROID
    const int key=qRound(sp*100);auto &cache=sizes();if(cache.contains(key))return cache.value(key);
    const qreal physical=QJniObject::callStaticMethod<jfloat>("io/github/davoda1/emcicalc/FontMetricsBridge",fixedSize()?"dpToPx":"spToPx","(F)F",jfloat(fixedSize()?sp*presetScale():sp));
    const qreal value=physical>0?physical/deviceRatio():sp;cache.insert(key,value);return value;
#else
    // Explicit opt-in host diagnostic curve, deliberately NOT an Android emulator.
    // Exercises size-dependent layout branches in the host geometry suite.
    const qreal scale=qApp?qApp->property("fontTestScale").toDouble():1.;
    if(fixedSize())return sp*presetScale();
    if(scale>1.)return sp+(scale-1.)*qMin(sp,qreal(18.));
    return sp;
#endif
}
inline qreal nominalFromPixels(qreal logical){
#ifdef Q_OS_ANDROID
    const qreal value=QJniObject::callStaticMethod<jfloat>("io/github/davoda1/emcicalc/FontMetricsBridge",fixedSize()?"pxToDp":"pxToSp","(F)F",jfloat(logical*deviceRatio()));
    // Undo the preset when deriving a new widget's immutable nominal size.
    // Otherwise repeated live changes would compound its scale.
    return value>0?(fixedSize()?value/presetScale():value):logical;
#else
    return fixedSize()?logical/presetScale():logical;
#endif
}
inline int pixels(qreal sp){return qMax(1,qRound(logicalPixels(sp)));}
inline qreal fontPixels(const QFont &font){return font.pixelSize()>0?font.pixelSize():QFontInfo(font).pixelSize();}
inline QFont font(qreal sp,QFont base){
#ifdef Q_OS_ANDROID
    const auto *screen=QGuiApplication::primaryScreen();
    base.setPointSizeF(logicalPixels(sp)*72./(screen?screen->logicalDotsPerInchY():72.));
#else
    base.setPixelSize(pixels(sp));
#endif
    return base;
}
inline void setNominal(QWidget *widget,qreal sp){
    widget->setProperty("androidNominalSp",sp);
#ifdef Q_OS_ANDROID
    widget->setFont(font(sp,widget->font()));
#else
    Q_UNUSED(sp);
#endif
}
inline qreal nominal(const QWidget *widget){const auto value=widget->property("androidNominalSp");return value.isValid()?value.toDouble():nominalFromPixels(fontPixels(widget->font()));}
inline QFont enlarged(const QWidget *widget,qreal factor){
#ifdef Q_OS_ANDROID
    return font(nominal(widget)*factor,widget->font());
#else
    QFont f=widget->font();if(f.pointSizeF()>0)f.setPointSizeF(f.pointSizeF()*factor);else f.setPixelSize(qRound(fontPixels(f)*factor));return f;
#endif
}
// Reuse the same nominal sizes for a preference or native configuration change.
// No widget/editor is reconstructed, so drafts, selection and workspaces survive.
inline void refresh(QApplication &application){
    sizes().clear();
    const auto nominal=application.property("androidApplicationNominalSp");
    if(!nominal.isValid())return; // Desktop/host behavior is unchanged by default.
    application.setFont(font(nominal.toDouble(),application.font()));
    QList<QPointer<QWidget>> widgets;
    for(auto *widget:QApplication::allWidgets())widgets<<widget;
    for(const auto &widget:widgets){
        if(!widget)continue;
        if(!widget->property("androidFontDerived").toBool()&&widget->property("androidNominalSp").isValid())widget->setFont(font(widget->property("androidNominalSp").toDouble(),widget->font()));
        widget->updateGeometry();
        if(widget->objectName()=="calculatorPanel"){QEvent changed(QEvent::FontChange);QApplication::sendEvent(widget,&changed);}
    }
}
inline void setSizeMode(const QString &value){
    const auto mode=normalizeSizeMode(value);
    if(!qApp||sizeMode()==mode)return;
    qApp->setProperty("androidFontSizeMode",mode);
    refresh(*qApp);
}
inline void initialize(QApplication &application){
#ifdef Q_OS_ANDROID
    // Resolve the actual native theme's nominal SP without Qt's persisted
    // density-keyed style cache. The inverse is only a fallback for a theme
    // that supplies no textAppearance size; no fontScale arithmetic is used.
    const QFont initial=application.font();
    const qreal defaultSp=QJniObject::callStaticMethod<jfloat>("io/github/davoda1/emcicalc/FontMetricsBridge","defaultSp","()F");
    const qreal nominalSp=defaultSp>0?defaultSp:nominalFromPixels(fontPixels(initial));
    application.setProperty("androidApplicationNominalSp",nominalSp);
    application.setFont(font(nominalSp,initial));
    auto *timer=new QTimer(&application);timer->setInterval(250);
    auto refreshConfiguration=[&application,lastGeneration=jlong(-1),lastRatio=qreal(-1)]() mutable {
        if(application.applicationState()!=Qt::ApplicationActive)return;
        const auto generation=QJniObject::callStaticMethod<jlong>("io/github/davoda1/emcicalc/FontMetricsBridge","generation","()J");
        const qreal ratio=deviceRatio();if(generation==lastGeneration&&qFuzzyCompare(ratio,lastRatio))return;
        lastGeneration=generation;lastRatio=ratio;
        AndroidFont::refresh(application);
    };
    QObject::connect(timer,&QTimer::timeout,&application,refreshConfiguration);
    QObject::connect(&application,&QGuiApplication::applicationStateChanged,timer,[timer](Qt::ApplicationState state){if(state==Qt::ApplicationActive)timer->start();else timer->stop();});
    timer->start();
#else
    Q_UNUSED(application);
#endif
}
}
