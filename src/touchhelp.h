// SPDX-License-Identifier: MIT
#pragma once
#include <QtWidgets>
#include "androidfont.h"
#include "keyartwork.h"

// Persistent readers survive configuration changes. Regenerate embedded key
// images with their text instead of retaining their opening size or colour.
class TouchHelpBrowser final : public QTextBrowser {
public:
    using QTextBrowser::QTextBrowser;
    void setHelpHtml(const QString &html){source_=html;refreshHelp();}
protected:
    void changeEvent(QEvent *event) override {
        QTextBrowser::changeEvent(event);
        if(event->type()==QEvent::FontChange || event->type()==QEvent::ApplicationFontChange ||
           event->type()==QEvent::PaletteChange || event->type()==QEvent::ApplicationPaletteChange)refreshHelp();
    }
    bool event(QEvent *event) override {
        const bool result=QTextBrowser::event(event);
        if(event->type()==QEvent::DevicePixelRatioChange)refreshHelp();
        return result;
    }
private:
    QString source_;
    void refreshHelp() {
        if(source_.isEmpty())return;
        const int scroll=verticalScrollBar()->value(),position=textCursor().position(),anchor=textCursor().anchor();
        QString html=source_;const QColor ink=palette().color(QPalette::Text);
        html.replace(QRegularExpression("(?<![-\\w])color\\s*:\\s*#[0-9a-fA-F]{3,8}"),"color:"+ink.name());
        setHtml(KeyArtwork::helpHtml(html,qCeil(AndroidFont::fontPixels(font())),devicePixelRatioF(),ink));
        QTextCursor cursor(document());cursor.setPosition(qMin(anchor,document()->characterCount()-1));
        cursor.setPosition(qMin(position,document()->characterCount()-1),QTextCursor::KeepAnchor);setTextCursor(cursor);
        verticalScrollBar()->setValue(scroll);
    }
};

