#include "api.h"
#include "AI/openai_message.h"
#include "AI/openai_request.h"
#include "QDate"
#include "QJsonObject"
#include "QtCore/qjsonarray.h"
#include "QtCore/qjsondocument.h"
#include "calendarwidget.h"
#include "event.h"
#include "mainwindow.h"
#include "eventmanager.h"

QList<APITool> API::toolList = {};

API::API()
{

}

QList<APITool> API::tools()
{
    if (toolList.isEmpty()) {
        generateTools();
    }
    return toolList;
}

void API::generateTools()
{
    QJsonObject parametersPropertiesObject {
        {"date", QJsonObject{{"type", "string"}, {"description", "The date of the event, e.g., 2024-02-10"}}},
        {"time", QJsonObject{{"type", "string"}, {"description", "The time of the event, e.g., 14:00"}}},
        {"description", QJsonObject{{"type", "string"}, {"description", "A description of the event"}}},
        {"category", QJsonObject{{"type", "string"}, {"enum", QJsonArray{"Task", "Habit", "Deadline"}}, {"description", "The category of the event"}}}
    };

    QJsonObject parametersObject {
        {"type", "object"},
        {"properties", parametersPropertiesObject},
        {"required", QJsonArray{"date", "time", "description", "category"}}
    };

    QJsonObject functionObject {
        {"name", "addEvent"},
        {"description", "Add an event to the calendar"},
        {"parameters", parametersObject}
    };

    QJsonObject addEventDescription {
        {"type", "function"},
        {"function", functionObject}
    };

    // Add the addEvent function and its description to the tools list
    toolList.append(APITool{&API::addEvent, addEventDescription});

    QJsonObject removeEventParametersPropertiesObject {
        {"id", QJsonObject{{"type", "string"}, {"description", "The unique identifier of the event"}}}
    };

    QJsonObject removeEventParametersObject {
        {"type", "object"},
        {"properties", removeEventParametersPropertiesObject},
        {"required", QJsonArray{"id"}}
    };

    QJsonObject removeEventFunctionObject {
        {"name", "removeEvent"},
        {"description", "Remove an event from the calendar"},
        {"parameters", removeEventParametersObject}
    };

    QJsonObject removeEventDescription {
        {"type", "function"},
        {"function", removeEventFunctionObject}
    };

    // Add the removeEvent function and its description to the tools list
    toolList.append(APITool{&API::removeEvent, removeEventDescription});
}

QJsonArray API::getToolsJsonArray()
{
    QJsonArray toolArray;
    foreach (APITool tool, tools()) {
        toolArray.append(tool.description);
    }
    return toolArray;
}

APITool API::getToolByName(const QString &name)
{
    foreach (APITool tool, tools()) {
        if (tool.getName() == name) {
            return tool;
        }
    }
    return APITool();
}

void API::processToolCalls(const QJsonArray &toolCalls, OpenAIRequest *chatRequest)
{
    if (toolCalls.isEmpty()) return;

    for (int i = 0; i < toolCalls.size(); i++) {
        QJsonObject toolCall = toolCalls.at(i).toObject();
        QJsonObject function = toolCall["function"].toObject();

        // call the function
        QString functionName = function["name"].toString();
        APITool functionToCall = API::getToolByName(functionName);

        QString argumentsStr = function["arguments"].toString();
        QJsonDocument doc = QJsonDocument::fromJson(argumentsStr.toUtf8());
        QJsonObject functionArgs = doc.object();

        printToolCall(functionName, functionArgs);

        QString functionResponse;
        if (functionToCall.isValid()) {
            functionResponse = functionToCall.execute(functionArgs);
        } else {
            functionResponse = functionName + " is not a valid function.";
        }

        // append the function response to conversation
        OpenAIMessage *toolMessage = new OpenAIMessage(functionResponse, OpenAIMessage::Role::Tool);
        toolMessage->setTool_call_id(toolCall["id"].toString());
        chatRequest->addMessage(toolMessage);
    }

    // request that the responses be summarized or that more function calls be made
    chatRequest->execute();
}

void API::printToolCall(const QString &name, const QJsonObject &args)
{
        QStringList debugStringList{name, "("};
        for (auto it = args.begin(); it != args.end(); ++it) {
        debugStringList << it.value().toVariant().typeName() << " " << it.value().toString() << ", ";
        }
        debugStringList << ")";
        qDebug() << debugStringList.join("");
}

QString API::addEvent(const QJsonObject &jsonObject)
{
    QDate date = QDate::fromString(jsonObject.value("date").toString(), "yyyy-MM-dd");
    QTime time = QTime::fromString(jsonObject.value("time").toString(), "HH:mm");
    QString description = jsonObject.value("description").toString();
    QString categoryString = jsonObject.value("category").toString();
    Category category = stringToCategory(categoryString);

    Event event;
    event.date = date;
    event.time = time;
    event.description = description;
    event.category = category;

    if (!event.isValid()) {
        return "Invalid event date or time.";
    }

    EventManager::self()->addEvent(event);
    MainWindow::self()->getCalendarWidget()->addEvent(event);

    return "Added event.";
}

QString API::removeEvent(const QJsonObject &jsonObject)
{
    QString id = jsonObject["id"].toString();

    if (!EventManager::self()->containsEvent(id)) {
        return "Event not found.";
    }

    Event event = EventManager::self()->getEvent(id);

    EventManager::self()->removeEvent(id);
    MainWindow::self()->getCalendarWidget()->removeEvent(event);

    return "Event removed";
}







