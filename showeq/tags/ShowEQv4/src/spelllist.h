/*
 * spelllist.h
 *
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

/*
 * Orig Author - Crazy Joe Divola (cjd1@users.sourceforge.net)
 * Date - 9/7/2001
 */

#ifndef SPELLLIST_H
#define SPELLLIST_H

#include <qvaluelist.h>
#include <qlistview.h>
#include <qpopupmenu.h>
#include <time.h>
#include <sys/time.h>
#include "spellshell.h"
#include "everquest.h"

#define SPELLCOL_SPELLNAME  0
#define SPELLCOL_SPELLID    1
#define SPELLCOL_CASTERID   2
#define SPELLCOL_CASTERNAME 3
#define SPELLCOL_TARGETID   4
#define SPELLCOL_TARGETNAME 5
#define SPELLCOL_CASTTIME   6
#define SPELLCOL_DURATION   7

class SpellListItem : public QListViewItem 
{
   public:
      SpellListItem(QListViewItem *parent);
      SpellListItem(QListView *parent = NULL);
      //virtual void paintCell( QPainter *p, const QColorGroup &cg,
      //                        int column, int width, int alignment );
      const QColor textColor();
      void setTextColor(const QColor &color);
      void update();
      void setSpellItem(SpellItem *);
      SpellItem* item();
      QString& category();
      void setCategory(QString& cat);
   private:
      QColor m_textColor;
      bool m_btextSet;
      SpellItem *m_item;
      QString m_category;
};

class SpellList : public QListView 
{
   Q_OBJECT
   public:
      SpellList(QWidget *parent = 0, const char *name = 0);
      void SelectItem(SpellItem *item);
      SpellListItem* Selected();
      SpellListItem* InsertSpell(SpellItem *item);
      void DeleteItem(SpellItem *item);
      //SpellItem* AddCategory(QString& name, QColor color = Qt::black);
      //void RemCategory(SpellListItem *);
      //void clearCategories();
      QColor pickSpellColor(SpellItem *item, QColor def = Qt::black);
      //QString& getCategory(SpellListItem *);
      SpellListItem* Find(SpellItem *);

   signals:
      void listUpdated();   // flags in spawns have changed
      void listChanged();   // categories have changed

   public slots:
      // SpellShell signals
      void addSpell(SpellItem *);
      void delSpell(SpellItem *);
      void changeSpell(SpellItem *);
      void selectSpell(SpellItem *);
      void clear();

      void mouseButtonClicked(int,QListViewItem *, const QPoint&, int);
      void rightButtonClicked(QListViewItem *, const QPoint&, int);
      void activated(int);

      void savePrefs(void);

   private:
      void selectAndOpen(SpellListItem *);
      QValueList<QString> m_categoryList;
      QValueList<SpellListItem *> m_spellList;
      QPopupMenu *m_menu;

      int mid_spellName, mid_spellId;
      int mid_casterId, mid_casterName;
      int mid_targetId, mid_targetName;
      int mid_casttime, mid_duration;
};

#endif
