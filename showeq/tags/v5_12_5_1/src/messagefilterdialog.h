/*
 * messagefilterdialog.h
 *
 * ShowEQ Distributed under GPL
 * http://seq.sf.net/
 */

#ifndef _MESSAGEFILTERDIALOG_H_
#define _MESSAGEFILTERDIALOG_H_

#include <stdint.h>

#include <qdialog.h>

//----------------------------------------------------------------------
// forward declarations
class MessageFilter;
class MessageFilters;

class QComboBox;
class QLineEdit;
class QLabel;
class QPushButton;
class QListBox;
class QListBoxItem;
class QGroupBox;

//----------------------------------------------------------------------
// MessageFilterDialog
class MessageFilterDialog : public QDialog
{
  Q_OBJECT
 public:
  MessageFilterDialog(MessageFilters* filters, const QString& caption,
		      QWidget* parent = 0, const char* name = 0);
  ~MessageFilterDialog();

 public slots:
   void newFilter();
   void addFilter();
   void updateFilter();
   void deleteFilter();

 protected slots:
   void anyTextChanged(const QString& newText);
   void messageTypeSelectionChanged();
   void existingFilterSelectionChanged(QListBoxItem * item);
   void removedFilter(uint32_t mask, uint8_t filter);
   void addedFilter(uint32_t mask, uint8_t filter, const MessageFilter& filter);

 protected:
   void clearFilter();
   void checkState();

  MessageFilters* m_filters;
  QListBox* m_existingFilters;
  QPushButton* m_new;
  QGroupBox* m_filterGroup;
  QLineEdit* m_name;
  QLineEdit* m_pattern;
  QListBox* m_messageTypes;
  QPushButton* m_add;
  QPushButton* m_update;
  QPushButton* m_delete;
  uint8_t m_currentFilterNum;
  const MessageFilter* m_currentFilter;
};

#endif // _MESSAGEFILTERDIALOG_H_
