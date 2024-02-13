/*
    SPDX-FileCopyrightText: 2003, 2004, 2005, 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DETAILINFODLG_H
#define DETAILINFODLG_H

#include <QMap>

#include <KPageDialog>

class DetailedGraphicalOverview;
class Element;
class OrbitsWidget;
class SpectrumViewImpl;

class KActionCollection;
class QLabel;
class QStackedWidget;
class QTextBrowser;
class KHTMLPart;

/**
 * @brief The dialog which shows all available information of an element
 * @author Carsten Niehaus
 */
class DetailedInfoDlg : public KPageDialog
{
    Q_OBJECT

public:
    explicit DetailedInfoDlg(int el, QWidget *parent = nullptr);
    ~DetailedInfoDlg() override;

    void setElement(int el);

    //     void setOverviewBackgroundColor(const QColor &bgColor);

    void setTableType(int ktt);

    /**
     * add <sup></sup> to the numbers
     @return the beatified string
     */
    QString beautifyOrbitalString(const QString &orbits);

private:
    enum DATATYPE {
        MISC = 0,
        ISOTOPES,
        DATA, /** <the overview about atomic and compound data */
        EXTRA /** Links to wikipedia and other sites where useful information can be found*/
    };

    Element *m_element;
    int m_elementNumber;

    KActionCollection *m_actionCollection;

    SpectrumViewImpl *m_spectrumview;
    QStackedWidget *m_spectrumStack;
    QLabel *m_spectrumLabel;

    QString isotopeTable() const;

    DetailedGraphicalOverview *dTab;
    // X         QLabel *piclabel;
    OrbitsWidget *wOrbits;
    QMap<QString, QTextBrowser *> m_htmlpages;

    int m_tableTyp;

    /**
     * Create the initial set of tabs. Used it *ONLY* once in the
     * constructor.
     */
    void createContent();
    void reloadContent();

    QString getHtml(DATATYPE);

    QString m_baseHtml;
    QString m_baseHtml2;
    // X         QString m_picsdir;

    /**
     * Add a new HTML page to the dialog.
     *
     * @param title The title of the tab, appears above the htmlview
     * @param icontext The name of the tab, appears belov or instead
     * of the icon
     * @param iconname The name of the icon
     * @returns the pointer to the resulting KHTMLPart, needed for
     * writing HTML code on it
     */
    QTextBrowser *addHTMLTab(const QString &title, const QString &icontext, const QString &iconname);
    /**
     * Change the HTML code in an HTML page.
     *
     * @param browser the QTextBrowser to edit
     * @param htmlcode the HTML code to display
     */
    void fillHTMLTab(QTextBrowser *browser, const QString &htmlcode);

    /**
     * Creates a localized link to Wikipedia.
     * https://en.wikipedia.org/wiki/link"
     * @param link the link inside wikipedia
     * @param displayString the displayed string to click on.
     */
    QString createWikiLink(QString link, QString displayString);

    /// overloaded function to add link as the displayed String
    QString createWikiLink(QString link);

private Q_SLOTS:
    void showNextElement();
    void showPreviousElement();
    /**
     * invoke the help of the correct chapter
     */
    virtual void slotHelp();

Q_SIGNALS:
    void elementChanged(int);
};

#endif // DETAILINFODLG_H
