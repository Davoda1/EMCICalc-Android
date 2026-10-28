// SPDX-License-Identifier: MIT
#pragma once

#include "engine.h"
#include "exchangerates.h"
#include <QWidget>
#include <functional>

class QLabel;
class QComboBox;
class QPushButton;
class QScrollArea;
class QBoxLayout;
class MathEditor;

// A reference-rate converter. Network and calculation work belongs to the
// owner; this widget only edits a draft and presents its verified result.
class CurrencyPanel final : public QWidget {
public:
    explicit CurrencyPanel(QWidget *parent = nullptr);
    MathEditor *amountEditor() const { return amount_; }
    MathEditor *resultEditor() const { return result_; }
    QComboBox *sourceCurrencyBox() const { return from_; }
    QComboBox *destinationCurrencyBox() const { return to_; }
    void setCatalog(QVector<CatalogEntry> entries);
    void setRateState(const ExchangeRateSnapshot &snapshot, bool refreshing, const QString &error = {});
    void setConversionResult(const CalcResult &result);
    void setBusy(bool busy);
    void setMathPresentation(bool structured, int grouping, QChar decimalSeparator);
    void retranslate(std::function<QString(const char *)> translate);
    void setHeaderVisible(bool visible);
    std::function<void(QString, QString, QString)> onConvert;
    std::function<void()> onRefresh;
    std::function<void()> onDraftChanged;
    std::function<void(QString)> onInsert;
    QSize minimumSizeHint() const override { return QSize(260, 300); }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QString trc(const char *text) const;
    QString currencyCode(const QComboBox *box) const;
    bool ratesAvailable() const;
    void rebuildCurrencies();
    void requestConversion();
    void invalidateConversion();
    void clearResult();
    void updateControls();
    void updateRateStatus();
    void swapCurrencies();

    QVector<CatalogEntry> catalog_;
    ExchangeRateSnapshot snapshot_;
    std::function<QString(const char *)> translate_;
    bool catalogLoaded_ = false, busy_ = false, refreshing_ = false;
    bool preferredCurrenciesSelected_ = false;
    QString refreshError_, serializedResult_, copyValue_, resultCurrency_;
    QLabel *title_ = nullptr, *note_ = nullptr, *amountLabel_ = nullptr;
    QLabel *fromLabel_ = nullptr, *toLabel_ = nullptr, *resultLabel_ = nullptr;
    QLabel *conversionStatus_ = nullptr, *rateStatus_ = nullptr, *rateDetails_ = nullptr;
    MathEditor *amount_ = nullptr, *result_ = nullptr;
    QComboBox *from_ = nullptr, *to_ = nullptr;
    QPushButton *convert_ = nullptr, *swap_ = nullptr, *refresh_ = nullptr;
    QPushButton *copy_ = nullptr, *insert_ = nullptr;
    QScrollArea *scroll_ = nullptr;
    QBoxLayout *resultActions_ = nullptr;
};
