/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2004-2005 James M. Cape <jcape@ignore-your.tv>.
 * Copyright (C) 2007-2008 William Jon McCann <mccann@jhu.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>

#include <float.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>

#define GCR_API_SUBJECT_TO_CHANGE
#include <gcr/gcr-base.h>

#include "act-user-private.h"
#include "accounts-user-generated.h"

/**
 * SECTION:act-user
 * @title: ActUser
 * @short_description: information about a user account
 *
 * An ActUser object represents a user account on the system.
 */

/**
 * ActUser:
 *
 * Represents a user account on the system.
 */

/**
 * ActUserAccountType:
 * @ACT_USER_ACCOUNT_TYPE_STANDARD: Normal non-administrative user
 * @ACT_USER_ACCOUNT_TYPE_ADMINISTRATOR: Administrative user
 *
 * Type of user account
 */

/**
 * ActUserPasswordMode:
 * @ACT_USER_PASSWORD_MODE_REGULAR: Password set normally
 * @ACT_USER_PASSWORD_MODE_SET_AT_LOGIN: Password will be chosen at next login
 * @ACT_USER_PASSWORD_MODE_NONE: No password set
 *
 * Mode for setting the user's password.
 */

#define ACT_USER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), ACT_TYPE_USER, ActUserClass))
#define ACT_IS_USER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), ACT_TYPE_USER))
#define ACT_USER_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), ACT_TYPE_USER, ActUserClass))

#define ACCOUNTS_NAME           "org.freedesktop.Accounts"
#define ACCOUNTS_USER_INTERFACE "org.freedesktop.Accounts.User"

enum {
        PROP_0,
        PROP_UID,
        PROP_USER_NAME,
        PROP_REAL_NAME,
        PROP_ACCOUNT_TYPE,
        PROP_PASSWORD_MODE,
        PROP_PASSWORD_HINT,
        PROP_HOME_DIR,
        PROP_SHELL,
        PROP_EMAIL,
        PROP_LOCATION,
        PROP_LOCKED,
        PROP_AUTOMATIC_LOGIN,
        PROP_SYSTEM_ACCOUNT,
        PROP_NONEXISTENT,
        PROP_LOCAL_ACCOUNT,
        PROP_LOGIN_FREQUENCY,
        PROP_LOGIN_TIME,
        PROP_LOGIN_HISTORY,
        PROP_X_HAS_MESSAGES,
        PROP_X_KEYBOARD_LAYOUTS,
        PROP_BACKGROUND_FILE,
        PROP_ICON_FILE,
        PROP_LANGUAGE,
        PROP_FORMATS_LOCALE,
        PROP_INPUT_SOURCES,
        PROP_X_SESSION,
        PROP_IS_LOADED
};

enum {
        CHANGED,
        SESSIONS_CHANGED,
        LAST_SIGNAL
};

struct _ActUser {
        GObject         parent;

        GDBusConnection *connection;
        AccountsUser    *accounts_proxy;
        GDBusProxy      *object_proxy;
        GCancellable    *get_all_call;
        char            *object_path;

        uid_t           uid;
        char           *user_name;
        char           *real_name;
        char           *password_hint;
        char           *home_dir;
        char           *shell;
        char           *email;
        char           *location;
        char          **x_keyboard_layouts;
        char           *background_file;
        char           *icon_file;
        char           *language;
        char           *formats_locale;
        GVariant       *input_sources;
        char           *x_session;
        GList          *our_sessions;
        GList          *other_sessions;
        int             login_frequency;
        gint64          login_time;
        GVariant       *login_history;

        ActUserAccountType  account_type;
        ActUserPasswordMode password_mode;

        guint           uid_set : 1;

        guint           x_has_messages : 1;
        guint           is_loaded : 1;
        guint           locked : 1;
        guint           automatic_login : 1;
        guint           system_account : 1;
        guint           local_account : 1;
        guint           nonexistent : 1;
};

struct _ActUserClass
{
        GObjectClass parent_class;
};

static void act_user_finalize     (GObject      *object);

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (ActUser, act_user, G_TYPE_OBJECT)

static int
session_compare (const char *a,
                 const char *b)
{
        if (a == NULL) {
                return 1;
        } else if (b == NULL) {
                return -1;
        }

        return strcmp (a, b);
}

void
_act_user_add_session (ActUser    *user,
                       const char *ssid,
                       gboolean    is_ours)
{
        GList *li;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (ssid != NULL);

        li = g_list_find_custom (user->our_sessions, ssid, (GCompareFunc)session_compare);
        if (li == NULL)
                li = g_list_find_custom (user->other_sessions, ssid, (GCompareFunc)session_compare);

        if (li == NULL) {
                g_debug ("ActUser: adding session %s", ssid);
                if (is_ours)
                        user->our_sessions = g_list_prepend (user->our_sessions, g_strdup (ssid));
                else
                        user->other_sessions = g_list_prepend (user->other_sessions, g_strdup (ssid));
                g_signal_emit (user, signals[SESSIONS_CHANGED], 0);
        } else {
                g_debug ("ActUser: session already present: %s", ssid);
        }
}

void
_act_user_remove_session (ActUser    *user,
                          const char *ssid)
{
        GList *li, **headp;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (ssid != NULL);

        headp = &(user->our_sessions);
        li = g_list_find_custom (user->our_sessions, ssid, (GCompareFunc)session_compare);
        if (li == NULL) {
                headp = &(user->other_sessions);
                li = g_list_find_custom (user->other_sessions, ssid, (GCompareFunc)session_compare);
        }

        if (li != NULL) {
                g_debug ("ActUser: removing session %s", ssid);
                g_free (li->data);
                *headp = g_list_delete_link (*headp, li);
                g_signal_emit (user, signals[SESSIONS_CHANGED], 0);
        } else {
                g_debug ("ActUser: session not found: %s", ssid);
        }
}

/**
 * act_user_get_num_sessions:
 * @user: a user
 *
 * Get the number of sessions for a user that are graphical and on the
 * same seat as the session of the calling process.
 *
 * Returns: the number of sessions
 */
guint
act_user_get_num_sessions (ActUser    *user)
{
        return g_list_length (user->our_sessions);
}

/**
 * act_user_get_num_sessions_anywhere:
 * @user: a user
 *
 * Get the number of sessions for a user on any seat of any type.
 * See also act_user_get_num_sessions().
 *
 * (Currently, this function is only implemented for systemd-logind.
 * For ConsoleKit, it is equivalent to act_user_get_num_sessions.)
 *
 * Returns: the number of sessions
 */
guint
act_user_get_num_sessions_anywhere (ActUser    *user)
{
        return (g_list_length (user->our_sessions)
                + g_list_length (user->other_sessions));
}

