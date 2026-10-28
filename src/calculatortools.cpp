// SPDX-License-Identifier: MIT
#include "calculatortools.h"
#include "androidfont.h"
#include <QScroller>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QJsonArray>
#include "financepanel.h"
#include "recurringlabel.h"
#include "matheditor.h"
#include "mathcaption.h"
#include <QComboBox>
#include <QApplication>
#include <QClipboard>
#include <QCompleter>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QGridLayout>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QStringListModel>
#include <QWheelEvent>
#include <algorithm>

namespace {
// Keep the catalog a real accessible item view while giving names, symbols and
// category paths distinct visual roles. No per-row widgets or fixed colours.
class CatalogDelegate final : public QStyledItemDelegate {
public:
    explicit CatalogDelegate(QObject *parent) : QStyledItemDelegate(parent) {}
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const override {
        return QSize(120, QFontMetrics(option.font).height() * 2 + 22);
    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        QStyleOptionViewItem opt(option); initStyleOption(&opt, index);
        opt.text.clear();
        const auto *widget = opt.widget;
        (widget ? widget->style() : QApplication::style())->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
        const bool selected = opt.state & QStyle::State_Selected;
        const QColor foreground = opt.palette.color(selected ? QPalette::HighlightedText : QPalette::Text);
        const QRect content = opt.rect.adjusted(10, 7, -10, -7);
        QFont nameFont = opt.font; nameFont.setBold(true);
        const QFontMetrics nameMetrics(nameFont), detailMetrics(opt.font);
        painter->save(); painter->setClipRect(opt.rect); painter->setPen(foreground); painter->setFont(nameFont);
        painter->drawText(content, Qt::AlignLeft | Qt::AlignTop,
                          nameMetrics.elidedText(index.data(Qt::UserRole + 2).toString(), Qt::ElideRight, content.width()));
        painter->setFont(opt.font);
        const QString token = index.data(Qt::UserRole + 4).toString();
        const QString action=index.data(Qt::UserRole).toString();
        const bool mathematical=index.data(Qt::UserRole+5).toBool()&&MathCaption::supports(action);
        const int tokenWidth = std::min(content.width(), (mathematical?qCeil(MathCaption::size(action,opt.font).width()):detailMetrics.horizontalAdvance(token)) + 12);
        const QRect chip(content.left(), content.bottom() - detailMetrics.height() - 3, tokenWidth, detailMetrics.height() + 3);
        QColor fill = foreground; fill.setAlpha(selected ? 32 : 20);
        painter->setBrush(fill); painter->setPen(Qt::NoPen); painter->drawRoundedRect(chip, 4, 4);
        painter->setPen(foreground);
        if(mathematical)MathCaption::paint(*painter,chip.adjusted(6,0,-6,0),opt.font,action,foreground,Qt::AlignLeft);
        else painter->drawText(chip.adjusted(6, 0, -6, 0), Qt::AlignVCenter | Qt::AlignLeft,
                               detailMetrics.elidedText(token, Qt::ElideRight, tokenWidth - 12));
        const QRect category(chip.right() + 9, chip.top(), std::max(0, content.right() - chip.right() - 9), chip.height());
        QColor muted = foreground; muted.setAlpha(selected ? 230 : 180); painter->setPen(muted);
        const QString path = index.data(Qt::UserRole + 3).toString().replace('/', QStringLiteral(" › "));
        painter->drawText(category, Qt::AlignVCenter | Qt::AlignLeft,
                          detailMetrics.elidedText(path, Qt::ElideLeft, category.width()));
        painter->restore();
    }
};
}

