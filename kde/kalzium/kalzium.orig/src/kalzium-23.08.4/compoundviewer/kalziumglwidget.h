/*
    SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2007-2008 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALZIUMGLWIDGET_H
#define KALZIUMGLWIDGET_H

#include <avogadro/qtopengl/glwidget.h>

#include "compoundviewer_export.h"

class COMPOUNDVIEWER_EXPORT KalziumGLWidget : public Avogadro::QtOpenGL::GLWidget
{
    Q_OBJECT
public:
    explicit KalziumGLWidget(QWidget *parent = nullptr);
    virtual ~KalziumGLWidget();

public slots:
    bool openFile(const QString &file);

protected:
    QByteArray m_lc_numeric;
};

#endif // KALZIUMGLWIDGET_H
