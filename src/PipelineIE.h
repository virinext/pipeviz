#ifndef PIPELINE_IMPORT_EXPORT_H_
#define PIPELINE_IMPORT_EXPORT_H_

#include <QString>
#include <QSharedPointer>

#include "GraphManager.h"

namespace PipelineIE
{
  bool Export(QSharedPointer<GraphManager> pgraph, const QString &fileName);
  bool Import(QSharedPointer<GraphManager> pgraph, const QString &fileName);
  bool Clear(QSharedPointer<GraphManager> pgraph);
};

#endif