CalculatorTools::CalculatorTools(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 8);
    title_ = new QLabel;
    QFont heading = title_->font(); heading.setBold(true); heading.setPointSizeF(heading.pointSizeF() + 2);
    title_->setFont(heading); AndroidFont::setNominal(title_,AndroidFont::nominal(this)+2); title_->setWordWrap(true); layout->addWidget(title_);
    statisticsMode_ = new QComboBox; statisticsMode_->setObjectName("statisticsMode");
    statisticsMode_->setMinimumWidth(0);statisticsMode_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    statisticsMode_->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);statisticsMode_->hide();layout->addWidget(statisticsMode_);
    pages_ = new QStackedWidget; layout->addWidget(pages_, 1);
    auto *browser = new QWidget;
    auto *browserLayout = new QVBoxLayout(browser); browserLayout->setContentsMargins(0, 0, 0, 0);
    auto *tabs = new QHBoxLayout;
    constants_ = new QPushButton; units_ = new QPushButton;functions_=new QPushButton;functions_->setObjectName("catalogFunctions");
    constants_->setCheckable(true); units_->setCheckable(true);functions_->setCheckable(true);
    tabs->addWidget(constants_); tabs->addWidget(units_);tabs->addWidget(functions_); browserLayout->addLayout(tabs);
    search_ = new QLineEdit; search_->setClearButtonEnabled(true); search_->setObjectName("catalogSearch");
    categories_ = new QComboBox; categories_->setObjectName("catalogCategory"); categories_->setMinimumWidth(0);
    categories_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    categories_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    categories_->installEventFilter(this);
    browserLayout->addWidget(search_); browserLayout->addWidget(categories_);
    auto*personalRow=new QHBoxLayout;browserMode_=new QComboBox;browserMode_->setObjectName("catalogPersonalFilter");browserMode_->setMinimumWidth(0);browserMode_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);browserMode_->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);browserMode_->setMinimumHeight(44);personalRow->addWidget(browserMode_,1);favorite_=new QPushButton("☆");favorite_->setObjectName("catalogFavorite");favorite_->setMinimumSize(44,44);favorite_->setMaximumWidth(56);favorite_->setEnabled(false);personalRow->addWidget(favorite_);example_=new QPushButton;example_->setObjectName("catalogExample");example_->setMinimumHeight(44);example_->setMinimumWidth(60);example_->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);example_->setEnabled(false);personalRow->addWidget(example_);browserLayout->addLayout(personalRow);
    list_ = new QListWidget; list_->setObjectName("definitionCatalog");
    list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); list_->setWordWrap(false);
    list_->setSpacing(2); list_->setTextElideMode(Qt::ElideRight);
    list_->setItemDelegate(new CatalogDelegate(list_));
    browserLayout->addWidget(list_, 1);
    catalogStatus_ = new QLabel; catalogStatus_->setWordWrap(true); browserLayout->addWidget(catalogStatus_);
    insert_ = new QPushButton; insert_->setEnabled(false); browserLayout->addWidget(insert_);
    pages_->addWidget(browser);

    auto *scroll = new QScrollArea; scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    scroll->setObjectName("unitConverterScroll"); scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->viewport()->installEventFilter(this);
    auto *converter = new QWidget; auto *form = new QVBoxLayout(converter); form->setContentsMargins(0, 0, 0, 0); form->setSpacing(12);
    converterNote_ = new QLabel; converterNote_->setWordWrap(true); converterNote_->setContentsMargins(2, 0, 2, 0); form->addWidget(converterNote_);
    auto *inputCard = new QFrame; inputCard->setObjectName("converterInputCard"); inputCard->setFrameShape(QFrame::StyledPanel);
    auto *inputForm = new QVBoxLayout(inputCard); inputForm->setContentsMargins(10, 9, 10, 10); inputForm->setSpacing(6); form->addWidget(inputCard);
    valueLabel_ = new QLabel; amount_ = new MathEditor; amount_->setObjectName("conversionValue");
    amount_->setMinimumHeight(54); amount_->setMaximumHeight(76); valueLabel_->setBuddy(amount_);
    inputForm->addWidget(valueLabel_); inputForm->addWidget(amount_);
    fromLabel_ = new QLabel; toLabel_ = new QLabel;
    from_ = new QComboBox; to_ = new QComboBox;
    unitCompletions_ = new QStringListModel(this);
    for (auto *box : {from_, to_}) {
        box->setEditable(true); box->setInsertPolicy(QComboBox::NoInsert); box->setMinimumWidth(0);
        box->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        box->completer()->setFilterMode(Qt::MatchContains);
        box->completer()->setCaseSensitivity(Qt::CaseInsensitive);
        box->installEventFilter(this); box->lineEdit()->installEventFilter(this);
        connect(box->completer(), qOverload<const QString &>(&QCompleter::activated), this, [this, box](const QString &text) {
            const QString token = completionTokens_.value(text);
            if (!token.isEmpty()) box->setCurrentIndex(box->findData(token));
        });
    }
    from_->setObjectName("conversionFrom"); to_->setObjectName("conversionTo");
    fromLabel_->setBuddy(from_); toLabel_->setBuddy(to_);
    auto *fromHeader = new QHBoxLayout; fromHeader->addWidget(fromLabel_); fromHeader->addStretch();
    swap_ = new QPushButton(QStringLiteral("⇅")); swap_->setObjectName("swapConversionUnits"); swap_->setMaximumWidth(44);
    fromHeader->addWidget(swap_); inputForm->addLayout(fromHeader); inputForm->addWidget(from_);
    inputForm->addWidget(toLabel_); inputForm->addWidget(to_);
    convert_ = new QPushButton; convert_->setObjectName("convertUnits"); inputForm->addSpacing(4); inputForm->addWidget(convert_);
    auto *outputCard = new QFrame; outputCard->setObjectName("converterOutputCard"); outputCard->setFrameShape(QFrame::StyledPanel);
    auto *outputForm = new QVBoxLayout(outputCard); outputForm->setContentsMargins(10, 9, 10, 10); outputForm->setSpacing(6); form->addWidget(outputCard);
    resultLabel_ = new QLabel; result_ = new MathEditor; result_->setReadOnly(true); result_->setResultPresentation(true); result_->setObjectName("conversionResult");
    resultLabel_->setWordWrap(true); resultLabel_->setMinimumWidth(0); resultLabel_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    result_->setMinimumHeight(64); result_->setMaximumHeight(112);
    for (auto *label : {valueLabel_, resultLabel_}) { QFont font = label->font(); font.setBold(true); label->setFont(font); }
    auto *resultHeader = new QHBoxLayout; resultHeader->addWidget(resultLabel_, 1);
    conversionMode_ = new QPushButton(QStringLiteral("S⇔D")); conversionMode_->setObjectName("toggleConversionExact");
    conversionMode_->setCheckable(true); resultHeader->addWidget(conversionMode_); outputForm->addLayout(resultHeader);
    auto *display = new QFrame; display->setObjectName("unitConversionDisplay"); display->setFrameShape(QFrame::StyledPanel);
    display->setBackgroundRole(QPalette::Base); display->setAutoFillBackground(true);
    auto *displayLayout = new QVBoxLayout(display); displayLayout->setContentsMargins(8, 7, 8, 6); displayLayout->setSpacing(3);
    auto *indicators = new QHBoxLayout; indicators->setSpacing(8);
    conversionModeIndicator_ = new QLabel; conversionModeIndicator_->setObjectName("unitConversionModeIndicator");
    conversionBaseIndicator_ = new QLabel(QStringLiteral("DEC")); conversionBaseIndicator_->setObjectName("unitConversionBaseIndicator");
    conversionBaseIndicator_->setFrameShape(QFrame::Box); conversionBaseIndicator_->setAlignment(Qt::AlignCenter);
    conversionBaseIndicator_->setContentsMargins(4, 1, 4, 1);
    for (auto *indicator : {conversionModeIndicator_, conversionBaseIndicator_}) {
        indicator->setForegroundRole(QPalette::Text); QFont font = indicator->font(); font.setBold(true); font.setPixelSize(11); indicator->setFont(font); AndroidFont::setNominal(indicator,11);
    }
    indicators->addWidget(conversionModeIndicator_); indicators->addStretch(); indicators->addWidget(conversionBaseIndicator_);
    displayLayout->addLayout(indicators); displayLayout->addWidget(result_); outputForm->addWidget(display);
    conversionStatus_ = new RecurringLabel; conversionStatus_->setWordWrap(true); conversionStatus_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    conversionStatus_->setObjectName("unitConversionStatus"); conversionStatus_->setForegroundRole(QPalette::Text); displayLayout->addWidget(conversionStatus_);
    insertResult_ = new QPushButton; insertResult_->setObjectName("insertConversionResult"); insertResult_->setEnabled(false);
    copyResult_ = new QPushButton; copyResult_->setObjectName("copyConversionResult"); copyResult_->setEnabled(false);
    outputForm->addWidget(copyResult_); outputForm->addWidget(insertResult_); form->addStretch();
    scroll->setWidget(converter); pages_->addWidget(scroll);

    auto *matrixPage = new QWidget;
    auto *matrixForm = new QVBoxLayout(matrixPage); matrixForm->setContentsMargins(0,0,0,0);
    matrixNote_ = new QLabel; matrixNote_->setWordWrap(true); matrixForm->addWidget(matrixNote_);
    auto *dimensions = new QGridLayout;
    matrixRowsLabel_ = new QLabel; matrixColumnsLabel_ = new QLabel;
    matrixRows_ = new QSpinBox; matrixRows_->setObjectName("matrixRows");
    matrixColumns_ = new QSpinBox; matrixColumns_->setObjectName("matrixColumns");
    for(auto *box:{matrixRows_,matrixColumns_}){box->setRange(1,32);box->setValue(2);box->setMinimumWidth(0);box->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);box->installEventFilter(this);}
    matrixRowsLabel_->setBuddy(matrixRows_); matrixColumnsLabel_->setBuddy(matrixColumns_);
    dimensions->addWidget(matrixRowsLabel_,0,0); dimensions->addWidget(matrixRows_,0,1);
    dimensions->addWidget(matrixColumnsLabel_,1,0); dimensions->addWidget(matrixColumns_,1,1);
    matrixForm->addLayout(dimensions);
    insertMatrix_ = new QPushButton; insertMatrix_->setObjectName("insertMatrix"); matrixForm->addWidget(insertMatrix_);
    connect(insertMatrix_, &QPushButton::clicked, this, [this]{if(onInsertMatrix)onInsertMatrix(matrixRows_->value(),matrixColumns_->value());});
    for(const auto &item:QList<QPair<const char*,QString>>{{"Determinant","det("},{"Inverse matrix","inverse("},{"Transpose","transpose("}}){
        auto *button=new QPushButton;button->setObjectName("matrixFunction_"+item.second.chopped(1));
        button->setProperty("translationKey",item.first);button->setProperty("insertToken",item.second);
        matrixFunctions_<<button;matrixForm->addWidget(button);
        connect(button,&QPushButton::clicked,this,[this,token=item.second]{if(onInsert)onInsert(token);});
    }
    matrixForm->addStretch();
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    auto *matrixScroll=new QScrollArea;matrixScroll->setWidgetResizable(true);matrixScroll->setFrameShape(QFrame::NoFrame);matrixScroll->setWidget(matrixPage);
    QScroller::grabGesture(matrixScroll->viewport(),QScroller::TouchGesture);pages_->addWidget(matrixScroll);
    QScroller::grabGesture(scroll->viewport(),QScroller::TouchGesture);
    QScroller::grabGesture(list_->viewport(),QScroller::TouchGesture);
