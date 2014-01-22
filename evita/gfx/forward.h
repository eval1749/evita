// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_gfx_forward_h)
#define INCLUDE_gfx_forward_h

#include <stdint.h>

#pragma warning(push)
// warning C4917: 'declarator' : a GUID can only be associated with a class,
// interface or namespace
#pragma warning(disable: 4917)
#include <d2d1.h>
#pragma warning(pop)

namespace gfx {
class Brush;
class FontFace;
class Graphics;
template<typename base, typename element> class Size_;
template<typename base, typename size> class Point_;
template<typename base, typename point, typename size> class Rect_;
typedef Size_<D2D1_SIZE_F, float> SizeF;
typedef Point_<D2D1_POINT_2F, SizeF> PointF;
typedef Rect_<D2D1_RECT_F, PointF, SizeF> RectF;
};

#endif //!defined(INCLUDE_gfx_forward_h)
