/****************************************************************************
**
** WWW.FISHMONITORING.COM.BR
**
** Copyright (C) 2013
**                     Luis Valdes <luisvaldes88@gmail.com>
**
** This file is part of the FishMonitoring project
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; version 2
** of the License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
****************************************************************************/

#include <QLocalSocket>
#include <QTcpSocket>

#include <logger.h>

#include <json/nodejsmessage.h>
#include "communicationservice.h"

CommunicationService::CommunicationService(QObject *parent) :
    CommunicationInterface(parent),
    m_localSocket(0)
{
    m_module = "ComunicatorModule";
    m_localSocket = new QLocalSocket(this);

    connect(m_localSocket, SIGNAL(connected()), SLOT(ipcConnected()));
    connect(m_localSocket, SIGNAL(disconnected()), SLOT(ipcDisconnected()));
    connect(m_localSocket, SIGNAL(readyRead()), SLOT(ipcReadyRead()));
    connect(m_localSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(ipcHandleError(QLocalSocket::LocalSocketError)));

    m_localSocket->connectToServer("RFIDMonitorDaemon");
}

QString CommunicationService::serviceName() const
{
    return "communication.service";
}

void CommunicationService::init()
{
}

ServiceType CommunicationService::type()
{
    return ServiceType::KCommunicator;
}

// Send a message directly without care about the content. Defined by interface.
void CommunicationService::sendMessage(QByteArray value)
{
    m_localSocket->write(value);
    // WARNING: bool QLocalSocket::flush () using this to force the sending of data
    m_localSocket->flush();
    Logger::instance()->writeRecord(Logger::severity_level::debug, "Communicator", Q_FUNC_INFO, QString("Message Sent: %1").arg(QString(value)));
}

// Build a message before send it. Receive the data and the message type and built the massage in protocol definition
void CommunicationService::sendMessage(QJsonObject data, QString type)
{
    QJsonDocument rootDoc;
    QJsonObject obj;

    obj.insert("type", type);
    obj.insert("datetime", QDateTime::currentDateTime().toString(Qt::ISODate));
    obj.insert("data", QJsonValue(data));
    rootDoc.setObject(obj);

    m_localSocket->write(rootDoc.toJson());
    m_localSocket->flush();
}

void CommunicationService::ipcConnected()
{
    sendMessage(QJsonObject(), "SYN");
}

void CommunicationService::ipcDisconnected()
{
    Logger::instance()->writeRecord(Logger::severity_level::debug, "Communicator", Q_FUNC_INFO, QString("CommunicationService -> Disconected from IPC Server."));
}

void CommunicationService::ipcReadyRead()
{
    QByteArray data = m_localSocket->readAll();

    json::NodeJSMessage nodeMessage;

    nodeMessage.read(QJsonDocument::fromJson(data).object());
    QString messageType(nodeMessage.type());

    if(messageType == "ACK-SYN"){
        Logger::instance()->writeRecord(Logger::severity_level::debug, m_module, Q_FUNC_INFO, QString("CommunicationService -> Connected successfully to IPC Server."));
    }
    else{
        emit messageReceived(data);
    }

    /*
    else if (messageType == "ACK-DATA") {

        // Remove the data just synced of database. Use the md5Diggest to know what remove.
        emit messageReceived(data);
        //        QJsonObject ackData(nodeMessage.jsonData());
        //        QString hash = ackData["md5diggest"].toString();
        //        Logger::instance()->writeRecord(Logger::severity_level::debug, m_module, Q_FUNC_INFO, QString("md5diggest: %1").arg(hash));

    } else if (messageType == "STOP"){
        // Stop all services and close system.
        emit messageReceived(data);

    } else if (messageType == "SLEEP"){
        // lost connection with server. Don't send records.
        Logger::instance()->writeRecord(Logger::severity_level::debug, m_module, Q_FUNC_INFO, QString("SLEEP MESSAGE"));
        emit messageReceived(QByteArray("sleep"));

    } else if (messageType == "READER-COMMAND") {

        // Only retun the comand. NOT IMPLEMENTED.
        QJsonObject response(nodeMessage.jsonData());
        response.insert("response", response.value("command"));
        response.remove("command");
        sendMessage(response, "READER-RESPONSE");

    }else if (messageType == "RELOAD"){

        // NOT USEFULL YET
        emit messageReceived("ReloadSettings");

    }else if (messageType == "ACK-UNKNOWN") {
        QJsonDocument unknown(nodeMessage.jsonData());
        QJsonObject dataObj(unknown.object().value("unknownmessage").toObject());
        Logger::instance()->writeRecord(Logger::severity_level::debug, "synchronizer", Q_FUNC_INFO, QString("Unknown Message: %1").arg(dataObj.value("type").toString()));
    }else if (messageType == "SYNC") {
        // comand to sync not-synced data.
        Logger::instance()->writeRecord(Logger::severity_level::debug, "synchronizer", Q_FUNC_INFO, "SYNC MESSAGE... !!");
        emit messageReceived(QByteArray("SYNC"));
    }
    else{
        QJsonObject unknownObj;

        unknownObj.insert("unknownmessage", QJsonValue(QJsonDocument::fromJson(data).object()));
        unknownObj.insert("errorinfo", QString("Could not understand this message"));
        sendMessage(unknownObj, "ACK-UNKNOWN");
    }
    */
}

void CommunicationService::ipcHandleError(QLocalSocket::LocalSocketError)
{
    Logger::instance()->writeRecord(Logger::severity_level::debug, m_module, Q_FUNC_INFO, QString("Error: %1 - %2").arg(m_localSocket->error()).arg(m_localSocket->errorString()));
}