#else
    pages_->addWidget(matrixPage);
#endif
    finance_=new FinancePanel;pages_->addWidget(finance_);
    finance_->onInsert=[this](QString text){if(onInsert)onInsert(text);};
    connect(statisticsMode_,&QComboBox::currentIndexChanged,this,[this](int index){if(view_==View::Statistics)pages_->setCurrentIndex(index==1?3:0);});

    connect(constants_, &QPushButton::clicked, this, [this]{ setView(View::Constants); });
    connect(units_, &QPushButton::clicked, this, [this]{ setView(View::Units); });
    connect(functions_,&QPushButton::clicked,this,[this]{setView(View::Functions);});
    connect(browserMode_,&QComboBox::currentIndexChanged,this,[this]{filter();});
    connect(favorite_,&QPushButton::clicked,this,[this]{toggleFavorite();});
    connect(example_,&QPushButton::clicked,this,[this]{showExample();});
    connect(search_, &QLineEdit::textChanged, this, [this]{ filter(); });
    connect(categories_, &QComboBox::currentIndexChanged, this, [this]{ filter(); });
    connect(list_, &QListWidget::currentRowChanged, this, [this](int row){insert_->setEnabled(row>=0);favorite_->setEnabled(row>=0);const bool saved=favorites_.contains(selectedBrowserKey());favorite_->setText(saved?QStringLiteral("★"):QStringLiteral("☆"));favorite_->setToolTip(trc(saved?"Remove favorite":"Add favorite"));favorite_->setAccessibleName(favorite_->toolTip());auto*i=list_->currentItem();example_->setEnabled(i&&!offlineExample(i->data(Qt::UserRole).toString(),i->data(Qt::UserRole+1).toBool(),i->data(Qt::UserRole+5).toBool()).isEmpty());});
    connect(list_, &QListWidget::itemActivated, this, [this]{ insertSelected(); });
    connect(insert_, &QPushButton::clicked, this, [this]{ insertSelected(); });
    connect(convert_, &QPushButton::clicked, this, [this]{ requestConversion(); });
    connect(conversionMode_, &QPushButton::clicked, this, [this]{
        setConversionExact(!conversionExact_);
        if (onConversionExactChanged) onConversionExactChanged(conversionExact_);
    });
    connect(from_, &QComboBox::currentTextChanged, this, [this]{ invalidateConversion(); });
    connect(to_, &QComboBox::currentTextChanged, this, [this]{ invalidateConversion(); });
    connect(swap_, &QPushButton::clicked, this, [this]{ swapUnits(); });
    connect(copyResult_, &QPushButton::clicked, this, [this]{ if (!serializedResult_.isEmpty()) QApplication::clipboard()->setText(serializedResult_); });
    amount_->onTextChanged = [this]{ invalidateConversion(); };
    amount_->onReturnPressed = [this]{ requestConversion(); };
    connect(from_->lineEdit(), &QLineEdit::returnPressed, this, [this]{ requestConversion(); });
    connect(to_->lineEdit(), &QLineEdit::returnPressed, this, [this]{ requestConversion(); });
    connect(insertResult_, &QPushButton::clicked, this, [this]{ if (!serializedResult_.isEmpty() && onInsert) onInsert(QStringLiteral("(") + serializedResult_ + QLatin1Char(')')); });
    retranslate({});
}

