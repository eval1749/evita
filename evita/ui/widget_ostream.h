// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_widget_ostream_h)
#define INCLUDE_evita_ui_widget_ostream_h

#include <ostream>

namespace ui {
class Widget;
}

std::ostream& operator<<(std::ostream& out, const ui::Widget& widget);
std::ostream& operator<<(std::ostream& out, const ui::Widget* widget);

#define DVLOG_WIDGET(n) \
    DVLOG(n) << __FUNCTION__ << " " << *this << \
        " hwnd=" << AssociatedHwnd() << " "

#define DEBUG_WIDGET_PRINTF(mp_format, ...) \
  DEBUG_PRINTF(DEBUG_WIDGET_FORMAT " " mp_format, \
    DEBUG_WIDGET_ARG(this), __VA_ARGS__)

#define DEBUG_WIDGET_FORMAT "%s@%p"
#define DEBUG_WIDGET_ARG(mp_widget) \
  ((mp_widget) ? (mp_widget)->class_name() : "null"), (mp_widget)

#endif //!defined(INCLUDE_evita_ui_widget_ostream_h)
