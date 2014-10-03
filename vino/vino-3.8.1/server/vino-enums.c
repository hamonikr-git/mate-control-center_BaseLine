


#include "vino-enums.h"

/* enumerations from "server/vino-server.h" */
#include "server/vino-server.h"
GType
vino_auth_method_get_type (void)
{
  static GType type = 0;

  if (!type)
  {
    static const GFlagsValue _vino_auth_method_values[] = {
      { VINO_AUTH_INVALID, "VINO_AUTH_INVALID", "invalid" },
      { VINO_AUTH_NONE, "VINO_AUTH_NONE", "none" },
      { VINO_AUTH_VNC, "VINO_AUTH_VNC", "vnc" },
      { 0, NULL, NULL }
    };

    type = g_flags_register_static ("VinoAuthMethod", _vino_auth_method_values);
  }

  return type;
}

/* enumerations from "server/vino-prompt.h" */
#include "server/vino-prompt.h"
GType
vino_prompt_response_get_type (void)
{
  static GType type = 0;

  if (!type)
  {
    static const GEnumValue _vino_prompt_response_values[] = {
      { VINO_RESPONSE_INVALID, "VINO_RESPONSE_INVALID", "invalid" },
      { VINO_RESPONSE_ACCEPT, "VINO_RESPONSE_ACCEPT", "accept" },
      { VINO_RESPONSE_REJECT, "VINO_RESPONSE_REJECT", "reject" },
      { 0, NULL, NULL }
    };

    type = g_enum_register_static ("VinoPromptResponse", _vino_prompt_response_values);
  }

  return type;
}

/* enumerations from "server/vino-status-icon.h" */
#include "server/vino-status-icon.h"
GType
vino_status_icon_visibility_get_type (void)
{
  static GType type = 0;

  if (!type)
  {
    static const GEnumValue _vino_status_icon_visibility_values[] = {
      { VINO_STATUS_ICON_VISIBILITY_INVALID, "VINO_STATUS_ICON_VISIBILITY_INVALID", "invalid" },
      { VINO_STATUS_ICON_VISIBILITY_ALWAYS, "VINO_STATUS_ICON_VISIBILITY_ALWAYS", "always" },
      { VINO_STATUS_ICON_VISIBILITY_CLIENT, "VINO_STATUS_ICON_VISIBILITY_CLIENT", "client" },
      { VINO_STATUS_ICON_VISIBILITY_NEVER, "VINO_STATUS_ICON_VISIBILITY_NEVER", "never" },
      { 0, NULL, NULL }
    };

    type = g_enum_register_static ("VinoStatusIconVisibility", _vino_status_icon_visibility_values);
  }

  return type;
}

/* enumerations from "server/vino-status-tube-icon.h" */
#include "server/vino-status-tube-icon.h"
GType
vino_status_tube_icon_visibility_get_type (void)
{
  static GType type = 0;

  if (!type)
  {
    static const GEnumValue _vino_status_tube_icon_visibility_values[] = {
      { VINO_STATUS_TUBE_ICON_VISIBILITY_INVALID, "VINO_STATUS_TUBE_ICON_VISIBILITY_INVALID", "invalid" },
      { VINO_STATUS_TUBE_ICON_VISIBILITY_ALWAYS, "VINO_STATUS_TUBE_ICON_VISIBILITY_ALWAYS", "always" },
      { VINO_STATUS_TUBE_ICON_VISIBILITY_CLIENT, "VINO_STATUS_TUBE_ICON_VISIBILITY_CLIENT", "client" },
      { VINO_STATUS_TUBE_ICON_VISIBILITY_NEVER, "VINO_STATUS_TUBE_ICON_VISIBILITY_NEVER", "never" },
      { 0, NULL, NULL }
    };

    type = g_enum_register_static ("VinoStatusTubeIconVisibility", _vino_status_tube_icon_visibility_values);
  }

  return type;
}