QString CalculatorTools::trc(const char *text) const { return translate_ ? translate_(text) : QString::fromUtf8(text); }

void CalculatorTools::retranslate(std::function<QString(const char *)> translate) {
    translate_ = std::move(translate);
    updateTitle();
    {const QSignalBlocker blocker(statisticsMode_);const int selected=statisticsMode_->currentIndex();statisticsMode_->clear();statisticsMode_->addItems({trc("Functions"),trc("Finance worksheets")});statisticsMode_->setCurrentIndex(std::max(0,selected));}
    statisticsMode_->setAccessibleName(trc("Statistics tools"));finance_->retranslate(translate_);
    constants_->setText(trc("Constants")); units_->setText(trc("Units"));functions_->setText(trc("Functions"));
    {QSignalBlocker blocker(browserMode_);const int index=browserMode_->currentIndex();browserMode_->clear();browserMode_->addItems({trc("All items"),trc("Favorites"),trc("Recent"),trc("Offline examples")});browserMode_->setCurrentIndex(std::max(0,index));}browserMode_->setAccessibleName(trc("Catalog filter"));example_->setText(trc("Example"));example_->setToolTip(trc("Show an offline example"));
    search_->setPlaceholderText(trc("Search name, symbol, or category")); search_->setAccessibleName(view_==View::Statistics?trc("Search statistical functions"):trc("Search constants and units"));
    categories_->setAccessibleName(trc("Category")); list_->setAccessibleName(view_==View::Statistics?trc("Statistics"):trc("Constants and units"));
    insert_->setText(trc("Insert selected item"));
    converterNote_->setText(trc("Enter a value, choose or type units, then convert. Compound units and metric prefixes are supported."));
    valueLabel_->setText(trc("Value")); amount_->setAccessibleName(trc("Value to convert"));
    fromLabel_->setText(trc("From unit")); toLabel_->setText(trc("To unit"));
    from_->setAccessibleName(trc("From unit")); to_->setAccessibleName(trc("To unit"));
    from_->lineEdit()->setPlaceholderText(trc("Choose or type a unit")); to_->lineEdit()->setPlaceholderText(trc("Choose or type a unit"));
    swap_->setToolTip(trc("Swap units")); swap_->setAccessibleName(trc("Swap units"));
    convert_->setText(busy_ ? trc("Converting…") : trc("Convert"));
    resultLabel_->setText(trc("Converted result")); result_->setAccessibleName(trc("Converted result"));
    setConversionExact(conversionExact_);
    insertResult_->setText(trc("Insert result"));
    copyResult_->setText(trc("Copy result"));
    matrixNote_->setText(trc("Choose the matrix dimensions. New cells are empty and must be filled before evaluation."));
    matrixRowsLabel_->setText(trc("Rows")); matrixColumnsLabel_->setText(trc("Columns"));
    matrixRows_->setAccessibleName(trc("Rows")); matrixColumns_->setAccessibleName(trc("Columns"));
    insertMatrix_->setText(matrixEditing_?trc("Resize matrix"):trc("Insert matrix"));
    for(auto *button:matrixFunctions_)button->setText(trc(button->property("translationKey").toByteArray().constData()));
    rebuildCategories();
}

void CalculatorTools::setCatalog(QVector<CatalogEntry> entries) {
    entries_ = std::move(entries);
    catalogLoaded_ = true;
    QStringList unitTokens;
    QMap<QString,QString> unitLabels;
    QStringList completions; completionTokens_.clear();
    for (const auto &entry : entries_) if (entry.unit) {
        unitTokens.append(entry.token); unitTokens.append(entry.aliases);
        const QString displayedToken = entry.displayToken.isEmpty() ? entry.token : entry.displayToken;
        unitLabels.insert(entry.token, displayedToken);
        const QString preferred = displayedToken + QStringLiteral(" — ") + entry.name;
        completions.append(preferred); completionTokens_.insert(preferred, entry.token);
        for (const auto &alias : entry.aliases) {
            if (alias == entry.token || alias.contains('_')) continue;
            const QString completion = alias + QStringLiteral(" — ") + entry.name;
            if (!completionTokens_.contains(completion)) { completions.append(completion); completionTokens_.insert(completion, entry.token); }
        }
    }
    unitTokens.removeDuplicates(); unitCompletions_->setStringList(completions);
    amount_->setUnitTokens(unitTokens); result_->setUnitTokens(unitTokens);
    amount_->setUnitLabels(unitLabels); result_->setUnitLabels(unitLabels);
    amount_->setConstantNames(constantTokens()); result_->setConstantNames(constantTokens());
    for (auto *box : {from_, to_}) {
        const QSignalBlocker blocker(box);
        const QString prior = unitText(box);
        box->clear();
        for (const auto &entry : entries_) if (entry.unit)
            box->addItem((entry.displayToken.isEmpty() ? entry.token : entry.displayToken) + QStringLiteral(" — ") + entry.name, entry.token);
        box->setCurrentIndex(box->findData(prior));
        if (box->currentIndex() < 0) box->setEditText(prior);
        box->completer()->setModel(unitCompletions_);
    }
    rebuildCategories();
}

QStringList CalculatorTools::constantTokens() const {
    QStringList names;for(const auto &entry:entries_)if(!entry.unit&&!entry.function)names<<entry.token;return names;
}

void CalculatorTools::setMatrixDimensions(int rows,int columns){
    matrixEditing_=rows>0&&columns>0;
    matrixRows_->setValue(matrixEditing_?rows:2);matrixColumns_->setValue(matrixEditing_?columns:2);
    insertMatrix_->setText(matrixEditing_?trc("Resize matrix"):trc("Insert matrix"));
}

