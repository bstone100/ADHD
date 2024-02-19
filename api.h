#ifndef API_H
#define API_H

#include "QtCore/qjsonobject.h"
#include <QObject>

class OpenAIRequest;

struct APITool {
    typedef QString (*FunctionPtr)(const QJsonObject &);

    FunctionPtr function;
    QJsonObject description;

    bool isValid() const {
        return function;
    }

    QString execute(const QJsonObject &jsonObject) const {
        return function(jsonObject);
    }

    QString getName() const {
        QJsonObject functionObject = description["function"].toObject();
        return functionObject["name"].toString();
    }
};

class API
{
public:
    API();

    static QList<APITool> tools();
    static void generateTools();
    static QJsonArray getToolsJsonArray();
    static APITool getToolByName(const QString &name);
    static void processToolCalls(const QJsonArray &toolCalls, OpenAIRequest *chatRequest);
    static void printToolCall(const QString &name, const QJsonObject &args);

    // the tools
    static QString addEvent(const QJsonObject &jsonObject);
    static QString removeEvent(const QJsonObject &jsonObject);

private:
    static QList<APITool> toolList;
};

#endif // API_H
