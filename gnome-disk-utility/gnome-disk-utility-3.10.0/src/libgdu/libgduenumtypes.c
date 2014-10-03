
/* Generated data (by glib-mkenums) */

#include "libgduenums.h"
#include "libgduenumtypes.h"

/* enumerations from "libgduenums.h" */
GType
gdu_format_duration_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GFlagsValue values[] = {
        { GDU_FORMAT_DURATION_FLAGS_NONE, "GDU_FORMAT_DURATION_FLAGS_NONE", "none" },
        { GDU_FORMAT_DURATION_FLAGS_SUBSECOND_PRECISION, "GDU_FORMAT_DURATION_FLAGS_SUBSECOND_PRECISION", "subsecond-precision" },
        { GDU_FORMAT_DURATION_FLAGS_NO_SECONDS, "GDU_FORMAT_DURATION_FLAGS_NO_SECONDS", "no-seconds" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_flags_register_static (g_intern_static_string ("GduFormatDurationFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}


/* Generated data ends here */

