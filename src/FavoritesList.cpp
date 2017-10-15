#include "FavoritesList.h"
#include "CustomSettings.h"
#include "Logger.h"

#include <QListWidget>

#include <gst/gst.h>


FavoritesList::FavoritesList (QWidget *pwgt)
: QListWidget (pwgt)
{
  loadFavorites();
}

FavoritesList::~FavoritesList ()
{
}

int FavoritesList::isFavorite(const QString& plugin_name)
{
  for (int i = 0; i < this->count (); i++) {
      QListWidgetItem *pitem = this->item (i);
      if (pitem->text () == plugin_name)
        return i;
  }
  return -1;
}

void FavoritesList::addFavorite (const QString& plugin_name)
{
  if (isFavorite(plugin_name) == -1)
    this->addItem(plugin_name);
  saveFavorites();
}

void FavoritesList::removeFavorite (const QString& plugin_name)
{
  int favorite = isFavorite(plugin_name);
  if (favorite != -1)
    delete this->takeItem(this->row(this->item(favorite)));
  saveFavorites();
}

void FavoritesList::loadFavorites()
{
  QStringList favorites = CustomSettings::loadFavoriteList();
  foreach (QString plugin_name, favorites) {
      this->addItem(plugin_name);
  }
  LOG_INFO("Just load favorites");
}

void FavoritesList::saveFavorites()
{
  QStringList favorites;

  for (int i = 0; i < this->count (); i++) {
      QListWidgetItem *pitem = this->item (i);
      favorites << pitem->text();
  }
  LOG_INFO("About to save favorites");
  CustomSettings::saveFavoriteList(favorites);
}
