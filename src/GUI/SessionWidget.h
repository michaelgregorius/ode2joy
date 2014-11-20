/*
 * Copyright (C) 2014 Michael Gregorius
 *
 * This file is part of ode2joy.
 *
 * ode2joy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ode2joy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ode2joy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SESSIONWIDGET_H_
#define SESSIONWIDGET_H_

#include <QWidget>
#include <QList>

#include "SessionObserver.h"

class Session;
class QVBoxLayout;

class SessionWidget : public QWidget, public SessionObserver
{
    Q_OBJECT

public:
    SessionWidget(QWidget *parent, Session* session);
    virtual ~SessionWidget();

    void setSession(Session *session);

    // Methods from SessionObserver
    virtual void onJoystickDriverChanged();
    virtual void onMIDIDriverChanged();

//private slots:
//    void onActionNewSession();

    // Private methods
private:
    void onSessionChanged();

    void removeJoystickWidgets();

private:
    Session *m_session;
    QVBoxLayout *m_layout;

    QList<QWidget*> m_addedWidgets;
};

#endif
