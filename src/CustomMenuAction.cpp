#include "CustomMenuAction.h"

CustomMenuAction::CustomMenuAction (const QString& displayName,
                                    QObject * parent)
: QAction (displayName, parent),
m_name (displayName)
{
}

CustomMenuAction::CustomMenuAction (const QString& displayName,
                                    const QString& name, QObject * parent)
: QAction (displayName, parent),
m_name (name)
{
}
