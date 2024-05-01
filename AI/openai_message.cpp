#include "openai_message.h"
#include "QtCore/qjsondocument.h"
#include "../calendareventmanager.h"

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
        return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
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

void OpenAIMessage::addScenegraph()
{
    m_scenegraph = CalendarEventManager::self()->getJsonObject();
    m_contentObject["scenegraph"] = m_scenegraph;
}

void OpenAIMessage::removeScenegraph()
{
    // keep the key just remove the value
    if (m_contentObject.contains("scenegraph")) {
        m_contentObject["scenegraph"] = "removed";
    }
}

void OpenAIMessage::addInstructions()
{
    m_instructions = "If the user asks about their schedule, tell them about it using information"
                     "from the scenegraph. Don't make any tool calls."
                     "Use natural language to describe dates and time.";

    m_contentObject["instructions"] = m_instructions;
}

void OpenAIMessage::removeInstructions()
{
    // keep the key just remove the value
    if (m_contentObject.contains("instructions")) {
        m_contentObject["instructions"] = "removed";
    }
}

void OpenAIMessage::addTimestamp()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    m_timestamp = currentDateTime.toString("yyyy-MM-dd ddd HH:mm");
    m_contentObject["timestamp"] = m_timestamp;
}

void OpenAIMessage::removeTimestamp()
{
    // keep the key just remove the value
    if (m_contentObject.contains("timestamp")) {
        m_contentObject["timestamp"] = "removed";
    }
}

QString OpenAIMessage::tool_call_id() const
{
    return m_tool_call_id;
}

void OpenAIMessage::setTool_call_id(const QString &newTool_call_id)
{
    m_tool_call_id = newTool_call_id;
}

QJsonArray OpenAIMessage::tool_calls() const
{
    return m_tool_calls;
}

void OpenAIMessage::setTool_calls(const QJsonArray &newTool_calls)
{
    m_tool_calls = newTool_calls;
}

QString OpenAIMessage::instructions() const
{
    return m_instructions;
}

QJsonObject OpenAIMessage::scenegraph() const
{
    return m_scenegraph;
}

QString OpenAIMessage::timestamp() const
{
    return m_timestamp;
}



