#include "ElementProperties.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QString>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QComboBox>

#include <gst/gst.h>

ElementProperties::ElementProperties (QSharedPointer<GraphManager> pGraph,
                                      const char *name, QWidget *parent,
                                      Qt::WindowFlags flags)
: QWidget (parent, flags),
m_pGraphManager (pGraph),
m_name (name)
{
  setWindowTitle (QString (name) + " properties");

  create ();
}

void
ElementProperties::addParamEnum (GParamSpec *param, GstElement *element,
                                 QGridLayout *play)
{
  GValue value = G_VALUE_INIT;

  g_value_init (&value, param->value_type);
  if (param->flags & G_PARAM_READABLE)
    g_object_get_property (G_OBJECT (element), param->name, &value);
  else {
    const GValue *valueDef = g_param_spec_get_default_value (param);
    g_value_copy (valueDef, &value);
  }

  QString propertyName = g_param_spec_get_name (param);
  int propertyValue;

  propertyValue = g_value_get_enum (&value);

  GParamSpecEnum *penumSpec = G_PARAM_SPEC_ENUM (param);

  if (!penumSpec)
    return;

  QComboBox *pcomBox = new QComboBox;

  for (std::size_t i = 0; i < penumSpec->enum_class->n_values; i++) {
    QVariant var (penumSpec->enum_class->values[i].value);
    QString valueName = penumSpec->enum_class->values[i].value_name;

    pcomBox->addItem (valueName, var);

    if (penumSpec->enum_class->values[i].value == propertyValue)
      pcomBox->setCurrentIndex (i);
  }

  int row = play->rowCount ();
  play->addWidget (new QLabel (propertyName), row, 0);
  play->addWidget (pcomBox, row, 1);
  m_values.insert (propertyName, pcomBox);
}

void
ElementProperties::addParamFlags (GParamSpec *param, GstElement *element,
                                  QGridLayout *play)
{
  GValue value = G_VALUE_INIT;

  g_value_init (&value, param->value_type);
  if (param->flags & G_PARAM_READABLE)
    g_object_get_property (G_OBJECT (element), param->name, &value);
  else {
    const GValue *valueDef = g_param_spec_get_default_value (param);
    g_value_copy (valueDef, &value);
  }

  QString propertyName = g_param_spec_get_name (param);
  size_t propertyValue;

  propertyValue = g_value_get_flags (&value);

  GParamSpecFlags *pflagsSpec = G_PARAM_SPEC_FLAGS (param);

  if (!pflagsSpec)
    return;

  QComboBox *pcomBox = new QComboBox;

  for (std::size_t i = 0; i < pflagsSpec->flags_class->n_values; i++) {
    QVariant var (pflagsSpec->flags_class->values[i].value);
    QString valueName = pflagsSpec->flags_class->values[i].value_name;

    pcomBox->addItem (valueName, var);

    if (pflagsSpec->flags_class->values[i].value == propertyValue)
      pcomBox->setCurrentIndex (i);
  }

  int row = play->rowCount ();

  play->addWidget (new QLabel (propertyName), row, 0);

  play->addWidget (pcomBox, row, 1);

  m_values.insert (propertyName, pcomBox);
}

