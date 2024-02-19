#pragma once

#include "QtCore/qjsonarray.h"
#include "QtCore/qjsonobject.h"
#include <QObject>

class OpenAIMessage: public QObject
{
    Q_OBJECT

public:
    enum class Role {
        System,
        User,
        Assistant,
        Tool
    };

    explicit OpenAIMessage(QObject *parent = nullptr);
    explicit OpenAIMessage(const QString& content, Role role, QObject *parent = nullptr);
    explicit OpenAIMessage(const QJsonObject &contentObject, Role role);
    ~OpenAIMessage();

    inline static QString roleToString(Role role)
    {
        switch (role) {
            case Role::System:
                return "system";
            case Role::User:
                return "user";
            case Role::Assistant:
                return "assistant";
            case Role::Tool:
                return "tool";
            default:
                return "unknown";
        }
    }

    inline static Role roleFromString(const QString& role)
    {
        if (role == "system") {
            return Role::System;
        } else if (role == "user") {
            return Role::User;
        } else if (role == "assistant") {
            return Role::Assistant;
        } else if (role == "tool") {
            return Role::Tool;
        } else {
            return Role::System;
        }
    }

    QString content() const;
    void setContent(const QString& content);


    Role role() const;
    void setRole(Role role);

    QJsonObject contentObject() const;
    void setContentObject(const QJsonObject &newContentObject);

    void setUserMessage(const QString &text);
    QString getUserMessage();

    void setToolResponses(const QJsonValue &value);
    QJsonValue getToolResponses();

    void addScenegraph();
    void removeScenegraph();

    void addPrimaryInstructions();
    void removePrimaryInstructions();

    void addSecondaryInstructions();
    void removeSecondaryInstructions();

signals:
    void contentChanged();
    void roleChanged();

private:
    QString m_content;
    QJsonObject m_contentObject;

    Role m_role;
};
