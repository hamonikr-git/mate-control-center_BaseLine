


#ifndef VINO_ENUMS_H_
#define VINO_ENUMS_H_

#include <glib-object.h>

G_BEGIN_DECLS

/* enumerations from server/vino-server.h */
GType vino_auth_method_get_type (void) G_GNUC_CONST;
#define VINO_TYPE_AUTH_METHOD (vino_auth_method_get_type())
/* enumerations from server/vino-prompt.h */
GType vino_prompt_response_get_type (void) G_GNUC_CONST;
#define VINO_TYPE_PROMPT_RESPONSE (vino_prompt_response_get_type())
/* enumerations from server/vino-status-icon.h */
GType vino_status_icon_visibility_get_type (void) G_GNUC_CONST;
#define VINO_TYPE_STATUS_ICON_VISIBILITY (vino_status_icon_visibility_get_type())
/* enumerations from server/vino-status-tube-icon.h */
GType vino_status_tube_icon_visibility_get_type (void) G_GNUC_CONST;
#define VINO_TYPE_STATUS_TUBE_ICON_VISIBILITY (vino_status_tube_icon_visibility_get_type())
G_END_DECLS

#endif /* VINO_ENUMS_H_ */