void
ElementProperties::addParamSimple (GParamSpec *param, GstElement *element,
                                   QGridLayout *play)
{
  bool readOnly = true;

  if (param->flags & G_PARAM_WRITABLE)
    readOnly = false;

  GValue value = G_VALUE_INIT;

  g_value_init (&value, param->value_type);
  if (param->flags & G_PARAM_READABLE)
    g_object_get_property (G_OBJECT (element), param->name, &value);
  else {
    const GValue *valueDef = g_param_spec_get_default_value (param);
    g_value_copy (valueDef, &value);
  }

  QString propertyName = g_param_spec_get_name (param);
  QString propertyValue;
  GType type = G_VALUE_TYPE (&value);

  bool skip = false;

  switch (type) {
    case G_TYPE_STRING: {
      const char *string_val = g_value_get_string (&value);
      propertyValue = string_val;
      break;
    }
    case G_TYPE_BOOLEAN: {
      gboolean bool_val = g_value_get_boolean (&value);
      propertyValue = QString::number (bool_val);
      break;
    }
    case G_TYPE_ULONG: {
      propertyValue = QString::number (g_value_get_ulong (&value));
      break;
    }
    case G_TYPE_LONG: {
      propertyValue = QString::number (g_value_get_long (&value));
      break;
    }
    case G_TYPE_UINT: {
      propertyValue = QString::number (g_value_get_uint (&value));
      break;
    }
    case G_TYPE_INT: {
      propertyValue = QString::number (g_value_get_int (&value));
      break;
    }
    case G_TYPE_UINT64: {
      propertyValue = QString::number (g_value_get_uint64 (&value));
      break;
    }
    case G_TYPE_INT64: {
      propertyValue = QString::number (g_value_get_int64 (&value));
      break;
    }
    case G_TYPE_FLOAT: {
      propertyValue = QString::number (g_value_get_float (&value));
      break;
    }
    case G_TYPE_DOUBLE: {
      propertyValue = QString::number (g_value_get_double (&value));
      break;
    }
    case G_TYPE_CHAR: {
      propertyValue = QString::number (g_value_get_schar (&value));
      break;
    }
    case G_TYPE_UCHAR: {
      propertyValue = QString::number (g_value_get_uchar (&value));
      break;
    }

    default: {
      if (type == g_type_from_name("GstCaps")) {
        GstCaps *gstcaps;
        g_object_get (G_OBJECT (element), param->name, &gstcaps, NULL);
        const char *string_val = gst_caps_to_string (gstcaps);
        if (gstcaps == NULL)
          string_val = "ANY";
        propertyValue = string_val;
        break;
      }

      skip = true;
      LOG_INFO("property %s not supported", propertyName.toStdString ().c_str ());
      break;
    }
  };

  int row = play->rowCount ();

  play->addWidget (new QLabel (propertyName), row, 0);

  QLineEdit *ple = new QLineEdit (propertyValue);
  ple->setReadOnly (readOnly);
  play->addWidget (ple, row, 1);
  if (!skip)
    m_values.insert (propertyName, ple);
  else
    ple->setReadOnly (true);
}

void
ElementProperties::create ()
{
  GstElement *element = gst_bin_get_by_name (
  GST_BIN (m_pGraphManager->m_pGraph), m_name.toStdString ().c_str ());

  if (!element)
    return;

  QGridLayout *play = new QGridLayout;

  GParamSpec **prop_specs;
  guint num_props;

  prop_specs = g_object_class_list_properties (G_OBJECT_GET_CLASS (element),
                                               &num_props);

  for (std::size_t i = 0; i < num_props; i++) {
    GParamSpec *param = prop_specs[i];

    if (G_IS_PARAM_SPEC_ENUM (param))
      addParamEnum (param, element, play);
    else if (G_IS_PARAM_SPEC_FLAGS (param))
      addParamFlags (param, element, play);
    else
      addParamSimple (param, element, play);
  }

  QVBoxLayout *pvblay = new QVBoxLayout;
  QWidget *pwgt = new QWidget (this);
  pwgt->setLayout (play);
  QScrollArea *pscroll = new QScrollArea (this);
  pscroll->setWidget (pwgt);

  pvblay->addWidget (pscroll);

  QHBoxLayout *phblay = new QHBoxLayout;

  QPushButton *pcmdApply = new QPushButton ("Apply");
  QPushButton *pcmdOk = new QPushButton ("OK");
  QPushButton *pcmdCancel = new QPushButton ("Cancel");

  phblay->addStretch (1);
  phblay->addWidget (pcmdApply);
  phblay->addWidget (pcmdCancel);
  phblay->addWidget (pcmdOk);

  pvblay->addLayout (phblay);

  QObject::connect (pcmdApply, SIGNAL (clicked ()), this,
                    SLOT (applyClicked ()));
  QObject::connect (pcmdCancel, SIGNAL (clicked ()), this, SLOT (close ()));
  QObject::connect (pcmdOk, SIGNAL (clicked ()), this, SLOT (okClicked ()));

  setLayout (pvblay);

  g_free (prop_specs);
  gst_object_unref (element);

}

