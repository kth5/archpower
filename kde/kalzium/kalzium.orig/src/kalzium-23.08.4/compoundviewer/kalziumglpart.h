/*
    SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2007-2008 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALZIUMGLPART_H
#define KALZIUMGLPART_H

#include <readonlypart.h>

class KalziumGLWidget;

class KalziumGLPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    KalziumGLPart(QWidget *, QObject *, const QVariantList &);
    virtual ~KalziumGLPart();

protected:
    bool openFile();

    KalziumGLWidget *m_widget;
};

#endif // KALZIUMGLPART_H
