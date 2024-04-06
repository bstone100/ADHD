#include "api.h"
#include "openai_message.h"
#include "openai_request.h"
#include "QDate"
#include "QJsonObject"
#include "QtCore/qjsonarray.h"
#include "QtCore/qjsondocument.h"
#include "../calendarevent.h"
#include "../mainwindow.h"
#include "../calendareventmanager.h"

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
        {"description", QJsonObject{{"type", "string"}, {"description", "A description of the event."}}},
        {"category", QJsonObject{{"type", "string"}, {"enum", QJsonArray{"Event", "Task", "Deadline"}}, {"description", "The category of the event."}}},
        {"allDay", QJsonObject{{"type", "boolean"}, {"description", "Whether the event is all day."}}},
        {"startDateTime", QJsonObject{{"type", "string"}, {"description", "The start date and time of the event in 'yyyy-MM-dd ddd HH:mm' format."}}},
        {"endDateTime", QJsonObject{{"type", "string"}, {"description", "The end date and time of the event in 'yyyy-MM-dd ddd HH:mm' format."}}},
        {"notificationDateTime", QJsonObject{{"type", "string"}, {"description", "The date and time to send the event's notification in 'yyyy-MM-dd ddd HH:mm' format."}}}
    };

    QJsonObject parametersObject {
        {"type", "object"},
        {"properties", parametersPropertiesObject},
        {"required", QJsonArray{"description", "category", "allDay", "startDateTime"}}
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

    MainWindow::self()->saveSettings();
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
    CalendarEvent event = CalendarEvent::fromJson(jsonObject);

    if (!event.isValid()) {
        return "Invalid event parameters.";
    }

    CalendarEventManager::self()->addEvent(event);
    CalendarEventManager::self()->scheduleEventNotification(event);
    MainWindow::self()->updateEventViews();

    return "Added event.";
}

QString API::removeEvent(const QJsonObject &jsonObject)
{
    QString id = jsonObject["id"].toString();

    CalendarEvent event = CalendarEventManager::self()->getEvent(id);

    if (!event.isValid()) {
        return "Event not found.";
    }

    CalendarEventManager::self()->removeEvent(id);
    MainWindow::self()->updateEventViews();

    return "Event removed";
}







