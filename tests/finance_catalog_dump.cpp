// SPDX-License-Identifier: MIT
// Emits the same schemas used by the native Statistics worksheets for docs.
#include "finance.h"
#include "finance_functions.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <iostream>

static QJsonObject fieldJson(const Finance::Field &f){
    QJsonObject o{{"key",f.key},{"label",f.label},{"type",f.type},{"help",f.help}};
    if(!f.initial.isUndefined()&&!f.initial.isNull())o.insert("default",f.initial);
    if(!f.choices.isEmpty())o.insert("choices",QJsonArray::fromStringList(f.choices));
    if(!f.columns.isEmpty()){QJsonArray a;for(const auto&c:f.columns)a.append(fieldJson(c));o.insert("columns",a);}
    return o;
}
int main(int argc,char **argv){
    QCoreApplication app(argc,argv);QJsonArray tools;
    for(const auto&t:Finance::tools()){
        QJsonArray fields;for(const auto&f:t.fields)fields.append(fieldJson(f));
        tools.append(QJsonObject{{"id",t.id},{"title",t.title},{"category",t.category},{"description",t.description},{"fields",fields},{"example",t.example},{"expression_example",t.expression},{"assumptions",QJsonArray::fromStringList(t.assumptions)}});
    }
    const QJsonObject output{{"version","1.0.0"},{"tools",tools},{"expression_helpers",QJsonArray::fromStringList(financeFunctionSignatures())}};
    std::cout<<QJsonDocument(output).toJson(QJsonDocument::Indented).constData();
}