static void
act_user_get_property (GObject    *object,
                       guint       param_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
        ActUser *user;

        user = ACT_USER (object);

        switch (param_id) {
        case PROP_UID:
                g_value_set_int (value, user->uid);
                break;
        case PROP_USER_NAME:
                g_value_set_string (value, user->user_name);
                break;
        case PROP_REAL_NAME:
                g_value_set_string (value, user->real_name);
                break;
        case PROP_ACCOUNT_TYPE:
                g_value_set_int (value, user->account_type);
                break;
        case PROP_PASSWORD_MODE:
                g_value_set_int (value, user->password_mode);
                break;
        case PROP_PASSWORD_HINT:
                g_value_set_string (value, user->password_hint);
                break;
        case PROP_HOME_DIR:
                g_value_set_string (value, user->home_dir);
                break;
        case PROP_LOGIN_FREQUENCY:
                g_value_set_int (value, user->login_frequency);
                break;
        case PROP_LOGIN_TIME:
                g_value_set_int64 (value, user->login_time);
                break;
        case PROP_LOGIN_HISTORY:
                g_value_set_variant (value, user->login_history);
                break;
        case PROP_SHELL:
                g_value_set_string (value, user->shell);
                break;
        case PROP_EMAIL:
                g_value_set_string (value, user->email);
                break;
        case PROP_LOCATION:
                g_value_set_string (value, user->location);
                break;
        case PROP_X_HAS_MESSAGES:
                g_value_set_boolean (value, user->x_has_messages);
                break;
        case PROP_X_KEYBOARD_LAYOUTS:
                g_value_set_boxed (value, g_strdupv (user->x_keyboard_layouts));
                break;
        case PROP_BACKGROUND_FILE:
                g_value_set_string (value, user->background_file);
                break;
        case PROP_ICON_FILE:
                g_value_set_string (value, user->icon_file);
                break;
        case PROP_LANGUAGE:
                g_value_set_string (value, user->language);
                break;
        case PROP_FORMATS_LOCALE:
                g_value_set_string (value, user->formats_locale);
                break;
        case PROP_INPUT_SOURCES:
                g_value_set_variant (value, user->input_sources);
                break;
        case PROP_X_SESSION:
                g_value_set_string (value, user->x_session);
                break;
        case PROP_LOCKED:
                g_value_set_boolean (value, user->locked);
                break;
        case PROP_AUTOMATIC_LOGIN:
                g_value_set_boolean (value, user->automatic_login);
                break;
        case PROP_SYSTEM_ACCOUNT:
                g_value_set_boolean (value, user->system_account);
                break;
        case PROP_LOCAL_ACCOUNT:
                g_value_set_boolean (value, user->local_account);
                break;
        case PROP_NONEXISTENT:
                g_value_set_boolean (value, user->nonexistent);
                break;
        case PROP_IS_LOADED:
                g_value_set_boolean (value, user->is_loaded);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
                break;
        }
}


