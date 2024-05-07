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
#include "qapplication.h""

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
        {"allDay", QJsonObject{{"type", "boolean"}, {"description", "Whether the event is all day."}}},
        {"startDateTime", QJsonObject{{"type", "string"}, {"description", "The start date and time of the event in 'yyyy-MM-dd HH:mm' format."}}},
        {"endDateTime", QJsonObject{{"type", "string"}, {"description", "The end date and time of the event in 'yyyy-MM-dd HH:mm' format."}}},
        {"notificationDateTime", QJsonObject{{"type", "string"}, {"description", "The date and time to send the event's notification in 'yyyy-MM-dd HH:mm' format."}}}
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

    toolList.append(APITool{&API::removeEvent, removeEventDescription});


    QJsonObject editEventParametersPropertiesObject {
        {"id", QJsonObject{{"type", "string"}, {"description", ""}}},
        {"description", QJsonObject{{"type", "string"}, {"description", ""}}},
        {"allDay", QJsonObject{{"type", "boolean"}, {"description", ""}}},
        {"startDateTime", QJsonObject{{"type", "string"}, {"description", ""}}},
        {"endDateTime", QJsonObject{{"type", "string"}, {"description", ""}}},
        {"notificationDateTime", QJsonObject{{"type", "string"}, {"description", ""}}}
    };

    QJsonObject editEventParametersObject {
        {"type", "object"},
        {"properties", editEventParametersPropertiesObject},
        {"required", QJsonArray{"id"}}
    };

    QJsonObject editEventFunctionObject {
        {"name", "editEvent"},
        {"description", "Edit an existing event in the calendar"},
        {"parameters", editEventParametersObject}
    };

    QJsonObject editEventDescription {
        {"type", "function"},
        {"function", editEventFunctionObject}
    };

    toolList.append(APITool{&API::editEvent, editEventDescription});


    QJsonObject getEventsInRangeParametersPropertiesObject {
        {"startDate", QJsonObject{{"type", "string"}, {"description", "The start date of the range, in yyyy-MM-dd format"}}},
        {"endDate", QJsonObject{{"type", "string"}, {"description", "The end date of the range, in yyyy-MM-dd format"}}}
    };

    QJsonObject getEventsInRangeParametersObject {
        {"type", "object"},
        {"properties", getEventsInRangeParametersPropertiesObject},
        {"required", QJsonArray{"startDate", "endDate"}}
    };

    QJsonObject getEventsInRangeFunctionObject {
        {"name", "getEventsInRange"},
        {"description", "Retrieve events within a specified date range"},
        {"parameters", getEventsInRangeParametersObject}
    };

    QJsonObject getEventsInRangeDescription {
        {"type", "function"},
        {"function", getEventsInRangeFunctionObject}
    };

    toolList.append(APITool{&API::getEventsInRange, getEventsInRangeDescription});


    QJsonObject getContextForRangeParametersPropertiesObject {
        {"startDate", QJsonObject{{"type", "string"}, {"description", ""}}},
        {"endDate", QJsonObject{{"type", "string"}, {"description", ""}}}
    };

    QJsonObject getContextForRangeParametersObject {
        {"type", "object"},
        {"properties", getContextForRangeParametersPropertiesObject},
        {"required", QJsonArray{"startDate", "endDate"}}
    };

    QJsonObject getContextForRangeFunctionObject {
        {"name", "getContextForRange"},
        {"description", "Show the days of the week corresponding to the dates in the range"},
        {"parameters", getContextForRangeParametersObject}
    };

    QJsonObject getContextForRangeDescription {
        {"type", "function"},
        {"function", getContextForRangeFunctionObject}
    };

    toolList.append(APITool{&API::getContextForRange, getContextForRangeDescription});
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

    bool onlyGettingEvents = true;
    for (int i = 0; i < toolCalls.size(); i++) {
        QJsonObject toolCall = toolCalls.at(i).toObject();
        QJsonObject function = toolCall["function"].toObject();

        // call the function
        QString functionName = function["name"].toString();
        APITool functionToCall = API::getToolByName(functionName);
        if (functionName != "getEventsInRange" && functionName != "getContextForRange") {
            onlyGettingEvents = false;
        }

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
        qDebug() << functionResponse;

        // append the function response to conversation
        OpenAIMessage *toolMessage = new OpenAIMessage(functionResponse, OpenAIMessage::Role::Tool);
        toolMessage->setTool_call_id(toolCall["id"].toString());
        chatRequest->addMessage(toolMessage);
    }

    MainWindow::self()->setAssistantWidgetText(onlyGettingEvents ? qApp->tr("Checking your schedule...") : qApp->tr("Completing tasks..."));

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

    return QString("Added event (id: %1).").arg(event.id);
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

    return "Event removed.";
}

QString API::editEvent(const QJsonObject &jsonObject)
{
    QString id = jsonObject["id"].toString();

    CalendarEvent event = CalendarEventManager::self()->getEvent(id);

    if (!event.isValid()) {
        return "Event not found.";
    }

    event.updateFromJson(jsonObject);

    CalendarEventManager::self()->removeEvent(event);

    CalendarEventManager::self()->addEvent(event);
    CalendarEventManager::self()->scheduleEventNotification(event);
    MainWindow::self()->updateEventViews();

    return "Event edited.";
}

QString API::getEventsInRange(const QJsonObject &jsonObject)
{
    QDate startDate = QDate::fromString(jsonObject["startDate"].toString(), "yyyy-MM-dd");
    QDate endDate = QDate::fromString(jsonObject["endDate"].toString(), "yyyy-MM-dd");

    QJsonArray eventArray = CalendarEventManager::self()->getEventsForDateRangeJson(startDate, endDate);

    QJsonDocument doc(eventArray);
    return doc.toJson(QJsonDocument::Compact);
}

QString API::getContextForRange(const QJsonObject &jsonObject)
{
    QDate startDate = QDate::fromString(jsonObject["startDate"].toString(), "yyyy-MM-dd");
    QDate endDate = QDate::fromString(jsonObject["endDate"].toString(), "yyyy-MM-dd");

    return getContextForDateRange(startDate, endDate);
}


QString API::getContextForDateRange(const QDate &startDate, const QDate &endDate)
{
    QString context;
    QDate currentDate = QDate::currentDate();

    for (int i = 0; i <= startDate.daysTo(endDate); i++) {
        QDate futureDate = startDate.addDays(i);
        QString dateString;

        // Check if it's the first or last date of the range
        if (i == 0 || i == startDate.daysTo(endDate)) {
            dateString = futureDate.toString("ddd d MMM");
        } else {
            dateString = futureDate.toString("ddd d");
        }

        // Check if the date is 'today'
        if (futureDate == currentDate) {
            dateString += " today";
        }

        dateString += ", ";
        context += dateString;
    }

    context.chop(2);  // Remove the last comma and space
    return context;
}










