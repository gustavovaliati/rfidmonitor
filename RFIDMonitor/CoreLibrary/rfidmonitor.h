/****************************************************************************
**
** WWW.FISHMONITORING.COM.BR
**
** Copyright (C) 2013
**                     Gustavo Valiati <gustavovaliati@gmail.com>
**                     Luis Valdes <luisvaldes88@gmail.com>
**                     Thiago R. M. Bitencourt <thiago.mbitencourt@gmail.com>
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

#ifndef RFIDMONITOR_H
#define RFIDMONITOR_H

#include <QObject>
#include <QMap>
#include <QMutex>

class QCoreApplication;
class QThread;

class CoreModule;
enum class ServiceType;
class Service;
struct RFIDMonitorPrivate;

/*!
 * \brief The RFIDMonitor class is the one in charge of loading all the modules and calling the main service module.
 */
class RFIDMonitor : public QObject
{
    Q_OBJECT
public:
    explicit RFIDMonitor(QObject *parent = 0);
    ~RFIDMonitor();
    
    /*!
     * \brief start loads the modules of the system, initialize them and then calls the main service.
     * \param app is used to get the parameters of the application
     */
    void start(const QCoreApplication &app);

    const QList<CoreModule *> & moduleList() const;
    QList<Service *> services(ServiceType type);
    Service * defaultService(ServiceType type);

    /*!
     * \brief setDefaultService
     * \param type
     */
    void setDefaultService(ServiceType type, QString name);

    bool isRunning();

    QString device();

public slots:
    void stop();
    void newMessage(QByteArray message);

private:
    RFIDMonitorPrivate *d_ptr;
};

#endif // RFIDMONITOR_H