// A held key explains itself in the calculator's display, above the fingers.
// This is a child surface, never a popup or dialog: it cannot steal the touch
// sequence, mouse grab, keyboard focus, or a modifier's second contact.
inline QFrame *showHeldTouchHelp(QWidget *owner, QFrame *existing, const QRect &area,
                                 const QString &html, const QString &description,
                                 const QColor &background, const QColor &foreground,
                                 const QColor &border, const QString &name = QStringLiteral("heldKeyHelp"),
                                 const QRect &heldKey = {}) {
    auto *frame=existing;
    if(!frame) {
        frame=new QFrame(owner);frame->setObjectName(name);
        frame->setAttribute(Qt::WA_TransparentForMouseEvents);
        frame->setFocusPolicy(Qt::NoFocus);
        auto *layout=new QVBoxLayout(frame);layout->setContentsMargins(10,6,10,6);
        auto *body=new QLabel(frame);body->setObjectName(name+"Body");
        body->setTextFormat(Qt::RichText);body->setWordWrap(true);
        body->setTextInteractionFlags(Qt::NoTextInteraction);
        body->setAttribute(Qt::WA_TransparentForMouseEvents);
        body->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        body->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
        layout->addWidget(body);
    }
    frame->setStyleSheet(QString("QFrame#%4 {background:%1;border:1px solid %3;border-radius:6px;} QLabel#%4Body {background:transparent;color:%2;border:0;padding:0;}")
                        .arg(background.name(),foreground.name(),border.name(),name));
    auto *body=frame->findChild<QLabel*>(name+"Body");
    frame->setProperty("sourceHelpHtml",html);frame->setProperty("helpArea",area);
    body->setAccessibleDescription(description);
    const QFont originalFont=body->font();
    const int preferredPixels=AndroidFont::pixels(qBound(13,area.width()/25,16));
    struct HelpLayout {QRect area;int width=0,height=0,pixels=0;QString html;bool complete=false;};
    const auto measure=[&](const QRect &available,const QString &content) {
        HelpLayout result;result.area=available;if(available.width()<48 || available.height()<32)return result;
        QFont font=originalFont;int pixels=preferredPixels;
        QTextDocument document;document.setDocumentMargin(0);
        QTextOption option=document.defaultTextOption();option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        document.setDefaultTextOption(option);
        font.setPixelSize(pixels);document.setDefaultFont(font);
        document.setHtml(KeyArtwork::helpHtml(content,pixels,owner->devicePixelRatioF()));
        int width=qMin(available.width(),qBound(220,qCeil(document.idealWidth())+22,440));
        for(;;) {
            font.setPixelSize(pixels);document.setDefaultFont(font);
            result.html=KeyArtwork::helpHtml(content,pixels,owner->devicePixelRatioF());document.setHtml(result.html);
            document.setTextWidth(qMax(1,width-22));
            result.complete=document.size().height()<=available.height()-14 && document.size().width()<=width-22;
            if(result.complete)break;
            if(width<available.width()){width=available.width();continue;}
            if(pixels<=11 || AndroidFont::pixels(13)>13)break;
            --pixels;
        }
        result.width=width;result.height=qMin(available.height(),qCeil(document.size().height())+14);result.pixels=pixels;return result;
    };
    HelpLayout layout=measure(area,html);
    if(!layout.complete && !heldKey.isEmpty()) {
        // Keep ordinary help in the display. Enlarged help may need more room;
        // measure spaces around the held key instead of clipping rows or
        // shrinking the user's font. The key/finger and touch sequence stay clear.
        const QRect bounds=owner->rect().adjusted(6,6,-6,-6),excluded=heldKey.adjusted(-8,-8,8,8);
        const QList<QRect> alternatives={
            QRect(bounds.left(),bounds.top(),bounds.width(),qMax(0,excluded.top()-bounds.top())),
            QRect(bounds.left(),excluded.bottom()+1,bounds.width(),qMax(0,bounds.bottom()-excluded.bottom())),
            QRect(bounds.left(),bounds.top(),qMax(0,excluded.left()-bounds.left()),bounds.height()),
            QRect(excluded.right()+1,bounds.top(),qMax(0,bounds.right()-excluded.right()),bounds.height())};
        for(const QRect &candidate:alternatives){const HelpLayout measured=measure(candidate,html);if(measured.complete){layout=measured;break;}}
        if(!layout.complete) {
            // A short landscape screen leaves tall spaces beside the finger.
            // Let each complete coloured row wrap across that space, retaining
            // every caption, icon and explanation at the same font size.
            const QRegularExpression row("<tr><td valign=\"top\" style=\"color:([^\"]+)\"><b>(.*?)</b></td><td valign=\"top\">(.*?)</td></tr>",QRegularExpression::DotMatchesEverythingOption);
            auto matches=row.globalMatch(html);QString rows;
            while(matches.hasNext()){const auto match=matches.next();rows+=QString("<tr><td valign=\"top\"><span style=\"color:%1\"><b>%2</b></span> &nbsp; %3</td></tr>").arg(match.captured(1),match.captured(2),match.captured(3));}
            if(!rows.isEmpty()) {
                const QString wrapped="<html><table cellspacing=\"3\" cellpadding=\"3\">"+rows+"</table></html>";
                for(const QRect &candidate:alternatives){const HelpLayout measured=measure(candidate,wrapped);if(measured.complete){layout=measured;break;}}
            }
        }
    }
    QFont font=originalFont;font.setPixelSize(layout.pixels);body->setFont(font);body->setText(layout.html);
    frame->setProperty("helpContentComplete",layout.complete);
    frame->setGeometry(layout.area.x()+(layout.area.width()-layout.width)/2,layout.area.y()+(layout.area.height()-layout.height)/2,layout.width,layout.height);
    frame->show();frame->raise();return frame;
}

// Touch help remains on screen until explicitly dismissed. It is independent
// of a hover timer, and has a bounded scrollable body and a large close action.
inline void showTouchHelp(QWidget *owner, const QString &title, const QString &text, const QString &closeText) {
    if(text.isEmpty())return;
    auto *dialog=new QDialog(owner->window());
    dialog->setObjectName("touchHelpDialog");
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(title);
    dialog->setWindowModality(Qt::WindowModal);
    auto *layout=new QVBoxLayout(dialog);
    auto *heading=new QLabel(title,dialog);heading->setWordWrap(true);
    QFont headingFont=heading->font();headingFont.setBold(true);heading->setFont(headingFont);
    layout->addWidget(heading);
    auto *body=new TouchHelpBrowser(dialog);body->setObjectName("touchHelpBody");
    // Hover tooltip accents were chosen for a tooltip background. Help is a
    // real reader surface; use its text colour so SHIFT/ALPHA stay readable.
    body->setOpenLinks(false);body->setHelpHtml(text);body->setMinimumSize(0,0);
    layout->addWidget(body,1);
    auto *close=new QPushButton(closeText,dialog);close->setObjectName("touchHelpClose");
    close->setMinimumHeight(48);layout->addWidget(close);
    QObject::connect(close,&QPushButton::clicked,dialog,&QDialog::accept);
    const QSize available=owner->window()->size();
    dialog->resize(qMax(1,qMin(480,available.width()-24)),qMax(1,qMin(430,available.height()-48)));
    QPointer<QWidget> previousFocus=QApplication::focusWidget();
    QObject::connect(dialog,&QDialog::finished,owner,[previousFocus]{if(previousFocus)previousFocus->setFocus(Qt::OtherFocusReason);});
    dialog->show();
}