static void
act_user_class_init (ActUserClass *class)
{
        GObjectClass *gobject_class;

        gobject_class = G_OBJECT_CLASS (class);

        gobject_class->finalize = act_user_finalize;
        gobject_class->get_property = act_user_get_property;

        g_object_class_install_property (gobject_class,
                                         PROP_REAL_NAME,
                                         g_param_spec_string ("real-name",
                                                              "Real Name",
                                                              "The real name to display for this user.",
                                                              NULL,
                                                              G_PARAM_READABLE));

        g_object_class_install_property (gobject_class,
                                         PROP_ACCOUNT_TYPE,
                                         g_param_spec_int ("account-type",
                                                           "Account Type",
                                                           "The account type for this user.",
                                                           ACT_USER_ACCOUNT_TYPE_STANDARD,
                                                           ACT_USER_ACCOUNT_TYPE_ADMINISTRATOR,
                                                           ACT_USER_ACCOUNT_TYPE_STANDARD,
                                                           G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_PASSWORD_MODE,
                                         g_param_spec_int ("password-mode",
                                                           "Password Mode",
                                                           "The password mode for this user.",
                                                           ACT_USER_PASSWORD_MODE_REGULAR,
                                                           ACT_USER_PASSWORD_MODE_NONE,
                                                           ACT_USER_PASSWORD_MODE_REGULAR,
                                                           G_PARAM_READABLE));

        g_object_class_install_property (gobject_class,
                                         PROP_PASSWORD_HINT,
                                         g_param_spec_string ("password-hint",
                                                              "Password Hint",
                                                              "Hint to help this user remember his password",
                                                              NULL,
                                                              G_PARAM_READABLE));

        g_object_class_install_property (gobject_class,
                                         PROP_UID,
                                         g_param_spec_int ("uid",
                                                           "User ID",
                                                           "The UID for this user.",
                                                           0, G_MAXINT, 0,
                                                           G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_USER_NAME,
                                         g_param_spec_string ("user-name",
                                                              "User Name",
                                                              "The login name for this user.",
                                                              NULL,
                                                              G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_HOME_DIR,
                                         g_param_spec_string ("home-directory",
                                                              "Home Directory",
                                                              "The home directory for this user.",
                                                              NULL,
                                                              G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_SHELL,
                                         g_param_spec_string ("shell",
                                                              "Shell",
                                                              "The shell for this user.",
                                                              NULL,
                                                              G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_EMAIL,
                                         g_param_spec_string ("email",
                                                              "Email",
                                                              "The email address for this user.",
                                                              NULL,
                                                              G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_LOCATION,
                                         g_param_spec_string ("location",
                                                              "Location",
                                                              "The location of this user.",
                                                              NULL,
                                                              G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_LOGIN_FREQUENCY,
                                         g_param_spec_int ("login-frequency",
                                                           "login frequency",
                                                           "login frequency",
                                                           0,
                                                           G_MAXINT,
                                                           0,
                                                           G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_LOGIN_TIME,
                                         g_param_spec_int64 ("login-time",
                                                             "Login time",
                                                             "The last login time for this user.",
                                                             0,
                                                             G_MAXINT64,
                                                             0,
                                                             G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_LOGIN_HISTORY,
                                         g_param_spec_variant ("login-history",
                                                               "Login history",
                                                               "The login history for this user.",
                                                               G_VARIANT_TYPE ("a(xxa{sv})"),
                                                               NULL,
                                                               G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_X_HAS_MESSAGES,
                                         g_param_spec_boolean ("xhas-messages",
                                                             "Has Messages",
                                                             "Whether the user has messages waiting.",
                                                             FALSE,
                                                             G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_X_KEYBOARD_LAYOUTS,
                                         g_param_spec_boxed ("xkeyboard-layouts",
                                                             "Keyboard layouts",
                                                             "The name of keyboard layouts for this user.",
                                                             G_TYPE_STRV,
                                                             G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_BACKGROUND_FILE,
                                         g_param_spec_string ("background-file",
                                                              "Background File",
                                                              "The path to a background for this user.",
                                                              NULL,
                                                              G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_ICON_FILE,
                                         g_param_spec_string ("icon-file",
                                                              "Icon File",
                                                              "The path to an icon for this user.",
                                                              NULL,
                                                              G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_LANGUAGE,
                                         g_param_spec_string ("language",
                                                              "Language",
                                                              "User's locale.",
                                                              NULL,
                                                              G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_FORMATS_LOCALE,
                                         g_param_spec_string ("formats_locale",
                                                              "Regional Formats",
                                                              "User's regional formats.",
                                                              NULL,
                                                              G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_INPUT_SOURCES,
                                         g_param_spec_variant ("input-sources",
                                                               "Input sources",
                                                               "User's input sources.",
                                                               G_VARIANT_TYPE ("aa{ss}"),
                                                               NULL,
                                                               G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_X_SESSION,
                                         g_param_spec_string ("x-session",
                                                              "X session",
                                                              "User's X session.",
                                                              NULL,
                                                              G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_IS_LOADED,
                                         g_param_spec_boolean ("is-loaded",
                                                               "Is loaded",
                                                               "Determines whether or not the user object is loaded and ready to read from.",
                                                               FALSE,
                                                               G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_NONEXISTENT,
                                         g_param_spec_boolean ("nonexistent",
                                                               "Doesn't exist",
                                                               "Determines whether or not the user object represents a valid user account.",
                                                               FALSE,
                                                               G_PARAM_READABLE));
        g_object_class_install_property (gobject_class,
                                         PROP_LOCKED,
                                         g_param_spec_boolean ("locked",
                                                               "Locked",
                                                               "Locked",
                                                               FALSE,
                                                              G_PARAM_READABLE));

        g_object_class_install_property (gobject_class,
                                         PROP_AUTOMATIC_LOGIN,
                                         g_param_spec_boolean ("automatic-login",
                                                               "Automatic Login",
                                                               "Automatic Login",
                                                               FALSE,
                                                               G_PARAM_READABLE));

        g_object_class_install_property (gobject_class,
                                         PROP_LOCAL_ACCOUNT,
                                         g_param_spec_boolean ("local-account",
                                                               "Local Account",
                                                               "Local Account",
                                                               FALSE,
                                                               G_PARAM_READABLE));

        g_object_class_install_property (gobject_class,
                                         PROP_SYSTEM_ACCOUNT,
                                         g_param_spec_boolean ("system-account",
                                                               "System Account",
                                                               "System Account",
                                                               FALSE,
                                                               G_PARAM_READABLE));


        /**
         * ActUser::changed:
         *
         * Emitted when the user accounts changes in some way.
         */
        signals [CHANGED] =
                g_signal_new ("changed",
                              G_TYPE_FROM_CLASS (class),
                              G_SIGNAL_RUN_LAST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__VOID,
                              G_TYPE_NONE, 0);
        /**
         * ActUser::sessions-changed:
         *
         * Emitted when the list of sessions for this user changes.
         */
        signals [SESSIONS_CHANGED] =
                g_signal_new ("sessions-changed",
                              G_TYPE_FROM_CLASS (class),
                              G_SIGNAL_RUN_LAST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__VOID,
                              G_TYPE_NONE, 0);
}

static void
act_user_init (ActUser *user)
{
        GError *error = NULL;

        user->local_account = TRUE;
        user->user_name = NULL;
        user->real_name = NULL;
        user->our_sessions = NULL;
        user->other_sessions = NULL;
        user->login_history = NULL;

        user->connection = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, &error);
        if (user->connection == NULL) {
                g_warning ("Couldn't connect to system bus: %s", error->message);
                g_error_free (error);
        }
}

static void
act_user_finalize (GObject *object)
{
        ActUser *user;

        user = ACT_USER (object);

        g_free (user->user_name);
        g_free (user->real_name);
        g_strfreev (user->x_keyboard_layouts);
        g_free (user->background_file);
        g_free (user->icon_file);
        g_free (user->language);
        g_free (user->object_path);
        g_free (user->password_hint);
        g_free (user->home_dir);
        g_free (user->shell);
        g_free (user->email);
        g_free (user->location);
        if (user->input_sources)
          g_variant_unref (user->input_sources);
        if (user->login_history)
          g_variant_unref (user->login_history);
        g_free (user->formats_locale);

        if (user->accounts_proxy != NULL) {
                g_object_unref (user->accounts_proxy);
        }

        if (user->object_proxy != NULL) {
                g_object_unref (user->object_proxy);
        }

        if (user->get_all_call != NULL) {
                g_object_unref (user->get_all_call);
        }

        if (user->connection != NULL) {
                g_object_unref (user->connection);
        }

        if (G_OBJECT_CLASS (act_user_parent_class)->finalize)
                (*G_OBJECT_CLASS (act_user_parent_class)->finalize) (object);
}

static void
set_is_loaded (ActUser  *user,
               gboolean  is_loaded)
{
        if (user->is_loaded != is_loaded) {
                user->is_loaded = is_loaded;
                g_object_notify (G_OBJECT (user), "is-loaded");
        }
}

/**
 * act_user_get_uid:
 * @user: the user object to examine.
 *
 * Retrieves the ID of @user.
 *
 * Returns: (transfer none): a pointer to an array of characters which must not be modified or
 *  freed, or %NULL.
 **/

uid_t
act_user_get_uid (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), -1);

        return user->uid;
}

/**
 * act_user_get_real_name:
 * @user: the user object to examine.
 *
 * Retrieves the display name of @user.
 *
 * Returns: (transfer none): a pointer to an array of characters which must not be modified or
 *  freed, or %NULL.
 **/
const char *
act_user_get_real_name (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        if (user->real_name == NULL ||
            user->real_name[0] == '\0') {
                return user->user_name;
        }

        return user->real_name;
}

/**
 * act_user_get_account_type:
 * @user: the user object to examine.
 *
 * Retrieves the account type of @user.
 *
 * Returns: a #ActUserAccountType
 **/
ActUserAccountType
act_user_get_account_type (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), ACT_USER_ACCOUNT_TYPE_STANDARD);

        return user->account_type;
}

/**
 * act_user_get_password_mode:
 * @user: the user object to examine.
 *
 * Retrieves the password mode of @user.
 *
 * Returns: a #ActUserPasswordMode
 **/
ActUserPasswordMode
act_user_get_password_mode (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), ACT_USER_PASSWORD_MODE_REGULAR);

        return user->password_mode;
}

/**
 * act_user_get_password_hint:
 * @user: the user object to examine.
 *
 * Retrieves the password hint set by @user.
 *
 * Returns: (transfer none): a pointer to an array of characters which must not be modified or
 *  freed, or %NULL.
 **/
const char *
act_user_get_password_hint (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->password_hint;
}

/**
 * act_user_get_home_dir:
 * @user: the user object to examine.
 *
 * Retrieves the home directory for @user.
 *
 * Returns: (transfer none): a pointer to an array of characters which must not be modified or
 *  freed, or %NULL.
 **/
const char *
act_user_get_home_dir (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->home_dir;
}

/**
 * act_user_get_shell:
 * @user: the user object to examine.
 *
 * Retrieves the shell assigned to @user.
 *
 * Returns: (transfer none): a pointer to an array of characters which must not be modified or
 *  freed, or %NULL.
 **/
const char *
act_user_get_shell (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->shell;
}

/**
 * act_user_get_email:
 * @user: the user object to examine.
 *
 * Retrieves the email address set by @user.
 *
 * Returns: (transfer none): a pointer to an array of characters which must not be modified or
 *  freed, or %NULL.
 **/
const char *
act_user_get_email (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->email;
}

/**
 * act_user_get_location:
 * @user: the user object to examine.
 *
 * Retrieves the location set by @user.
 *
 * Returns: (transfer none): a pointer to an array of characters which must not be modified or
 *  freed, or %NULL.
 **/
const char *
act_user_get_location (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->location;
}

/**
 * act_user_get_user_name:
 * @user: the user object to examine.
 *
 * Retrieves the login name of @user.
 *
 * Returns: (transfer none): a pointer to an array of characters which must not be modified or
 *  freed, or %NULL.
 **/

const char *
act_user_get_user_name (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->user_name;
}

/**
 * act_user_get_login_frequency:
 * @user: a #ActUser
 *
 * Returns the number of times @user has logged in.
 *
 * Returns: the login frequency
 */
int
act_user_get_login_frequency (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), 0);

        return user->login_frequency;
}

/**
 * act_user_get_login_time:
 * @user: a #ActUser
 *
 * Returns the last login time for @user.
 *
 * Returns: (transfer none): the login time
 */
gint64
act_user_get_login_time (ActUser *user) {
        g_return_val_if_fail (ACT_IS_USER (user), 0);

        return user->login_time;
}

/**
 * act_user_get_login_history:
 * @user: a #ActUser
 *
 * Returns the login history for @user.
 *
 * Returns: (transfer none): a pointer to GVariant of type "a(xxa{sv})"
 * which must not be modified or freed, or %NULL.
 */
const GVariant *
act_user_get_login_history (ActUser *user) {
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->login_history;
}

/**
 * act_user_collate:
 * @user1: a user
 * @user2: a user
 *
 * Organize the user by login frequency and names.
 *
 * Returns: negative if @user1 is before @user2, zero if equal
 *    or positive if @user1 is after @user2
 */
int
act_user_collate (ActUser *user1,
                  ActUser *user2)
{
        const char *str1;
        const char *str2;
        int         num1;
        int         num2;
        guint       len1;
        guint       len2;

        g_return_val_if_fail (ACT_IS_USER (user1), 0);
        g_return_val_if_fail (ACT_IS_USER (user2), 0);

        num1 = user1->login_frequency;
        num2 = user2->login_frequency;

        if (num1 > num2) {
                return -1;
        }

        if (num1 < num2) {
                return 1;
        }


        len1 = g_list_length (user1->our_sessions);
        len2 = g_list_length (user2->our_sessions);

        if (len1 > len2) {
                return -1;
        }

        if (len1 < len2) {
                return 1;
        }

        /* if login frequency is equal try names */
        if (user1->real_name != NULL) {
                str1 = user1->real_name;
        } else {
                str1 = user1->user_name;
        }

        if (user2->real_name != NULL) {
                str2 = user2->real_name;
        } else {
                str2 = user2->user_name;
        }

        if (str1 == NULL && str2 != NULL) {
                return -1;
        }

        if (str1 != NULL && str2 == NULL) {
                return 1;
        }

        if (str1 == NULL && str2 == NULL) {
                return 0;
        }

        return g_utf8_collate (str1, str2);
}

/**
 * act_user_is_logged_in:
 * @user: a #ActUser
 *
 * Returns whether or not #ActUser is currently graphically logged in
 * on the same seat as the seat of the session of the calling process.
 *
 * Returns: %TRUE or %FALSE
 */
gboolean
act_user_is_logged_in (ActUser *user)
{
        return user->our_sessions != NULL;
}

/**
 * act_user_is_logged_in_anywhere:
 * @user: a #ActUser
 *
 * Returns whether or not #ActUser is currently logged in in any way
 * whatsoever.  See also act_user_is_logged_in().
 *
 * (Currently, this function is only implemented for systemd-logind.
 * For ConsoleKit, it is equivalent to act_user_is_logged_in.)
 *
 * Returns: %TRUE or %FALSE
 */
gboolean
act_user_is_logged_in_anywhere (ActUser *user)
{
        return user->our_sessions != NULL || user->other_sessions != NULL;
}

/**
 * act_user_get_locked:
 * @user: a #ActUser
 *
 * Returns whether or not the #ActUser account is locked.
 *
 * Returns: %TRUE or %FALSE
 */
gboolean
act_user_get_locked (ActUser *user)
{
        return user->locked;;
}

/**
 * act_user_get_automatic_login:
 * @user: a #ActUser
 *
 * Returns whether or not #ActUser is automatically logged in at boot time.
 *
 * Returns: %TRUE or %FALSE
 */
gboolean
act_user_get_automatic_login (ActUser *user)
{
        return user->automatic_login;
}

/**
 * act_user_is_system_account:
 * @user: a #ActUser
 *
 * Returns whether or not #ActUser represents a 'system account' like
 * 'root' or 'nobody'.
 *
 * Returns: %TRUE or %FALSE
 */
gboolean
act_user_is_system_account (ActUser *user)
{
        return user->system_account;
}

/**
 * act_user_is_local_account:
 * @user: the user object to examine.
 *
 * Retrieves whether the user is a local account or not.
 *
 * Returns: (transfer none): %TRUE if the user is local
 **/
gboolean
act_user_is_local_account (ActUser   *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), FALSE);

        return user->local_account;
}

/**
 * act_user_is_nonexistent:
 * @user: the user object to examine.
 *
 * Retrieves whether the user is nonexistent or not.
 *
 * Returns: (transfer none): %TRUE if the user is nonexistent
 **/
gboolean
act_user_is_nonexistent (ActUser   *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), FALSE);

        return user->nonexistent;
}

/**
 * act_user_get_x_has_messages:
 * @user: a #ActUser
 *
 * Returns whether @user has messages waiting for them.
 *
 * Returns: whether messages exist
 */
gboolean
act_user_get_x_has_messages (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), FALSE);

        return user->x_has_messages;
}

/**
 * act_user_get_x_keyboard_layouts:
 * @user: a #ActUser
 *
 * Returns the name of the account keyboard layouts belonging to @user.
 *
 * Returns: (transfer none): names of keyboard layouts
 */
const char * const *
act_user_get_x_keyboard_layouts (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return (const char * const *) user->x_keyboard_layouts;
}

/**
 * act_user_get_background_file:
 * @user: a #ActUser
 *
 * Returns the path to the account background belonging to @user.
 *
 * Returns: (transfer none): a path to a background
 */
const char *
act_user_get_background_file (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->background_file;
}

/**
 * act_user_get_icon_file:
 * @user: a #ActUser
 *
 * Returns the path to the account icon belonging to @user.
 *
 * Returns: (transfer none): a path to an icon
 */
const char *
act_user_get_icon_file (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->icon_file;
}

/**
 * act_user_get_language:
 * @user: a #ActUser
 *
 * Returns the path to the configured locale of @user.
 *
 * Returns: (transfer none): a path to an icon
 */
const char *
act_user_get_language (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->language;
}

/**
 * act_user_get_formats_locale:
 * @user: a #ActUser
 *
 * Returns the path to the configured formats locale of @user.
 *
 * Returns: (transfer none): a path to an icon
 */
const char *
act_user_get_formats_locale (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->formats_locale;
}

/**
 * act_user_get_input_sources:
 * @user: a #ActUser
 *
 * Returns the input sources of @user.
 *
 * Returns: (transfer none): a list of input sources
 */
GVariant *
act_user_get_input_sources (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->input_sources;
}

/**
 * act_user_get_x_session:
 * @user: a #ActUser
 *
 * Returns the path to the configured X session for @user.
 *
 * Returns: (transfer none): a path to an icon
 */
const char *
act_user_get_x_session (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->x_session;
}

/**
 * act_user_get_object_path:
 * @user: a #ActUser
 *
 * Returns the user accounts service object path of @user,
 * or %NULL if @user doesn't have an object path associated
 * with it.
 *
 * Returns: (transfer none): the object path of the user
 */
const char *
act_user_get_object_path (ActUser *user)
{
        g_return_val_if_fail (ACT_IS_USER (user), NULL);

        return user->object_path;
}

/**
 * act_user_get_primary_session_id:
 * @user: a #ActUser
 *
 * Returns the id of the primary session of @user, or %NULL if @user
 * has no primary session.  The primary session will always be
 * graphical and will be chosen from the sessions on the same seat as
 * the seat of the session of the calling process.
 *
 * Returns: (transfer none): the id of the primary session of the user
 */
const char *
act_user_get_primary_session_id (ActUser *user)
{
        if (user->our_sessions == NULL) {
                g_debug ("User %s is not logged in here, so has no primary session",
                         act_user_get_user_name (user));
                return NULL;
        }

        /* FIXME: better way to choose? */
        return user->our_sessions->data;
}

static gboolean
strv_equal (const char **a, const char **b)
{
        gint i;
        gboolean same = TRUE;

        if (a == b)
                return TRUE;
        else if (a == NULL || b == NULL)
                return FALSE;

        for (i = 0; a[i]; i++) {
                if (g_strcmp0 (a[i], b[i]) != 0)
                        return FALSE;
        }

        if (g_strcmp0 (a[i], b[i]) != 0)
                return FALSE;

        return TRUE;
}

static void
collect_props (const gchar *key,
               GVariant    *value,
               ActUser     *user)
{
        gboolean handled = TRUE;

        if (strcmp (key, "Uid") == 0) {
                guint64 new_uid;

                new_uid = g_variant_get_uint64 (value);
                if (!user->uid_set || (guint64) user->uid != new_uid) {
                        user->uid = (uid_t) new_uid;
                        user->uid_set = TRUE;
                        g_object_notify (G_OBJECT (user), "uid");
                }
        } else if (strcmp (key, "UserName") == 0) {
                const char *new_user_name;

                new_user_name = g_variant_get_string (value, NULL);
                if (g_strcmp0 (user->user_name, new_user_name) != 0) {
                        g_free (user->user_name);
                        user->user_name = g_strdup (new_user_name);
                        g_object_notify (G_OBJECT (user), "user-name");
                }
        } else if (strcmp (key, "RealName") == 0) {
                const char *new_real_name;

                new_real_name = g_variant_get_string (value, NULL);
                if (g_strcmp0 (user->real_name, new_real_name) != 0) {
                        g_free (user->real_name);
                        user->real_name = g_strdup (new_real_name);
                        g_object_notify (G_OBJECT (user), "real-name");
                }
        } else if (strcmp (key, "AccountType") == 0) {
                int new_account_type;

                new_account_type = g_variant_get_int32 (value);
                if ((int) user->account_type != new_account_type) {
                        user->account_type = (ActUserAccountType) new_account_type;
                        g_object_notify (G_OBJECT (user), "account-type");
                }
        } else if (strcmp (key, "PasswordMode") == 0) {
                int new_password_mode;

                new_password_mode = g_variant_get_int32 (value);
                if ((int) user->password_mode != new_password_mode) {
                        user->password_mode = (ActUserPasswordMode) new_password_mode;
                        g_object_notify (G_OBJECT (user), "password-mode");
                }
        } else if (strcmp (key, "PasswordHint") == 0) {
                const char *new_password_hint;

                new_password_hint = g_variant_get_string (value, NULL);
                if (g_strcmp0 (user->password_hint, new_password_hint) != 0) {
                        g_free (user->password_hint);
                        user->password_hint = g_strdup (new_password_hint);
                        g_object_notify (G_OBJECT (user), "password-hint");
                }
        } else if (strcmp (key, "HomeDirectory") == 0) {
                const char *new_home_dir;

                new_home_dir = g_variant_get_string (value, NULL);
                if (g_strcmp0 (user->home_dir, new_home_dir) != 0) {
                        g_free (user->home_dir);
                        user->home_dir = g_strdup (new_home_dir);
                        g_object_notify (G_OBJECT (user), "home-directory");
                }
        } else if (strcmp (key, "Shell") == 0) {
                const char *new_shell;

                new_shell = g_variant_get_string (value, NULL);
                if (g_strcmp0 (user->shell, new_shell) != 0) {
                        g_free (user->shell);
                        user->shell = g_strdup (new_shell);
                        g_object_notify (G_OBJECT (user), "shell");
                }
        } else if (strcmp (key, "Email") == 0) {
                const char *new_email;

                new_email = g_variant_get_string (value, NULL);
                if (g_strcmp0 (user->email, new_email) != 0) {
                        g_free (user->email);
                        user->email = g_strdup (new_email);
                        g_object_notify (G_OBJECT (user), "email");
                }
        } else if (strcmp (key, "Location") == 0) {
                const char *new_location;

                new_location = g_variant_get_string (value, NULL);
                if (g_strcmp0 (user->location, new_location) != 0) {
                        g_free (user->location);
                        user->location = g_strdup (new_location);
                        g_object_notify (G_OBJECT (user), "location");
                }
        } else if (strcmp (key, "Locked") == 0) {
                gboolean new_locked_state;

                new_locked_state = g_variant_get_boolean (value);
                if (new_locked_state != user->locked) {
                        user->locked = new_locked_state;
                        g_object_notify (G_OBJECT (user), "locked");
                }
        } else if (strcmp (key, "AutomaticLogin") == 0) {
                gboolean new_automatic_login_state;

                new_automatic_login_state = g_variant_get_boolean (value);
                if (new_automatic_login_state != user->automatic_login) {
                        user->automatic_login = new_automatic_login_state;
                        g_object_notify (G_OBJECT (user), "automatic-login");
                }
        } else if (strcmp (key, "SystemAccount") == 0) {
                gboolean new_system_account_state;

                new_system_account_state = g_variant_get_boolean (value);
                if (new_system_account_state != user->system_account) {
                        user->system_account = new_system_account_state;
                        g_object_notify (G_OBJECT (user), "system-account");
                }
        } else if (strcmp (key, "LocalAccount") == 0) {
                gboolean new_local;

                new_local = g_variant_get_boolean (value);
                if (user->local_account != new_local) {
                        user->local_account = new_local;
                        g_object_notify (G_OBJECT (user), "local-account");
                }
        } else if (strcmp (key, "LoginFrequency") == 0) {
                int new_login_frequency;

                new_login_frequency = (int) g_variant_get_uint64 (value);
                if ((int) user->login_frequency != (int) new_login_frequency) {
                        user->login_frequency = new_login_frequency;
                        g_object_notify (G_OBJECT (user), "login-frequency");
                }
        } else if (strcmp (key, "LoginTime") == 0) {
                gint64 new_login_time = g_variant_get_int64 (value);

                if (user->login_time != new_login_time) {
                        user->login_time = new_login_time;
                        g_object_notify (G_OBJECT (user), "login-time");
                }
        } else if (strcmp (key, "LoginHistory") == 0) {
                GVariant *new_login_history = value;

                if (user->login_history == NULL ||
                    !g_variant_equal (user->login_history, new_login_history)) {
                        if (user->login_history)
                          g_variant_unref (user->login_history);
                        user->login_history = g_variant_ref (new_login_history);
                        g_object_notify (G_OBJECT (user), "login-history");
                }
        } else if (strcmp (key, "XHasMessages") == 0) {
                gboolean new_has_messages;

                new_has_messages = g_variant_get_boolean (value);
                if (new_has_messages != user->x_has_messages) {
                        user->x_has_messages = new_has_messages;
                        g_object_notify (G_OBJECT (user), "xhas-messages");
                }
        } else if (strcmp (key, "XKeyboardLayouts") == 0) {
                const char **new_x_keyboard_layouts;

                new_x_keyboard_layouts = g_variant_get_strv (value, NULL);
                if (!strv_equal (user->x_keyboard_layouts, new_x_keyboard_layouts)) {
                        g_strfreev (user->x_keyboard_layouts);
                        user->x_keyboard_layouts = g_strdupv (new_x_keyboard_layouts);
                        g_object_notify (G_OBJECT (user), "xkeyboard-layouts");
                }
        } else if (strcmp (key, "BackgroundFile") == 0) {
                const char *new_background_file;

                new_background_file = g_variant_get_string (value, NULL);
                if (g_strcmp0 (user->background_file, new_background_file) != 0) {
                        g_free (user->background_file);
                        user->background_file = g_strdup (new_background_file);
                        g_object_notify (G_OBJECT (user), "background-file");
                }
        } else if (strcmp (key, "IconFile") == 0) {
                const char *new_icon_file;

                new_icon_file = g_variant_get_string (value, NULL);
                if (g_strcmp0 (user->icon_file, new_icon_file) != 0) {
                        g_free (user->icon_file);
                        user->icon_file = g_strdup (new_icon_file);
                        g_object_notify (G_OBJECT (user), "icon-file");
                }
        } else if (strcmp (key, "Language") == 0) {
                const char *new_language;

                new_language = g_variant_get_string (value, NULL);
                if (g_strcmp0 (user->language, new_language) != 0) {
                        g_free (user->language);
                        user->language = g_strdup (new_language);
                        g_object_notify (G_OBJECT (user), "language");
                }

        } else if (strcmp (key, "FormatsLocale") == 0) {
                const char *new_formats_locale;

                new_formats_locale = g_variant_get_string (value, NULL);
                if (g_strcmp0 (user->formats_locale, new_formats_locale) != 0) {
                        g_free (user->formats_locale);
                        user->formats_locale = g_strdup (new_formats_locale);
                        g_object_notify (G_OBJECT (user), "formats_locale");
                }

        } else if (strcmp (key, "InputSources") == 0) {
                GVariant *sources;

                g_variant_get (value, "@aa{ss}", &sources);

                if (!user->input_sources || !g_variant_equal (sources, user->input_sources)) {
                        if (user->input_sources)
                                g_variant_unref (user->input_sources);
                        user->input_sources = g_variant_ref (sources);
                        g_object_notify (G_OBJECT (user), "input-sources");
                }

                g_variant_unref (sources);
        } else if (strcmp (key, "XSession") == 0) {
                const char *new_x_session;

                new_x_session = g_variant_get_string (value, NULL);
                if (g_strcmp0 (user->x_session, new_x_session) != 0) {
                        g_free (user->x_session);
                        user->x_session = g_strdup (new_x_session);
                        g_object_notify (G_OBJECT (user), "x-session");
                }
        } else {
                handled = FALSE;
        }

        if (!handled) {
                g_debug ("unhandled property %s", key);
        }
}

static void
on_get_all_finished (GObject        *object,
                     GAsyncResult   *result,
                     gpointer data)
{
        GDBusProxy  *proxy = G_DBUS_PROXY (object);
        ActUser     *user = data;
        GError      *error;
        GVariant    *res;
        GVariantIter *iter;
        gchar       *key;
        GVariant    *value;

        g_assert (G_IS_DBUS_PROXY (user->object_proxy));
        g_assert (user->object_proxy == proxy);

        error = NULL;
        res = g_dbus_proxy_call_finish (proxy, result, &error);

        if (! res) {
                g_debug ("Error calling GetAll() when retrieving properties for %s: %s",
                         user->object_path, error->message);
                g_error_free (error);

                if (!user->is_loaded) {
                        set_is_loaded (user, TRUE);
                }
                return;
        }

        g_object_unref (user->get_all_call);
        user->get_all_call = NULL;

        g_variant_get (res, "(a{sv})", &iter);
        while (g_variant_iter_next (iter, "{sv}", &key, &value)) {
                collect_props (key, value, user);
                g_free (key);
                g_variant_unref (value);
        }
        g_variant_iter_free (iter);
        g_variant_unref (res);

        if (!user->is_loaded) {
                set_is_loaded (user, TRUE);
        }

        g_signal_emit (user, signals[CHANGED], 0);
}

static void
update_info (ActUser *user)
{
        g_assert (G_IS_DBUS_PROXY (user->object_proxy));

        if (user->get_all_call != NULL) {
                g_cancellable_cancel (user->get_all_call);
                g_object_unref (user->get_all_call);
        }

        user->get_all_call = g_cancellable_new ();
        g_dbus_proxy_call (user->object_proxy,
                           "GetAll",
                           g_variant_new ("(s)", ACCOUNTS_USER_INTERFACE),
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           user->get_all_call,
                           on_get_all_finished,
                           user);
}

static void
changed_handler (AccountsUser *object,
                 gpointer   *data)
{
        ActUser *user = ACT_USER (data);

        update_info (user);
}

/**
 * _act_user_update_as_nonexistent:
 * @user: the user object to update.
 *
 * Set's the 'non-existent' property of @user to #TRUE
 * Can only be called before the user is loaded.
 **/
void
_act_user_update_as_nonexistent (ActUser *user)
{
        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (!act_user_is_loaded (user));
        g_return_if_fail (user->object_path == NULL);

        user->nonexistent = TRUE;
        g_object_notify (G_OBJECT (user), "nonexistent");

        set_is_loaded (user, TRUE);
}

/**
 * _act_user_update_from_object_path:
 * @user: the user object to update.
 * @object_path: the object path of the user to use.
 *
 * Updates the properties of @user from the accounts service via
 * the object path in @object_path.
 **/
void
_act_user_update_from_object_path (ActUser    *user,
                                   const char *object_path)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (object_path != NULL);
        g_return_if_fail (user->object_path == NULL);

        user->object_path = g_strdup (object_path);

        user->accounts_proxy = accounts_user_proxy_new_sync (user->connection,
                                                             G_DBUS_PROXY_FLAGS_NONE,
                                                             ACCOUNTS_NAME,
                                                             user->object_path,
                                                             NULL,
                                                             &error);
        if (!user->accounts_proxy) {
                g_warning ("Couldn't create accounts proxy: %s", error->message);
                g_error_free (error);
                return;
        }
        g_dbus_proxy_set_default_timeout (G_DBUS_PROXY (user->accounts_proxy), INT_MAX);

        g_signal_connect (user->accounts_proxy, "changed", G_CALLBACK (changed_handler), user);

        user->object_proxy = g_dbus_proxy_new_sync (user->connection,
                                                    G_DBUS_PROXY_FLAGS_NONE,
                                                    0,
                                                    ACCOUNTS_NAME,
                                                    user->object_path,
                                                    "org.freedesktop.DBus.Properties",
                                                    NULL,
                                                    &error);
        if (!user->object_proxy) {
                g_warning ("Couldn't create accounts property proxy: %s", error->message);
                g_error_free (error);
                return;
        }

       update_info (user);
}

void
_act_user_update_login_frequency (ActUser    *user,
                                  int         login_frequency)
{
        if (user->login_frequency != login_frequency) {
                user->login_frequency = login_frequency;
                g_object_notify (G_OBJECT (user), "login-frequency");
        }
}

static void
copy_sessions_lists (ActUser *user,
                     ActUser *user_to_copy)
{
        GList *node;

        for (node = g_list_last (user_to_copy->our_sessions);
             node != NULL;
             node = node->prev) {
                user->our_sessions = g_list_prepend (user->our_sessions, g_strdup (node->data));
        }

        for (node = g_list_last (user_to_copy->other_sessions);
             node != NULL;
             node = node->prev) {
                user->other_sessions = g_list_prepend (user->other_sessions, g_strdup (node->data));
        }
}

void
_act_user_load_from_user (ActUser    *user,
                          ActUser    *user_to_copy)
{
        if (!user_to_copy->is_loaded) {
                return;
        }

        /* loading users may already have a uid, user name, or session list
         * from creation, so only update them if necessary
         */
        if (!user->uid_set) {
                user->uid = user_to_copy->uid;
                g_object_notify (G_OBJECT (user), "uid");
        }

        if (user->user_name == NULL) {
                user->user_name = g_strdup (user_to_copy->user_name);
                g_object_notify (G_OBJECT (user), "user-name");
        }

        if (user->our_sessions == NULL && user->other_sessions == NULL) {
                copy_sessions_lists (user, user_to_copy);
                g_signal_emit (user, signals[SESSIONS_CHANGED], 0);
        }

        g_free (user->real_name);
        user->real_name = g_strdup (user_to_copy->real_name);
        g_object_notify (G_OBJECT (user), "real-name");

        g_free (user->password_hint);
        user->password_hint = g_strdup (user_to_copy->real_name);
        g_object_notify (G_OBJECT (user), "password-hint");

        g_free (user->home_dir);
        user->home_dir = g_strdup (user_to_copy->home_dir);
        g_object_notify (G_OBJECT (user), "home-directory");

        g_free (user->shell);
        user->shell = g_strdup (user_to_copy->shell);
        g_object_notify (G_OBJECT (user), "shell");

        g_free (user->email);
        user->email = g_strdup (user_to_copy->email);
        g_object_notify (G_OBJECT (user), "email");

        g_free (user->location);
        user->location = g_strdup (user_to_copy->location);
        g_object_notify (G_OBJECT (user), "location");

        g_free (user->icon_file);
        user->icon_file = g_strdup (user_to_copy->icon_file);
        g_object_notify (G_OBJECT (user), "icon-file");

        g_free (user->language);
        user->language = g_strdup (user_to_copy->language);
        g_object_notify (G_OBJECT (user), "language");

        if (user_to_copy->input_sources != user->input_sources) {
                if (user->input_sources)
                        g_variant_unref (user->input_sources);
                user->input_sources = user_to_copy->input_sources;
                if (user->input_sources)
                        g_variant_ref (user->input_sources);
                g_object_notify (G_OBJECT (user), "input-sources");
        }

        g_free (user->x_session);
        user->x_session = g_strdup (user_to_copy->x_session);
        g_object_notify (G_OBJECT (user), "x-session");

        user->login_frequency = user_to_copy->login_frequency;
        g_object_notify (G_OBJECT (user), "login-frequency");

        user->login_time = user_to_copy->login_time;
        g_object_notify (G_OBJECT (user), "login-time");

        user->login_history = user_to_copy->login_history ? g_variant_ref (user_to_copy->login_history) : NULL;
        g_object_notify (G_OBJECT (user), "login-history");

        user->account_type = user_to_copy->account_type;
        g_object_notify (G_OBJECT (user), "account-type");

        user->password_mode = user_to_copy->password_mode;
        g_object_notify (G_OBJECT (user), "password-mode");

        user->nonexistent = user_to_copy->nonexistent;
        g_object_notify (G_OBJECT (user), "nonexistent");

        set_is_loaded (user, TRUE);
}

/**
 * act_user_is_loaded:
 * @user: a #ActUser
 *
 * Determines whether or not the user object is loaded and ready to read from.
 * #ActUserManager:is-loaded property must be %TRUE before calling
 * act_user_manager_list_users()
 *
 * Returns: %TRUE or %FALSE
 */
gboolean
act_user_is_loaded (ActUser *user)
{
        return user->is_loaded;
}

/**
 * act_user_set_email:
 * @user: the user object to alter.
 * @email: an email address
 *
 * Assigns a new email to @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_email (ActUser    *user,
                    const char *email)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (email != NULL);
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_email_sync (user->accounts_proxy,
                                                email,
                                                NULL,
                                                &error)) {
                g_warning ("SetEmail call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}

/**
 * act_user_set_x_has_messages:
 * @user: the user object to alter.
 * @has_messages: whether the user has messages waiting
 *
 * Sets a new has-messages status for @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_x_has_messages (ActUser  *user,
                             gboolean  has_messages)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_xhas_messages_sync (user->accounts_proxy,  
                                                         has_messages,
                                                         NULL,
                                                         &error)) {
                g_warning ("SetXHasMessages call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}

/**
 * act_user_set_x_keyboard_layouts:
 * @user: the user object to alter.
 * @keyboard_layouts: names of keyboard layouts
 *
 * Assigns a new set of keyboard layouts for @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_x_keyboard_layouts (ActUser    *user,
                                 const char * const *keyboard_layouts)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (keyboard_layouts != NULL);
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_xkeyboard_layouts_sync (user->accounts_proxy,
                                                            keyboard_layouts,
                                                            NULL,
                                                            &error)) {
                g_warning ("SetXKeyboardLayouts call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}

/**
 * act_user_set_formats_locale:
 * @user: the user object to alter.
 * @formats_locale: a locale (e.g. en_US.utf8)
 *
 * Assigns a new formats locale for @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_formats_locale (ActUser    *user,
                             const char *formats_locale)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (formats_locale != NULL);
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_formats_locale_sync (user->accounts_proxy,
                                                         formats_locale,
                                                         NULL,
                                                         &error)) {
                g_warning ("SetFormatsLocale call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}

/**
 * act_user_set_language:
 * @user: the user object to alter.
 * @language: a locale (e.g. en_US.utf8)
 *
 * Assigns a new locale for @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_language (ActUser    *user,
                       const char *language)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (language != NULL);
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_language_sync (user->accounts_proxy,
                                                   language,
                                                   NULL,
                                                   &error)) {
                g_warning ("SetLanguage call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}

/**
 * act_user_set_input_sources:
 * @user: the user object to alter.
 * @sources: a list of input sources
 *
 * Assigns new input sources for @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_input_sources (ActUser  *user,
                            GVariant *sources)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));
        g_return_if_fail (g_variant_is_of_type (sources, G_VARIANT_TYPE ("aa{ss}")));

        if (!accounts_user_call_set_input_sources_sync (user->accounts_proxy,
                                                        sources,
                                                        NULL,
                                                        &error)) {
                g_warning ("SetInputSources call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}

/**
 * act_user_set_x_session:
 * @user: the user object to alter.
 * @x_session: an x session (e.g. gnome)
 *
 * Assigns a new x session for @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_x_session (ActUser    *user,
                        const char *x_session)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (x_session != NULL);
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_xsession_sync (user->accounts_proxy,
                                                   x_session,
                                                   NULL,
                                                   &error)) {
                g_warning ("SetXSession call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}


/**
 * act_user_set_location:
 * @user: the user object to alter.
 * @location: a location
 *
 * Assigns a new location for @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_location (ActUser    *user,
                       const char *location)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (location != NULL);
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_location_sync (user->accounts_proxy,
                                                   location,
                                                   NULL,
                                                   &error)) {
                g_warning ("SetLocation call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}

/**
 * act_user_set_background_file:
 * @user: the user object to alter.
 * @background_file: path to an background
 *
 * Assigns a new background for @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_background_file (ActUser    *user,
                              const char *background_file)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (background_file != NULL);
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_background_file_sync (user->accounts_proxy,
                                                          background_file,
                                                          NULL,
                                                          &error)) {
                g_warning ("SetBackgroundFile call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}

/**
 * act_user_set_user_name:
 * @user: the user object to alter.
 * @user_name: a new user name
 *
 * Assigns a new username for @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_user_name (ActUser    *user,
                        const char *user_name)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (user_name != NULL);
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_user_name_sync (user->accounts_proxy,
                                                    user_name,
                                                    NULL,
                                                    &error)) {
                g_warning ("SetUserName call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}

/**
 * act_user_set_real_name:
 * @user: the user object to alter.
 * @real_name: a new name
 *
 * Assigns a new name for @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_real_name (ActUser    *user,
                        const char *real_name)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (real_name != NULL);
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_real_name_sync (user->accounts_proxy,
                                                    real_name,
                                                    NULL,
                                                    &error)) {
                g_warning ("SetRealName call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}

/**
 * act_user_set_icon_file:
 * @user: the user object to alter.
 * @icon_file: path to an icon
 *
 * Assigns a new icon for @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_icon_file (ActUser    *user,
                        const char *icon_file)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (icon_file != NULL);
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_icon_file_sync (user->accounts_proxy,
                                                    icon_file,
                                                    NULL,
                                                    &error)) {
                g_warning ("SetIconFile call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}

/**
 * act_user_set_account_type:
 * @user: the user object to alter.
 * @account_type: a #ActUserAccountType
 *
 * Changes the account type of @user.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_account_type (ActUser            *user,
                           ActUserAccountType  account_type)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_account_type_sync (user->accounts_proxy,
                                                    account_type,
                                                    NULL,
                                                    &error)) {
                g_warning ("SetAccountType call failed: %s", error->message);
                g_error_free (error);
                return;
        }
}

/**
 * act_user_set_password:
 * @user: the user object to alter.
 * @password: a password
 * @hint: a hint to help user recall password
 *
 * Changes the password of @user to @password.
 * @hint is displayed to the user if they forget the password.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_password (ActUser             *user,
                       const gchar         *password,
                       const gchar         *hint)
{
        GHashTable *table;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (password != NULL);

        table = g_hash_table_new (g_direct_hash, g_direct_equal);
        g_hash_table_insert (table, GINT_TO_POINTER (ACT_USER_PASSWORD_REGULAR),
                             (gpointer) password);
        g_hash_table_insert (table, GINT_TO_POINTER (ACT_USER_PASSWORD_HINT),
                             (gpointer) hint);

        act_user_set_multiple_passwords (user, table);

        g_hash_table_unref (table);
}

/**
 * act_user_set_multiple_passwords:
 * @user: the user object to alter.
 * @password_map: (element-type guint utf8): a #GHashTable mapping
 *                #ActUserPasswordType to a plain-text password
 *
 * Changes password, password hint and PIN of @user, using the
 * values taken from @password_map.
 *
 * Note this function is synchronous and ignores errors.
 **/
/* Note on the API: what we really want here is a list of (int-string)
   values, but short of passing #GVariants (which is ugly), we can't
   do that in a introspectable way without a boxed struct
*/
void
act_user_set_multiple_passwords (ActUser             *user,
                                 GHashTable          *password_map)
{
        GError *error = NULL;
        GcrSecretExchange *exchange;
        GHashTableIter iter;
        GVariantBuilder builder;
        char *exchange_begin;
        gpointer key, value;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (password_map != NULL);
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_begin_set_password_sync (user->accounts_proxy,
                                                         &exchange_begin,
                                                         NULL, &error)) {
                g_warning ("BeginSetPassword call failed: %s", error->message);
                g_error_free (error);
                return;
        }

        exchange = gcr_secret_exchange_new (GCR_SECRET_EXCHANGE_PROTOCOL_1);
        gcr_secret_exchange_receive (exchange, exchange_begin);
        g_free (exchange_begin);

        g_variant_builder_init (&builder, G_VARIANT_TYPE ("a(us)"));
        g_hash_table_iter_init (&iter, password_map);
        while (g_hash_table_iter_next (&iter, &key, &value)) {
                char *next_str;

                next_str = gcr_secret_exchange_send (exchange, value, -1);
                g_variant_builder_add (&builder, "(us)",
                                       GPOINTER_TO_INT (key),
                                       next_str);
                g_free (next_str);
        }

        if (!accounts_user_call_continue_set_password_sync (user->accounts_proxy,
                                                            g_variant_builder_end (&builder),
                                                            NULL,
                                                            &error)) {
                g_warning ("ContinueSetPassword call failed: %s", error->message);
                g_error_free (error);
        }

        g_object_unref (exchange);
}

/**
 * act_user_set_password_mode:
 * @user: the user object to alter.
 * @password_mode: a #ActUserPasswordMode
 *
 * Changes the password of @user.  If @password_mode is
 * ACT_USER_PASSWORD_MODE_SET_AT_LOGIN then the user will
 * be asked for a new password at the next login.  If @password_mode
 * is ACT_USER_PASSWORD_MODE_NONE then the user will not require
 * a password to log in.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_password_mode (ActUser             *user,
                            ActUserPasswordMode  password_mode)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_password_mode_sync (user->accounts_proxy,
                                                        (gint) password_mode,
                                                        NULL,
                                                        &error)) {
                g_warning ("SetPasswordMode call failed: %s", error->message);
                g_error_free (error);
        }
}

/**
 * act_user_set_locked:
 * @user: the user object to alter.
 * @locked: whether or not the account is locked
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_locked (ActUser  *user,
                     gboolean  locked)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_locked_sync (user->accounts_proxy,
                                                 locked,
                                                 NULL,
                                                 &error)) {
                g_warning ("SetLocked call failed: %s", error->message);
                g_error_free (error);
        }
}

/**
 * act_user_set_automatic_login:
 * @user: the user object to alter
 * @enabled: whether or not to autologin for user.
 *
 * If enabled is set to %TRUE then this user will automatically be logged in
 * at boot up time.  Only one user can be configured to auto login at any given
 * time, so subsequent calls to act_user_set_automatic_login() override previous
 * calls.
 *
 * Note this function is synchronous and ignores errors.
 **/
void
act_user_set_automatic_login (ActUser   *user,
                              gboolean  enabled)
{
        GError *error = NULL;

        g_return_if_fail (ACT_IS_USER (user));
        g_return_if_fail (ACCOUNTS_IS_USER (user->accounts_proxy));

        if (!accounts_user_call_set_automatic_login_sync (user->accounts_proxy,
                                                          enabled,
                                                          NULL,
                                                          &error)) {
                g_warning ("SetAutomaticLogin call failed: %s", error->message);
                g_error_free (error);
        }
}
