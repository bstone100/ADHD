#include "openai_message.h"
#include "QtCore/qjsondocument.h"
#include "../mainwindow.h"
#include "../calendarwidget.h"

OpenAIMessage::OpenAIMessage(QObject *parent):
    QObject(parent)
{
    
}

OpenAIMessage::OpenAIMessage(const QString& content, OpenAIMessage::Role role, QObject *parent):
    QObject(parent),
    m_content(content),
    m_role(role)
{

}

OpenAIMessage::OpenAIMessage(const QJsonObject &contentObject, Role role):
    m_contentObject(contentObject),
    m_role(role)
{

}

OpenAIMessage::~OpenAIMessage()
{

}

QString OpenAIMessage::content() const
{
    if (!m_contentObject.isEmpty()) {
        QJsonDocument doc(m_contentObject);
        return QString::fromUtf8(doc.toJson());
    }
    return m_content;
}

void OpenAIMessage::setContent(const QString& content)
{
    if (m_content != content) {
        m_content = content;
        emit contentChanged();
    }
}

OpenAIMessage::Role OpenAIMessage::role() const
{
    return m_role;
}

void OpenAIMessage::setRole(OpenAIMessage::Role role)
{
    if (m_role != role) {
        m_role = role;
        emit roleChanged();
    }
}

QJsonObject OpenAIMessage::contentObject() const
{
    return m_contentObject;
}

void OpenAIMessage::setContentObject(const QJsonObject &newContentObject)
{
    m_contentObject = newContentObject;
}

void OpenAIMessage::setUserMessage(const QString &text)
{
    m_contentObject["user_message"] = text;
}

QString OpenAIMessage::getUserMessage()
{
    return m_contentObject.value("user_message").toString();
}

void OpenAIMessage::setToolResponses(const QJsonValue &value)
{
    m_contentObject["tool_responses"] = value;
}

QJsonValue OpenAIMessage::getToolResponses()
{
    return m_contentObject.value("tool_responses");
}

void OpenAIMessage::addScenegraph()
{
    m_contentObject["scenegraph"] = MainWindow::self()->getCalendarWidget()->getJsonObject();
}

void OpenAIMessage::removeScenegraph()
{
    // keep the key just remove the value
    if (m_contentObject.contains("scenegraph")) {
        m_contentObject["scenegraph"] = "removed";
    }
}

void OpenAIMessage::addPrimaryInstructions()
{
    m_contentObject["instructions"] = "You will respond to this message in perfect JSON format."
                                      "It will be parsed with QJsonDocument::fromJson."

                                      "You will respond with a JSON object containing two keys."
                                      "The first key is assistant_message, and the second is tool_calls."

                                      "If you think you can make tool calls to complete the user's request,"
                                      "then you will fill the tool_calls array and leave assistant_message empty."
                                      "Each array element will have keys name and arguments."
                                      "Only use tools that have been provided to you."
                                      "If the user wants information about their schedule, you can find it in the scenegraph."

                                      "If you can't complete the user's request with tool calls,"
                                      "then you will fill assistant_message with your response and leave tool_calls empty.";
}

void OpenAIMessage::removePrimaryInstructions()
{
    // keep the key just remove the value
    if (m_contentObject.contains("instructions")) {
        m_contentObject["instructions"] = "removed";
    }
}

void OpenAIMessage::addSecondaryInstructions()
{
    m_contentObject["instructions"] = "You will respond to this message in perfect JSON format."
                                      "It will be parsed with QJsonDocument::fromJson."

                                      "You will respond with a JSON object containing only key assistant_message."
                                      "Your assistant_message will summarize the results of your previous tool calls.";
}

void OpenAIMessage::removeSecondaryInstructions()
{
    // keep the key just remove the value
    if (m_contentObject.contains("instructions")) {
        m_contentObject["instructions"] = "removed";
    }
}



