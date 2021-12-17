/*
 * Copyright (C) 2016-2021 Ole André Vadla Ravnås <oleavr@nowsecure.com>
 *
 * Licence: wxWindows Library Licence, Version 3.1
 */

/**
 * ApiResolver:
 *
 * Resolves query strings into memory locations where matching functions reside.
 *
 * ## Using `ApiResolver`
 *
 * ```c
 * void
 * start (void)
 * {
 *   g_autoptr(ApiResolver) resolver = gum_api_resolver_make ("module");
 *
 *   gum_api_resolver_enumerate_matches (resolver, "exports:libc.so!open*",
 *                                       instrument_libc_open, NULL, NULL);
 * }
 *
 * static gboolean
 * instrument_libc_open (const GumApiDetails *details,
 *                       gpointer user_data)
 * {
 *   g_print ("Found %s at %" G_GINT64_MODIFIER "x\n",
 *            details->name,
 *            details->address);
 * }
 * ```
 */

#include "gumapiresolver.h"

#include "gummoduleapiresolver.h"
#ifdef HAVE_DARWIN
# include "backend-darwin/gumobjcapiresolver.h"
#endif

#include <string.h>

G_DEFINE_INTERFACE (GumApiResolver, gum_api_resolver, G_TYPE_OBJECT)

static void
gum_api_resolver_default_init (GumApiResolverInterface * iface)
{
}

/**
 * gum_api_resolver_make:
 * @type: the resolver type to make
 *
 * Makes an API resolver of the given @type, allowing you to quickly find
 * functions by name, with globs permitted. Precisely which resolvers are
 * available depends on the current platform and runtimes loaded in the current
 * process. Available resolvers are currently:
 *
 *  - `module`: Resolves exported and imported functions of shared libraries
 *    currently loaded. Always available.
 *  - `objc`: Resolves Objective-C methods of classes currently loaded. Available
 *    on macOS and iOS in processes that have the Objective-C runtime loaded.
 *
 * The resolver will load the minimum amount of data required on creation, and
 * lazy-load the rest depending on the queries it receives. You should use the
 * same instance for a batch of queries, but recreate it for future batches to
 * avoid looking at stale data.
 *
 * Returns: (nullable) (transfer full): the newly created resolver instance
 */
GumApiResolver *
gum_api_resolver_make (const gchar * type)
{
  if (strcmp (type, "module") == 0)
    return gum_module_api_resolver_new ();

#ifdef HAVE_DARWIN
  if (strcmp (type, "objc") == 0)
    return gum_objc_api_resolver_new ();
#endif

  return NULL;
}

/**
 * gum_api_resolver_enumerate_matches:
 * @self: a resolver
 * @query: the query to perform
 * @func: (scope call): the function called with each match
 * @user_data: (nullable): the data to pass to @func
 * @error: (nullable): return location for a #GError
 *
 * Performs the resolver-specific @query, optionally suffixed with `/i` to
 * perform case-insensitive matching. Calls @func with each match found.
 */
void
gum_api_resolver_enumerate_matches (GumApiResolver * self,
                                    const gchar * query,
                                    GumFoundApiFunc func,
                                    gpointer user_data,
                                    GError ** error)
{
  GUM_API_RESOLVER_GET_IFACE (self)->enumerate_matches (self, query, func,
      user_data, error);
}
