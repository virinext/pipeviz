#ifndef FAVORITES_LIST_H_
#define FAVORITES_LIST_H_

#include <QString>
#include <QListWidgetItem>

class FavoritesList: public QListWidget
{
  Q_OBJECT
public:
  FavoritesList(QWidget *pwgt = NULL);
  ~FavoritesList();

  int  isFavorite(const QString& plugin_name);

  void addFavorite(const QString& plugin_name);
  void removeFavorite(const QString& plugin_name);

private:
  void loadFavorites();
  void saveFavorites();
};

#endif