void CalculatorTools::updateTitle(){
    title_->setText(view_==View::Converter?trc("Unit converter"):view_==View::Statistics?trc("Statistics"):view_==View::Matrix?trc("Matrices"):view_==View::Functions?trc("Functions"):trc("Constants and units"));
}

void CalculatorTools::setHeaderVisible(bool visible) { title_->setVisible(visible); }

void CalculatorTools::setView(View view) {
    view_ = view;
    pages_->setCurrentIndex(view == View::Converter ? 1 : view==View::Matrix?2:view==View::Statistics&&statisticsMode_->currentIndex()==1?3:0);
    statisticsMode_->setVisible(view==View::Statistics);
    updateTitle();
    constants_->setVisible(view!=View::Statistics);units_->setVisible(view!=View::Statistics);functions_->setVisible(view!=View::Statistics);
    search_->setAccessibleName(view==View::Statistics?trc("Search statistical functions"):trc("Search constants and units"));
    list_->setAccessibleName(view==View::Statistics?trc("Statistics"):trc("Constants and units"));
    search_->clear();
    constants_->setChecked(view == View::Constants); units_->setChecked(view == View::Units);functions_->setChecked(view==View::Functions);
    rebuildCategories();
}

void CalculatorTools::setFinanceVisible(bool visible) {
    statisticsMode_->setCurrentIndex(visible?1:0);
    if(view_!=View::Statistics)setView(View::Statistics);
}

void CalculatorTools::rebuildCategories() {
    const QString previous = categories_->currentData().toString();
    const QSignalBlocker blocker(categories_);
    categories_->clear(); categories_->addItem(trc("All categories"), QString());
    QStringList categories;
    const auto entries = view_==View::Statistics?statisticsCatalog():entries_;
    for (const auto &entry : entries) if (matchesView(entry) && !entry.category.isEmpty() && !categories.contains(entry.category)) categories.append(entry.category);
    categories.sort(Qt::CaseInsensitive);
    for (const auto &category : categories) categories_->addItem(QString(category).replace('/', QStringLiteral(" › ")), category);
    const int index = categories_->findData(previous); categories_->setCurrentIndex(std::max(0, index));
    filter();
}

void CalculatorTools::filter() {
    list_->clear();
    const QString search = search_->text().trimmed();
    const QString category = categories_->currentData().toString();
    auto entries = view_==View::Statistics?statisticsCatalog():entries_;
    if(browserMode_->currentIndex()==2)std::stable_sort(entries.begin(),entries.end(),[this](const CatalogEntry&a,const CatalogEntry&b){auto key=[this](const CatalogEntry&e){return(e.unit?QString("u:"):(e.function||view_==View::Statistics)?QString("f:"):QString("c:"))+e.token;};return recent_.indexOf(key(a))<recent_.indexOf(key(b));});
    for (int index = 0; index < entries.size(); ++index) {
        const auto &entry = entries.at(index);
        if (!matchesView(entry) || (!category.isEmpty() && category != entry.category)) continue;
        const QString key=(entry.unit?QStringLiteral("u:"):(entry.function||view_==View::Statistics)?QStringLiteral("f:"):QStringLiteral("c:"))+entry.token;
        if(browserMode_->currentIndex()==1&&!favorites_.contains(key))continue;
        if(browserMode_->currentIndex()==2&&!recent_.contains(key))continue;
        if(browserMode_->currentIndex()==3&&offlineExample(entry.token,entry.unit,entry.function||view_==View::Statistics).isEmpty())continue;
        if (!search.isEmpty() && !(entry.name + QLatin1Char(' ') + entry.token + QLatin1Char(' ') + entry.displayToken + QLatin1Char(' ') + entry.category + QLatin1Char(' ') + entry.aliases.join(' ')).contains(search, Qt::CaseInsensitive)) continue;
        const QString displayedToken = entry.displayToken.isEmpty() ? entry.token : entry.displayToken;
        auto *item = new QListWidgetItem(entry.name + QLatin1Char('\n') + displayedToken, list_);
        item->setData(Qt::UserRole, entry.token);
        item->setData(Qt::UserRole+1, entry.unit);
        item->setData(Qt::UserRole+2, (favorites_.contains(key)?QStringLiteral("★ "):QString())+entry.name);
        item->setData(Qt::UserRole+3, entry.category);
        item->setData(Qt::UserRole+4, displayedToken);item->setData(Qt::UserRole+5,entry.function||view_==View::Statistics);item->setData(Qt::UserRole+6,key);
        if((entry.function||view_==View::Statistics)&&MathCaption::supports(entry.token)) {
            item->setData(Qt::AccessibleDescriptionRole,entry.name + QLatin1Char('\n') + displayedToken + QLatin1Char('\n') + entry.category);
            item->setToolTip("<html>"+entry.name.toHtmlEscaped()+"<br>"+MathCaption::imageHtml(entry.token,displayedToken,palette().color(QPalette::Text),AndroidFont::pixels(14),devicePixelRatioF())+"<br>"+entry.category.toHtmlEscaped()+"</html>");
        } else item->setToolTip(entry.name + QLatin1Char('\n') + displayedToken + QLatin1Char('\n') + entry.category);
    }
    insert_->setEnabled(false);favorite_->setEnabled(false);example_->setEnabled(false);
    catalogStatus_->setText(view_!=View::Statistics&&!catalogLoaded_ ? trc("Loading definitions…") : entries.isEmpty() ? trc("Definitions are unavailable.") : list_->count() == 0 ? trc("No matching items") : view_==View::Statistics?trc("Select a function to insert empty arguments."):trc("Select an item to insert its symbol."));
}

void CalculatorTools::insertSelected() {
    const auto *item = list_->currentItem();
    if (!item || !onInsert) return;
    const QString token=(item->data(Qt::UserRole+1).toBool()?QStringLiteral(" "):QString())+item->data(Qt::UserRole).toString();const QString key=item->data(Qt::UserRole+6).toString();recent_.removeAll(key);recent_.prepend(key);while(recent_.size()>100)recent_.removeLast();if(onBrowserStateChanged)onBrowserStateChanged();onInsert(token);
}

