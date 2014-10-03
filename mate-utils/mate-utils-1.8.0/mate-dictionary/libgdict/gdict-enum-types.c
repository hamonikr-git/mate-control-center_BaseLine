
/* Generated data (by glib-mkenums) */

#include "gdict-enum-types.h"
/* enumerations from "./gdict-context.h" */
#include "./gdict-context.h"
GType
gdict_context_error_get_type (void)
{
  static GType g_enum_type_id = 0;
  if (G_UNLIKELY (g_enum_type_id == 0))
    {
      static const GEnumValue values[] = {
        { GDICT_CONTEXT_ERROR_PARSE, "GDICT_CONTEXT_ERROR_PARSE", "parse" },
        { GDICT_CONTEXT_ERROR_NOT_IMPLEMENTED, "GDICT_CONTEXT_ERROR_NOT_IMPLEMENTED", "not-implemented" },
        { GDICT_CONTEXT_ERROR_INVALID_DATABASE, "GDICT_CONTEXT_ERROR_INVALID_DATABASE", "invalid-database" },
        { GDICT_CONTEXT_ERROR_INVALID_STRATEGY, "GDICT_CONTEXT_ERROR_INVALID_STRATEGY", "invalid-strategy" },
        { GDICT_CONTEXT_ERROR_INVALID_COMMAND, "GDICT_CONTEXT_ERROR_INVALID_COMMAND", "invalid-command" },
        { GDICT_CONTEXT_ERROR_NO_MATCH, "GDICT_CONTEXT_ERROR_NO_MATCH", "no-match" },
        { GDICT_CONTEXT_ERROR_NO_DATABASES, "GDICT_CONTEXT_ERROR_NO_DATABASES", "no-databases" },
        { GDICT_CONTEXT_ERROR_NO_STRATEGIES, "GDICT_CONTEXT_ERROR_NO_STRATEGIES", "no-strategies" },
        { 0, NULL, NULL }
      };
      g_enum_type_id = g_enum_register_static("GdictContextError", values);
    }

  return g_enum_type_id;
}
/* enumerations from "./gdict-client-context.h" */
#include "./gdict-client-context.h"
GType
gdict_client_context_error_get_type (void)
{
  static GType g_enum_type_id = 0;
  if (G_UNLIKELY (g_enum_type_id == 0))
    {
      static const GEnumValue values[] = {
        { GDICT_CLIENT_CONTEXT_ERROR_SOCKET, "GDICT_CLIENT_CONTEXT_ERROR_SOCKET", "socket" },
        { GDICT_CLIENT_CONTEXT_ERROR_LOOKUP, "GDICT_CLIENT_CONTEXT_ERROR_LOOKUP", "lookup" },
        { GDICT_CLIENT_CONTEXT_ERROR_NO_CONNECTION, "GDICT_CLIENT_CONTEXT_ERROR_NO_CONNECTION", "no-connection" },
        { GDICT_CLIENT_CONTEXT_ERROR_SERVER_DOWN, "GDICT_CLIENT_CONTEXT_ERROR_SERVER_DOWN", "server-down" },
        { 0, NULL, NULL }
      };
      g_enum_type_id = g_enum_register_static("GdictClientContextError", values);
    }

  return g_enum_type_id;
}
/* enumerations from "./gdict-source.h" */
#include "./gdict-source.h"
GType
gdict_source_transport_get_type (void)
{
  static GType g_enum_type_id = 0;
  if (G_UNLIKELY (g_enum_type_id == 0))
    {
      static const GEnumValue values[] = {
        { GDICT_SOURCE_TRANSPORT_DICTD, "GDICT_SOURCE_TRANSPORT_DICTD", "dictd" },
        { GDICT_SOURCE_TRANSPORT_INVALID, "GDICT_SOURCE_TRANSPORT_INVALID", "invalid" },
        { 0, NULL, NULL }
      };
      g_enum_type_id = g_enum_register_static("GdictSourceTransport", values);
    }

  return g_enum_type_id;
}
GType
gdict_source_error_get_type (void)
{
  static GType g_enum_type_id = 0;
  if (G_UNLIKELY (g_enum_type_id == 0))
    {
      static const GEnumValue values[] = {
        { GDICT_SOURCE_ERROR_PARSE, "GDICT_SOURCE_ERROR_PARSE", "parse" },
        { GDICT_SOURCE_ERROR_INVALID_NAME, "GDICT_SOURCE_ERROR_INVALID_NAME", "invalid-name" },
        { GDICT_SOURCE_ERROR_INVALID_TRANSPORT, "GDICT_SOURCE_ERROR_INVALID_TRANSPORT", "invalid-transport" },
        { GDICT_SOURCE_ERROR_INVALID_BAD_PARAMETER, "GDICT_SOURCE_ERROR_INVALID_BAD_PARAMETER", "invalid-bad-parameter" },
        { 0, NULL, NULL }
      };
      g_enum_type_id = g_enum_register_static("GdictSourceError", values);
    }

  return g_enum_type_id;
}
/* enumerations from "./gdict-utils.h" */
#include "./gdict-utils.h"
GType
gdict_context_prop_get_type (void)
{
  static GType g_enum_type_id = 0;
  if (G_UNLIKELY (g_enum_type_id == 0))
    {
      static const GEnumValue values[] = {
        { GDICT_CONTEXT_PROP_FIRST, "GDICT_CONTEXT_PROP_FIRST", "first" },
        { GDICT_CONTEXT_PROP_LOCAL_ONLY, "GDICT_CONTEXT_PROP_LOCAL_ONLY", "local-only" },
        { GDICT_CONTEXT_PROP_LAST, "GDICT_CONTEXT_PROP_LAST", "last" },
        { 0, NULL, NULL }
      };
      g_enum_type_id = g_enum_register_static("GdictContextProp", values);
    }

  return g_enum_type_id;
}
GType
gdict_status_code_get_type (void)
{
  static GType g_enum_type_id = 0;
  if (G_UNLIKELY (g_enum_type_id == 0))
    {
      static const GEnumValue values[] = {
        { GDICT_STATUS_INVALID, "GDICT_STATUS_INVALID", "invalid" },
        { GDICT_STATUS_N_DATABASES_PRESENT, "GDICT_STATUS_N_DATABASES_PRESENT", "n-databases-present" },
        { GDICT_STATUS_N_STRATEGIES_PRESENT, "GDICT_STATUS_N_STRATEGIES_PRESENT", "n-strategies-present" },
        { GDICT_STATUS_DATABASE_INFO, "GDICT_STATUS_DATABASE_INFO", "database-info" },
        { GDICT_STATUS_HELP_TEXT, "GDICT_STATUS_HELP_TEXT", "help-text" },
        { GDICT_STATUS_SERVER_INFO, "GDICT_STATUS_SERVER_INFO", "server-info" },
        { GDICT_STATUS_CHALLENGE, "GDICT_STATUS_CHALLENGE", "challenge" },
        { GDICT_STATUS_N_DEFINITIONS_RETRIEVED, "GDICT_STATUS_N_DEFINITIONS_RETRIEVED", "n-definitions-retrieved" },
        { GDICT_STATUS_WORD_DB_NAME, "GDICT_STATUS_WORD_DB_NAME", "word-db-name" },
        { GDICT_STATUS_N_MATCHES_FOUND, "GDICT_STATUS_N_MATCHES_FOUND", "n-matches-found" },
        { GDICT_STATUS_CONNECT, "GDICT_STATUS_CONNECT", "connect" },
        { GDICT_STATUS_QUIT, "GDICT_STATUS_QUIT", "quit" },
        { GDICT_STATUS_AUTH_OK, "GDICT_STATUS_AUTH_OK", "auth-ok" },
        { GDICT_STATUS_OK, "GDICT_STATUS_OK", "ok" },
        { GDICT_STATUS_SEND_RESPONSE, "GDICT_STATUS_SEND_RESPONSE", "send-response" },
        { GDICT_STATUS_SERVER_DOWN, "GDICT_STATUS_SERVER_DOWN", "server-down" },
        { GDICT_STATUS_SHUTDOWN, "GDICT_STATUS_SHUTDOWN", "shutdown" },
        { GDICT_STATUS_BAD_COMMAND, "GDICT_STATUS_BAD_COMMAND", "bad-command" },
        { GDICT_STATUS_BAD_PARAMETERS, "GDICT_STATUS_BAD_PARAMETERS", "bad-parameters" },
        { GDICT_STATUS_COMMAND_NOT_IMPLEMENTED, "GDICT_STATUS_COMMAND_NOT_IMPLEMENTED", "command-not-implemented" },
        { GDICT_STATUS_PARAMETER_NOT_IMPLEMENTED, "GDICT_STATUS_PARAMETER_NOT_IMPLEMENTED", "parameter-not-implemented" },
        { GDICT_STATUS_NO_ACCESS, "GDICT_STATUS_NO_ACCESS", "no-access" },
        { GDICT_STATUS_USE_SHOW_INFO, "GDICT_STATUS_USE_SHOW_INFO", "use-show-info" },
        { GDICT_STATUS_UNKNOWN_MECHANISM, "GDICT_STATUS_UNKNOWN_MECHANISM", "unknown-mechanism" },
        { GDICT_STATUS_BAD_DATABASE, "GDICT_STATUS_BAD_DATABASE", "bad-database" },
        { GDICT_STATUS_BAD_STRATEGY, "GDICT_STATUS_BAD_STRATEGY", "bad-strategy" },
        { GDICT_STATUS_NO_MATCH, "GDICT_STATUS_NO_MATCH", "no-match" },
        { GDICT_STATUS_NO_DATABASES_PRESENT, "GDICT_STATUS_NO_DATABASES_PRESENT", "no-databases-present" },
        { GDICT_STATUS_NO_STRATEGIES_PRESENT, "GDICT_STATUS_NO_STRATEGIES_PRESENT", "no-strategies-present" },
        { 0, NULL, NULL }
      };
      g_enum_type_id = g_enum_register_static("GdictStatusCode", values);
    }

  return g_enum_type_id;
}

/* Generated data ends here */

