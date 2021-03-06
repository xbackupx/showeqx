/*
 * spawnpointlist.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Borrowed from:  SINS Distributed under GPL
 * Portions Copyright 2001 Zaphod (dohpaz@users.sourceforge.net). 
 *
 * For use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */

#ifndef SPAWNPOINTLIST_H
#define SPAWNPOINTLIST_H

#include <qtimer.h>
#include <qpopupmenu.h>

#include "seqlistview.h"
#include "seqwindow.h"
#include "spawnmonitor.h"

// constants
const int tSpawnPointCoord1 = 0;
const int tSpawnPointCoord2 = 1;
const int tSpawnPointCoord3 = 2;
const int tSpawnPointRemaining = 3;
const int tSpawnPointName = 4;
const int tSpawnPointLast = 5;
const int tSpawnPointSpawned = 6;
const int tSpawnPointCount = 7;
const int tSpawnPointMaxCols = 8;

// forward declarations
class SpawnPointList;
class SpawnPointListItem;
class SpawnPointListMenu;
class SpawnPointWindow;

class SpawnPointListItem: public QListViewItem
{
public:
  SpawnPointListItem(QListView* parent, const SpawnPoint* spawn);

  void update(void);
  virtual void paintCell(QPainter *p, const QColorGroup &cg, 
			 int column, int width, int alignment );
  
  const QColor textColor() const { return m_textColor; }
  void setTextColor(const QColor &color);
  const SpawnPoint* spawnPoint() { return m_spawnPoint; }

 protected:
  QColor m_textColor;
  const SpawnPoint* m_spawnPoint;
};


//--------------------------------------------------
// SpawnListMenu
class SpawnPointListMenu : public QPopupMenu
{
   Q_OBJECT

 public:
  SpawnPointListMenu(SpawnPointList* spawnPointList, 
		     QWidget* parent = 0, const char* name = 0);
  virtual ~SpawnPointListMenu();
  void setCurrentItem(const SpawnPointListItem* item);

 protected slots:
   void init_menu(void);
   void rename_item(int id);
   void toggle_col( int id );
   void set_font(int id);
   void set_caption(int id);

 protected:
  SpawnPointList* m_spawnPointList;
  const SpawnPointListItem* m_currentItem;
  int m_id_rename;
  int m_id_cols[tSpawnPointMaxCols];
};

class SpawnPointList : public SEQListView
{
  Q_OBJECT

 public:
  SpawnPointList(SpawnMonitor* spawnMonitor, 
		     QWidget* parent = 0, const char* name = 0);
  SpawnPointListMenu* menu();
 public slots:
  void rightButtonClicked(QListViewItem*, const QPoint&, int);
  void renameItem(const SpawnPointListItem* item);
  void refresh();
  void newSpawnPoint(const SpawnPoint* sp);
  void clear();
  void handleSelectItem(QListViewItem* item);

 protected:
  SpawnMonitor* m_spawnMonitor;
  SpawnPointListMenu* m_menu;
  QTimer* m_timer;
  const SpawnPoint* m_selected;
  bool m_aboutToPop;
};

class SpawnPointWindow : public SEQWindow
{
  Q_OBJECT

 public:
  SpawnPointWindow(SpawnMonitor* spawnMonitor, 
		   QWidget* parent = 0, const char* name = 0);
  ~SpawnPointWindow();
  SpawnPointList* spawnPointList() { return m_spawnPointList; }

 public slots:
  virtual void savePrefs(void);

 protected:
  SpawnPointList* m_spawnPointList;
};

#endif // SPAWNPOINTLIST_H
