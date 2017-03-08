/*
	This file is part of the UrgBenri application.

	Copyright (c) 2016 Mehrez Kristou.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Please contact kristou@hokuyo-aut.jp for more details.

*/

#ifndef SCIP_TERMINAL_WIDGET_H
#define SCIP_TERMINAL_WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QMutex>
#include <QTranslator>

#include "UrgDevice.h"
#include "Thread.h"

#include "HelperPluginInterface.h"

namespace Ui
{
class ScipTerminalHelperPlugin;
}

using namespace qrk;
using namespace std;

class ScipTerminalHelperPlugin : public HelperPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(HelperPluginInterface)
    Q_PLUGIN_METADATA(IID "org.kristou.UrgBenri.ScipTerminalHelperPlugin")
    PLUGIN_FACTORY(ScipTerminalHelperPlugin)
public:
    ScipTerminalHelperPlugin(QWidget* parent = 0);
    virtual ~ScipTerminalHelperPlugin(void);

    QString pluginName() const{return tr("SCIP Terminal");}
    QIcon pluginIcon() const {return QIcon(":/ScipTerminalHelperPlugin/tabIcon");}
    QString pluginDescription() const {return tr("SCIP Terminal");}
    PluginVersion pluginVersion() const {return PluginVersion(1, 0, 0);}
    QString pluginAuthorName() const {return "Mehrez Kristou";}
    QString pluginAuthorContact() const {return "mehrez@kristou.com";}
    int pluginLoadOrder() const {return 0;}
    bool pluginIsExperimental() const { return false; }

    QString pluginLicense() const { return "GPL"; }
    QString pluginLicenseDescription() const { return "GPL"; }

    void saveState(QSettings &settings);
    void restoreState(QSettings &settings);

    void loadTranslator(const QString &locale);

signals:
    void connectionDataReady(const QTime &time, const QByteArray &data);

public slots:
    void onLoad(PluginManagerInterface *manager);
    void onUnload();

private slots:
    void sendText(const QString &term);
    void recieveConnectionData(const QTime &time,const QByteArray &data);
    void decodeSelection();

protected:
    void changeEvent(QEvent *event);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

private:
    ScipTerminalHelperPlugin(const ScipTerminalHelperPlugin &rhs);
    ScipTerminalHelperPlugin &operator = (const ScipTerminalHelperPlugin &rhs);

    Ui::ScipTerminalHelperPlugin* ui;
    UrgDevice* m_sensor;
    Thread m_receptionThread;
    QColor m_txColor;
    QColor m_rxColor;
    QTranslator m_translator;

    int m_historySize;

    void updateText(const QTime &time, const QColor col, const QString &s);

    virtual void setDevice(UrgDevice* urg_);

    int historySize() const;
    void setHistorySize(int historySize);

    static int receptionThreadProcess(void* args);

    //Functions interface
    QVariant setDeviceMethod(const QVariantList &vars);
};

#endif /* !SCIP_TERMINAL_WIDGET_H */