QVector<CatalogEntry> CalculatorTools::statisticsCatalog() const {
    struct Function { const char *token,*name,*category; const char *signature=nullptr; };
    // Canonical names and signatures from the bundled libqalculate definitions.
    // No user data, coefficients or sample values are inserted by these tools.
    static const Function functions[]{
        {"number(","Number of samples","Descriptive statistics"},
        {"total(","Sum of values","Descriptive statistics"},
        {"min(","Minimum","Descriptive statistics"},
        {"max(","Maximum","Descriptive statistics"},
        {"range(","Range","Descriptive statistics"},
        {"median(","Median","Descriptive statistics"},
        {"mode(","Mode (most frequent)","Descriptive statistics"},
        {"percentile(","Percentile","Descriptive statistics"},
        {"quartile(","Quartile","Descriptive statistics"},
        {"decile(","Decile","Descriptive statistics"},
        {"iqr(","Interquartile range","Descriptive statistics"},
        {"mean(","Arithmetic mean","Means"},
        {"geomean(","Geometric mean","Means"},
        {"harmmean(","Harmonic mean","Means"},
        {"rms(","Quadratic mean (RMS)","Means"},
        {"weighmean(","Weighted mean","Means"},
        {"trimmean(","Trimmed mean","Means"},
        {"winsormean(","Winsorized mean","Means"},
        {"var(","Sample variance","Dispersion"},
        {"varp(","Population variance","Dispersion"},
        {"stdev(","Sample standard deviation","Dispersion"},
        {"stdevp(","Population standard deviation","Dispersion"},
        {"stderr(","Standard error","Dispersion"},
        {"meandev(","Mean deviation","Dispersion"},
        {"poolvar(","Pooled variance","Dispersion"},
        {"cov(","Covariance","Correlation"},
        {"pearson(","Pearson correlation","Correlation"},
        {"spearman(","Spearman correlation","Correlation"},
        {"linearfit(","Linear regression","Regression"},
        {"quadraticfit(","Quadratic regression","Regression"},
        {"cubicfit(","Cubic regression","Regression"},
        {"loanpayment(","Loan payment","Loans and borrowing","loanpayment(principal, periodic_rate, periods, beginning)"},
        {"taxadd(","Add tax","Tax, VAT & invoices","taxadd(net, rate)"},
        {"taxremove(","Remove included tax","Tax, VAT & invoices","taxremove(gross, rate)"},
        {"taxpart(","Extract included tax","Tax, VAT & invoices","taxpart(gross, rate)"},
        {"brackettax(","Progressive tax","Tax, VAT & invoices","brackettax(income, brackets, deduction, credit, refundable)"},
        {"grossup(","Net-to-gross solver","Tax, VAT & invoices","grossup(target, flat_rate, precision)"},
        {"grossupbrackets(","Net-to-gross solver (progressive)","Tax, VAT & invoices","grossupbrackets(target, brackets, precision)"},
        {"markup(","Markup (profit ÷ cost)","Pricing & business","markup(cost, price)"},
        {"margin(","Margin (profit ÷ selling price)","Pricing & business","margin(cost, price)"},
        {"pricefrommarkup(","Selling price from target markup","Pricing & business","pricefrommarkup(cost, markup)"},
        {"pricefrommargin(","Selling price from target margin","Pricing & business","pricefrommargin(cost, margin)"},
        {"breakeven(","Break-even & target profit","Pricing & business","breakeven(fixed_costs, unit_price, variable_cost)"},
        {"targetsales(","Target profit","Pricing & business","targetsales(fixed_costs, unit_price, variable_cost, target_profit)"},
        {"prorate(","Proration","Pricing & business","prorate(amount, portion, whole)"},
        {"allocate(","Monetary allocation","Pricing & business","allocate(amount, weights, precision)"},
        {"runway(","Constant-burn runway","Pricing & business","runway(cash, net_burn)"},
        {"npv(","Net present value (period-one first flow)","Investments, savings and economics","npv(rate, cash_flows)"},
        {"npv0(","Net present value (explicit time zero)","Investments, savings and economics","npv0(rate, cash_flows)"},
        {"xnpv(","Dated net present value (XNPV)","Investments, savings and economics","xnpv(rate, cash_flows, Julian_dates, basis)"},
        {"irr(","Internal rate of return (IRR)","Investments, savings and economics","irr(cash_flows)"},
        {"xirr(","Dated internal rate of return (XIRR)","Investments, savings and economics","xirr(cash_flows, Julian_dates, basis)"},
        {"datejd(","Date as Julian day","Investments, savings and economics","datejd(year, month, day)"},
        {"cagr(","Compound annual growth rate","Investments, savings and economics","cagr(start, end, years)"},
        {"realreturn(","Real return after inflation","Investments, savings and economics","realreturn(nominal, inflation)"},
        {"inflationadjust(","Inflation and purchasing power","Investments, savings and economics","inflationadjust(amount, rate, periods)"},
        {"fxnet(","Currency conversion after fees","Investments, savings and economics","fxnet(amount, destination_per_source, spread, source_fee, destination_fee)"}
    };
    QVector<CatalogEntry> entries;
    for(const auto &item:functions) {
        CatalogEntry entry;entry.token=QString::fromLatin1(item.token);entry.name=trc(item.name);entry.category=trc(item.category);entry.function=true;
        // Signatures and templates share the same worker-produced contract.
        // Historical display strings above are not an arity source.
        for(const auto &loaded:entries_)if(loaded.function&&(loaded.token==entry.token||loaded.metadata.name+"("==entry.token||loaded.metadata.aliases.contains(entry.token.chopped(1)))){entry.metadata=loaded.metadata;entry.aliases=loaded.aliases;break;}
        entry.displayToken=entry.metadata.signature.isEmpty()?entry.token:entry.metadata.signature;
        entries.append(entry);
    }
    return entries;
}

QString CalculatorTools::unitText(QComboBox *box) const {
    if (box->currentIndex() >= 0 && box->currentText() == box->itemText(box->currentIndex())) return box->currentData().toString();
    const QString text = box->currentText().trimmed();
    return completionTokens_.value(text, text);
}