void
ElementProperties::applyClicked ()
{
  GstElement *element = gst_bin_get_by_name (
  GST_BIN (m_pGraphManager->m_pGraph), m_name.toStdString ().c_str ());

  if (!element)
    return;

  QMap<QString, QWidget *>::iterator itr = m_values.begin ();

  for (; itr != m_values.end (); itr++) {
    GParamSpec *param = g_object_class_find_property (
    G_OBJECT_GET_CLASS (element), itr.key ().toStdString ().c_str ());

    if (!param) {
      LOG_INFO("problem with setting %s property", itr.key ().toStdString ().c_str ());
      continue;
    }

    if (!(param->flags & G_PARAM_WRITABLE))
      continue;

    QString valStr;

    if (dynamic_cast<QLineEdit *> (itr.value ()))
      valStr = ((QLineEdit *) itr.value ())->text ();
    else if (dynamic_cast<QComboBox *> (itr.value ())) {
      QComboBox *pcomBox = (QComboBox *) itr.value ();
      int val = pcomBox->itemData (pcomBox->currentIndex ()).toInt ();
      valStr = QString::number (val);
    }

    std::string tmpStr = itr.key ().toStdString ();
    const char *propName = tmpStr.c_str ();

    if (G_IS_PARAM_SPEC_ENUM (param) || G_IS_PARAM_SPEC_FLAGS (param)) {
      if (dynamic_cast<QComboBox *> (itr.value ())) {
        QComboBox *pcomBox = (QComboBox *) itr.value ();
        int val = pcomBox->itemData (pcomBox->currentIndex ()).toInt ();
        g_object_set (G_OBJECT (element), propName, val, NULL);
      }
    }
    else {
      GType type = param->value_type;
      switch (type) {
        case G_TYPE_STRING: {
          g_object_set (G_OBJECT (element), propName,
                        valStr.toStdString ().c_str (), NULL);
          break;
        }
        case G_TYPE_BOOLEAN: {
          gboolean val = valStr.toInt ();
          g_object_set (G_OBJECT (element), propName, val, NULL);
          break;
        }
        case G_TYPE_ULONG: {
          gulong val = valStr.toULong ();
          g_object_set (G_OBJECT (element), propName, val, NULL);
          break;
        }
        case G_TYPE_LONG: {
          glong val = valStr.toLong ();
          g_object_set (G_OBJECT (element), propName, val, NULL);
          break;
        }
        case G_TYPE_UINT: {
          guint val = valStr.toUInt ();
          g_object_set (G_OBJECT (element), propName, val, NULL);
          break;
        }
        case G_TYPE_INT: {
          gint val = valStr.toInt ();
          g_object_set (G_OBJECT (element), propName, val, NULL);
          break;
        }
        case G_TYPE_UINT64: {
          guint64 val = valStr.toULongLong ();
          g_object_set (G_OBJECT (element), propName, val, NULL);
          break;
        }
        case G_TYPE_INT64: {
          gint64 val = valStr.toLongLong ();
          g_object_set (G_OBJECT (element), propName, val, NULL);
          break;
        }
        case G_TYPE_FLOAT: {
          gfloat val = valStr.toFloat ();
          g_object_set (G_OBJECT (element), propName, val, NULL);
          break;
        }
        case G_TYPE_DOUBLE: {
          gdouble val = valStr.toDouble ();
          g_object_set (G_OBJECT (element), propName, val, NULL);
          break;
        }
        case G_TYPE_CHAR: {
          gchar val = valStr.toInt ();
          g_object_set (G_OBJECT (element), propName, val, NULL);
          break;
        }
        case G_TYPE_UCHAR: {
          guchar val = valStr.toUInt ();
          g_object_set (G_OBJECT (element), propName, val, NULL);
          break;
        }
        default: {
          if (type == g_type_from_name("GstCaps")) {
            GstCaps *oldval;
            GstCaps *newval = gst_caps_from_string (valStr.toStdString ().c_str ());
            g_object_get (G_OBJECT (element), propName, &oldval, NULL);

            if (oldval != newval && oldval != NULL) {
              /* Release old */
              gst_caps_unref (oldval);
            }

            g_object_set (G_OBJECT (element), propName, newval, NULL);
            break;
          }

          LOG_INFO("property %s not supported", itr.key ());
          break;
        }
      };
    }
  }

  gst_object_unref (element);

  delete layout ();
  qDeleteAll (
  children ());

create  ();
}

void
ElementProperties::okClicked ()
{
  applyClicked ();
  close ();
}
