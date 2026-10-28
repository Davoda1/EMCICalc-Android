// SPDX-License-Identifier: MIT
#pragma once

#include "engine.h"
#include <QWidget>
#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <functional>

class QLabel;
class QLineEdit;
class QComboBox;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QSpinBox;
class QStringListModel;
class MathEditor;
class FinancePanel;

// Offline definition browser and explicit unit conversion. No engine calls are
// made by this widget: its owner dispatches onConvert to the serialized worker.
class CalculatorTools final : public QWidget {
public:
    enum class View { Constants, Units, Converter, Statistics, Matrix, Functions };
    explicit CalculatorTools(QWidget *parent = nullptr);
    void setCatalog(QVector<CatalogEntry> entries);
    QStringList constantTokens() const;
    void setMatrixDimensions(int rows = 0, int columns = 0);
    void setView(View view);
    void setHeaderVisible(bool visible);
    View view() const { return view_; }
    void retranslate(std::function<QString(const char *)> translate);
    void setConversionResult(const CalcResult &result);
    // Presentation only: the owner reformats its stored canonical result.
    // Setting a saved preference does not emit onConversionExactChanged.
    void setConversionExact(bool exact);
    bool conversionExact() const { return conversionExact_; }
    void setBusy(bool busy);
    void setMathPresentation(bool structured, int grouping, QChar decimalSeparator);
    MathEditor *amountEditor() const { return amount_; }
    MathEditor *resultEditor() const { return result_; }
    QComboBox *sourceUnitEditor() const { return from_; }
    QComboBox *destinationUnitEditor() const { return to_; }
    QListWidget *catalogList() const { return list_; }
    FinancePanel *financePanel() const { return finance_; }
    void setFinanceVisible(bool visible);
    QJsonObject saveBrowserState() const;
    bool restoreBrowserState(const QJsonObject&,QString *error=nullptr);
    std::function<void()> onBrowserStateChanged;
    std::function<void(QString)> onInsert;
    std::function<void(int, int)> onInsertMatrix;
    std::function<void(QString, QString, QString)> onConvert;
    // Invalidates an owner-managed asynchronous conversion when its draft changes.
    std::function<void()> onDraftChanged;
    std::function<void(bool)> onConversionExactChanged;
    QSize minimumSizeHint() const override {
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
        return QSize(220,200);
#else
        return QSize(260,300);
#endif
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QString trc(const char *text) const;
    void rebuildCategories();
    void filter();
    void insertSelected();
    void toggleFavorite();
    void showExample();
    QString selectedBrowserKey() const;
    QString offlineExample(QString token,bool unit,bool function) const;
    bool matchesView(const CatalogEntry&) const;
    void requestConversion();
    void clearConversion(); void invalidateConversion(); void refreshApproximation();
    void swapUnits();
    QString unitText(QComboBox *box) const;
    QVector<CatalogEntry> statisticsCatalog() const;
    void updateTitle();
    std::function<QString(const char *)> translate_;
    QVector<CatalogEntry> entries_;
    View view_ = View::Constants;
    bool busy_ = false;
    bool catalogLoaded_ = false;
    bool conversionExact_ = true;
    int grouping_ = 0;
    QChar decimalSeparator_ = QLatin1Char('.');
    QString approximation_,approximateCopyValue_; int approximateRepeatStart_=-1,approximateRepeatLength_=0;
    bool approximationIsApproximate_ = false;
    QStackedWidget *pages_ = nullptr;
    QComboBox *statisticsMode_ = nullptr;
    FinancePanel *finance_ = nullptr;
    QLabel *title_ = nullptr;
    QSpinBox *matrixRows_ = nullptr, *matrixColumns_ = nullptr;
    QLabel *matrixNote_ = nullptr, *matrixRowsLabel_ = nullptr, *matrixColumnsLabel_ = nullptr;
    QPushButton *insertMatrix_ = nullptr;
    QList<QPushButton *> matrixFunctions_;
    bool matrixEditing_ = false;
    QPushButton *constants_ = nullptr;
    QPushButton *units_ = nullptr;
    QPushButton *functions_ = nullptr,*favorite_ = nullptr,*example_ = nullptr;
    QComboBox *browserMode_ = nullptr;
    QSet<QString> favorites_;
    QStringList recent_;
    QLineEdit *search_ = nullptr;
    QComboBox *categories_ = nullptr;
    QListWidget *list_ = nullptr;
    QLabel *catalogStatus_ = nullptr;
    QPushButton *insert_ = nullptr;
    QLabel *valueLabel_ = nullptr;
    QLabel *fromLabel_ = nullptr;
    QLabel *toLabel_ = nullptr;
    QLabel *resultLabel_ = nullptr;
    QLabel *converterNote_ = nullptr;
    QLabel *conversionStatus_ = nullptr;
    QLabel *conversionModeIndicator_ = nullptr;
    QLabel *conversionBaseIndicator_ = nullptr;
    MathEditor *amount_ = nullptr;
    MathEditor *result_ = nullptr;
    QComboBox *from_ = nullptr;
    QComboBox *to_ = nullptr;
    QPushButton *convert_ = nullptr;
    QPushButton *conversionMode_ = nullptr;
    QPushButton *swap_ = nullptr;
    QPushButton *copyResult_ = nullptr;
    QPushButton *insertResult_ = nullptr;
    QStringListModel *unitCompletions_ = nullptr;
    QHash<QString, QString> completionTokens_;
    QString serializedResult_;
};