bool CalculatorTools::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::Wheel) {
        auto *widget = qobject_cast<QWidget *>(watched);
        auto *wheel = static_cast<QWheelEvent *>(event);
        for (QWidget *parent = widget ? widget->parentWidget() : nullptr; parent; parent = parent->parentWidget()) {
            if (auto *area = qobject_cast<QScrollArea *>(parent)) {
                // Some Qt styles ignore pixel-only touchpad gestures on a
                // scrollbar. Apply their exact distance for both controls and
                // the page viewport, without editing the hovered selector.
                if (!wheel->pixelDelta().isNull()) {
                    auto *bar = area->verticalScrollBar();
                    bar->setValue(bar->value() - wheel->pixelDelta().y());
                    event->accept(); return true;
                }
                if (watched == area->viewport()) return false;
                QWheelEvent forwarded(area->viewport()->mapFromGlobal(wheel->globalPosition().toPoint()), wheel->globalPosition(),
                                      wheel->pixelDelta(), wheel->angleDelta(), wheel->buttons(), wheel->modifiers(),
                                      wheel->phase(), wheel->inverted(), wheel->source());
                QApplication::sendEvent(area->viewport(), &forwarded);
                break;
            }
        }
        event->accept(); return true;
    }
    return QWidget::eventFilter(watched, event);
}

void CalculatorTools::swapUnits() {
    if (busy_) return;
    const QString from = unitText(from_), to = unitText(to_);
    const QSignalBlocker sourceBlocker(from_), destinationBlocker(to_);
    auto select = [](QComboBox *box, const QString &token) {
        const int index = box->findData(token);
        box->setCurrentIndex(index);
        if (index < 0) box->setEditText(token);
    };
    select(from_, to); select(to_, from);
    invalidateConversion();
}

void CalculatorTools::requestConversion() {
    if (busy_) return;
    clearConversion();
    const QString value = amount_->text().trimmed(), source = unitText(from_), destination = unitText(to_);
    if (value.isEmpty() || source.isEmpty() || destination.isEmpty()) {
        conversionStatus_->setText(trc("Enter a value and both units."));
        if (value.isEmpty()) amount_->setFocus(); else if (source.isEmpty()) from_->setFocus(); else to_->setFocus();
        return;
    }
    if (!amount_->isComplete()) {
        conversionStatus_->setText(trc("Complete the value before converting.")); amount_->setFocus(); return;
    }
    if (onConvert) onConvert(value, source, destination);
}

void CalculatorTools::clearConversion() {
    serializedResult_.clear(); approximation_.clear();approximateCopyValue_.clear(); approximationIsApproximate_ = false;approximateRepeatStart_=-1;approximateRepeatLength_=0;static_cast<RecurringLabel*>(conversionStatus_)->setRepeatingRange();static_cast<RecurringLabel*>(conversionStatus_)->setExactCopyValue({});
    result_->setText({}); result_->setRecurringDigits(-1, 0, {}); result_->setResultRelation({}); conversionStatus_->clear(); insertResult_->setEnabled(false); copyResult_->setEnabled(false);
}

void CalculatorTools::invalidateConversion() {
    clearConversion();
    if (busy_) setBusy(false);
    if (onDraftChanged) onDraftChanged();
}

void CalculatorTools::setConversionResult(const CalcResult &result) {
    setBusy(false);
    if (result.ok) {
        serializedResult_ = result.serialized;
        result_->setText(result.result);result_->setRecurringDigits(result.resultRepeatStart,result.resultRepeatLength,result.resultCopyValue);
        result_->setResultRelation(result.resultApproximate ? QStringLiteral("≈") : QStringLiteral("="));
        approximation_ = result.approximate;
        approximationIsApproximate_ = result.approximateApproximate;approximateRepeatStart_=result.approximateRepeatStart;approximateRepeatLength_=result.approximateRepeatLength;approximateCopyValue_=result.approximateCopyValue;
        refreshApproximation();
        insertResult_->setEnabled(!serializedResult_.isEmpty());
        copyResult_->setEnabled(!serializedResult_.isEmpty());
    } else {
        clearConversion(); conversionStatus_->setText(result.error);
    }
}

void CalculatorTools::setBusy(bool busy) {
    busy_ = busy;
    amount_->setEnabled(!busy); from_->setEnabled(!busy); to_->setEnabled(!busy); convert_->setEnabled(!busy); swap_->setEnabled(!busy);
    conversionMode_->setEnabled(!busy);
    convert_->setText(busy ? trc("Converting…") : trc("Convert"));
}

void CalculatorTools::setConversionExact(bool exact) {
    conversionExact_ = exact;
    conversionMode_->setChecked(!exact);
    conversionModeIndicator_->setText(trc(exact ? "Exact" : "Decimal"));
    const QString mode = trc(exact ? "Exact" : "Decimal");
    conversionMode_->setAccessibleName(trc("Exact / Decimal"));
    conversionMode_->setAccessibleDescription(mode);
    conversionMode_->setToolTip(trc("Exact / Decimal") + QStringLiteral(" · ") + mode);
    conversionModeIndicator_->setAccessibleName(trc("Exact / Decimal"));
    conversionBaseIndicator_->setAccessibleName(trc("Decimal") + QStringLiteral(" · DEC"));
}

void CalculatorTools::setMathPresentation(bool structured, int grouping, QChar decimalSeparator) {
    grouping_ = grouping; decimalSeparator_ = decimalSeparator;
    amount_->setStructuredInput(structured);
    result_->setStructuredInput(true);
    for (auto *editor : {amount_, result_}) editor->setNumberFormat(grouping, decimalSeparator);
    if (!approximation_.isEmpty())refreshApproximation();
}

void CalculatorTools::refreshApproximation(){
 const QString prefix=approximation_.isEmpty()?QString():approximationIsApproximate_?QStringLiteral("≈ "):QStringLiteral("= ");
 conversionStatus_->setText(prefix+formatNumericDisplay(approximation_,grouping_,decimalSeparator_));
 int start=-1,length=0;if(approximateRepeatStart_>=0&&approximateRepeatLength_>0&&approximateRepeatStart_+approximateRepeatLength_<=approximation_.size()){
  const int before=formatNumericDisplay(approximation_.left(approximateRepeatStart_),grouping_,decimalSeparator_).size();start=prefix.size()+before;length=formatNumericDisplay(approximation_.left(approximateRepeatStart_+approximateRepeatLength_),grouping_,decimalSeparator_).size()-before;
 }
 static_cast<RecurringLabel*>(conversionStatus_)->setRepeatingRange(start,length);static_cast<RecurringLabel*>(conversionStatus_)->setExactCopyValue(approximateCopyValue_);
}

bool CalculatorTools::matchesView(const CatalogEntry&e)const{return view_==View::Statistics?true:view_==View::Units?e.unit:view_==View::Functions?e.function:!e.unit&&!e.function;}
QString CalculatorTools::selectedBrowserKey()const{return list_->currentItem()?list_->currentItem()->data(Qt::UserRole+6).toString():QString();}
void CalculatorTools::toggleFavorite(){const QString key=selectedBrowserKey();if(key.isEmpty())return;if(favorites_.contains(key))favorites_.remove(key);else{if(favorites_.size()>=256){catalogStatus_->setText(trc("Maximum 256 favorites."));return;}favorites_.insert(key);}if(onBrowserStateChanged)onBrowserStateChanged();filter();}
QString CalculatorTools::offlineExample(QString token,bool unit,bool function)const{
    if(unit)return "3*("+token+")";
    if(!function)return "2*("+token+")";
    static const QMap<QString,QString> examples{
        {"sin(","sin(pi/6 rad)"},{"cos(","cos(pi/3 rad)"},{"tan(","tan(pi/4 rad)"},
        {"sqrt(","sqrt(8)"},{"root(","root(27,3)"},{"abs(","abs(-7)"},{"ln(","ln(e^2)"},{"log(","log(100,10)"},{"exp(","exp(1)"},
        {"gamma(","gamma(5)"},{"factorial(","factorial(5)"},{"factorial2(","factorial2(7)"},{"factorialn(","factorialn(7,2)"},
        {"binomial(","binomial(5,2)"},{"permutation(","permutation(5,2)"},
        {"mean(","mean([5,12,13])"},{"median(","median([5,12,13])"},{"total(","total([5,12,13])"},{"stdev(","stdev([5,12,13])"},{"stdevp(","stdevp([5,12,13])"},
        {"var(","var([5,12,13])"},{"varp(","varp([5,12,13])"},{"min(","min(5,12,13)"},{"max(","max(5,12,13)"},
        {"solve(","solve(x^2-5*x+6=0,x)"},{"diff(","diff(x^3,x)"},{"integrate(","integrate(x^2,0,1,x)"},{"limit(","limit(sin(x)/x,0,x)"},
        {"sum(","sum(x,1,10,x)"},{"product(","product(x,1,5,x)"},
        {"det(","det([1,2;3,4])"},{"inverse(","inverse([1,2;3,4])"},{"transpose(","transpose([1,2;3,4])"},{"rref(","rref([1,2;3,4])"},
        {"factorform(","factorform(x^2-1)"},{"expand(","expand((x+1)^3)"},{"simplify(","simplify((x^2-1)/(x-1))"},
        {"taxadd(","taxadd(100,0.27)"},{"taxremove(","taxremove(127,0.27)"},{"loanpayment(","loanpayment(10000,0.005,36,0)"}
    };
    for(const auto &entry:entries_)if(entry.function&&(entry.token==token||entry.metadata.name+"("==token)&&!entry.metadata.example.isEmpty())return entry.metadata.example;
    return examples.value(token);
}
void CalculatorTools::showExample(){const auto*item=list_->currentItem();if(!item)return;const QString example=offlineExample(item->data(Qt::UserRole).toString(),item->data(Qt::UserRole+1).toBool(),item->data(Qt::UserRole+5).toBool());if(example.isEmpty())return;const QString key=selectedBrowserKey();QDialog dialog(this);dialog.setWindowTitle(trc("Offline example"));auto*layout=new QVBoxLayout(&dialog);auto*note=new QLabel(trc("Examples are inserted only on request and are never evaluated automatically."));note->setWordWrap(true);layout->addWidget(note);auto*source=new QPlainTextEdit(example);source->setReadOnly(true);source->setAccessibleName(trc("Offline example"));layout->addWidget(source);auto*buttons=new QDialogButtonBox(QDialogButtonBox::Cancel);auto*insert=buttons->addButton(trc("Insert example"),QDialogButtonBox::AcceptRole);layout->addWidget(buttons);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);connect(insert,&QPushButton::clicked,&dialog,&QDialog::accept);dialog.resize(std::min(420,width()),std::min(300,height()));if(dialog.exec()==QDialog::Accepted&&onInsert){recent_.removeAll(key);recent_.prepend(key);while(recent_.size()>100)recent_.removeLast();if(onBrowserStateChanged)onBrowserStateChanged();onInsert(example);}}
QJsonObject CalculatorTools::saveBrowserState()const{QStringList favorites=favorites_.values();favorites.sort();return{{"schema",1},{"favorites",QJsonArray::fromStringList(favorites)},{"recent",QJsonArray::fromStringList(recent_)}};}
bool CalculatorTools::restoreBrowserState(const QJsonObject&o,QString*error){if(o.isEmpty())return true;auto fail=[&]{if(error)*error=trc("Invalid catalog preferences.");return false;};if(o.value("schema").toInt()!=1||!o.value("favorites").isArray()||!o.value("recent").isArray()||o.value("favorites").toArray().size()>256||o.value("recent").toArray().size()>100)return fail();QSet<QString>favorites;QStringList recent;for(const char*field:{"favorites","recent"})for(const auto&v:o.value(field).toArray()){QString key=v.toString();if(!v.isString()||key.size()<3||key.size()>512||key.contains(QChar(0))||!QStringList{"u:","c:","f:"}.contains(key.left(2)))return fail();if(QLatin1String(field)=="favorites")favorites.insert(key);else if(!recent.contains(key))recent.append(key);}favorites_=favorites;recent_=recent;filter();return true;}
